#include "../inc/std_types.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include "gpio/gpio.h"

#include "adc/adc.h"
#include "adc/adc_lcfg.h"

#define LED_CHANNEL_0   GPIO_CHANNEL_PA2
#define LED_CHANNEL_1   GPIO_CHANNEL_PA3
#define LED_CHANNEL_2   GPIO_CHANNEL_PA4
#define LED_CHANNEL_3   GPIO_CHANNEL_PA5
#define LED_CHANNEL_4   GPIO_CHANNEL_PA6

#define DIGIT_DIFF 5
#define in_between(x, y, z) (x > (y - z)) && (x < (y + z))
#define ADC_DIGITS (4095)
#define ADC_REF_VOLTAGE (5.0)
#define UBAT_DIVIDER (34.8)
#define ubat_digit_to_volt(x) (((float32)(x * ADC_REF_VOLTAGE) / ADC_DIGITS) * UBAT_DIVIDER)

typedef enum
{
   LIPO_CELL_1 = 355,
   LIPO_CELL_2 = 490,
   LIPO_CELL_3 = 568,
   LIPO_CELL_4 = 600,
   LIPO_CELL_5 = 632,
   LIPO_CELL_6 = 644,
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
   default:
      gpio_WriteChannel(LED_CHANNEL_0, TRUE);
      gpio_WriteChannel(LED_CHANNEL_1, FALSE);
      gpio_WriteChannel(LED_CHANNEL_2, TRUE);
      gpio_WriteChannel(LED_CHANNEL_3, FALSE);
      gpio_WriteChannel(LED_CHANNEL_4, TRUE);
      break;
   }
}


/* 80%, 60%, 40%, 20% of 1 * 3.7V, 5* 3.7V, ... 6 * 3.7V */
float32 cellArray[6][4] =
{
      {2.96, 2.22, 1.5, 0.7},
      {5.9, 4.4, 3.0, 1.5},
      {8.9, 6.7, 4.4, 2.2},
      {11.8, 8.9, 5.9, 3.0},
      {14.8, 11.1, 7.4, 3.7},
      {17.7, 13.3, 8.9, 4.4},
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
   ledPercentIndicatorType ledPercentIndicator;

   ledPercentIndicator = LED_FULL;

   if(ubatVoltage < cellArray[cells][0])
   {
      ledPercentIndicator = LED_UNDER_80_PERCENT;
   }
   if(ubatVoltage < cellArray[cells][1])
   {
      ledPercentIndicator = LED_UNDER_60_PERCENT;
   }
   if(ubatVoltage < cellArray[cells][2])
   {
      ledPercentIndicator = LED_UNDER_40_PERCENT;
   }
   if(ubatVoltage < cellArray[cells][3])
   {
      ledPercentIndicator = LED_UNDER_20_PERCENT;
   }

   return ledPercentIndicator;
}

int main()
{
   uint16 lipoSwitchChannel = 0;
   uint16 ubatChannel = 0;
   uint8 lipo_switch = 0;
   float32 ubatVoltage;
   ledPercentIndicatorType led = LED_FULL;

   gpio_init();
   adc_init(ADC_CALLBACK_NULL_PTR);


   sei(); /* Enable the interrupts */
   while(1)
   {
      adc_setChannel(ADC_CHANNEL_0);
      lipoSwitchChannel = adc_read10bit();
      lipo_switch = checkLipoSwitch(lipoSwitchChannel);

      adc_setChannel(ADC_CHANNEL_1);
      ubatChannel = adc_read10bit();
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
      _delay_ms(500);
   }
   return 0;
}
