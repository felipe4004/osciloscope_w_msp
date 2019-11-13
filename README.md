# osciloscope_w_msp

## Objetivo
Esse projeto tem o objetivo de fornecer um osciloscópio portátil com uma média frequência de
amostragem, utilizando um MCU da família MSP430, de fácil modificação. Tendo foco na simplicidade,
será necessário somente uma ponta de prova de resistência conhecida e uma tela TFT com driver ILI9341,
em seu uso mais básico. Para um melhor ajuste, pode-se usar rotary encoders para definir o trigger,
segundos/div e volts/div.

# Features
 -- Taxa de amostragem até ~180 kSa/s.
 -- Frequência máxima de entrada 400 kHz.
 -- Ajuste fino e grosso por velocidade de movimento dos encoders.
 -- Ajuste de trigger.
 -- Tensão máxima de entrada de 10 volts.
 -- Display LCD TFT.
 -- Caracterização do sinal de entrada: frequência, valor médio, valor pico-a-pico e amplitude.

# Blocos Funcionais 
O osciloscópio contará com os seguintes blocos:
 1) Bloco de amostragem
 	- Responsável por captar o sinal de entrada pelo periférico ADC12. Terá um buffer
 	de entrada para proteger o MCU.
 2) Bloco de correção
	- Utilizando multiplicação em hardware, será feita a correção das não-linearidades
	  características da curva do ADC, por polinômio interpolador de terceira ordem.
 3) Bloco de ajuste
    - Por meio de rotary encoders, será feito o ajuste das escalas de tempo e escala
    vertical de tensão. Serão sinais digitais em entrada nos pinos de GPIO.
    - Haverá também o ajuste de trigger, para melhor visualização do sinal, por meio de
    um potenciômetro.
 4) Bloco de visualização
    - O sinal amostrado será exibido em um display LCD TFT 2.8" utilizando o driver
    ILI9341 em barramento paralelo de 8-bits (modo 8080-I).
 5) Bloco de processamento
 	- O próprio processamento da MCU. Será responsável pelo controle do display e pelo
 	gerenciamento dos periféricos.
