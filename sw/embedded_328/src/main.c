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
   LED_UNDER_20_PERCENT = 4
}ledPercentIndicatorType;

/* 80%, 60%, 40%, 20% of 6 * 3.7V, 5* 3.7V, ... 1 * 3.7V */
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
   char sprintfBuffer[100];

   ledPercentIndicatorType ledPercentIndicator;

   ledPercentIndicator = LED_FULL;
//
//   sprintf(sprintfBuffer, "cellArray[%i][%i] = %f\n\r", cells -1, 0, cellArray[cells - 1][0]);
//   uart_puts(sprintfBuffer);
//   sprintf(sprintfBuffer, "cellArray[%i][%i] = %f\n\r", cells -1, 1, cellArray[cells - 1][1]);
//   uart_puts(sprintfBuffer);
//   sprintf(sprintfBuffer, "cellArray[%i][%i] = %f\n\r", cells -1, 2, cellArray[cells - 1][2]);
//   uart_puts(sprintfBuffer);
//   sprintf(sprintfBuffer, "cellArray[%i][%i] = %f\n\r", cells -1, 3, cellArray[cells - 1][3]);
//   uart_puts(sprintfBuffer);

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

      if(lipo_switch > 0)
      {
         led = checkUbatState(lipo_switch, ubatVoltage);
      }
      else
      {
         led = 99;
      }
      sprintf(sprintfBuf, "ubat voltage: %.1f, lipo cells: %d, led: %d\n\r", ubatVoltage, lipo_switch, led);
      uart_puts(sprintfBuf);
	_delay_ms(500);
   }
   return 0;
}
