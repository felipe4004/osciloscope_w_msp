#include <msp430.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define phasea  BIT5
#define phaseb  BIT4
#define sw      BIT3
#define SIZE    100
#define LENGTHX 317
#define LENGTHY 208


volatile unsigned char buffer[SIZE] = {0};

// Variaveis para controle
volatile char ymod = 1; // controle vertical
// Variavel para sinal do ADC12
unsigned char signal = 0;

// Variaveis para o encoder
volatile int adc_conv;
volatile int adc_read;
volatile unsigned int sel_time = 0;
volatile char mode;
volatile unsigned int sel_mode = 0x00;
volatile unsigned char disp_val = 0;

float vrms = 0, vavg=0;
int count = 0;

void TIMERA0_CFG();
void ADC12_CFG();
void ENCODER_CFG();
void SPI_CFG();
void DMA_CFG();

int rms_cal (volatile int *buf){
    int xrms;
    unsigned int j;
    for(j=98;j<=0;j--){
        xrms+=buf[j]*buf[j];
    }
    xrms/=98;
    return sqrtf(xrms);
}

int avg_cal (volatile int *buf){
    unsigned int j;
    int xavg;
    for(j=98; j<=0 ; j--){
        xavg = buf[j];
    }
    return xavg/=98;
}


int main(void)
{

	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    TIMERA0_CFG();
    ADC12_CFG();
    ENCODER_CFG();
    SPI_CFG();

	__enable_interrupt();

	while(1) {
	}
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_A_ISR(void) {
    signal = (ADC12MEM0*LENGTHY/255);
    if(UCA0IFG & UCTXIFG) {
        UCA0TXBUF = signal;
    }
}

#pragma vector = PORT1_VECTOR
__interrupt void sw_push(void){
    switch (P1IV){
        case 0x02:
        case 0x04:
        case 0x06:
        case 0x08:                              //Ordenamento dos TAIV para separacao das funcoes
                                                //do encoder.
            P1IFG &= ~0x08;
            sel_mode++;
        switch (sel_mode){
        case    0x00:
            //selecao do tempo//
            mode = 1;
            break;
        case    0x01:
            mode= 2;
            break;
        case    0x02:
            mode= 3;
            break;
        default:
            mode= 4;
            sel_mode = 0x00;
            break;
        }
            break;
        case 0x0A:
            P1IFG &= ~0x0A;
            switch (mode){
            case 1:
                if((P1IN & phasea)){            //phase a e b sao utilizados para a verificacao
                    sel_time++;                    //da diferenca de fase.
                }
                else{
                    sel_time--;

            }
           switch (sel_time){
           case 0x00:
               TA0CCR0 = 49999;
               break;
           case 0x01:
               TA0CCR0 = 9999;
               break;
           case 0x02:
               TA0CCR0 = 4999;
               break;
           case 0x03:
               TA0CCR0 = 999;
               break;
           case 0x04:
               TA0CCR0 = 499;
               break;
           default:
               sel_time = 0x00;
           }
                }
            break;
        case 0x0C:
        case 0x0E:
        case 0x10:
        default:
          P1IFG = 0x00;
          break;
    }
        //TACCR é atualizado com o valor obtido do encoder.
}

void SPI_CFG() {
    P3SEL |= BIT4 | BIT3; // Enable SPI SOMI and SIMO on pins 3.4, 3.3
    P2SEL |= BIT7; // Enable SPI Clock output on pin 2.7
    // Software reset in order to manipulate control registers
    UCA0CTL1 = UCSWRST;
    // Configurations for SPI Mode 0
    // Sample on rising, non-inverted clock polarity, master mode, 3-pin SPI,
    // synchronous mode
    UCA0CTL0 = UCCKPH | UCCKPL | UCMST | UCMSB | UCMODE_0 | UCSYNC;
    // SMCLK as source (1 MHz)
    UCA0CTL1 |= UCSSEL_2;
    // SMCLK / 2
    UCA0BR0 = 0x02;
    UCA0BR1 = 0x00;
    // Disable modulation
    UCA0MCTL = 0;
    UCA0CTL1 &= ~UCSWRST; // Start SPI state machine
    // Enable interrupts
    UCA0TXBUF = 0; // Reset transmit buffer
    //UCA0IE |= UCTXIE;
}

void TIMERA0_CFG() {
//-----------Configuracoes do TIMER0A ---------------//
    // Seleciona SMCLK como fonte | conta ate CCR0 | limpa o timer
    TA0CTL = TASSEL_2 | MC_3 | TACLR;

    /*
     * O timer definira o comprimento div/s, com os seguintes espaçamentos
     * 0.05 s  : CCR0 = 49999
     * 0.01 s  : CCR0 = 9999
     * 0.005 s : CCR0 = 4999
     * 0.001 s : CCR0 = 999
     * 0.0005 s: CCR0 = 499
     *
     * O valor minimo do CCR0 deve ser de 265 para que ocorra uma amostragem correta
     * no ADC12
     */

    // Tempo de amostragem: t_sample > (R_S + R_I) × ln(2^(n+1)) × C_I + 800 ns
    // Para R_S = 10R, R_I = 1k, n=12, C_I = 20pF --> t_sample > 0.984 us ~ 1 us
    TA0CCR0 = 99;

    // CCR1 OUT --> ADC12SHS_1
    // Modo de saida Reset/Set.
    TA0CCTL1 = OUTMOD_7;

}

void ADC12_CFG() {
//-----------Configuracoes do ADC12_A ---------------//
    P6SEL |= BIT0; // Selecionar o ADC12_A no pino 6.0

    REFCTL0 &= ~REFMSTR; // Tensao de referencia sera aquela do
                         // modulo interno do ADC ao invez daquela
                         // do modulo REF

    ADC12CTL0 &= ~ADC12ENC;
    // ligar o ADC12 | liga a referencia do ADC | referencia de 2.5 V | 256 ciclos de clk para amostragem
    ADC12CTL0 = ADC12ON | ADC12REFON | ADC12REF2_5V | ADC12SHT0_8;

    // fonte da borda de subida do sample e hold | SMCLK como clk do ADC | Sample-and-hold pelo sampling timer (256 ciclos)
    // | modo de sequencia de conversao single-channel repetitivo
    ADC12CTL1 = ADC12SHS_1 | ADC12SSEL_3 | ADC12SHP | ADC12CONSEQ_2;
    ADC12CTL2 = ADC12RES_0; // Resolucao de 12 bits

    // Habilita o ADC12_A
    // Mudancas nos parametros do ADC12_A devem ser feitas
    // ANTES de habilitar o ADC12_A!
    ADC12CTL0 |= ADC12ENC;

    // Habilita as interrupcoes
    ADC12IE = ADC12IE0;
}

void ENCODER_CFG() {
    P2DIR |= BIT4;                          //P2.4 sendo selecionado para saida
    P2SEL |= BIT4 | BIT0;                   //P2.4 sendo selecionado para funcao PWM e P2.0 para funcao de captura
    P2REN |= BIT0;
//    P2OUT |= BIT0;

    P1REN |= BIT5 | BIT4 | BIT3;           //Resistores internos em modo pull-up
    P1OUT |= BIT5 | BIT4 | BIT3;
    P1IE  |= phaseb | sw;                   // interrupt enable dos pinos
    P1IES |= phaseb | sw;
    P1IFG = 0x00;

    P4DIR |= BIT1 | BIT2;
    P4OUT = 0x00;
    P2DIR |= BIT7;
    P2OUT = 0x00;
}

void DMA_CFG(){
//--------------inicializando os enderecos de memoria referentes ao DMA-----------//
    __data16_write_addr((unsigned short) &DMA0SA,(unsigned long) &ADC12MEM0); //endereco unico de origem


    __data16_write_addr((unsigned short) &DMA0DA,(unsigned long) &adc_read); //bloco de enderecos de destino

    DMA0SZ = 50;
    DMA0CTL |= DMADT_4 | DMADSTINCR_3;
    DMACTL0 |= DMA0TSEL_24;
    DMA0CTL |= DMAEN;

    __data16_write_addr((unsigned short) &DMA1SA,(unsigned long) &ADC12MEM0); //endereco unico de origem

    __data16_write_addr((unsigned short) &DMA1DA,(unsigned long) &adc_conv); //endereco unico de destino

    DMA1SZ=1;
    DMA1CTL |= DMADT_4 | DMAIE;
    DMACTL0 |= DMA1TSEL_24;
    DMA1CTL |= DMAEN;
 }

