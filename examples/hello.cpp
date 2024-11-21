#include <iostream>

extern "C" {

#include "../fibers_lib/fibers.h"

}

using namespace std;

int fiber1(fl_fiber *f, void *data) {
    // printf("Start fiber 1\n");
    fl_fiberid fid = fl_fiber_id(f);

    try {
        for (int i = 0; i <= 10; i++) {
            for (int j = 0; j < 1024 * 1000; j++);
            printf("Hello world #%2d from fiber %2zu and thread %3zu\n", i, fid, pthread_self() % 1000 + 1);
            fl_fiber_yeild(f);
            throw "";
        }
    } catch (...) {
        printf("Throw in catch section\n");
        throw "";
    }


    return 0;
}

int main() {
    fl_executor *ctx;
    int err = fl_executor_create(&ctx);
    if (err) {
        fprintf(stderr, "Ошибка создания контекста\n");
        return -1;
    }

    for (int i = 0; i < 1; i++) {
        fl_fiber_create(ctx, fiber1, NULL);
    }

    fl_executor_start(ctx);
//    fl_fiber_create(ctx, fiber1, NULL);
//    fl_fiber_create(ctx, fiber1, NULL);


    fl_executor_join(ctx);

    printf("Все задачи контекста завершены\n");

    return 0;
}