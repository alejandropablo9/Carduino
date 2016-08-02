#include <EEPROM.h>
#include <UTouch.h>
#include <UTFT.h>

//display 400x250
UTFT lcd(ILI9327,38,39,40,41);
UTouch touch( 6, 5, 4, 3, 2);

//Fuentes del display
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];

int pulsePin = A0;                 // Pulse Sensor purple wire connected to analog pin 0

// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, must be seeded! 
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

//
const int pinTrans = 8;
const int DATOS = 7;

int band = 0;
int displayScreen;
int x = 0;
int y = 0;
int count = 0;
int bpm1 = 0;
int bpm2 = 0; 

void homeScreenDraw();
void interruptSetup();
void drawFrame(int x1, int y1, int x2, int y2);
void drawScreenPulso();
void drawScreenPulso();
void getPpm();
void guardarPulso();
void pulsarOtravez();

void setup(){
  //Inicializamos el display TFT 
  lcd.InitLCD();
  lcd.clrScr();
  touch.InitTouch();
  touch.setPrecision(PREC_HI);
  pinMode(pinTrans, OUTPUT);
  displayScreen = 0;
  homeScreenDraw();
  Serial.begin(115200);             // we agree to talk fast!
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS  
}

void loop(){
  //PANTALLA PRINCIPAL
  if(displayScreen == 0){
    if(touch.dataAvailable()){
      touch.read(); 
      x = touch.getX();
      y = touch.getY();
      Serial.print(x);
      Serial.print(", ");
      Serial.println(y);
      if((x >= 180) && (x <= 235) && (y >= 60) && (y <= 180)){
        drawFrame(99, 140, 299, 180);
        displayScreen = 1;
        lcd.clrScr();
        drawScreenHistorial();
      }
     if((x >= 250) && (x <= 300) && (y >= 60) && (y <= 180)){
        drawFrame(99, 190, 299, 230);
        displayScreen = 2;
        lcd.clrScr();
        drawScreenPulso();
        digitalWrite(pinTrans, HIGH);
        BPM = bpm1 = bpm2 = count = 0;
      }
    }
  }

  //PANTALLA HISTORIAL
  if(displayScreen == 1){
    if(touch.dataAvailable()){
      touch.read(); 
      x = touch.getX();
      y = touch.getY();

      Serial.print(x);
      Serial.print(", ");     
      Serial.println(y);
      
      if((x >= 1) && (x <= 35) && (y >= 5) && (y <= 35)){
        lcd.clrScr();
        homeScreenDraw();
        displayScreen  = 0;
        digitalWrite(pinTrans, LOW);   
      }
      if((x >= 270) && (x <= 300) && (y >= 150) && (y <= 210)){
        drawFrame(250, 205, 355, 230);
        limpiarEEPROM();
        mensaje("Borrado");
      }
    }
  }
  
  //PANTALLA MEDIR PULSO
  if(displayScreen == 2){
    getPpm();
    if(touch.dataAvailable()){
      touch.read();  
      x = touch.getX();
      y = touch.getY();
      Serial.print(x);
      Serial.print(", ");     
      Serial.println(y);
      if((x >= 1) && (x <= 35) && (y >= 5) && (y <= 35)){
        lcd.clrScr();
        homeScreenDraw();
        displayScreen  = 0;
        digitalWrite(pinTrans, LOW);   
      }
      //boton guardar
      if((x >= 130) && (x <= 170) && (y >= 130) && (y <= 210)){
        drawFrame(220, 100, 350, 130);
        guardarPulso();
      }
      //boton otra vez
      if((x >= 180) && (x <= 220) && (y >= 130) && (y <= 210)){        
        drawFrame(220, 140, 350, 170);
        pulsarOtravez();
      }
    }    
  } 
}

void homeScreenDraw(){
  //color de fondo
  lcd.setBackColor(0, 0, 0);
  lcd.setColor(255, 255, 255); // Sets color to white
  lcd.setFont(BigFont); // Sets font to big
  lcd.print("Carduino", CENTER, 10); // Prints the string on the screen
  lcd.setColor(255, 0, 0); // Sets color to red
  lcd.drawLine(0,32,399,32); // Draws the red line
  lcd.setColor(255, 255, 255); // Sets color to white
  lcd.setFont(SmallFont); // Sets the font to small
  lcd.print("Alesco, inc", CENTER, 41); // Prints the string
  lcd.setFont(BigFont);
  lcd.print("Selecciona una accion", CENTER, 64);

  //Boton Historial
  lcd.setColor(253, 17, 17);
  lcd.fillRoundRect(99, 140, 299, 180);
  lcd.setColor(255, 255, 255);
  lcd.drawRoundRect(99, 140, 299, 180);
  lcd.setFont(BigFont);
  lcd.setBackColor(253, 17, 17);
  lcd.print("Historial", CENTER, 152);
  
  //Boton medir pulso
  lcd.setColor(253, 17, 17);
  lcd.fillRoundRect(99, 190, 299, 230);
  lcd.setColor(255, 255, 255);
  lcd.drawRoundRect(99, 190, 299, 230);
  lcd.setFont(BigFont);
  lcd.setBackColor(253, 17, 17);
  lcd.print("Medir Pulso", CENTER, 202);

}

void drawFrame(int x1, int y1, int x2, int y2) {
  lcd.setColor(253, 17, 17);
  lcd.drawRoundRect (x1, y1, x2, y2);
  while (touch.dataAvailable())
    touch.read();
    lcd.setColor(255, 255, 255);
    lcd.drawRoundRect (x1, y1, x2, y2);
}

void botonBackMenu(){
  lcd.setColor(100, 155, 203);
  lcd.fillRoundRect (10, 10, 60, 36);
  lcd.setColor(255, 255, 255);
  lcd.drawRoundRect (10, 10, 60, 36);
  lcd.setFont(BigFont);
  lcd.setBackColor(100, 155, 203);
  lcd.print("<-", 18, 15);
  lcd.setBackColor(0, 0, 0);
  lcd.setFont(SmallFont); 
  lcd.print("Regresar al menu", 70, 26);
  lcd.drawLine(0,50,399,50);
  lcd.setBackColor(0, 0, 0);
}
void drawScreenHistorial(){
  botonBackMenu();
  
  lcd.setColor(255, 255, 255);
  lcd.drawLine(0,50,399,50);
  lcd.drawLine(0, 200, 399, 200);

  lcd.setFont(SmallFont);
  lcd.print("30ppm", 0, 185);
  lcd.print("100ppm", 0, 120);
  lcd.print("170ppm", 0, 55);

  lcd.setColor(255, 0, 0);
  int x1 = 50; 
  
  for(int i = 1; i <= DATOS; i++){
    if(EEPROM[i] != 0){
      lcd.drawLine(x1, 190 - (EEPROM[i-1] - 30), x1 + 50, 190 - (EEPROM[i] - 30));
    }
    x1+=50;
  }

  lcd.setColor(255, 17, 17);
  lcd.fillRoundRect(250, 205, 355, 230);
  lcd.setColor(255, 255, 255);
  lcd.drawRoundRect(250, 205, 355, 230);
  lcd.setFont(BigFont);
  lcd.setBackColor(255, 17, 17);
  lcd.print("Borrar", 255, 210);
}

void drawScreenPulso(){
  botonBackMenu();

  //boton guardar
  lcd.setColor(17, 253, 17);
  lcd.fillRoundRect(220, 100, 350, 130);
  lcd.setColor(255, 255, 255);
  lcd.drawRoundRect(220, 100, 350, 130);
  lcd.setFont(BigFont);
  lcd.setBackColor(17, 253, 17);
  lcd.print("guardar", 230, 105);

  //boton otra vez 
  lcd.setColor(17, 253, 17);
  lcd.fillRoundRect(220, 140, 350, 170);
  lcd.setColor(255, 255, 255);
  lcd.drawRoundRect(220, 140, 350, 170);
  lcd.setFont(BigFont);
  lcd.setBackColor(17, 253, 17);
  lcd.print("otravez", 230, 145); 

  lcd.setColor(255, 255, 255);
  lcd.drawLine(0, 200, 399, 200);
} 

void getPpm(){
  bpm2 = bpm1;
  bpm1 = BPM; 
  if(QS == true){
    lcd.setColor(0, 0, 0);
    lcd.fillRect(0, 210, 350, 230);    
    if(bpm2 == BPM){
      count++;
      if(count == 2){
        digitalWrite(pinTrans, LOW);  
      }
    }
    lcd.setFont(SevenSegNumFont);
    lcd.setColor(0, 255, 0);
    lcd.printNumI(BPM, 20, 100, 3, '0');  
    lcd.setFont(BigFont);
    lcd.print("ppm", 120, 130);   
    QS = false;
  }else{        
     if(BPM == 0){   
        lcd.setBackColor(0, 0, 0);
        lcd.setColor(0, 255, 0);
        lcd.setFont(SevenSegNumFont);
        lcd.printNumI(BPM, 20, 100, 3, '0');
        lcd.setFont(BigFont);
        lcd.print("ppm", 120, 130);

        lcd.setColor(255, 255, 255);  
        lcd.setFont(SmallFont);
        lcd.setBackColor(0, 0, 0); 
        lcd.print("Coloque el dedo en el sensor...", LEFT, 210); 
        delay(2000);
     } 
      if(!digitalRead(pinTrans)){  
        lcd.setBackColor(0, 0, 0);
        lcd.setColor(0, 0, 255);
        lcd.setFont(SevenSegNumFont);
        lcd.printNumI(bpm1, 20, 100, 3, '0');
        lcd.setFont(BigFont);
        lcd.print("ppm", 120, 130);
     }
  }
}

void limpiarEEPROM(){
  for(int i = 1; i <= DATOS; i++){
        EEPROM.write(i, 0);
  }
}

void guardarPulso(){
  if((BPM != 0) && (digitalRead(pinTrans) == LOW)){
    int i;    
    for(i = 1; i <= DATOS; i++){
      if(EEPROM.read(i) == 0){
        break;
      }
    }
    if(i > DATOS){
      for(i = 1; i <= DATOS; i++){
        EEPROM.write(i, 0);
      }
      i = 1;
    }   
    EEPROM.write(i, BPM);
  }
}

void pulsarOtravez(){
  if(count == 2 && !digitalRead(pinTrans)){
    BPM = bpm1 = bpm2 = count = 0;
    digitalWrite(pinTrans, HIGH);
  }
}

void mensaje(String m){
  lcd.setBackColor(0, 0, 0);
  lcd.setColor(255, 255, 255);
  lcd.setFont(SmallFont);
  lcd.print(m, LEFT, 210);
  delay(1000);
  lcd.setColor(0, 0, 0);
  lcd.fillRoundRect(0, 205, 200, 290);
}
          
 
