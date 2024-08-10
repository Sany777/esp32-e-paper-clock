#ifndef DEVICE_GPIO_H
#define DEVICE_GPIO_H




#ifdef __cplusplus
extern "C" {
#endif

void device_gpio_init();
int device_get_joystick_btn();
int device_set_pin(int pin, unsigned state);


#define GPIO_WAKEUP_PIN     (34)
#define AHT21_EN_PIN        (23)
#define SIG_OUT_PIN         (18) 
#define MPU6500_EN_PIN      (19)
#define I2C_MASTER_SCL_IO   (26)       
#define I2C_MASTER_SDA_IO   (25)        
#define EP_ON_PIN           (22)



enum CMD{
    BUT_RIGHT,
    BUT_PRESS,
    BUT_LEFT,
    NO_IN_DATA = -1,
};


enum PinoutInfo{
    EP_CS       = 4,
    EP_DC       = 16,
    EP_RST      = 17,
    EP_BUSY     = 5,
    EP_SDA      = 15,
    EP_SCL      = 14,
};


#ifdef __cplusplus
}
#endif























#endif