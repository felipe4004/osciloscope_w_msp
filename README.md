# osciloscope_w_msp

## Objetivo
Esse projeto tem o objetivo de fornecer um oscilosc�pio port�til com uma m�dia frequ�ncia de
amostragem, utilizando um MCU da fam�lia MSP430, de f�cil modifica��o. Tendo foco na simplicidade,
ser� necess�rio somente uma ponta de prova de resist�ncia conhecida e uma tela TFT com driver ILI9341,
em seu uso mais b�sico. Para um melhor ajuste, pode-se usar rotary encoders para definir o trigger,
segundos/div e volts/div.

# Features
 -- Taxa de amostragem at� ~180 kSa/s.
 -- Frequ�ncia m�xima de entrada 400 kHz.
 -- Ajuste fino e grosso por velocidade de movimento dos encoders.
 -- Ajuste de trigger.
 -- Tens�o m�xima de entrada de 10 volts.
 -- Display LCD TFT.
 -- Caracteriza��o do sinal de entrada: frequ�ncia, valor m�dio, valor pico-a-pico e amplitude.

# Blocos Funcionais 
O oscilosc�pio contar� com os seguintes blocos:
 1) Bloco de amostragem
 	- Respons�vel por captar o sinal de entrada pelo perif�rico ADC12. Ter� um buffer
 	de entrada para proteger o MCU.
 2) Bloco de corre��o
	- Utilizando multiplica��o em hardware, ser� feita a corre��o das n�o-linearidades
	  caracter�sticas da curva do ADC, por polin�mio interpolador de terceira ordem.
 3) Bloco de ajuste
    - Por meio de rotary encoders, ser� feito o ajuste das escalas de tempo e escala
    vertical de tens�o. Ser�o sinais digitais em entrada nos pinos de GPIO.
    - Haver� tamb�m o ajuste de trigger, para melhor visualiza��o do sinal, por meio de
    um potenci�metro.
 4) Bloco de visualiza��o
    - O sinal amostrado ser� exibido em um display LCD TFT 2.8" utilizando o driver
    ILI9341 em barramento paralelo de 8-bits (modo 8080-I).
 5) Bloco de processamento
 	- O pr�prio processamento da MCU. Ser� respons�vel pelo controle do display e pelo
 	gerenciamento dos perif�ricos.
