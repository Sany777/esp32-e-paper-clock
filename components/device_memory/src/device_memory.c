#include "device_memory.h"
#include "device_macro.h"

#include "nvs.h"

const char *SPACE_NAME = "nvs";

int read_flash(const char* data_key, unsigned char *buf, unsigned data_size)
{
    if(data_size == 0)return ESP_OK;
    if(buf == NULL)return ESP_ERR_NO_MEM;
    nvs_handle_t nvs_handle_clock;
    CHECK_AND_RET_ERR(nvs_open(SPACE_NAME, NVS_READONLY, &nvs_handle_clock));
    CHECK_AND_RET_ERR(nvs_get_blob(nvs_handle_clock, data_key, buf, &data_size));
    nvs_close(nvs_handle_clock);
    return ESP_OK;
}

int write_flash(const char* data_key, unsigned char *buf, unsigned data_size)
{
    if(data_size == 0)return ESP_OK;
    if(buf == NULL)return ESP_ERR_NO_MEM;
    nvs_handle_t nvs_handle_clock;
    CHECK_AND_RET_ERR(nvs_open(SPACE_NAME, NVS_READWRITE, &nvs_handle_clock));
    CHECK_AND_RET_ERR(nvs_set_blob(nvs_handle_clock, data_key, buf, data_size));
    CHECK_AND_RET_ERR(nvs_commit(nvs_handle_clock));
    nvs_close(nvs_handle_clock);
    return ESP_OK;
}

