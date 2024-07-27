#include "clock_system.h"

#include "stdlib.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"


EventGroupHandle_t clock_event_group;
clock_data_t clock_data;


void clock_system_init()
{
    clock_event_group = xEventGroupCreate();
    

}


// uint32_t get_device_state()
// {
//     return xEventGroupGetBits(clock_event_group);
// }

// uint32_t set_device_state(uint32_t _bits)
// {
//     return xEventGroupSetBits(clock_event_group, (uint32_t _bits));
// }

// uint32_t clear_device_state(uint32_t _bits)
// {
//     return xEventGroupClearbit(clock_event_group, (uint32_t _bits));
// }

// uint32_t wait_bits(uint32_t _bits)
// {
//     return xEventGroupWaitbits(clock_event_group,(uint32_t _bits),pdFALSE,pdFALSE,10000/portTICK_PERIOD_MS);
// }