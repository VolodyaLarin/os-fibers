#include "./fibers.h"

#include <memory.h>
#include <pthread.h>

#include <ucontext.h>
#include <sys/sysinfo.h>

#define FL_STACK_SIZE (1024 * 1024 * 10)
#define FL_THREAD_MAX 32


typedef enum {
    fiber_created,
    fiber_paused,
    fiber_running,
    fiber_stopped
} fiber_status;


struct fl_fiber_struct {
    fl_fiber *next;

    fl_executor *exec;

    fl_fiberid id;
    fiber_status status;
    void *data;

    int (*init)(fl_fiber *, void *);

    void *stack;

    size_t thread_id;

    ucontext_t context;
};

struct fl_executor_thread_data {
    size_t id;
    fl_executor *executor;

    ucontext_t context;
};

struct fl_executor_struct {
    int thread_count;
    pthread_t thread_ids[FL_THREAD_MAX];
    struct fl_executor_thread_data thread_data[FL_THREAD_MAX];

    _Atomic size_t fiber_last_id;

    pthread_mutex_t fibers_queue_lock;
    fl_fiber *fiber_first;
    fl_fiber *fiber_last;

    _Atomic int fiber_count;
};

void fiber_wrapper(fl_fiber *fiber) {
    if (!fiber) {
        DEBUG_LOG0("fiber is nullptr");
        return;
    }

    DEBUG_LOG1("FIBER WRAPPER STARTED id: %zu\n", fiber ? fiber->id : -1);

    fiber->init(fiber, fiber->data);
    fiber->status = fiber_stopped;

    DEBUG_LOG1("FIBER WRAPPER STOPPED id: %zu\n", fiber ? fiber->id : -1);
    fl_fiber_yeild(fiber);

    return;
}

int fl_fiber_destroy(fl_fiber *fiber) {
    fiber->exec->fiber_count--;
    free(fiber->stack);
    free(fiber);

    return 0;
}

fl_fiber *_pop_queue(fl_executor *e) {
    DEBUG_LOG0("POP FIBER QUEUE\n");

    pthread_mutex_lock(&e->fibers_queue_lock);

    fl_fiber *current_fiber = e->fiber_first;
    if (current_fiber) {
        e->fiber_first = current_fiber->next;
    }

    pthread_mutex_unlock(&e->fibers_queue_lock);

    return current_fiber;
}

void _push_queue(fl_executor *e, fl_fiber *fiber) {
    DEBUG_LOG1("PUSH FIBER %zu\n", fiber->id);

    fiber->next = NULL;

    pthread_mutex_lock(&e->fibers_queue_lock);

    if (!e->fiber_first) {
        e->fiber_first = fiber;
        e->fiber_last = fiber;
    } else {
        e->fiber_last->next = fiber;
        e->fiber_last = fiber;
    }

    pthread_mutex_unlock(&e->fibers_queue_lock);
}


void *fl_execute(struct fl_executor_thread_data *data) {
    DEBUG_LOG0("EXEC CREATED\n");

    fl_executor *e = data->executor;


    while (e->fiber_count) {
        fl_fiber *current_fiber = _pop_queue(e);
        if (!current_fiber) {
            continue;
        }

        if (current_fiber->status == fiber_created) {
            current_fiber->status = fiber_paused;

            getcontext(&current_fiber->context);
            current_fiber->context.uc_link = &data->context;
            current_fiber->context.uc_stack.ss_sp = current_fiber->stack;
            current_fiber->context.uc_stack.ss_size = FL_STACK_SIZE;

            makecontext(&current_fiber->context, (void (*)(void)) fiber_wrapper, 1, current_fiber);
        }

        if (current_fiber->status == fiber_paused) {
            DEBUG_LOG1("SWAPPING CTX TO %zu\n", current_fiber->id);

            current_fiber->thread_id = data->id;
            current_fiber->status = fiber_running;
            swapcontext(&data->context, &current_fiber->context);
        }

        if (current_fiber->status == fiber_stopped) {
            fl_fiber_destroy(current_fiber);
        } else {
            _push_queue(e, current_fiber);
        }

    }

    DEBUG_LOG0("Exit executor\n");

    return 0;
}

// Создение контекста
int fl_executor_create(fl_executor **e) {
    if (!e) {
        return fiber_invalid_args;
    }
    fl_executor *exec = calloc(sizeof(fl_executor), 1);
    exec->thread_count = get_nprocs() * 2;
    if (exec->thread_count > FL_THREAD_MAX) {
        exec->thread_count = FL_THREAD_MAX;
    }


    exec->thread_count = 2;

    pthread_mutex_init(&exec->fibers_queue_lock, NULL);

    *e = exec;

    return fiber_ok;
}

// Запуск контекста
int fl_executor_start(fl_executor *exec) {
    if (!exec)
        return fiber_invalid_args;
    if (exec->thread_ids[0] != 0)
        return fiber_ok;

    for (int i = 0; i < exec->thread_count; i++) {
        exec->thread_data[i].id = i;
        exec->thread_data[i].executor = exec;

        pthread_create(exec->thread_ids + i, NULL, (void *(*)(void *)) fl_execute, &exec->thread_data[i]);
    }

    return fiber_ok;
}

// Отмена контекста
int fl_executor_cancel(fl_executor *exec) {
    if (!exec)
        return fiber_invalid_args;

    for (int i = 0; i < exec->thread_count; i++) {
        pthread_cancel(exec->thread_ids[i]);
    }

    return fiber_ok;
}

// Ожидание завершения контекста
int fl_executor_join(fl_executor *exec) {
    for (int i = 0; i < exec->thread_count; i++) {
        pthread_join(exec->thread_ids[i], NULL);
    }

    return 0;
}

fl_fiberid fl_fiber_create(fl_executor *exec, int fun(fl_fiber *, void *),
                           void *data) {
    exec->fiber_count++;

    fl_fiberid _fiber_id = ++exec->fiber_last_id;

    fl_fiber *fiber = calloc(sizeof(fl_fiber), 1);
    if (!fiber)
        return 0;

    fiber->stack = calloc(FL_STACK_SIZE, 1);
    if (!fiber->stack) {
        exec->fiber_count--;
        free(fiber);
        return 0;
    }

    fiber->id = _fiber_id;
    fiber->data = data;
    fiber->exec = exec;
    fiber->init = fun;


    _push_queue(exec, fiber);

    return _fiber_id;
}

// Ожидание завершения легковесного потока
int fl_fiber_join(fl_executor *executor, fl_fiberid id) {
    if (!executor) {
        DEBUG_LOG0("Invalid executor ptr\n");
        return fiber_invalid_args;
    }
    if (id < executor->fiber_last_id) {
        DEBUG_LOG0("Invalid fiber id\n");
        return fiber_invalid_args;
    }

    for (fl_fiber *i = executor->fiber_last; i; i = i->next) {
        if (i->id == id) {
            while (i->status != fiber_stopped);
            return 0;
        }
    }

    return 0;
}

// Получение номера текущего легковесного потока
fl_fiberid fl_fiber_id(fl_fiber *fiber) {
    return fiber->id;
}

void fl_fiber_yeild(fl_fiber *fiber) {
    DEBUG_LOG0("SWAPPING CTX TO EXEC\n");
    if (fiber->status == fiber_running)
        fiber->status = fiber_paused;

    swapcontext(&fiber->context, &fiber->exec->thread_data[fiber->thread_id].context);
}
