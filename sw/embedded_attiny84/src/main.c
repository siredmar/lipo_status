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
