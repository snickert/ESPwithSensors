

/*
esp32_RGB_and_Temp_Sensor

In this Program a combination of serveral RGB-Sensors TCS34725 and Temperature-Sensors DS18B20
are reading the actual temperature as well as the actual RBG-Values and send them in a HTTP-Post over WLAN to a
Server in form of JSON-Packages

Autor: pte-kb & jan wickert

to setup change:
Wlan:
ssid
password

MSB:
uuid
token

*/
//Include of all libraries witch are used
//--------------------------------------------------------------------------------------------
#include <Wire.h> //in Adafruit_TCS34725.cpp sind bei Wire.begin(2,0) die sda und scl pins definiert
#include "Adafruit_TCS34725.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>
#include <WiFi.h> 
#include <HTTPClient.h>

//Definition of all constants
//--------------------------------------------------------------------------------------------
#define ONE_WIRE_PIN 14 // Here is the Number of the Pin Connection for the Temperature Sensor
#define BAUD_RATE 115200 // Baudrate for serial communication
#define TEMP_PRECISION 8 // Precision of the Temperaturesensor
#define MESSZYKLEN 100 // Number of Measurement that should be done before a solution is calculated
#define DELAYBETWEENMEASUREMENT 2000 // The time to wait between a measurement of the sensors
#define MSB_ADDRESS "http://atm.virtualfortknox.de/msb/rest/data/" //The adress of the data where the post-message should send to

//Initialization of all global Prameters in this program
//--------------------------------------------------------------------------------------------
//colorsensor
//--------------------------------------------------------------------------------------------
//integrationszeit der dioden wählen achtung wird sie zulange gewählt läuft der uint32_t über
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
uint8_t rgbSDA[] = {21,19};
uint8_t rgbSCL[] = {22,18};
uint8_t numberRGB = sizeof(rgbSDA)/sizeof(rgbSDA[0]);
uint16_t clear[MESSZYKLEN], red[MESSZYKLEN], green[MESSZYKLEN], blue[MESSZYKLEN] ;
uint32_t mittelwertClear, mittelwertRed, mittelwertGreen, mittelwertBlue, mittelwerteRGB;
uint32_t mittelwertCleardark, mittelwertReddark, mittelwertGreendark, mittelwertBluedark, mittelwerteRGBdark;
//colorsensor end

//temperaturesensor
//--------------------------------------------------------------------------------------------
const uint8_t numberTemp = 2;
OneWire oneWire(ONE_WIRE_PIN); // Setup a instance of the OneWire Bus to communicate with any OneWire devices
DallasTemperature tempSensor(&oneWire); // Connect the OneWire Bus to the Dallas Temperature Sensor
DeviceAddress tempDevice[numberTemp];
//temperaturesensor end

//wifi settings
//--------------------------------------------------------------------------------------------
const char* ssid = "";
const char* password = "";
HTTPClient http;
//wifi end

//MSB settings
//--------------------------------------------------------------------------------------------
String uuid = "";
String token= "";
String adressRegistration = "http://atm.virtualfortknox.de/msb/rest/smartobject/register/";
//MSB settings end

//method for setup a RGB-Sensor
//--------------------------------------------------------------------------------------------
void setupRGB(int setupPinSDA){
  pinMode(setupPinSDA, OUTPUT); //LED for viewing the status
  digitalWrite(setupPinSDA, HIGH);
  Serial.println("\n[SETUP][SENSOR]RGB-Sensor is initialising:");
  //Start of the connection and test if connection can be established
  //CARE !!! The Adafruit Libary only use the standart SDA and SCL Pins - Adafruit.h must be overwritten
  if (tcs.begin()) {
    Serial.println("[SETUP][SENSOR]Found sensor");
    Serial.println("[SETUP][SENSOR]RGB Sensor ready ...\n");
  }else {
    Serial.println("[SETUP][SENSOR]No TCS34725 Sensor found ... check your connections");
    while (1); // halt!
  }
  digitalWrite(setupPinSDA, HIGH);
}

//method for setup the Temperature-Sensors
//--------------------------------------------------------------------------------------------
void setupTemp(){
  tempSensor.begin(); // Initialize the Sensor Library
  Serial.println("\n[SETUP][SENSOR]Temperatur-Sensoren are initialising:");
  int numberTempTest = tempSensor.getDeviceCount();
  if (numberTempTest != 0){
    Serial.printf("[SETUP][SENSOR]Number of found temperature sensors: %d\n", numberTemp);
    Serial.println("[SETUP][SENSOR]Temperatur Sensors ready ...\n");
  }else{
    Serial.println("[SETUP][SENSOR]No DS18B20 Sensors found ... check your connections");
  }
  for (uint8_t j = 0; j<numberTempTest; j++){
    tempSensor.getAddress(tempDevice[j],j); // Looking for the addresses of the sensors
    tempSensor.setResolution(tempDevice[j], TEMP_PRECISION);
    if (!tempSensor.getAddress(tempDevice[j], j)) Serial.println("[SETUP][SENSOR]Unable to find address for a Device"); 
  }
}
//method for changing the address of the I2C-BUS
//--------------------------------------------------------------------------------------------
void changeI2C(int i2cSCL, int i2cSDA){
  // Hier muss eine Funktion hin, welche den Aufruf tcs.begin(i2cSCL,i2cSDA) enthält
}

//method for measure the values of the RGB-Sensor
//--------------------------------------------------------------------------------------------
void measureRGB(bool powerLED,int pinSCL, int pinSDA){
  Serial.println("\n[SENSOR]RGB measuring begin");
  changeI2C(pinSDA, pinSCL);
  if (powerLED==true) tcs.setInterrupt(false);  // turn on colorsensorLED
  for (uint8_t i=0; i<MESSZYKLEN;i++)
  {
    delay(60);  // takes 50ms to read
    tcs.getRawData(&red[i], &green[i], &blue[i], &clear[i]);
  }
  if (powerLED==true) tcs.setInterrupt(true);  // turn off colorsensorLED
  
}

//method for the calculate and print the avarage of the measurements with LED
//--------------------------------------------------------------------------------------------
void calculateAvarage(bool calculateDark, int* transferValues){
  if (calculateDark == true){
    mittelwertClear = 0;
    mittelwertRed   = 0;
    mittelwertGreen = 0;
    mittelwertBlue  = 0;
    mittelwerteRGB = 0;
    for (int i=0; i<MESSZYKLEN;i++){
      mittelwertClear += clear[i];
      mittelwertRed   += red[i];
      mittelwertGreen += green[i];
      mittelwertBlue  += blue[i];
    }
    mittelwertClear /= MESSZYKLEN;
    mittelwertRed   /= MESSZYKLEN;
    mittelwertGreen /= MESSZYKLEN;
    mittelwertBlue  /= MESSZYKLEN;
    mittelwerteRGB = mittelwertRed + mittelwertGreen + mittelwertBlue;
    Serial.println("[SENSOR]--------------------------------------------------------------------");
    Serial.print("[SENSOR]Clear:\t"); Serial.print(mittelwertClear);
    Serial.print("\tRed:\t"); Serial.print(mittelwertRed);
    Serial.print("\tGreen:\t"); Serial.print(mittelwertGreen);
    Serial.print("\tBlue:\t"); Serial.println(mittelwertBlue);
    Serial.print("[SENSOR]R+G+B:\t\t");Serial.println(mittelwerteRGB);
    Serial.println("[SENSOR]RGB measuring end");
    int collectedValues[] = {mittelwertRed, mittelwertGreen, mittelwertBlue, mittelwerteRGB};
    transferValues = collectedValues;
  }
  else{
    mittelwertCleardark = 0;
    mittelwertReddark   = 0;
    mittelwertGreendark = 0;
    mittelwertBluedark  = 0;    
    mittelwerteRGBdark = 0;
    for (int i=0; i<MESSZYKLEN;i++){
      mittelwertCleardark += clear[i];
      mittelwertReddark   += red[i];
      mittelwertGreendark += green[i];
      mittelwertBluedark  += blue[i];
    }
    mittelwertCleardark /= MESSZYKLEN;
    mittelwertReddark   /= MESSZYKLEN;
    mittelwertGreendark /= MESSZYKLEN;
    mittelwertBluedark  /= MESSZYKLEN;
    mittelwerteRGBdark = mittelwertReddark + mittelwertGreendark + mittelwertBluedark;
    Serial.println("[SENSOR]--------------------------------------------------------------------");
    Serial.print("[SENSOR]Clear Dark:\t"); Serial.print(mittelwertCleardark);
    Serial.print("\tRed Dark:\t"); Serial.print(mittelwertReddark);
    Serial.print("\tGreen Dark:\t"); Serial.print(mittelwertGreendark);
    Serial.print("\tBlue Dark:\t"); Serial.println(mittelwertBluedark);
    Serial.print("[SENSOR]R+G+B Dark:\t");Serial.println(mittelwerteRGBdark);
    Serial.println("[SENSOR]RGB measuring end");
    int collectedValuesdark[] = {mittelwertReddark, mittelwertGreendark, mittelwertBluedark, mittelwerteRGBdark};
    transferValues = collectedValuesdark;
  }
}

//method for the caclulation of the temperature with a defined Sensor
//--------------------------------------------------------------------------------------------
float measureTemp(DeviceAddress tempAddress){
  float temperatureC = 0;
  Serial.println("\n[SENSOR]Temperature measuring begin");
  tempSensor.requestTemperatures();
  Serial.println("[SENSOR]--------------------------------------------------------------------");
  Serial.println("[SENSOR]Temperature requesting");
  temperatureC = tempSensor.getTempC(tempAddress);       
  Serial.printf("[SENSOR]Measured Temperature: \t%0.1f",temperatureC);
  Serial.println("\n[SENSOR]Temperature measuring finished");
  return temperatureC;
}

void sendRegistration() {
  Serial.print("\n[HTTP] begin Data sending...\n");
  String registerPayload = 
  "{ "
  "  \"@class\": \"SmartObject\", "
  "  \"uuid\": \""+uuid+"\", "
  "  \"description\": \"Uberwachung des Schaltschrankes mittels RGB und Temperatur Sensoren\", "
  "  \"name\": \"ESP32 Schaltschrank Sensor\", "
  "  \"token\": \""+token+"\", "
  "  \"events\": [ "
  "    { "
  "      \"eventId\": \"sensorvalue\", "
  "      \"name\": \"Schaltschrank Sensor\", "
  "      \"description\": \"Description for Event 5\", "
  "      \"dataFormat\": { "
  "        \"dataObject\": { "
  "          \"type\": \"object\", "
  "          \"properties\": { "
  "            \"rgb1\": { "
  "              \"$ref\": \"#/definitions/rgbSensor\" "
  "            }, "
  "            \"rgb2\": { "
  "              \"$ref\": \"#/definitions/rgbSensor\" "
  "            }, "
  "            \"temp1\": { "
  "              \"type\": \"number\", "
  "              \"format\": \"float\" "
  "            }, "
  "            \"temp2\": { "
  "              \"type\": \"number\", "
  "              \"format\": \"float\" "
  "            } "
  "          } "
  "        }, "
  "        \"rgbSensor\": { "
  "          \"type\": \"object\", "
  "          \"properties\": { "
  "            \"red\": { "
  "              \"type\": \"integer\", "
  "              \"format\": \"int32\" "
  "            }, "
  "            \"green\": { "
  "              \"type\": \"integer\", "
  "              \"format\": \"int32\" "
  "            }, "
  "            \"blue\": { "
  "              \"type\": \"integer\", "
  "              \"format\": \"int32\" "
  "            }, "
  "            \"rgbRaw\": { "
  "              \"type\": \"integer\", "
  "              \"format\": \"int32\" "
  "            }, "
  "            \"rgbCorrected\": { "
  "              \"type\": \"integer\", "
  "              \"format\": \"int32\" "
  "            } "
  "          } "
  "        } "
  "      } "
  "    }, "
  "    { "
  "      \"eventId\": \"sensorcalibrate\", "
  "      \"name\": \"Schaltschrank Sensor Kalibrierung\", "
  "      \"description\": \"Dieser Wert dient als Kalibrierung nach Wechsel des Filters\", "
  "      \"dataFormat\": { "
  "        \"dataObject\": { "
  "          \"type\": \"object\", "
  "          \"properties\": { "
  "            \"rgb1\": { "
  "              \"$ref\": \"#/definitions/rgbSensor\" "
  "            }, "
  "            \"rgb2\": { "
  "              \"$ref\": \"#/definitions/rgbSensor\" "
  "            }, "
  "            \"temp1\": { "
  "              \"type\": \"number\", "
  "              \"format\": \"float\" "
  "            }, "
  "            \"temp2\": { "
  "              \"type\": \"number\", "
  "              \"format\": \"float\" "
  "            } "
  "          } "
  "        }, "
  "        \"rgbSensor\": { "
  "          \"type\": \"object\", "
  "          \"properties\": { "
  "            \"red\": { "
  "              \"type\": \"integer\", "
  "              \"format\": \"int32\" "
  "            }, "
  "            \"green\": { "
  "              \"type\": \"integer\", "
  "              \"format\": \"int32\" "
  "            }, "
  "            \"blue\": { "
  "              \"type\": \"integer\", "
  "              \"format\": \"int32\" "
  "            }, "
  "            \"rgbRaw\": { "
  "              \"type\": \"integer\", "
  "              \"format\": \"int32\" "
  "            }, "
  "            \"rgbCorrected\": { "
  "              \"type\": \"integer\", "
  "              \"format\": \"int32\" "
  "            } "
  "          } "
  "        } "
  "      } "
  "    } "
  "  ] "
  "} ";

  http.begin(MSB_ADDRESS); //HTTP
  http.addHeader("Content-Type", "application/json");
  // start connection and send HTTP header
  int httpCode = http.POST(registerPayload);
  Serial.print("[HTTP] Data send...\n");
  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  }
  else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

}

void sendSensorEvent(int rLight0,int gLight0,int bLight0,int rgbLight0,int rDark0,int gDark0,int bDark0,int rgbDark0,int rLight1,int gLight1,int bLight1,int rgbLight1,int rDark1,int gDark1,int bDark1,int rgbDark1,float temp1,float temp2){
  String eventPayload = 
  "{ "
  "  \"uuid\": \""+uuid+"\", "
  "  \"dataObject\": { "
  "    \"rgb1\":{\"red\":\""+rLight0+"\",\"green\":\""+gLight0+"\",\"blue\":\""+bLight0+"\",\"rgbRaw\":\""+rgbLight0+"\",\"rgbCorrected\":\""+(rgbLight0-rgbDark0)+"\"}, "
  "    \"rgb2\":{\"red\":\""+rLight1+"\",\"green\":\""+gLight1+"\",\"blue\":\""+bLight1+"\",\"rgbRaw\":\""+rgbLight1+"\",\"rgbCorrected\":\""+(rgbLight1-rgbDark1)+"\"}, "
  "    \"temp1\":\""+temp1+"\", "
  "    \"temp2\":\""+temp2+"\" "
  "    }, "
  "  \"eventId\": \"sensorvalue\" "
  "} ";

  http.begin(MSB_ADDRESS); //HTTP
  http.addHeader("Content-Type", "application/json");
  // start connection and send HTTP header
  int httpCode = http.POST(eventPayload);
  Serial.print("[HTTP] Data send...\n");
  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  }
  else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
}

//setup
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(BAUD_RATE); // Initialization of the serial communication
//colorsensor setup
//--------------------------------------------------------------------------------------------
  for (uint8_t k=0; k<numberRGB; k++){
  setupRGB(rgbSDA[k]);
  }
//temperaturesensor setup
//--------------------------------------------------------------------------------------------
  setupTemp();
  
//msb-SmartObject registration
  sendRegistration();
}


//loop
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
void loop() {
   //Working with a defined number of RGB-Sensors
    Serial.printf("\n[SENSOR]RGB-Sensor: %d",1);
    measureRGB(false,rgbSCL[0],rgbSDA[0]);
    int light0[4];
    calculateAvarage(true,light0);
    Serial.printf("\n[SENSOR]RGB-Sensor: %d",1);    
    measureRGB(true,rgbSCL[0],rgbSDA[0]);
    int dark0[4];
    calculateAvarage(false,dark0);

    Serial.printf("\n[SENSOR]RGB-Sensor: %d",2);
    measureRGB(false,rgbSCL[1],rgbSDA[1]);
    int light1[4];
    calculateAvarage(true,light1);
    Serial.printf("\n[SENSOR]RGB-Sensor: %d",2);    
    measureRGB(true,rgbSCL[1],rgbSDA[1]);
    int dark1[4];
    calculateAvarage(false,dark1);

    float temp1 = measureTemp(tempDevice[0]);
    float temp2 = measureTemp(tempDevice[1]);

    
  //Working with a defined number of Temperature Sensors

  sendSensorEvent(light0[0],light0[1],light0[2],light0[3],dark0[0],dark0[1],dark0[2],dark0[3],light1[0],light1[1],light1[2],light1[3],dark1[0],dark1[1],dark1[2],dark1[3],temp1, temp2);
  delay(5000);
}

