#pragma once

#include <stdlib.h>

#define ALLOW_DEBUG_LOG

typedef enum {
    fiber_ok = 0,
    fiber_invalid_args = -1,
} fiber_error;

// Описывает единицу потока выполнения (runtime) легковесных потоков (fiber)
// Executor
struct fl_executor_struct;
typedef struct fl_executor_struct fl_executor;

// Легковесный поток
struct fl_fiber_struct;
typedef struct fl_fiber_struct fl_fiber;
typedef size_t fl_fiberid;

// Канал
struct fl_channel_struct;
typedef struct fl_channel_struct fl_channel;

//************************************************ */

// Создение контекста
int fl_executor_create(fl_executor **);

// Запуск контекста
int fl_executor_start(fl_executor *);

// Отмена контекста
int fl_executor_cancel(fl_executor *);

// Ожидание завершения контекста
int fl_executor_join(fl_executor *);

//************************************************ */

// Создание легковесноого потока
fl_fiberid fl_fiber_create(fl_executor *, int(fl_fiber *, void *), void *data);

// Ожидание завершения легковесного потока
int fl_fiber_join(fl_executor *, fl_fiberid);

//************************************************ */

// Получение номера текущего легковесного потока
fl_fiberid fl_fiber_id(fl_fiber *);

// Передача управления от текущего легковесного потока к следующему
void fl_fiber_yeild(fl_fiber *);

//************************************************ */


#ifdef ALLOW_DEBUG_LOG

#include <stdio.h>

#define DEBUG_LOG0(msg)   do {       \
    printf("%3zu#DEBUG(%s): " msg, pthread_self() % 1000, __func__);  \
} while(0);

#define DEBUG_LOG1(msg, arg0)   do {  \
    printf("%3zu#DEBUG(%s): " msg, pthread_self() % 1000, __func__, arg0);  \
} while(0);

#endif

#ifndef ALLOW_DEBUG_LOG

#define DEBUG_LOG0(msg) do{}while(0)
#define DEBUG_LOG1(msg, arg0) do{}while(0)

#endif




// Мультипоток для файберов 

// Конфиги по желанию для файберов 
// Стек можнов настоящем или куче

// Существует вероятность кто-то из стандартных функций может поругаться, что стек находится где-то. 
// с точки зрения потока 


// Можно ограничииться кол-вом файберов, что выделить 
// механизмы мьютексов и т.д. для синхронизациии между потоками
// Пул потоков -- резализ


// Появилась тема -- кого-то ядра нужно разобраться как ядро работает free rtos 

// Unit tests -- обеспечить покрытие данных 
// Основные компонентов -- юзерспейсе, потом перенисти в ядро уже отлаженные компоненты. 

// Изоляция процессов -- появилась мысль, что на самом деле процессы не всегда изолированы. 
// Написать свой аля дебагером -- который будет имитировать ядро 

// В виндах есть спец привелегия для отладки, есть базовые механизмы для лазания в других процессах. 

// fork clone clone2
// strace




// Как развернет стек исключение плюсовое