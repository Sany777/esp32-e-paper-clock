#ifndef CLOCK_MEMORY_H
#define CLOCK_MEMORY_H

#include "clock_system.h"

enum DataType{
	DATA_PWD,
	DATA_API,
	DATA_CITY,
	DATA_SSID,
	DATA_NOTIF,
	DATA_FLAGS,
	DATA_NOTIF_NUM,
	DATA_OFFSET,
	END_OF_DATA_TYPES,
};


void memory_write(const clock_data_t *main_data, const int data_identificator);
void memory_read(clock_data_t *main_data, const int data_identificator);
void memory_read_all(clock_data_t *main_data);
void memory_read_offset(int32_t *offset);
void memory_write_offset(const int32_t offset);


#endif