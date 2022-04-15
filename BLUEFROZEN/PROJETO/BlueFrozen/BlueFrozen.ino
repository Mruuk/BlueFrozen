 //BIBLIOTECAS USADAS
#include <SoftReset.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"
//#include <avr/wdt.h>
//---------------------------------------//

//DEFINIÇOES
// Porta do pino de sinal do DS18B20
#define ONE_WIRE_BUS 28

//Reset arduino
#define _SOFT_RESTART_H
#define soft_restart()        


//Pinos WIFI
#define ADAFRUIT_CC3000_IRQ   3  //Pino de interrupcao (D2 ou D3)
#define ADAFRUIT_CC3000_VBAT  5  //Pode ser qualquer pino digital
#define ADAFRUIT_CC3000_CS    10 //Preferencialmente pino 10 do Arduino Uno
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, 
                                         ADAFRUIT_CC3000_VBAT,SPI_CLOCK_DIVIDER); 

#define WLAN_SSID       "Quantum Go 2"   //Nome da rede - Ate 32 caractereres
#define WLAN_PASS       "12345678"     //Senha da rede
#define WLAN_SECURITY   WLAN_SEC_WPA2  //Tipo de seguranca


//--------------------------------------------------------------//

//VARIAVEIS GLOBAIS

// Define uma instancia do oneWire para comunicacao com o sensor
OneWire oneWire(ONE_WIRE_BUS);

// Armazena temperaturas minima e maxima
float tempMin = 999;
float tempMax = 0;
float tempC;
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;

// Inicializa o LCD
LiquidCrystal lcd(22, 23, 24, 25, 26, 27);
int screenWidth = 20;  
int screenHeight = 4;

//SCROLL LCD
String line1 = "SEJA BEM VINDO";
String line2 = " BLUEFROZEN"; 
int stringStart, stringStop = 0;  
int scrollCursor = screenWidth;  
int tamanho =0; 
 //---------------------------------------------------//
 
//prototypes
void printarSerial();
void bemVindo();
void scroll_sup();
void iniciar();
//void enviarParaThing();
void lerSensor();
void mostrarLcd();
void openSystem();
//void resolveWebsite();
 
//-------------------------------------------------------//


//FUNÇOES PRINCIPAIS

//SETUP PARA CHAMAR AS FUNÇOES
void setup(void)
{
  // Iniciar sistema
  iniciar();
  
  // Mensagem do BlueFrozen
  bemVindo();
  delay(300);
  
  // Localiza e mostra enderecos dos sensores 
  openSystem();
  
}
void mostra_endereco_sensor(DeviceAddress deviceAddress){
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

//-------------------------------------------------------------

//LOOP 
void loop(){
  // Le a informacao do sensor
  lerSensor();

  // Mostra dados no serial monitor
  printarSerial(tempC);
  
  // Mostra dados no LCD  
  mostrarLcd();
  delay(5000);
}

//FUNÇOES 

//SCROLL
void scroll_sup(){
  
  lcd.clear();  
  if(stringStart == 0 && scrollCursor > 0)
  {  
    scrollCursor--;  
    stringStop++;  
  } else if (stringStart == stringStop){  
    stringStart = stringStop = 0;  
    scrollCursor = screenWidth;  
  } else if (stringStop == line1.length() && scrollCursor == 0) {  
    stringStart++;  
  } else {  
    stringStart++;  
    stringStop++;  
  } 
}
//---------

//ABRIR SISTEMA
void openSystem(){
  // Localiza e mostra enderecos dos sensores
  Serial.println("Localizando sensores DS18B20...");
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Localizando sensores DS18B20...");
  delay(3000);
  
  Serial.print("Foram encontrados: ");
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Foram encontrados: ");
  
  
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" sensores.");

  lcd.setCursor(19,0);
  lcd.print(sensors.getDeviceCount(), DEC);
  delay(2000);
  
  if (!sensors.getAddress(sensor1, 0)) {
     Serial.println("Sensores nao encontrados !"); 
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("Sensores nao encontrados!");
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("Reiniciado o sistema");
     lcd.setCursor(0,3);
     lcd.print("Verifique o sensor");
     delay(5000);
     openSystem();
  }

  // Mostra o endereco do sensor encontrado no barramento
  Serial.print("Endereco sensor: ");
  mostra_endereco_sensor(sensor1); 
  Serial.println();
  Serial.println();
  Serial.println(F("Inicializando..."));
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Inicializando...");
  
  if (!cc3000.begin())
  {
    Serial.println(F("Shield nao encontrado. Verifique as conexoes !"));
    while(1){
        soft_restart();
    }
  }
  
  Serial.println(F("Tentando conectar-se a rede: ")); Serial.println(WLAN_SSID);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Tentando conectar-se"); 
  lcd.setCursor(0,1);
  lcd.print("a rede: "); 
  lcd.setCursor(8,1);
  lcd.print(WLAN_SSID);
  
  
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) 
  {
    Serial.println(F("Falha !"));
    while(1);
  }
  Serial.println(F("Conectado!"));
  
  Serial.println(F("Requisitando endereco DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); 
  }  
  //Exibe as informacoes da conexao
  while (! displayConnectionDetails()) 
  {
    delay(1000);
  }
  
  Serial.println(F("Aguardando conexao..."));
  
}
//ABRIR SISTEMA TERMINA AQUI

//INICIAR
void iniciar(){
    Serial.begin(115200);
    sensors.begin();
    lcd.begin(20, 4);
    pinMode(A2, INPUT); 
}
//FIM DO INICIAR

//INICIO DO DA FUNÇAO PARA LER O SENSOR
void lerSensor(){
  // Le a informacao do sensor
  sensors.requestTemperatures();
  tempC = sensors.getTempC(sensor1);
  
  // Atualiza temperaturas minima e maxima
  if (tempC < tempMin)
  {
    tempMin = tempC;
  }
  if (tempC > tempMax)
  {
    tempMax = tempC;
  }
  
  //resolveWebsite();
}
//FIM DO LER SENSOR

//MOSTRAR NA SERIAL
void printarSerial(float temp){
   // Mostra dados no serial monitor
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Min : ");
  Serial.print(tempMin);
  Serial.print(" Max : ");
  Serial.println(tempMax);
}
//FIM

//MOSTRAR NA LCD
void mostrarLcd(){ 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp.:       ");
  lcd.write(223);
  lcd.print("C");
  lcd.setCursor(7,0);
  lcd.print(tempC);
  lcd.setCursor(0,1);
  lcd.print("L: ");
  lcd.setCursor(3,1);
  lcd.print(tempMin,1);
  lcd.setCursor(8,1);
  lcd.print("H: ");
  lcd.setCursor(11,1);
  lcd.print(tempMax,1);
  lcd.setCursor(5,3);
  lcd.print("BLUEFORZEN");
  delay(5000);
}
//FIM

//MOSTRAR BEM VINDO NA TELA
void bemVindo(){
 
   for(int i=0; i<53; i++){   
      lcd.setCursor(scrollCursor, 0);  
      lcd.print(line1.substring(stringStart,stringStop));  
      lcd.setCursor(4, 2);  
      lcd.print(line2);
      
      delay(250); 
      
      scroll_sup(); //Chama a rotina que executa o scroll   
      //Verifica o tamanho da string  
      tamanho = line1.length();  
      if (stringStart == tamanho)  
      {  
        stringStart = 0;  
        stringStop = 0;  
      }
   }   
}

//FIM ------------

//FUNÇAO BOOLEAN PARA DETALHES DAS CONEXAO
bool displayConnectionDetails(void){
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Nao foi possivel ler o endereco IP!\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  } 
}
//FIM DA BOOLEAN E DAS FUNÇOES 
