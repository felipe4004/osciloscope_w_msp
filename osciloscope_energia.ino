#include <math.h>
#define N 10
double periodo[N] = {0};
char b = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(P6_0, INPUT);
  Serial.begin(9600);
  Serial1.begin(9600);
  double t=2*PI/N;
  for(int i=1; i<N; i++) {
    periodo[i] = map(sin(t*i)*N, -N, N, 10, 190);
  }
}
volatile int n = 0;
void loop() {
  // put your main code here, to run repeatedly: 
  String val = "";
  if(Serial.available() > 0) {
  while((Serial.available() > 0) && (b != '\n')) {
      b = Serial.read();
      val += String(b);
  }
  
  if(val != "") {
  Serial.println(val);
    delay(500);
    sendCommand(val);
    val = "";
  }
  } else {
  n++;
  val = "add ";
  val += 1;
  val += ",";
  val += 0;
  val += ",";
  val += map(3(A0), 0, 4095, 0, 255);
  //val += (int)periodo[n];
  delay(10);
  Serial.print(n);
  Serial.print("\t[DEBUG] val = ");
  Serial.println(val);
  sendCommand(val);
  
  if(n == N-1) n = 0;
  }
}

void sendCommand(String cmd) {
  Serial1.print(cmd);
  for(int i=0; i<3; i++) Serial1.write(0xff);
}
