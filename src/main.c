#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/proc.h>
#include <mruby/variable.h>
#include <mruby/hash.h>
#include <mruby/class.h>
#include <mruby/string.h>

#include <picohttpparser.h>

#define DEFAULT_PORT 7000
#define DEFAULT_BACKLOG 128

uv_loop_t *loop;
struct sockaddr_in addr;
mrb_state *mrb;

// environment
mrb_value mruv;

// user script
mrb_value config_script;
mrb_value handler;

typedef struct
{
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req)
{
    write_req_t *wr = (write_req_t *)req;
    free(wr->buf.base);
    free(wr);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

void on_write_complete(uv_write_t *req, int status)
{
    if (status)
    {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free_write_req(req);
}

void on_read_chunk(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    struct sockaddr_storage name;
    int name_length;

    int r = uv_tcp_getsockname((uv_tcp_t *)client, (struct sockaddr *)&name, &name_length);

    if (r != 0)
    {
        fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t *)client, NULL);
        free(buf->base);
        return;
    }

    if (name.ss_family != AF_INET)
    {
        fprintf(stderr, "Expected IPV4 socket");
        uv_close((uv_handle_t *)client, NULL);
    }

    struct sockaddr_in *sin = (struct sockaddr_in *)&name;

    struct in_addr ip_address;
    const char *ip = inet_ntoa(sin->sin_addr);

    if (nread < 0)
    {
        if (nread != UV_EOF)
        {
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
            uv_close((uv_handle_t *)client, NULL);
        }
        free(buf->base);
    }
    else if (nread > 0)
    {
        write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));

        mrb_value context = mrb_hash_new(mrb);
        mrb_hash_set(mrb, context, mrb_symbol_value(mrb_intern_cstr(mrb, "ip_addresss")), mrb_str_new_cstr(mrb, ip));

        mrb_value event = mrb_hash_new(mrb);
        mrb_hash_set(mrb, event, mrb_symbol_value(mrb_intern_cstr(mrb, "body")), mrb_str_new_cstr(mrb, buf->base));

        mrb_value ret = mrb_funcall(mrb, config_script, "handler", (mrb_int)2, event, context);

        mrb_value ret_as_str = mrb_obj_as_string(mrb, ret);
        const char *ret_as_cstr_tmp = mrb_string_value_cstr(mrb, &ret_as_str);

        // NOTE: ret_as_cstr is passed into uv_buf_init freed by libuv on complete
        ssize_t ret_as_cstr_sz = sizeof(char) * (strlen(ret_as_cstr_tmp) + 2);
        char *ret_as_cstr = (char *)malloc(ret_as_cstr_sz);
        snprintf(ret_as_cstr, ret_as_cstr_sz, "%s\n", ret_as_cstr_tmp);

        req->buf = uv_buf_init(ret_as_cstr, ret_as_cstr_sz);
        uv_write((uv_write_t *)req, client, &req->buf, 1, on_write_complete);
        return;
    }
}

void on_new_connection(uv_stream_t *server, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);

    if (uv_accept(server, (uv_stream_t *)client) == 0)
    {
        uv_read_start((uv_stream_t *)client, alloc_buffer, on_read_chunk);
    }
    else
    {
        uv_close((uv_handle_t *)client, NULL);
    }
}

int main()
{
    int error = 0;

    // Load config.rb file
    FILE *fp = fopen("config.rb", "r");
    if (!fp)
    {
        fprintf(stderr, "Failed to open `config.rb`\n");
        return 1;
    }

    // Setup mrb_state
    mrb = mrb_open();
    if (!mrb)
    {
        fprintf(stderr, "Failed to open mrb\n");
        return 1;
    }

    // Create internal state
    mruv = mrb_hash_new(mrb);
    mrb_hash_set(mrb, mruv, mrb_symbol_value(mrb_intern_cstr(mrb, "VERSION")), mrb_str_new_lit(mrb, "v1.0.0"));
    mrb_gv_set(mrb, mrb_intern_lit(mrb, "$mruv"), mruv);

    // Load file
    config_script = mrb_load_file(mrb, fp);

    if (mrb->exc)
    {
        error = 1;
        goto CLEANUP;
    }

    // Check handler exists
    mrb_method_t handler = mrb_method_search_vm(mrb, &mrb->object_class, mrb_intern_lit(mrb, "handler"));
    if (!handler)
    {
        fprintf(stderr, "`handler` method does not exist\n");
        error = 1;
        goto CLEANUP;
    }

    // Run event loop
    loop = uv_default_loop();
    uv_tcp_t server;

    uv_tcp_init(loop, &server);
    uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);
    uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);

    int r = uv_listen((uv_stream_t *)&server, DEFAULT_BACKLOG, on_new_connection);
    if (r)
    {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return 1;
    }

    error = uv_run(loop, UV_RUN_DEFAULT);

CLEANUP:
    mrb_close(mrb);
    fclose(fp);

    return error;
}
