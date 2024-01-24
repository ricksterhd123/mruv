#include "mruv_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include <mruby.h>
#include <mruby/compile.h>

int main(void)
{
    mrb_state *mrb = mrb_open();
    if (!mrb)
    { /* handle error */
    }
    // mrb_load_string(mrb, str) to load from NULL terminated strings
    // mrb_load_nstring(mrb, str, len) for strings without null terminator or with known length
    mrb_load_string(mrb, "puts 'hello world'");
    mrb_close(mrb);

    uv_loop_t *loop = malloc(sizeof(uv_loop_t));
    uv_loop_init(loop);

    printf("Now quitting.\n");
    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);
    free(loop);
    return 0;
}
