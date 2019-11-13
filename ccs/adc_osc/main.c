#include <msp430.h> 


/**
 * main.c
 */

volatile int buffer[8000] = {0};
volatile int n = 0;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

//-----------Configuracoes do SMCLK -----------------//
	// Selecionar tap 25 da faixa 4 p/ f = 25.075 MHz
	UCSCTL0 = 0x1900;
	UCSCTL1 = DCORSEL_4;
	UCSCTL4 = 0x03;

//-----------Configuracoes do TIMER0A ---------------//
	// Seleciona ACLK como fonte | conta ate CCR0 | limpa o timer
	TA0CTL = TASSEL_1 | MC_3 | TACLR;

	// Conta ate XXXX para dar um periodo de amostragem do ADC12_A
	// de YY s.
	// Tempo de amostragem: t_sample > (R_S + R_I) × ln(2^(n+1)) × C_I + 800 ns
	// Para R_S = 10R, R_I = 1k, n=12, C_I = 20pF --> t_sample > 0.984 us ~ 1 us
	TA0CCR0 = 15999;
	
	// CCR1 OUT --> ADC12SHS_1
	// Modo de saida Reset/Set.
	TA0CTL1 = OUTMOD_7;

//-----------Configuracoes do ADC12_A ---------------//
	P6SEL |= BIT0; // Selecionar o ADC12_A no pino 6.0

	REFCTL0 &= ~REFMSTR; // Tensao de referencia sera aquela do
	                     // modulo interno do ADC ao invez daquela
	                     // do modulo REF

	// ligar o ADC12 | liga a referencia do ADC | referencia de 2.5 V | no. ciclos de clk para amostragem
	// modo mais rapido possivel de amostragem = ADC12MSC
	ADC12CTL0 = ADC12ON | ADC12REFON | ADC12REF2_5V | ADC12SHT0_12 | ADC12MSC;

	// fonte da borda de subida do sample e hold | fonte do sinal SAMPCON | SMCLK como clk do ADC
	// | modo de sequencia de conversao
	ADC12CTL1 = ADC12SHS_1 | ADC12SHP | ADC12SSEL_3 | ADC12CONSEQ_0;
	ADC12CTL2 = ADC12RES_2; // Resolucao de 12 bits

	// Habilita o ADC12_A
	// Mudancas nos parametros do ADC12_A devem ser feitas
	// ANTES de habilitar o ADC12_A!
	ADC12CTL0 |= ADC12ENC;

	// Habilita interrupcoes
	ADC12IE = ADC12IE0;


	__enable_interrupt();

	while(1) {
	    ADC12CTL0 |= ADC12SC;
	};
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_A_ISR(void) {
    buffer[n] = ADC12MEM0;
    if(n == 7999) ADC12IE &= ~ADC12IE0;
    else n++;
}
