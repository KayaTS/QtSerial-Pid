//*****************************************************

//** LCD ekran için kütüphane ve ayarlamalar.

//*****************************************************

//** Motor ayarlamaları
#define motorPin 6
#define motorIn1 7
#define motorIn2 8
#define sag 0
#define sol 1
int motorHiz = 0;
bool motorYon = 0;
//*****************************************************

//** Sensor ayarlamaları
#define sensor 2
unsigned long start = 0;
unsigned long bitir = 0;
int stepYeni = 0;
float stepEski = 0;
float temp = 0;
double realRPM = 60;
//*****************************************************

//** Pid ayarlamaları
int pwm = 0;
int output = 0;
int p=0,d=0, i=0;
char parametre;
int deger;
double yeniHata = 0; // hedefRPM - realRPM 
double sonHata = 0; //
double toplamHata = 0; 
double hedefRPM = 0;
//*****************************************************
//** Moving average filtresi 
int ix = 0;
int filtreToplam = 0;
int veri[5];
int average = 0;

void setup() {
  Serial.begin(9600);
  pinMode(sensor,INPUT_PULLUP);
  pinMode(motorPin, OUTPUT);
  pinMode(motorIn1, OUTPUT);
  digitalWrite(motorIn1, HIGH); // sağ sol bilgisi gelmeden önce varsayılan olarak motor sağ yöne döner
  pinMode(motorIn2, OUTPUT);
  digitalWrite(motorIn2, LOW);
  attachInterrupt(digitalPinToInterrupt(sensor), counter, RISING);
  analogWrite(motorPin, 255);
}

void loop() {
    //Sensörden 1 saniyelik veri okuma
    start = millis();
    bitir = start + 100;/*
    while( millis() < bitir){
      if(digitalRead(sensor)){
        stepYeni = stepYeni + 1; 
        while(digitalRead(sensor));
      }
    }*/
    if( millis() < bitir){
      temp=stepYeni - stepEski;
      stepEski = stepYeni;
      realRPM = ( temp / 30 ) * 600;
      
    filtreToplam = filtreToplam - veri[ix];
    veri[ix] = realRPM;
    filtreToplam = filtreToplam + realRPM;
    if(ix == 4)
      ix = 0; 
    else 
      ix++; 
    average = filtreToplam / 5;
    Serial.print(realRPM);
    Serial.print(",");
    Serial.flush();
    }
    if ( Serial.available() > 0 ){
      parametre = Serial.read();
      deger = Serial.parseInt();
      set_kontrolor(parametre, deger);
      set_pid(p, i, d, realRPM, hedefRPM);
    }
    
}
void counter(){
   stepYeni = stepYeni + 1;
  }
void set_pid(int p, int i, int d, double realRPM, double hedefRPM){
  //PID program

 // if (hedefRPM > 0){ //eger motor hizi artirilirsa
    yeniHata = hedefRPM - realRPM;
    output = yeniHata*p + toplamHata*i + (yeniHata - sonHata)*d;
    /*
    Arduino 0 ile 255 arasında bir değer ile pwm çıkışı verebiliyor. Benim sistemimde 12 volt güç olmasına rağmen
    5v dc motor kullandığım için maksimum olarak 6v çıkış vereceğim. Arduino pwm biti başına 0,047 volt verir. 6 v için
    maksimum 6/0,047=127 bit pwm vermek gerekir.
    -----
    output değeri 700'ü geçse bile pwm karşılığı 127 olacaktır. Bu yüzden outputun değerini 0 ile 127 değeri arasında map ederek
    pwm değerini hesaplarız.
    */
    if(output > 700) 
      output = 700;
    pwm = map(output, 0, 700, 0, 127);
    toplamHata = toplamHata + yeniHata; // İntegrator için toplam bulunur
    sonHata = yeniHata;  // Derivative için elimizdeki hata bir sonraki adımda eski hata olacak
    //clamping
    if (toplamHata >4000) toplamHata = 4000;
    if (toplamHata <-4000) toplamHata = -4000;
  /*}
  else{
    yeniHata = 0;
    sonHata = 0;
    toplamHata = 0;
    pwm = 0;
  }*/
  
  analogWrite(motorPin, pwm); // Motora map ettiğimiz pwm değerini göndeririz. Map işlemi sayesinde pwm 127den fazla olamaz ve sistemin satürasyonu engellenmiş olur.
  
  }
  
void set_kontrolor(char parametre, int deger){
  
  /*
  if (parametre == 'p'){
      p = deger;
      Serial.print(deger);
      lcd.print(deger);
      return;
    }
  if (parametre == 'd'){
      d = deger;
      Serial.print(deger);
      lcd.print(deger);
      return;
    }
  if (parametre == 'i'){
      i = deger;
      Serial.print(deger);
      lcd.print(deger);
      return;
    }
  */
    
  switch(parametre){
    case 'p': 
      p = deger;
    case 'i':
      i = deger;
    case 'd':
      d = deger;
    case 'm':
      hedefRPM = deger;
    case 'y':
      if(deger == 0){
        motorYon = sag;
        digitalWrite(motorIn1, HIGH); //  motor sağ yöne döner;
        digitalWrite(motorIn2, LOW);      
      }else{
        motorYon = sol;
        digitalWrite(motorIn1, LOW); //  motor sol yöne döner;
        digitalWrite(motorIn2, HIGH);     
      }
     }
    /*
  switch(parametre){
    case 'p': 
      p = deger;
      lcd.setCursor(0,0);
      lcd.print("KP");
      lcd.setCursor(0,1);
      lcd.print(deger);
      break;
    case 'i':
      i = deger;
      lcd.setCursor(3,0);
      lcd.print("KI");
      lcd.setCursor(3,1);
      lcd.print(deger);
      break;
    case 'd':
      d = deger;
      lcd.setCursor(6,0);
      lcd.print("KD");
      lcd.setCursor(6,1);
      lcd.print(deger);
      break;
    case 'm':
      hedefRPM = deger;
      lcd.setCursor(9,0);
      lcd.print("hiz");
      lcd.setCursor(9,1);
      lcd.print(deger);
      break;
    case 'y':
      lcd.setCursor(13,0);
      lcd.print("yon");
      lcd.setCursor(13,1);
      if(deger == 0){
        motorYon = sag;
        lcd.print("sag");
        digitalWrite(motorIn1, HIGH); //  motor sağ yöne döner;
        digitalWrite(motorIn2, LOW);      
      }else{
        motorYon = sol;
        lcd.print("sol");
        digitalWrite(motorIn1, LOW); //  motor sol yöne döner;
        digitalWrite(motorIn2, HIGH);     
      }
      break;
    default:
     lcd.setCursor(0,0);
     lcd.print("Hata");
    }*/
  }
