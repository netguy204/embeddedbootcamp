/** \file adc.c
*
* @brief ADC Driver
*
* @par
* COPYRIGHT NOTICE: (c) 2014 Barr Group, LLC.
* All rights reserved.
*/

#include <assert.h>
#include <stdint.h>				
#include <stdio.h>

#include "os.h"
#include "iorx63n.h"

#include "adc.h"
#include "common.h"
#include  <bsp_glcd.h>


// Message Queue for ISR->Task Communication
static OS_Q  g_adc_q;


#define ADC_SOURCE_VR1      2

#define ADC_INTERRUPT_AFTER_SCAN    0x10
#define ADC_START                   0x80

#define BIT(n)              (1 << (n))


// NOTE: Refer to p. 1,698 to 1,727  of Processor_UsersManual_Hardware.pdf
typedef struct
{
  uint8_t           control;                      // Offset 0x00
  uint8_t  const    _unused1[3];
  uint16_t          channel_select0;              // Offset 0x04
  uint16_t          channel_select1;              // Offset 0x06
  uint16_t          value_addition_mode_select0;  // Offset 0x08
  uint16_t          value_addition_mode_select1;  // Offset 0x0A
  uint8_t           value_addition_count_select;  // Offset 0x0C
  uint8_t  const    _unused2;
  uint16_t          control_extended;             // Offset 0x0E
  uint8_t           start_trigger_select;         // Offset 0x10
  uint8_t  const    _unused3;
  uint16_t          extended_input_control;       // Offset 0x12
  uint8_t  const    _unused4[6];
  uint16_t          temperature_sensor_data;      // Offset 0x1A
  uint16_t          internal_reference_data;      // Offset 0x1C
  uint8_t  const    _unused5[2];
  uint16_t          data[21];                     // Offset 0x20
} adc_t;

adc_t volatile * const p_adc = (adc_t *) 0x00089000;
#define adc     (*p_adc)


/*!
* @brief Configure the ADC hardware to read Potentiometer VR1 and interrupt.
*/
void
adc_init (void)
{
    OS_ERR err;
    
    /* create our reply queue */
    OSQCreate(&g_adc_q, "ADC Queue", 1, &err);
    assert(OS_ERR_NONE == err);
    
    /* Protection off */
    SYSTEM.PRCR.WORD = 0xA503u;            
    
    /* Cancel the S12AD module clock stop mode */
    MSTP_S12AD = 0;

    /* Protection on */
    SYSTEM.PRCR.WORD = 0xA500u;        

    // Select the 12-bit ADC. In the HW course, this detail is in BSP_Init().
    SYSTEM.MSTPCRA.BIT.MSTPA17 = 0;

    /* Use the AN000 (Potentiometer) pin 
       as an I/O for peripheral functions */
    PORT4.PMR.BYTE = 0x01;

    // Enable A/D interrupts at an appropriate priority, as instructed.
    uint8_t * p_IER = (uint8_t *)0x0008720C;
    uint8_t * p_IPR = (uint8_t *)0x00087366;

    *p_IPR = 9;                         // Set interrupt priority.
    *p_IER |= 0x40;                     // Unmask A/D interrupt.

	// Configure the A/D to perform a single scan and interrupt.
    adc.control = ADC_INTERRUPT_AFTER_SCAN;
    adc.channel_select0 = BIT(ADC_SOURCE_VR1);
}

uint16_t
adc_read() {
    OS_ERR err;
    OS_MSG_SIZE  msg_size;
    
    // Trigger ADC conversion.
    adc.control |= ADC_START;
    
    uint16_t * p_sample = (uint16_t *)
                          OSQPend(&g_adc_q, 0, OS_OPT_PEND_BLOCKING, &msg_size, NULL, &err);
    assert(OS_ERR_NONE == err);
    
    return *p_sample;
}

/*!
*
* @brief ADC Interrupt Handler
*/
void
adc_isr (void)
{
    static uint16_t	 sample;    // NOTE: Not on the stack; so address is valid.
    OS_ERR	         err;


    // Read from the A/D converter and reduce the range from 12-bit to 10-bit.
    sample = adc.data[ADC_SOURCE_VR1] >> 2;
	
    // Send the address of the sample via a message queue.
    OSQPost(&g_adc_q, (void *)&sample, sizeof(sample), OS_OPT_POST_FIFO, &err);	
    assert(OS_ERR_NONE == err || OS_ERR_Q_MAX == err);
}
