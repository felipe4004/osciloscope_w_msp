#include <msp430.h> 


/**
 * main.c
 */


    //ADC12CTL0 &= ~ADC12SC;
    //ADC12CTL0 |= ADC12SC;

volatile int val = 0;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	P6SEL |= BIT0; // ADC option select

	REFCTL0 &= ~REFMSTR;

	ADC12CTL0 = ADC12ON | ADC12REFON | ADC12REF2_5V | ADC12SHT0_12 | ADC12MSC;
	//ADC12CTL0 = ADC12ON | ADC12SHT02 | ADC12MSC;
	ADC12CTL1 = ADC12SHS_0 | ADC12SHP | ADC12CONSEQ_0;
	ADC12CTL2 = ADC12RES_2; // Resolucao de 12 bits

	ADC12CTL0 |= ADC12ENC;

	ADC12IE = ADC12IE0;



	__enable_interrupt();

	while(1) {
	    ADC12CTL0 |= ADC12SC;
	};
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_A_ISR(void) {
    val = ADC12MEM0;
}
