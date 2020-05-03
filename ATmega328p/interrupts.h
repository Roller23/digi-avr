#ifndef __INTERRUPTS_
#define __INTERRUPTS_

// Data taken from 328p io gcc-avr lib header

typedef enum {
  RESET_vect = 0, /* Reset Interrupt Request */
  INT0_vect, /* External Interrupt Request 0 */
  INT1_vect, /* External Interrupt Request 1 */
  PCINT0_vect, /* Pin Change Interrupt Request 0 */
  PCINT1_vect, /* Pin Change Interrupt Request 0 */
  PCINT2_vect, /* Pin Change Interrupt Request 1 */
  WDT_vect, /* Watchdog Time-out Interrupt */
  TIMER2_COMPA_vect, /* Timer/Counter2 Compare Match A */
  TIMER2_COMPB_vect, /* Timer/Counter2 Compare Match B */
  TIMER2_OVF_vect, /* Timer/Counter2 Overflow */
  TIMER1_CAPT_vect, /* Timer/Counter1 Capture Event */
  TIMER1_COMPA_vect, /* Timer/Counter1 Compare Match A */
  TIMER1_COMPB_vect, /* Timer/Counter1 Compare Match B */
  TIMER1_OVF_vect, /* Timer/Counter1 Overflow */
  TIMER0_COMPA_vect, /* TimerCounter0 Compare Match A */
  TIMER0_COMPB_vect, /* TimerCounter0 Compare Match B */
  TIMER0_OVF_vect, /* Timer/Couner0 Overflow */
  SPI_STC_vect, /* SPI Serial Transfer Complete */
  USART_RX_vect, /* USART Rx Complete */
  USART_UDRE_vect, /* USART, Data Register Empty */
  USART_TX_vect, /* USART Tx Complete */
  ADC_vect, /* ADC Conversion Complete */
  EE_READY_vect, /* EEPROM Ready */
  ANALOG_COMP_vect, /* Analog Comparator */
  TWI_vect, /* Two-wire Serial Interface */
  SPM_READY_vect /* Store Program Memory Read */
} Interrupt_vector_t;

#endif // __INTERRUPTS_