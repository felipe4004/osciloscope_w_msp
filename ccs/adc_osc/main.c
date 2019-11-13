#include <msp430.h> 
#include <math.h>


#define phasea  BIT5
#define phaseb  BIT4
#define sw      BIT3


volatile int buffer[100] = {0};
volatile int n = 0;
volatile int adc_read[100];
volatile int i = 0;
float vrms =0, vavg=0;
int count = 0;

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

	//configuracao das portas


    P2DIR |= BIT4;                          //P2.4 sendo selecionado para saida
    P2SEL |= BIT4;                          //P2.4 sendo selecionado para funcao PWM

    P1REN |= BIT5 | BIT4 | BIT3;           //Resistores internos em modo pull-up
    P1OUT |= BIT5 | BIT4 | BIT3;
    P1IE  |= phaseb | sw;                   // interrupt enable dos pinos
    P1IES |= phaseb | sw;
    P1IFG = 0x00;


//-----------Configuracoes do SMCLK -----------------//
	// Selecionar tap 25 da faixa 4 p/ f = 25.075 MHz
	UCSCTL0 = 0x1900;
	UCSCTL1 = DCORSEL_4;
	UCSCTL4 = 0x03;

//----------Configuracao do ACLK---------------------//


	    UCSCTL4 |= SELA_1;                      //VLO sendo fonte do ACLK

//-----------Configuracao do timer para controle do pwm via encoder----//
	    TA2CCTL1 = OUTMOD_7;                    //Timer A2 configurado no modo reset/set
	    TA2CCR0 = 167;
	    TA2CCR1 = count;                        //TACCR2 controlado pelo encoder
	    TA2CTL |= TASSEL_1 | MC_1 | TACLR;


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
	TA0CCTL1 = OUTMOD_7;

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

	// Habilita as interrupcoes
	ADC12IE = ADC12IE0;


	__enable_interrupt();

	while(1) {
	    ADC12CTL0 |= ADC12SC;
        if(i==98){
            vrms = rms_cal(adc_read);
            vavg = avg_cal(adc_read);
        }
        else{
            LPM0;
        }
	};
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_A_ISR(void) {
    buffer[n] = ADC12MEM0;
    if(n == 99) ADC12IE &= ~ADC12IE0;
    else n++;
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
        count = 0x00;
        break;
    case 0x0A:
        P1IFG &= ~0x0A;
            if((P1IN & phasea)){            //phase a e b sao utilizados para a verificacao
                count++;                    //da diferenca de fase.
            }
            else{
                count--;
            }
        break;
    case 0x0C:
    case 0x0E:
    case 0x10:
    default:
        break;
    }
    if(count >= 167){                       // funcao para os valores de pico max e minimo, sem VREF.
        count = 167;
    }
    if(count <= 0){
        count =0;

   }
    TA2CCR1 = count;                        //TACCR é atualizado com o valor obtido do encoder.
}
