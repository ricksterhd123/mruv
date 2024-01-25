#include "mruv_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <mruby.h>
#include <mruby/compile.h>

#define DEFAULT_PORT 7000
#define DEFAULT_BACKLOG 128

uv_loop_t *loop;
struct sockaddr_in addr;
mrb_state *mrb;
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

void echo_write(uv_write_t *req, int status)
{
    if (status)
    {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free_write_req(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread > 0)
    {
        write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base, nread);

        uv_write((uv_write_t *)req, client, &req->buf, 1, echo_write);
        return;
    }

    if (nread < 0)
    {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t *)client, NULL);
    }

    free(buf->base);
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
        uv_read_start((uv_stream_t *)client, alloc_buffer, echo_read);
    }
    else
    {
        uv_close((uv_handle_t *)client, NULL);
    }
}

mrb_value add_event_handler(mrb_state *mrb, mrb_value self)
{
    const char* event_name;
    mrb_value event_handler_block;

    mrb_int n_args = mrb_get_args(mrb, "&!", &event_handler_block);

    printf("%ld\n", n_args);

    return mrb_str_new_lit(mrb, "hello");
}

int main()
{
    FILE *fp = fopen("config.rb", "r");

    if (!fp)
    {
        fprintf(stderr, "Failed to open `config.rb`\n");
        return 1;
    }

    mrb = mrb_open();

    if (!mrb)
    {
        fprintf(stderr, "Failed to open mrb\n");
        return 1;
    }

    mrb_define_method(mrb, mrb->kernel_module, "add_event_handler", add_event_handler, MRB_ARGS_REQ(2));

    config_script = mrb_load_file(mrb, fp);
    loop = uv_default_loop();

    // mrb_int i = 99;
    // mrb_funcall(mrb, config_script, "method_name", 1, mrb_fixnum_value(i));

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

    int result = uv_run(loop, UV_RUN_DEFAULT);
    mrb_close(mrb);
    fclose(fp);

    return result;
}
