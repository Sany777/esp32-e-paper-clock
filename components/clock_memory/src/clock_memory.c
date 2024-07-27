#include "clock_memory.h"

#include "nvs.h"

void memory_write(const clock_data_t *main_data, const int data_identificator) 
{
	nvs_handle_t nvs_handle_clock;
    nvs_open("nvs", NVS_READWRITE, &nvs_handle_clock);
	switch(data_identificator) {
		case DATA_PWD :
		{
			nvs_set_str(nvs_handle_clock, "password", main_data->pwd); 
			break;
		}
		case DATA_API :
		{
			nvs_set_str(nvs_handle_clock, "api", main_data->api_key); 
			break;
		}
		case DATA_CITY :
		{
			nvs_set_str(nvs_handle_clock, "city", main_data->city_name); 
			break;
		} 	
		case DATA_SSID :
		{
			nvs_set_str(nvs_handle_clock, "ssid", main_data->ssid); 
			break;
		}
		case DATA_FLAGS :
		{
			uint32_t bits = get_device_state();
			nvs_set_u32(nvs_handle_clock, "flag", bits); 
			break;
		}
		case DATA_NOTIF :
		{
			nvs_set_u32(nvs_handle_clock, "notif_num", main_data->notification_num);
			nvs_set_blob(nvs_handle_clock, "notif", main_data->notification, main_data->notification_num*sizeof(int));							  	
			break;
		}
		case DATA_OFFSET : 
		{
			nvs_set_i32(nvs_handle_clock, "offset", main_data->time_offset);
			break;
		}
		default : break;
	}
	nvs_commit(nvs_handle_clock);
	nvs_close(nvs_handle_clock);
}


void memory_read(clock_data_t *main_data, const int data_identificator) 
{
	nvs_handle_t nvs_handle_clock = NULL;
	nvs_open("nvs", NVS_READWRITE, &nvs_handle_clock);
	size_t len;
	switch(data_identificator) {
		case DATA_PWD : 
		{
			len = MAX_STR_LEN;
			nvs_get_str(nvs_handle_clock, "password", main_data->pwd, &len); 
			break;
		}
		case DATA_API :
		{
			len = API_LEN+1;
			nvs_get_str(nvs_handle_clock, "api", main_data->api_key, &len); 
			break;
		}
		case DATA_CITY :
		{
			len = MAX_STR_LEN;
			nvs_get_str(nvs_handle_clock, "city", main_data->city_name, &len); 
			break;
		}
		case DATA_SSID :
		{
			len = MAX_STR_LEN;
			nvs_get_str(nvs_handle_clock, "ssid", main_data->ssid, &len); 
			break;
		}
		case DATA_FLAGS :
		{
			uint32_t flags;
			nvs_get_u32(nvs_handle_clock, "flag", &flags); 
			xEventGroupSetBits(clock_event_group, flags&STORED_FLAGS);
			break;
		}
		case DATA_OFFSET : 
		{
			nvs_get_i32(nvs_handle_clock, "offset", &main_data->time_offset);
			break;
		}
		case DATA_NOTIF :
		{
			uint32_t notif_num;
			nvs_get_u32(nvs_handle_clock, "notif_num", &notif_num); 
			len = notif_num * sizeof(int);
			if(main_data->notification){
				free(main_data->notification);
				main_data->notification = NULL;
			}
			main_data->notification = (int*)malloc(len);
			nvs_get_blob(nvs_handle_clock, "notif", (uint8_t *)main_data->notification, &len); 		
			main_data->notification_num = notif_num;				  	
			break;
		}
		default : break;
	}
	nvs_close(nvs_handle_clock);
}

void memory_read_all(clock_data_t *main_data) 
{
	for(uint8_t i=0; i<END_OF_DATA_TYPES; ++i) {
		memory_read(main_data, i);
	}
}

void memory_read_offset(int32_t *offset)
{
	nvs_handle_t nvs_handle_clock;
	nvs_open("nvs", NVS_READWRITE, &nvs_handle_clock);
	nvs_get_i32(nvs_handle_clock, "offset", offset);
	nvs_close(nvs_handle_clock);
}

void memory_write_offset(const int32_t offset)
{
	nvs_handle_t nvs_handle_clock;
	nvs_open("nvs", NVS_READWRITE, &nvs_handle_clock);
	nvs_set_i32(nvs_handle_clock, "offset", offset);
	nvs_close(nvs_handle_clock);
}