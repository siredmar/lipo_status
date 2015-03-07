/* *************************************************************************************************
 * file:        adc.c
 *
 *          The adc module.
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
 *      bit7  bit6  bit5  bit4  bit3  bit2  bit1  bit0   register
 *      -----+-----+-----+-----+-----+-----+-----+-----+---------+
 *      ADEN  ADSC  ADATE ADIF  ADIE  ADPS2 ADPS1 ADPS0| ADCSRA
 *      –     ACME  –     –     –     ADTS2 ADTS1 ADTS0| ADCSRB
 *      REFS1 REFS0 ADLAR MUX4  MUX3  MUX2  MUX1  MUX0 | ADMUX
 *
 *
 * todo: ADC: think about how to prescale adc right automatically
 * todo: ADC: what about averaging in ISR? freaky or geeky?
 *
 * copyright:   http://creativecommons.org/licenses/by-nc-sa/3.0/
 **************************************************************************************************/
/* ------------------------------------ INCLUDES ------------------------------------------------ */
#include "adc.h"


/* ------------------------------------ DEFINES ------------------------------------------------- */

/* ------------------------------------ TYPE DEFINITIONS ---------------------------------------- */

/* ------------------------------------ GLOBAL VARIABLES ---------------------------------------- */

/* ------------------------------------ PRIVATE VARIABLES --------------------------------------- */

static adc_ConfigType  adcConfig;

static volatile const adc_RegisterAddressType adcRegisterAdresses_as =
{
        (uint8*) ADC_ADCL_ADDRESS,
        (uint8*) ADC_ADCH_ADDRESS,
        (uint8*) ADC_ADCSRA_ADDRESS,
        (uint8*) ADC_ADCSRB_ADDRESS,
        (uint8*) ADC_ADMUX_ADDRESS,
        (uint8*) ADC_DIDR0_ADDRESS
};


/* ------------------------------------ PROTOTYPES ---------------------------------------------- */
static uint16 adc_getResult8bit(void);
static uint16 adc_getResult10bit(void);


/* ------------------------------------ GLOBAL FUNCTIONS ---------------------------------------- */

void adc_init(const adc_ConfigType *configPtr)
{
    if (configPtr == ADC_CALLBACK_NULL_PTR)
    {
        configPtr = (const adc_ConfigType*)adc_getLcfgData();
    }

    adcConfig.enableState_e         = (adc_EnableStateType_e)         (0x01 & configPtr->enableState_e);
    adcConfig.interruptState_e      = (adc_InterruptStateType_e)      (0x01 & configPtr->interruptState_e);
    adcConfig.prescalerControl_e    = (adc_PrescalerType_e)           (0x07 & configPtr->prescalerControl_e);
    adcConfig.triggerControl_e      = (adc_TriggerType_e)                     configPtr->triggerControl_e;
    adcConfig.referenceControl_e    = (adc_ReferenceType_e)           (0x03 & configPtr->referenceControl_e);
    adcConfig.defaultChannel_e      = (adc_ChannelType_e)             (0x07 & configPtr->defaultChannel_e);
    adcConfig.digitalInputDisable_e = (adc_DigitalInputDisableType_e)         configPtr->digitalInputDisable_e;
    adcConfig.callbackFunc_pv       = (adc_CallbackType)                      configPtr->callbackFunc_pv;
    adcConfig.averageControl_e      = (adc_AverageType_e)             (0x07 & configPtr->averageControl_e);

    /* enable ADC and set prescaler */
    *(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) = \
            (adcConfig.enableState_e      << ADC_ADEN) | \
            (adcConfig.prescalerControl_e << ADC_ADPS0);

    /* selecting voltage reference, result alignment and ADC channel */
    *(adcRegisterAdresses_as.adc_MuxRegister_pui8) =  \
            (adcConfig.referenceControl_e << ADC_REFS0) | \
            (adcConfig.defaultChannel_e   << ADC_MUX0);

    /* wait for some ADC clock cycles to take the settings */
    //_delay_ms(1);

/* ---------------------------------------------------------------------------------------------- */
    /* dummy read out to discard the invalid first conversion value */
    *(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) |= (1 << ADC_ADSC);
    while (!(*(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) & (1 << ADC_ADIF)));
    *(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) |= (1 << ADC_ADIF);
/* ---------------------------------------------------------------------------------------------- */

    /* set the trigger sources if needed */
    if (adcConfig.triggerControl_e != ADC_TRIGGER_SINGLE_SHOT)
    {
        *(adcRegisterAdresses_as.adc_ControlAndStatusRegisterB_pui8)  = (adcConfig.triggerControl_e << ADC_ADTS0);
        *(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) |= (1 << ADC_ADATE);
    }

    /* configure adc interrupt */
    *(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) |=  (adcConfig.interruptState_e << ADC_ADIE);
}

void adc_disableDigitalInput(const adc_ChannelType_e channels)
{
    /* disable digital system of given port pin */
    *(adcRegisterAdresses_as.adc_DigitalInputDisableRegister_pui8) = channels;
    adcConfig.digitalInputDisable_e = channels;
}

void adc_setChannel(const adc_ChannelType_e channel)
{
    uint8 autoTriggerFlag = 0;

    /* disable auto trigger if running */
    if (*(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) & (1 << ADC_ADATE))
    {
        autoTriggerFlag = 1;
        *(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) &= ~(1 << ADC_ADATE);
    }

    /* wait if a conversion is in progress */
    while(*(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) & (1 << ADC_ADSC));

    /* save channel in local config */
    adcConfig.defaultChannel_e = (adc_ChannelType_e) (0x07 & channel);

    /* clear and set channel in register */
    *(adcRegisterAdresses_as.adc_MuxRegister_pui8) &= 0xE0;
    *(adcRegisterAdresses_as.adc_MuxRegister_pui8) |= (adcConfig.defaultChannel_e << ADC_MUX0);

    /* enable auto trigger if previously set */
    if (autoTriggerFlag != 0) {
        *(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) |= (1 << ADC_ADATE);
    }
}

uint8 adc_read8bit(void)
{
    uint16 result_ui16 = 0;

    /* start conversion */
    *(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) |= (1 << ADC_ADSC);

    if(adcConfig.interruptState_e == ADC_INTERRUPT_DISABLED)
    {
        /* wait for end of conversion, fetch adc value and clear the flag */
        while (!(*(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) & (1 << ADC_ADIF)));
        result_ui16 = adc_getResult8bit();
        *(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) |= (1 << ADC_ADIF);
    }
    else
    {
        /* NOTE: interrupts and callback function must be configured correctly! */
    }

    return (uint8)(result_ui16 >> 2);
}

uint16 adc_read10bit(void)
{
    uint16 result_ui16 = 0;

    /* start conversion */
    *(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) |= (1 << ADC_ADSC);

    if(adcConfig.interruptState_e == ADC_INTERRUPT_DISABLED)
    {
        /* wait for end of conversion, fetch adc value and clear the flag */
        while (!(*(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) & (1 << ADC_ADIF)));
        result_ui16 = adc_getResult10bit();
        *(adcRegisterAdresses_as.adc_ControlAndStatusRegisterA_pui8) |= (1 << ADC_ADIF);
    }
    else
    {
        /* NOTE: interrupts and callback function must be configured correctly! */
    }

    return result_ui16;
}

uint16 adc_read8bitAverage(void)
{
   uint16 avResult_ui16 = 0;
   uint8 averages_ui8;

   averages_ui8 = adcConfig.averageControl_e;

   for(uint8 avCnt_ui8 = 0; avCnt_ui8 <= (1 << averages_ui8); avCnt_ui8++)
   {
       avResult_ui16 += adc_read8bit();
   }

   avResult_ui16 = (uint16) (avResult_ui16 >> averages_ui8);

   return avResult_ui16;
//   return ((uint8)(avResult_ui16 & 0x00FF));
}

uint16 adc_read10bitAverage(void)
{
   uint16 avResult_ui16 = 0;
   uint8 averages_ui8;

   averages_ui8 = adcConfig.averageControl_e;

   for(uint8 avCnt_ui8 = 0; avCnt_ui8 <= (1 << averages_ui8); avCnt_ui8++)
   {
       avResult_ui16 += adc_read10bit();
   }

   avResult_ui16 = (uint16) (avResult_ui16 >> averages_ui8);

   return avResult_ui16;
}


/* ------------------------------------ PRIVATE FUNCTIONS --------------------------------------- */

static uint16 adc_getResult8bit(void)
{
    return *(volatile uint16 *)(adcRegisterAdresses_as.adc_DataRegisterLow_pui8);
}

static uint16 adc_getResult10bit(void)
{
    return *(volatile uint16 *)(adcRegisterAdresses_as.adc_DataRegisterLow_pui8);
}


/* ------------------------------------ INTERRUPT SERVICE ROUTINES ------------------------------ */

ISR(ADC_vect)
{
   uint16 adcResult_ui16 = 0;

   adcResult_ui16 = adc_getResult10bit();

   if(adcConfig.callbackFunc_pv != ADC_CALLBACK_NULL_PTR)
   {
      adcConfig.callbackFunc_pv(adcResult_ui16);
   }
   else
   {
      /* do nothing */
   }
}
/* ************************************ E O F *************************************************** */
