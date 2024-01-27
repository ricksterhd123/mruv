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

// mrb_value add_event_handler(mrb_state *mrb, mrb_value self)
// {
//     const char *event_name;
//     mrb_value blk;

//     mrb_get_args(mrb, "&", &blk);
//     if (mrb_nil_p(blk))
//     {
//         mrb_raise(mrb, E_ARGUMENT_ERROR, "tried to add event handler without a block");
//     }

//     struct RProc *p = mrb_proc_ptr(blk);
//     if (!MRB_PROC_STRICT_P(p))
//     {
//         struct RProc *p2 = MRB_OBJ_ALLOC(mrb, MRB_TT_PROC, p->c);
//         mrb_proc_copy(mrb, p2, p);
//         p2->flags |= MRB_PROC_STRICT;
//         return mrb_obj_value(p2);
//     }

//     MRB_METHOD_FROM_PROC(mbr, p);
//     return mrb_vm_run(mrb, p, self, 0);
// }

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

    if (mrb->exc) {
        error = 1;
        goto CLEANUP;
    }

    // Attempt to get handler
    mrb_method_t handler = mrb_method_search_vm(mrb, &mrb->object_class, mrb_intern_lit(mrb, "handler"));

    if (!handler) {
        fprintf(stderr, "`handler` method does not exist\n");
        error = 1;
        goto CLEANUP;
    } else {
        printf("Found method `handler`\n");
    }

    // struct RProc* handler_proc = MRB_METHOD_PROC(handler);


    // mrb_value handler_result = mrb_vm_run(mrb, handler_proc, );
    // mrb_value handler_result_string = mrb_obj_as_string(mrb, handler_result);

    // printf("result: %s\n", mrb_string_value_cstr (mrb, &handler_result_string));

    // loop = uv_default_loop();
    // uv_tcp_t server;

    // uv_tcp_init(loop, &server);
    // uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);
    // uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);

    // int r = uv_listen((uv_stream_t *)&server, DEFAULT_BACKLOG, on_new_connection);
    // if (r)
    // {
    //     fprintf(stderr, "Listen error %s\n", uv_strerror(r));
    //     return 1;
    // }

    // int result = uv_run(loop, UV_RUN_DEFAULT);

CLEANUP:
    mrb_close(mrb);
    fclose(fp);

    return error;
}
