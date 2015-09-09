const byte Mon_Pin(A10);
const double steinhart_A(1.1254e-3);
const double steinhart_B(2.3469e-4);
const double steinhart_C(0.8584e-7);


void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(9600);
  
  pinMode(Mon_Pin,INPUT);
  analogReference(EXTERNAL);
  analogReadResolution(12);
  analogReadAveraging(32);

}

void loop() {
  // put your main code here, to run repeatedly:
  int val = analogRead(Mon_Pin);
  Serial.println(val);
  delay(1000);
  

}
