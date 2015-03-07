/* *************************************************************************************************
 * file:        adc.h
 *
 *          The adc module header.
 *
 * author:      Armin Schlegel, Mr. L.
 * date:        09.10.2014
 * version:     0.3   worky, testing
 *
 * file history:
 *          09.10.2014  A. Schlegel file created, basic version
 *          14.10.2014  Mr. L.      strutural improvements, nicify layout, add comments
 *
 * notes:
 *          - none -
 *
 * todo: ADC: generate a continous naming scheme
 * todo: ADC: decision if continous setting of enum values is desired or not
 *
 * copyright:   http://creativecommons.org/licenses/by-nc-sa/3.0/
 **************************************************************************************************/
#ifndef _ADC_H_
#define _ADC_H_
/* ============================================================================================== */
/* ------------------------------------ INCLUDES ------------------------------------------------ */

#include <avr/io.h>
#include "std_types.h"
#include "adc_lcfg.h"
#include "adc_cfg.h"
#include "../gpio/gpio.h"


/* ------------------------------------ DEFINES ------------------------------------------------- */

#define ADC_CALLBACK_NULL_PTR ((void*)0)


/* ------------------------------------ TYPE DEFINITIONS ---------------------------------------- */

typedef void (*adc_CallbackType)(uint16);

typedef enum
{
    ADC_REFERENCE_AREF = 0U,
    ADC_REFERENCE_AVCC,
    ADC_REFERENCE_INTERNAL_1V1,
    ADC_REFERENCE_INTERNAL_2V56
}adc_ReferenceType_e;

/* differential and fix voltage input are not implemented yet */
typedef enum
{
    ADC_CHANNEL_0 = 0U,
    ADC_CHANNEL_1,
    ADC_CHANNEL_2,
    ADC_CHANNEL_3,
    ADC_CHANNEL_4,
    ADC_CHANNEL_5,
    ADC_CHANNEL_6,
    ADC_CHANNEL_7
}adc_ChannelType_e;

typedef enum
{
    ADC_INTERRUPT_DISABLED = 0U,
    ADC_INTERRUPT_ENABLED
}adc_InterruptStateType_e;

typedef enum
{
    ADC_MODULE_DISABLED = 0U,
    ADC_MODULE_ENABLED
}adc_EnableStateType_e;

/* auto trigger source - used if ADATE = 1 */
typedef enum
{
    ADC_TRIGGER_SINGLE_SHOT             = 0xFFU,    // single shot
    ADC_TRIGGER_FREE_RUNNING            = 0U,
    ADC_TRIGGER_ANALOG_COMPERATOR       = 1U,
    ADC_TRIGGER_EXT_INT0                = 2U,
    ADC_TRIGGER_TIMER0_COMPARE_MATCH    = 3U,
    ADC_TRIGGER_TIMER0_OVERFLOW         = 4U,
    ADC_TRIGGER_TIMER1_OVERFLOW         = 5U,
    ADC_TRIGGER_TIMER1_COMPARE_MATCH    = 6U,
    ADC_TRIGGER_TIMER1_CAPTURE_EVENT    = 7U
}adc_TriggerType_e;

typedef enum
{
    ADC_CLOCK_PRESCALER_2   = 1U,
    ADC_CLOCK_PRESCALER_4   = 2U,
    ADC_CLOCK_PRESCALER_8   = 3U,
    ADC_CLOCK_PRESCALER_16  = 4U,
    ADC_CLOCK_PRESCALER_32  = 5U,
    ADC_CLOCK_PRESCALER_64  = 6U,
    ADC_CLOCK_PRESCALER_128 = 7U
}adc_PrescalerType_e;


typedef enum
{
    ADC_DIGITAL_INPUT_DISABLE_NONE = 0U,
    ADC_DIGITAL_INPUT_DISABLE_PIN0 = 0x01U,
    ADC_DIGITAL_INPUT_DISABLE_PIN1 = 0x02U,
    ADC_DIGITAL_INPUT_DISABLE_PIN2 = 0x04U,
    ADC_DIGITAL_INPUT_DISABLE_PIN3 = 0x08U,
    ADC_DIGITAL_INPUT_DISABLE_PIN4 = 0x10U,
    ADC_DIGITAL_INPUT_DISABLE_PIN5 = 0x20U,
    ADC_DIGITAL_INPUT_DISABLE_PIN6 = 0x40U,
    ADC_DIGITAL_INPUT_DISABLE_PIN7 = 0x80U,
    ADC_DIGITAL_INPUT_DISABLE_ALL  = 0xFF
}adc_DigitalInputDisableType_e;

typedef enum
{
    ADC_AVERAGE_NONE = 0U,
    ADC_AVERAGE_2_SAMPLES,
    ADC_AVERAGE_4_SAMPLES,
    ADC_AVERAGE_8_SAMPLES,
    ADC_AVERAGE_16_SAMPLES,
    ADC_AVERAGE_32_SAMPLES
}adc_AverageType_e;

typedef struct
{
    adc_EnableStateType_e           enableState_e;
    adc_InterruptStateType_e        interruptState_e;
    adc_PrescalerType_e             prescalerControl_e;
    adc_TriggerType_e               triggerControl_e;
    adc_ReferenceType_e             referenceControl_e;
    adc_ChannelType_e               defaultChannel_e;
    adc_DigitalInputDisableType_e   digitalInputDisable_e;
    adc_CallbackType                callbackFunc_pv;
    adc_AverageType_e               averageControl_e;
}adc_ConfigType;

typedef struct
{
    uint8* adc_DataRegisterLow_pui8;
    uint8* adc_DataRegisterHigh_pui8;
    uint8* adc_ControlAndStatusRegisterA_pui8;
    uint8* adc_ControlAndStatusRegisterB_pui8;
    uint8* adc_MuxRegister_pui8;
    uint8* adc_DigitalInputDisableRegister_pui8;
} adc_RegisterAddressType;


/* ------------------------------------ GLOBAL VARIABLES ---------------------------------------- */


/* ------------------------------------ PROTOTYPES ---------------------------------------------- */

void adc_init(const adc_ConfigType *configPtr);
void adc_disableDigitalInput(const adc_ChannelType_e channels);
void adc_setChannel(const adc_ChannelType_e channel);
uint16 adc_read10bit(void);
uint16 adc_read10bitAverage(void);
uint8 adc_read8bit(void);
uint16 adc_read8bitAverage(void);

/* ************************************ E O F *************************************************** */
#endif /* _ADC_H_ */
