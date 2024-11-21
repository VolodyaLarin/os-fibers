#include "../fibers_lib/fibers.h"
#include "../fibers_lib/channels.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


typedef struct {
  fl_channel *input;
  fl_channel *output;
} chanels_ctx;

int pingpong(fl_fiber *f, void *d) {

  fl_fiberid fid = fl_fiber_id(f);
  chanels_ctx *data = d;

  char buffer[255] = "";

  while (fl_channel_pop(f, data->input, buffer, 255) > 0) {
    printf("%zu: get %s\n", fid, buffer);
    if (!strncmp("ping", buffer, 255)) {
      fl_channel_write(data->output, "pong", 5);
    } else if (!strncmp("pong", buffer, 255)) {
      fl_channel_write(data->output, "pong", 5);
    } else {
      fprintf(stderr, "%zu: undefined command\n", fid);
    }
  }

  printf("%zu: end \n", fid);

  return 0;
}

int main() {
  fl_executor *ctx;
  int err = fl_executor_create(&ctx);
  if (err) {
    fprintf(stderr, "Ошибка создания контекста\n");
    return -1;
  }

  fl_channel *channel1, *channel2;
  fl_channel_create(&channel1);
  fl_channel_create(&channel2);

  chanels_ctx chanels_1 = {};
  chanels_ctx chanels_2 = {};

  fl_fiber_create(ctx, pingpong, &chanels_1);
  fl_fiber_create(ctx, pingpong, &chanels_2);

  fl_channel_write(channel1, "ping", 5);

  fl_executor_start(ctx);

  sleep(5);

  fl_channel_close(channel1);
  fl_channel_close(channel2);

  printf("Все каналы закрыты\n");

  fl_executor_join(ctx);

  printf("Все задачи контекста завершены\n");

  return 0;
}