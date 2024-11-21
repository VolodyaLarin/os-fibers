//
// Created by volodya on 10.10.24.
//

#ifndef LAB01_FIBERS_CHANNELS_H
#define LAB01_FIBERS_CHANNELS_H

#include "fibers.h"


// Создание канала
int fl_channel_create(fl_channel **);
// Запись из канала, блокируемая операция
int fl_channel_write(fl_channel *, const char *data, size_t size);
// Чтение из канала, легковесный поток приостанавливается до завершения операции
int fl_channel_read(fl_fiber *, fl_channel *, char *data, size_t size);
// Чтение из канала пакета
int fl_channel_pop(fl_fiber *, fl_channel *, char *data, size_t max_size);
// Закрытие канала
int fl_channel_close(fl_channel *);



#endif //LAB01_FIBERS_CHANNELS_H
