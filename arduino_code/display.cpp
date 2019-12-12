#include <stdlib.h>
#include <SPI.h>
#include <UTFTGLUE.h>              //use GLUE class and constructor
UTFTGLUE myGLCD(0,A2,A1,A3,A4,A0); //all dummy args

#define PLOTPOSX1  0
#define PLOTPOSY1  14
#define PLOTPOSX2  319 // Internal size of the rectangular area (with borders)
#define PLOTPOSY2  225 // Internal size of the rectangular area (with borders)
#define TDIVPOSX   173
#define TDIVPOSY   2
#define VDIVPOSX   243
#define VDIVPOSY   2
#define LENGTHX    (PLOTPOSX2-PLOTPOSX1-3) / 2
#define LENGTHY    (PLOTPOSY2-PLOTPOSY1-3) / 2
#define SPACING    24 // Space between each dot-1
#define GETMIDDLE(a)  (a & 0x01) ? ((a-1)/2) + 1 : a / 2

const int center[2] = {GETMIDDLE(PLOTPOSX1 + PLOTPOSX2), GETMIDDLE(PLOTPOSY1 + PLOTPOSY2)};

#define gety(y)   (center[1] - y)
#define getx(x)   (center[0] - x)

unsigned char dotsy[240] = {0};
unsigned char dotsx[320] = {0};
char temp[4];

typedef enum _datamode {
  data,
  time,
  volts
} datamode;

unsigned char dado, index;
datamode modo_dado = data;

typedef union _entrada {
  unsigned int val;
  unsigned char b[4];
} entrada;

entrada valorTempoDiv_prev;
entrada valorTempoDiv;
entrada valorVoltsDiv_prev;
entrada valorVoltsDiv;

void atualizaVoltsDiv(int val);
void atualizaTempoDiv(int val);

void setup()
{
  Serial.begin(9600);
  pinMode(SS, INPUT_PULLUP);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, INPUT);
  SPCR |= _BV(SPE);
  SPI.attachInterrupt();  //allows SPI interrupt

  randomSeed(analogRead(0));
  Serial.print("center: ");
  Serial.print(center[0]);
  Serial.print(center[1]);
  Serial.println();

  // Setup the LCD
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);

 

// Clear the screen and draw the frame
  myGLCD.clrScr();

  myGLCD.setColor(255, 0, 0); // Retangulo vermelho
  myGLCD.fillRect(0, 0, 319, PLOTPOSY1-1); // Preenche retangulo de cima
  delay(500);
  myGLCD.setColor(64, 64, 64); // Retangulo cinza
  myGLCD.fillRect(0, PLOTPOSY2+1, PLOTPOSX2+1, PLOTPOSY2+14); // Preenche retangulo de baixo
  myGLCD.setColor(255, 255, 255); // Letra branca
  myGLCD.setBackColor(255, 0, 0); // Fundo vermelho
  myGLCD.print("Osciloscopio - MSP430F5529", LEFT, 2);
  atualizaTempoDiv(0);
  atualizaVoltsDiv(0);
  myGLCD.setBackColor(64, 64, 64); // Fundo cinza
  myGLCD.setColor(255,255,0); // Letra amarela

/*   myGLCD.print("f=??.??", 3, 227);
  myGLCD.print("Avg=??.??", 73, 227);
  myGLCD.print("Max=??.??", 143, 227);
  myGLCD.print("Min=??.??", 213, 227);
 */
  myGLCD.setColor(0, 0, 255); // Azul
  myGLCD.drawRect(PLOTPOSX1, PLOTPOSY1, PLOTPOSX2, PLOTPOSY2); // Retangulo azul envolvendo o plot

  // Draw crosshairs
  myGLCD.setColor(0, 0, 255); //Azul
  myGLCD.setBackColor(0, 0, 0); // Cor de fundo preta

  for(int i=PLOTPOSX1 + ((PLOTPOSX2 - PLOTPOSX1 + 1) % (SPACING+1))/2; i<=PLOTPOSX2; i+=SPACING+1) {
    dotsx[i] = 1;
    for(int j=PLOTPOSY1 + ((PLOTPOSY2 - PLOTPOSY1 + 1) % (SPACING+1))/2; j<PLOTPOSY2; j+=SPACING+1) {
      if(i == center[0]) {
        myGLCD.drawLine(center[0]-1, j, center[0]+1, j);
        dotsx[center[0]-1] = 1;
        dotsx[center[0]] = 1;
        dotsx[center[0]+1] = 1;
      }
      else if(j == center[1]) {
        myGLCD.drawLine(i, center[1]-1, i, center[1]+1);
        dotsy[center[1]-1] = 1;
        dotsy[center[1]] = 1;
        dotsy[center[1]+1] = 1;
      }
      myGLCD.drawPixel(i, j);
      dotsy[j] = 1;
    }
  }

  myGLCD.setColor(255,255,255);
  myGLCD.drawPixel(center[0], center[1]);
  myGLCD.setColor(0,0,255);
}

unsigned char bufentrada[PLOTPOSX2+1] = {0};
unsigned char buftela[PLOTPOSX2+1] = {0};
int x = 1, i = 0, j;
bool ok=false;

void loop()
{
  while(ok) {
    for(i=1;i<PLOTPOSX2;i++) {
      // Checar se mudaram as divisoes de tempo para mostrar na tela
      if((valorTempoDiv.val != valorTempoDiv_prev.val) && (modo_dado == time)) {
        atualizaTempoDiv(valorTempoDiv.val);
        valorTempoDiv_prev = valorTempoDiv;
      }
      if((valorVoltsDiv.val != valorVoltsDiv_prev.val) && (modo_dado == volts)) {
        atualizaVoltsDiv(valorVoltsDiv.val);
        valorVoltsDiv_prev = valorVoltsDiv;
      }

      if(i + 1 < PLOTPOSX2)
        j = i + 1;
      else
        j = 1;

      if ((dotsx[j] & dotsy[buftela[j]]))
        myGLCD.setColor(0,0,255); // restaura o ponto
      else
        myGLCD.setColor(0,0,0); // restaura o fundo preto
      myGLCD.drawPixel(j,buftela[j]); // efetivamente escreve o pixel

      myGLCD.setColor(0,255,255);
      myGLCD.drawPixel(i,bufentrada[i]);
      buftela[i] = bufentrada[i];
    }
    ok=false;
  }
}

ISR (SPI_STC_vect) {
  dado = SPDR; // Receber o dado por SPI

  switch(modo_dado) { // Se estamos esperando terminar um valor inteiro ou se eh so um ponto para plotar
    case data: // Se estivermos no modo dado, podemos a qualquer momento receber um valor de tempodiv ou voltsdiv
    switch(dado) {
      case 0xFF: // 0xFF eh o comando para tempodiv
        modo_dado = time; // Mudamos para o modo tempodiv
        break;
      case 0xFE: // 0xFE eh o comando para voltsdiv
        modo_dado = volts; // Mudamos para o modo voltsdiv
        break;
      default: // Se nao for nenhum comando, entao eh so um ponto
        if(!ok) {
          if(x == PLOTPOSX2){
            x = 1;
            ok = true;
          }
          else {
            x++;
            bufentrada[x] = 2*center[1]-SPDR-PLOTPOSY1;
            if(bufentrada[x] == 1) bufentrada[x]++;
          }
        }
    }
    break;

  case time: // Se estamos no modo tempo, devemos esperar o inteiro ser enviado
      if(index <= 3) { // Transferimos byte a byte o inteiro a ser lido
        valorTempoDiv.b[index] = dado;
        if(index == 3) { // Se ja chegamos no fim
          index = 0; // Zeramos o indice para a proxima vez
          modo_dado = data; // Se ja tivermos recebido o inteiro, mudamos para modo de dados de volta
        }
        else index++;
      }
  break;

  case volts:
      if(index <= 3) { // Transferimos byte a byte o inteiro a ser lido
        valorVoltsDiv.b[index] = dado;
        if(index == 3) { // Se ja chegamos no fim
          index = 0; // Zeramos o indice para a proxima vez
          modo_dado = data; // Se ja tivermos recebido o inteiro, mudamos para modo de dados de volta
        }
        else index++;
      }
  break;
 
  default: break;
  }
}

void atualizaTempoDiv(int val) {
  char store[50] = "ms=";
  myGLCD.setBackColor(255, 0, 0);
  myGLCD.setColor(255, 0, 0);
  myGLCD.fillRect(TDIVPOSX+7, TDIVPOSY, VDIVPOSX-1, PLOTPOSY1-1);
  myGLCD.setColor(0,255,0); // Letra verde
  itoa(val, temp, 10);
  strcat(store, temp);
  myGLCD.print(store, TDIVPOSX, TDIVPOSY);
  myGLCD.setColor(0,255,255);
}

void atualizaVoltsDiv(int val) {
  char store[50] = "mV=";
  myGLCD.setBackColor(255, 0, 0);
  myGLCD.setColor(255, 0, 0);
  myGLCD.fillRect(VDIVPOSX+14, VDIVPOSY, VDIVPOSX+20, PLOTPOSY1-1);
  myGLCD.setColor(0,255,0); // Letra verde
  itoa(val, temp, 10);
  strcat(store, temp);
  myGLCD.print(store, VDIVPOSX, VDIVPOSY);
  myGLCD.setColor(0,255,255);
}