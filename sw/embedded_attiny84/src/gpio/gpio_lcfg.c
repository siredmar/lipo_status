#include "gpio.h"

const gpio_ConfigType gpio_initialConfiguration_s =
{
      {
       /* PORT A */
            {
                  GPIO_FALSE,
                  GPIO_CHANNEL_PA0,
                  GPIO_INPUT,
                  GPIO_PIN_INITIAL_HIGH
            },
            {
                  GPIO_FALSE,
                  GPIO_CHANNEL_PA1,
                  GPIO_INPUT,
                  GPIO_PIN_INITIAL_HIGH
            },
            {
                  GPIO_TRUE,
                  GPIO_CHANNEL_PA2,
                  GPIO_OUTPUT,
                  GPIO_PIN_INITIAL_LOW
            },
            {
                  GPIO_TRUE,
                  GPIO_CHANNEL_PA3,
                  GPIO_INPUT,
                  GPIO_PIN_INITIAL_LOW
            },
            {
                  GPIO_TRUE,
                  GPIO_CHANNEL_PA4,
                  GPIO_INPUT,
                  GPIO_PIN_INITIAL_LOW
            },
            {
                  GPIO_TRUE,
                  GPIO_CHANNEL_PA5,
                  GPIO_OUTPUT,
                  GPIO_PIN_INITIAL_LOW
            },
            {
                  GPIO_TRUE,
                  GPIO_CHANNEL_PA6,
                  GPIO_INPUT,
                  GPIO_PIN_INITIAL_LOW
            },
            {
                  GPIO_FALSE,
                  GPIO_CHANNEL_PA7,
                  GPIO_INPUT,
                  GPIO_PIN_INITIAL_HIGH
            },

      /* PORT B */
            {
                  GPIO_FALSE,
                  GPIO_CHANNEL_PB0,
                  GPIO_INPUT,
                  GPIO_PIN_INITIAL_HIGH
            },
            {
                  GPIO_FALSE,
                  GPIO_CHANNEL_PB1,
                  GPIO_INPUT,
                  GPIO_PIN_INITIAL_HIGH
            },
            {
                  GPIO_FALSE,
                  GPIO_CHANNEL_PB2,
                  GPIO_INPUT,
                  GPIO_PIN_INITIAL_HIGH
            },
            {
                  GPIO_FALSE,
                  GPIO_CHANNEL_PB3,
                  GPIO_INPUT,
                  GPIO_PIN_INITIAL_HIGH
            }
      }
};





const void *gpio_getlcfgdata(void)
{
   return ((void*) &gpio_initialConfiguration_s);
}
