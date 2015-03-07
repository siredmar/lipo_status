#include "../inc/std_types.h"
#include "uart/uart.h"
#include "twi/twimaster.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include "gpio/gpio.h"

#include "adc/adc.h"
#include "adc/adc_lcfg.h"

#define LED_CHANNEL_0   GPIO_CHANNEL_PB4
#define LED_CHANNEL_1   GPIO_CHANNEL_PB3
#define LED_CHANNEL_2   GPIO_CHANNEL_PB2
#define LED_CHANNEL_3   GPIO_CHANNEL_PB1
#define LED_CHANNEL_4   GPIO_CHANNEL_PB0

#define DIGIT_DIFF 8
#define in_between(x, y, z) (x > (y - z)) && (x < (y + z))
#define ADC_DIGITS (4095)
#define ADC_REF_VOLTAGE (5.0)
#define UBAT_DIVIDER (34.8)
#define ubat_digit_to_volt(x) (((float32)(x * ADC_REF_VOLTAGE) / ADC_DIGITS) * UBAT_DIVIDER)

typedef enum
{
   LIPO_CELL_1 = 358,
   LIPO_CELL_2 = 494,
   LIPO_CELL_3 = 572,
   LIPO_CELL_4 = 607,
   LIPO_CELL_5 = 638,
   LIPO_CELL_6 = 650,
   LIPO_CELL_NONE = 0
}lipoCellDigitsType;

typedef enum
{
   SWITCH_CELL_NONE = 0,
   SWITCH_CELL_1,
   SWITCH_CELL_2,
   SWITCH_CELL_3,
   SWITCH_CELL_4,
   SWITCH_CELL_5,
   SWITCH_CELL_6,
}lipoCellSwitchType;

typedef enum
{
   LED_FULL             = 0,
   LED_UNDER_80_PERCENT = 1,
   LED_UNDER_60_PERCENT = 2,
   LED_UNDER_40_PERCENT = 3,
   LED_UNDER_20_PERCENT = 4,
   LED_INVALID          = 5
}ledPercentIndicatorType;

void showLedStatus(ledPercentIndicatorType led)
{
   static uint8 blink = 0;
   switch(led)
   {
   case LED_FULL:
      gpio_WriteChannel(LED_CHANNEL_0, TRUE);
      gpio_WriteChannel(LED_CHANNEL_1, TRUE);
      gpio_WriteChannel(LED_CHANNEL_2, TRUE);
      gpio_WriteChannel(LED_CHANNEL_3, TRUE);
      gpio_WriteChannel(LED_CHANNEL_4, TRUE);
      break;

   case LED_UNDER_80_PERCENT:
      gpio_WriteChannel(LED_CHANNEL_0, FALSE);
      gpio_WriteChannel(LED_CHANNEL_1, TRUE);
      gpio_WriteChannel(LED_CHANNEL_2, TRUE);
      gpio_WriteChannel(LED_CHANNEL_3, TRUE);
      gpio_WriteChannel(LED_CHANNEL_4, TRUE);
      break;

   case LED_UNDER_60_PERCENT:
      gpio_WriteChannel(LED_CHANNEL_0, FALSE);
      gpio_WriteChannel(LED_CHANNEL_1, FALSE);
      gpio_WriteChannel(LED_CHANNEL_2, TRUE);
      gpio_WriteChannel(LED_CHANNEL_3, TRUE);
      gpio_WriteChannel(LED_CHANNEL_4, TRUE);
      break;

   case LED_UNDER_40_PERCENT:
      gpio_WriteChannel(LED_CHANNEL_0, FALSE);
      gpio_WriteChannel(LED_CHANNEL_1, FALSE);
      gpio_WriteChannel(LED_CHANNEL_2, FALSE);
      gpio_WriteChannel(LED_CHANNEL_3, TRUE);
      gpio_WriteChannel(LED_CHANNEL_4, TRUE);
      break;

   case LED_UNDER_20_PERCENT:
      gpio_WriteChannel(LED_CHANNEL_0, FALSE);
      gpio_WriteChannel(LED_CHANNEL_1, FALSE);
      gpio_WriteChannel(LED_CHANNEL_2, FALSE);
      gpio_WriteChannel(LED_CHANNEL_3, FALSE);
      gpio_WriteChannel(LED_CHANNEL_4, TRUE);
      break;

   case LED_INVALID:
      if(blink % 2 == 0)
      {
         gpio_WriteChannel(LED_CHANNEL_0, TRUE);
      }
      else
      {
         gpio_WriteChannel(LED_CHANNEL_0, FALSE);
      }
      blink++;
      break;
   default:

      break;
   }
}


/* 80%, 60%, 40%, 20% of 1 to 6 Cells */
float32 cellArray[6][4] =
{
      { 4.02,   3.84,    3.66,    3.48}, /* 1 Cell */
      { 8.04,   7.68,    7.32,    6.96}, /* 2 Cells */
      {12.06,  11.52,   10.98,   10.44}, /* 3 Cells */
      {16.08,  15.36,   14.64,   13.92}, /* 4 Cells */
      {20.10,  19.20,   18.30,   17.40}, /* 5 Cells */
      {24.12,  23.04,   21.96,   20.88}, /* 6 Cells */
};

lipoCellSwitchType checkLipoSwitch(uint16 adc_channel)
{
   lipoCellSwitchType lipoSwitch = SWITCH_CELL_NONE;
   if(in_between(adc_channel, LIPO_CELL_1, DIGIT_DIFF))
   {
      lipoSwitch = SWITCH_CELL_1;
   }
   else if(in_between(adc_channel, LIPO_CELL_2, DIGIT_DIFF))
   {
      lipoSwitch = SWITCH_CELL_2;
   }
   else if(in_between(adc_channel, LIPO_CELL_3, DIGIT_DIFF))
   {
      lipoSwitch = SWITCH_CELL_3;
   }
   else if(in_between(adc_channel, LIPO_CELL_4, DIGIT_DIFF))
   {
      lipoSwitch = SWITCH_CELL_4;
   }
   else if(in_between(adc_channel, LIPO_CELL_5, DIGIT_DIFF))
   {
      lipoSwitch = SWITCH_CELL_5;
   }
   else if(in_between(adc_channel, LIPO_CELL_6, DIGIT_DIFF))
   {
      lipoSwitch = SWITCH_CELL_6;
   }
   else
   {
      lipoSwitch = SWITCH_CELL_NONE;
   }
   return lipoSwitch;
}



ledPercentIndicatorType checkUbatState(lipoCellSwitchType cells, float32 ubatVoltage)
{
   char sprintfBuffer[100];

   ledPercentIndicatorType ledPercentIndicator;

   ledPercentIndicator = LED_FULL;

   if(ubatVoltage < cellArray[cells - 1][0])
   {
      ledPercentIndicator = LED_UNDER_80_PERCENT;
   }
   if(ubatVoltage < cellArray[cells - 1][1])
   {
      ledPercentIndicator = LED_UNDER_60_PERCENT;
   }
   if(ubatVoltage < cellArray[cells - 1][2])
   {
      ledPercentIndicator = LED_UNDER_40_PERCENT;
   }
   if(ubatVoltage < cellArray[cells - 1][3])
   {
      ledPercentIndicator = LED_UNDER_20_PERCENT;
   }

   return ledPercentIndicator;
}

int main()
{
   uint16 lipoSwitchChannel = 0;
   uint16 ubatChannel = 0;
   char sprintfBuf[100];
   uint8 lipo_switch = 0;
   float32 ubatVoltage;
   uint8 led = 0;


   uart_init(RECEPTION_DISABLED, TRANSMISSION_ENABLED, INTERRUPT_DISABLED);
   uart_puts("\n\r");
   gpio_init();
   adc_init(ADC_CALLBACK_NULL_PTR);


   sei(); /* Enable the interrupts */
   while(1)
   {
      adc_setChannel(ADC_CHANNEL_0);
      _delay_ms(100);
      lipoSwitchChannel = adc_read10bit();
      _delay_ms(100);
      lipo_switch = checkLipoSwitch(lipoSwitchChannel);

      _delay_ms(100);
      adc_setChannel(ADC_CHANNEL_1);
      ubatChannel = adc_read10bit();
      _delay_ms(100);
      ubatVoltage = ubat_digit_to_volt(ubatChannel);

      if(lipo_switch > SWITCH_CELL_NONE)
      {
         led = checkUbatState(lipo_switch, ubatVoltage);
      }
      else
      {
         led = LED_INVALID;
      }
      showLedStatus(led);

      sprintf(sprintfBuf, "switch raw: %d, raw: %d, ubat voltage: %.2f, lipo cells: %d, led: %d\n\r", lipoSwitchChannel, ubatChannel, ubatVoltage, lipo_switch, led);
      uart_puts(sprintfBuf);
      _delay_ms(100);
   }
   return 0;
}
