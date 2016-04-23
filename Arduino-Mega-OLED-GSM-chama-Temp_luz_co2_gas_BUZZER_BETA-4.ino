
/* Project's name: IoT Dima's Project
 * Last update: 04-10-16
 *www.hectonpdomingos.com.br/iot.php
 *
 *IO ports and description
 
Temperature Sensor  LM35 - IO 8    -  5v
Light sensor LDR - IO 9 - precisa de um resistor 
Buzzer (sound) - IO 2
Relay - IO 6 and 7 (arduino Uno)
Fire Sensor - IO Digital =  7 -- IO Analogica -  A1
MQ2 Sensor -  IO 10
LGP, Smoke, Inflammable Gas SENSOR MQ7 - IO 40
OLED - SDA -> SDA20 | SDL > SDL21 | VCC ->IOREF (SEGUNDO PINO, AO LADO DO RESET ) | GND -> GND
*/


//Libraries
#include "U8glib.h"
#include "SIM900.h"
//Disabled call SIM 900 library
//#include <call.h>
#include <SoftwareSerial.h>
//Relay Robocore disabled
//#include <SerialRelay.h>
//Liquid Crystal disabled due the change to oled display
//#include <LiquidCrystal.h>
#define INTERNAL1V1 2


//### BUZZER de alerta
int buzzer = 2;
//Count  fire alarm 
int contador = 0;
//count  gas alarm
int contadorgas = 0;
//count co2 alarm
int contadorco2 = 0;
//count gsm erro
int contadorgsmerror = 0;

//###########  GSM sim900 ##################
#include "sms.h"
SMSGSM sms;

//Simple sketch to send and receive SMS.

int numdata;
boolean started=false;
char smsbuffer[160];
char n[20];

//debug begin
char sms_position;
char phone_number[20]; // array for the phone number string
char sms_text[100];
int i;
//debug end
//############ FIM GSM SIM900   ############



//RELAY Robocore
//QUANTIDADE DE MODULOS 1
//const byte NumModules = 1;
//SerialRelay relays(6,7,NumModules); // (data, clock, number of modules)
/*dESATIVAR E ATIVAR  - RIMEIRO NUMERO É REFERENTE AO MODULO E ULTIMO A PLACA
 *   relays.SetRelay(1, SERIAL_RELAY_ON, 1);   // turn the relay ON 
delay(1000);
 * 
 */

//Sensor de Temperatura -- IO A8
const int LM35 = A8;
float temperatura = 0;
int ADClido = 0;


//sensor de luz  - IO 9
const int LDR = 9;
int ValorLido = 0;
int ValorLido2 = 0;
int porcento = 0;

int sensorValue;
//###############################//
//SENSOR DE CHAMA - Usando uma porta digital 7 e outra analogica  A1

int pino_D0 = 53;
int pino_A0 = A7;
 
int valor_a = 0;
int valor_d = 0;

//fim sensor de chama
 
//porta 10 nao pode ser mudada pois é a PWM
//LiquidCrystal lcd(10, 30, 28, 26, 24, 22);


//######### CALCULO PARA DETECÇÃO DE GASES ################   
/************************Hardware Related Macros************************************/
#define MQ_PIN (A10) //define which analog input channel you are going to use
#define RL_VALUE (5) //define the load resistance on the board, in kilo ohms
#define RO_CLEAN_AIR_FACTOR (9.83) //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,
//which is derived from the chart in datasheet

/***********************Software Related Macros************************************/
#define CALIBARAION_SAMPLE_TIMES (50) //define how many samples you are going to take in the calibration phase
#define CALIBRATION_SAMPLE_INTERVAL (500) //define the time interal(in milisecond) between each samples in the
//cablibration phase
#define READ_SAMPLE_INTERVAL (50) //define how many samples you are going to take in normal operation
#define READ_SAMPLE_TIMES (5) //define the time interal(in milisecond) between each samples in
//normal operation

/**********************Application Related Macros**********************************/
#define GAS_LPG (0)
#define GAS_CO (1)
#define GAS_SMOKE (2)

/*****************************Globals***********************************************/
float LPGCurve[3] = {2.3,0.21,-0.47}; //two points are taken from the curve.
//with these two points, a line is formed which is "approximately equivalent"
//to the original curve.
//data format:{ x, y, slope}; point1: (lg200, 0.21), point2: (lg10000, -0.59)
float COCurve[3] = {2.3,0.72,-0.34}; //two points are taken from the curve.
//with these two points, a line is formed which is "approximately equivalent"
//to the original curve.
//data format:{ x, y, slope}; point1: (lg200, 0.72), point2: (lg10000, 0.15)
float SmokeCurve[3] ={2.3,0.53,-0.44}; //two points are taken from the curve.
//with these two points, a line is formed which is "approximately equivalent"
//to the original curve.
//data format:{ x, y, slope}; point1: (lg200, 0.53), point2: (lg10000, -0.22)
float Ro = 10; //Ro is initialized to 10 kilo ohms



//TELA OLED

U8GLIB_SSD1306_128X64_2X u8g(U8G_I2C_OPT_NONE);




//MAIN SCREEM
void draw() 
{
  
  //Comandos graficos para o display devem ser colocados aqui
  //Seleciona a fonte de texto
  u8g.setFont(u8g_font_8x13B);

  //Linha superior - temperatura 
  u8g.setPrintPos(5, 15) ;
  //Mostra o valor da variavel Temperatura na posicao especificada
  u8g.print(temperatura);
  
  u8g.drawCircle(28,8,3);
  u8g.drawStr( 45, 15, "C");
  u8g.drawStr( 70, 15, "CO");
  u8g.drawStr( 90, 15, "2");
  u8g.setPrintPos(77, 15) ;
  //Mostra o valor da variavel Temperatura na posicao especificada
  //u8g.print(mq7); 
  //Hora
  //u8g.setFont(u8g_font_fub30);

 
  u8g.setFont(u8g_font_5x7);
  u8g.drawStr(5, 30, "Sensor ");
 /* if (temperatura == 2){
    u8g.drawStr(5, 40, "Temp ");
  }
  if (temperatura < 2){
    u8g.drawStr(5, 40, "Temp ");
  }
  if (temperatura > 2){
    u8g.drawStr(5, 40, "Temp ");}
 */ 
  //if (valor_d != 1){    
    u8g.drawStr(90, 30, "Alertas ");    
    u8g.drawStr(5, 40, " FIRE");
    u8g.setPrintPos(90, 40) ;
    u8g.print(contador);
     u8g.drawStr(5, 50, " GAS-Smoke-LGP");
    u8g.setPrintPos(90, 50) ;
    u8g.print(contadorgas);
    //u8g.drawStr(5, 60, " Sensor de Co2");
    //u8g.setPrintPos(90, 60) ;
   // u8g.print(contadorco2);
    u8g.drawStr(5, 60, " GSM Module");
    u8g.setPrintPos(90, 60) ;
    u8g.print(contadorgsmerror);
 // }
  
 // u8g.drawStr(5, , "Dima, está tudo bem :) ");
  //u8g.drawStr(5, 50, "perigo encontrado. :)");
  //Texto - AM
  //u8g.setFont(u8g_font_5x7);
  //u8g.drawStr( 115, 33, "AM");
  //moldura relogio
  u8g.drawRFrame(0,18, 128, 46, 4);
  //Desenho bateria
  //u8g.drawRFrame(105, 3, 20,12 , 0);
  //u8g.drawBox(125, 6, 2,6);
  //u8g.drawBox(107, 5, 4,8);
  //u8g.drawBox(114, 5, 4,8);
  //Desenho linhas sinal
 // u8g.drawVLine(99,0, 15);
 // u8g.drawVLine(98,0, 15);
 // u8g.drawVLine(96,4, 11);
 // u8g.drawVLine(95,4, 11);
 // u8g.drawVLine(93,8, 7);
 // u8g.drawVLine(92,8, 7);
 // u8g.drawVLine(90,12, 3);
 // u8g.drawVLine(89,12, 3);
}


//FIM TELA OLED
void setup() {

// CARRREGANDO MODULO GSM

//Serial connection.
    Serial.begin(9600);
    Serial.println("GSM Shield testing.");
    if (gsm.begin(9600)) 
    {
        Serial.println("\nstatus=READY");
        started=true;
    } 
    else 
        Serial.println("\nstatus=IDLE");
        contadorgsmerror = contadorgsmerror + 1;
         if(started) 
    {
      if (sms.SendSMS("+55339090984199539", "Arduino SMS"))
        Serial.println("\nSMS sent OK");

     }

// FIM CARREGAMENTO DO MODULO GSM
Serial.begin(9600);

Serial.println("inicialiando setup");
if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }

  
Serial.begin(9600);
//para usar no arduino UNO
//analogReference(INTERNAL);
analogReference(INTERNAL1V1);
//Se estiver usando Arduino Mega, use INTERNAL1V1
//se estiver usando Arduino Leonardo, remova esta linha pois o Leonardo não aceita
//este comando


//BUZZER
 Serial.println("INICIALIZADO MODULO DE SINAL SONORO");
pinMode(buzzer, OUTPUT);
delay(500);
pinMode(buzzer, LOW);
delay(500);
pinMode(buzzer, HIGH);
delay(500);
pinMode(buzzer, LOW);
delay(500);
pinMode(buzzer, HIGH);
delay(500);
pinMode(buzzer, LOW);
 
//sensor de chama
Serial.println("INICIALIZADO MODULO DE CHAMA");
pinMode(pino_A0, INPUT);
pinMode(pino_D0, INPUT);
//fim sensor de chama

//Gas sensor MQ-2

//lcd.begin(16, 2);
//lcd.setCursor(0,0);
Serial.println("calibrando mq2");
//lcd.setCursor(0,1);
//lcd.print("Calibrando MQ-2");
Ro = MQCalibration(MQ_PIN);
//lcd.begin(16, 2);
//lcd.setCursor(0,0);
//lcd.print("Concluido.");
//delay(1000);
//lcd.begin(16, 2);
//lcd.setCursor(0,0);
//lcd.print("Ro=");
//lcd.setCursor(6,0);
//lcd.print(Ro);
//delay(1000);
//lcd.begin(16, 2);
//lcd.setCursor(1,0);
//lcd.print(Ro);
//lcd.begin(16, 2);
//lcd.setCursor(0,0);
//lcd.print("kohm");
//lcd.setCursor(6,0);
//lcd.print("kohm");
//delay(1000);
// FIM 




Serial.println("Inicialiando loop");
}


void loop() {

   if(started) 
    {
        //Read if there are messages on SIM card and print them.
        sms_position=sms.IsSMSPresent(SMS_UNREAD);
        if (sms_position) 
        {
            // read new SMS
            Serial.print("SMS postion:");
            Serial.println(sms_position,DEC);
            sms.GetSMS(sms_position, phone_number, sms_text, 100);
            // now we have phone number string in phone_num
            Serial.println(phone_number);
            // and SMS text in sms_text
            Serial.println(sms_text);
        }   
        else
        {
            Serial.println("NO NEW SMS,WAITTING");
        }     
        delay(1000);
    }

//OLED
u8g.firstPage();  
  do
  {
    draw();
  } while( u8g.nextPage() );
   
delay(50);

Serial.println("Saindo do loop");
  
//sensor de chama

Serial.println("Sensor de Chama");
int valor_a = analogRead(pino_A0);
int valor_d = digitalRead(pino_D0);
 if (valor_d != 1)
 {

  //Send SMS in case detects FIRE
  sms.SendSMS("+55339090987615157", "Detectado FOGO!");
  Serial.println("Detectado FOGO!");
   Serial.println("SINAL SONORO");
  pinMode(buzzer, HIGH);
  contador = contador + 1;
  
 }
 else{
  contador = 0;
  Serial.println("NAO FOI DETECTADO FOGO");
  pinMode(buzzer, LOW);
  
 }

 
delay(500);

//FIM SENSOR DE CHAMA



  
  //CARREGANDO DADOS TEMP
Serial.println("INICIALIZADO MODULO DE TEMPERATURA");
ADClido = analogRead(LM35);
temperatura = ADClido * 0.1075268817;
Serial.println("Temperatura"); 
Serial.println(temperatura);
//u8g.drawStr(5, 40, "VALOR IGUAL A 2 ");


//CARREGANDO DADOS SOBRE LUZ E CONVERTE EM  %
 Serial.println("INICIALIZADO MODULO DE LUMINOSIDADE");
ValorLido = analogRead(LDR);
ValorLido2 = map(ValorLido, 0, 1000, 0, 1000);
porcento = (ValorLido2 * 9 )/ 100;
//lcd.begin(16, 2);
//lcd.setCursor(0,0);
//lcd.print("Luminosidade: ");
//lcd.setCursor(0,1);
//lcd.println(porcento);
//lcd.setCursor(5,1);
//lcd.print("%");
//delay(3000);

//lcd.clear();



//TEMPERATURA
//lcd.begin(16, 2);
//lcd.setCursor(0,0);
//lcd.print("Temperatura:  ");
//lcd.setCursor(0,1);
//lcd.print(temperatura); 
//lcd.setCursor(7,1);
//lcd.write(B11011111);
//lcd.print("C");
//delay(2000);
//lcd.clear();


//sensor de monoxico de carbono - MQ7
Serial.println("LEITURA DO SENSOR MQ7 - MONOXICO DE CARBONO");

int i;
for (i =0; i<3; i++) {
// mq7 is on IO 40
int mq7 = analogRead(40);   
//lcd.setCursor(0,0);
//Serial.print("Nivel de CO2:"); 
//lcd.setCursor(0,1);
Serial.println(mq7);
if(mq7 > 400)
{
  //SEND SMS IN CASE DETECT HIGH CO2 LEVEL
  sms.SendSMS("+55339090987615157", "Niveis altos de CO2!");
  Serial.println("Niveis altos de CO2!");
  contadorco2 = contadorco2 + 1;
  Serial.println("SINAL SONORO");
  pinMode(buzzer, HIGH);
  delay(2000);
  pinMode(buzzer, LOW);
}

}



//SENSOR MQ-2
//lcd.clear();

mq2();

Serial.println("Sensor MQ2 - Gas inflamável/fumaça/lgp");
  
}



//Sensor de gas
void mq2(){

Serial.println("INICIALIZADO MODULO DO SENSOR MQ2");
 int i;

 for (i=0; i <10;i++) {
//lcd.begin(16, 2);
//lcd.setCursor(0,0);
//lcd.print("LPG: CO: Fumaca");
//lcd.setCursor(0,1);
//lcd.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG) );
//envia sms
if(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG) > 0)
{
  
  Serial.println("Detectado gas de cozinha!");
  Serial.println("SINAL SONORO");
   pinMode(buzzer, OUTPUT);
  pinMode(buzzer, HIGH);
  sms.SendSMS("+55339090987615157", "Detectado gas de cozinha!");
  Serial.println("DESLIGANDO SINAL SONORO");
  contadorgas = contadorgas + 1;
  pinMode(buzzer, LOW);
}
//lcd.setCursor(5,1);
//lcd.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO) );
//envia SMS
if(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO) > 0)
{
  
    Serial.println("Detectado GAS inflamável!");
    Serial.println("SINAL SONORO");
    pinMode(buzzer, OUTPUT);
    pinMode(buzzer, HIGH);
    contadorgas = contadorgas + 1;
    sms.SendSMS("+55339090987615157", "Detectado GAS inflamável!");
    Serial.println("DESLIGANDO SINAL SONORO");
    pinMode(buzzer, LOW);
}
//lcd.setCursor(10,1);
//lcd.print(MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE) );
if (MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE) > 0)
{
  
   Serial.println("Detectado FUMAÇA!");
   Serial.println("SINAL SONORO");
   pinMode(buzzer, OUTPUT);
   pinMode(buzzer, HIGH);
   contadorgas = contadorgas + 1;
   sms.SendSMS("+55339090987615157", "Detectado FUMAÇA!");
   Serial.println("DESLIGANDO SINAL SONORO");
   pinMode(buzzer, LOW);
}
delay(500);
 }
 loop();
 
   Serial.println("Entrando no loop");
}




/****************** MQResistanceCalculation ****************************************
Input: raw_adc - raw value read from adc, which represents the voltage
Output: the calculated sensor resistance
Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
across the load resistor and its resistance, the resistance of the sensor
could be derived.
************************************************************************************/
float MQResistanceCalculation(int raw_adc)
{
return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}

/***************************** MQCalibration ****************************************
Input: mq_pin - analog channel
Output: Ro of the sensor
Remarks: This function assumes that the sensor is in clean air. It use
MQResistanceCalculation to calculates the sensor resistance in clean air
and then divides it with RO_CLEAN_AIR_FACTOR. RO_CLEAN_AIR_FACTOR is about
10, which differs slightly between different sensors.
************************************************************************************/
float MQCalibration(int mq_pin)
{
int i;
float val=0;

for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) { //take multiple samples
val += MQResistanceCalculation(analogRead(mq_pin));
delay(CALIBRATION_SAMPLE_INTERVAL);
}
val = val/CALIBARAION_SAMPLE_TIMES; //calculate the average value

val = val/RO_CLEAN_AIR_FACTOR; //divided by RO_CLEAN_AIR_FACTOR yields the Ro
//according to the chart in the datasheet

return val;
}
/***************************** MQRead *********************************************
Input: mq_pin - analog channel
Output: Rs of the sensor
Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
The Rs changes as the sensor is in the different consentration of the target
gas. The sample times and the time interval between samples could be configured
by changing the definition of the macros.
************************************************************************************/
float MQRead(int mq_pin)
{
int i;
float rs=0;

for (i=0;i<READ_SAMPLE_TIMES;i++) {
rs += MQResistanceCalculation(analogRead(mq_pin));
delay(READ_SAMPLE_INTERVAL);
}

rs = rs/READ_SAMPLE_TIMES;

return rs;
}

/***************************** MQGetGasPercentage **********************************
Input: rs_ro_ratio - Rs divided by Ro
gas_id - target gas type
Output: ppm of the target gas
Remarks: This function passes different curves to the MQGetPercentage function which
calculates the ppm (parts per million) of the target gas.
************************************************************************************/
int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
if ( gas_id == GAS_LPG ) {
return MQGetPercentage(rs_ro_ratio,LPGCurve);
} else if ( gas_id == GAS_CO ) {
return MQGetPercentage(rs_ro_ratio,COCurve);
} else if ( gas_id == GAS_SMOKE ) {
return MQGetPercentage(rs_ro_ratio,SmokeCurve);
}

return 0;
}

/***************************** MQGetPercentage **********************************
Input: rs_ro_ratio - Rs divided by Ro
pcurve - pointer to the curve of the target gas
Output: ppm of the target gas
Remarks: By using the slope and a point of the line. The x(logarithmic value of ppm)
of the line could be derived if y(rs_ro_ratio) is provided. As it is a
logarithmic coordinate, power of 10 is used to convert the result to non-logarithmic
value.
************************************************************************************/
int MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}
