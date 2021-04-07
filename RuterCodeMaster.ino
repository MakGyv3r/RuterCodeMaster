#include <WiFi.h>
#include "config_wifi.h"
#include <ArduinoJson.h>
#include <SocketIoClient.h>
#include "EOTAUpdate.h"
#include <HardwareSerial.h>

#define EEPROM_SSID 0
#define EEPROM_PASS 100
#define EEPROM_ID 200
#define EEPROM_MOTOR_CURRENT_SUB 250


#define TX2_pin  17
#define RX2_pin  16


//Config_OTA
//const char * const   VERSION_STRING = "0.1";
  const unsigned short VERSION_NUMBER = 1;
  const char * const   UPDATE_URL     = "http://morning-falls-78321.herokuapp.com/updateMaster.txt";
//  const char * const   UPDATE_URL     = "http://0db0f6a29cd7.ngrok.io/updateMaster.txt";
  EOTAUpdate updater(UPDATE_URL, VERSION_NUMBER);

////socket.io init////
   char host[34] = "morning-falls-78321.herokuapp.com"; // Socket.IO Server Address
   int port=80; // Socket.IO Port Address
//   char host[34] = "192.168.1.121";
//   int port=5000; // Socket.IO Port Addres

///socket funnctions///   
  char path[33] = "/socket.io/?transport=websocket"; // Socket.IO Base Path
  SocketIoClient  webSocket;
//socket funnctions
//  void socket_Connected(const char * payload, size_t length);
//  void socket_event(const char * payload, size_t length);
//  void socket_motor_change(const char * payload, size_t length);
//  void check_sensore1(const char * payload, size_t length);
//  void auto_watering_change(const char * payload, size_t length) ; 
//  void check_update_progrem(const char * payload, size_t length);
//  void insert_motor_current_Sub(const char * payload, size_t length) ;
//  void send_version(const char * payload, size_t length);
 

//Config_wifi
  Config_wifi wifi;
  
// product ID
  String id;
   
//struct of veribales that are received from plant
typedef struct DataStruct{ 
  int task;
  String plantIdNumber="000000003";
  int batteryStatus;
  int moistureStatus;
  int lightStatus;
  bool motorState=false;
  bool waterState;
  int motorCurrentSub=300;
  int irrigatePlantOption=3;
  bool autoIrrigateState=false;
  bool irrigatePlantWorking;
  unsigned short versuionNumber;
  String ssid;
  String pass;
  bool massgeSuccess;
} DataStruct;
DataStruct Data;
int task;

//functions
//void batteryStatus();
//void checkUpdateProgrem();

void sendtask();
void swithTaskSlave( int taskReceive,const JsonDocument& local_doc);

void setup() {
    Serial.begin(115200);
  Serial2.begin(115200/*, SERIAL_8N1, RX2_pin, TX2_pin*/);
  Serial.println("i am your fater");
   // wifi.wifiSetupNew();
  //  id="\""+wifi.readStringEEPROM(EEPROM_ID)+"\"";
    
    // Setup 'on' listen events
//    webSocket.on("connect", socket_Connected);
//    webSocket.on("event", socket_event);
//    webSocket.on("send_progrem_Version", send_version);
//    webSocket.on("state_change_request", socket_motor_change);// change the motor state
//    webSocket.on("check_sensore", check_sensore); // send data to the server with the sensore average reading
//    webSocket.on("autoWatering_change_request", auto_watering_change); // send data to the server with the sensore average reading
//    webSocket.on("update_progrem", check_update_progrem); // see if there is update_progrem
//    webSocket.on("get_motor_current_Sub", insert_motor_current_Sub); // see if there is update_progre
//    webSocket.begin(host, port, path);
}

void loop() {
  if (Serial.available() > 0) {//task that are recived from user
    task = Serial.readString().toInt() ;
    StaticJsonDocument<200> doc;
    Serial.print("I received: ");
    Serial.println(task);
    doc["task"] = task;
    switch(task) {
    case 1:
    doc["motorCurrentSub"]=Data.motorCurrentSub;
    doc["plantIdNumber"]=Data.plantIdNumber;
    Serial.println(Data.plantIdNumber);//delete before prduction
    break;
    case 2:
        doc["irrigatePlantOption"]=Data.irrigatePlantOption;
        doc["motorCurrentSub"]=Data.motorCurrentSub;
        doc["plantIdNumber"]=Data.plantIdNumber;
        Serial.println(Data.plantIdNumber);//delete before prduction
      break;
    case 3:
      break;
    case 4:
        doc["autoIrrigateState"]=!Data.autoIrrigateState;
        doc["motorCurrentSub"]=Data.motorCurrentSub;
        Data.autoIrrigateState=!Data.autoIrrigateState;
      break;
//  case 5:
//      batteryStatus();
//    break;
    case 6:
        doc["motorState"]=!Data.motorState;
        doc["motorCurrentSub"]=Data.motorCurrentSub;
        Data.motorState=!Data.motorState;
      break;
    case 7:
//      checkUpdateProgrem();
    break;
    }
   serializeJson(doc,Serial2); 
  }
  
  boolean messageReady = false;
  String message = "";
    if(Serial2.available()) {
      Serial.println("recived new massage ");//delete before prduction
      message = Serial2.readString();
      messageReady = true;
    }
    if (messageReady){
      DynamicJsonDocument doc2(1024);  
        // Attempt to deserialize the JSON-formatted message
      DeserializationError error = deserializeJson(doc2,message);
      if(error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }
      if (messageReady){
      Data.task = doc2["task"];
      //delay(50);
      Serial.println("task recive:"+ (String)Data.task);
      swithTaskSlave(Data.task,  doc2);
      }
    }
}

void swithTaskSlave(int taskReceive,const JsonDocument& local_doc){//task recived from Slave 
  switch(taskReceive) {
    case 1:
//      plantInitialization
     Data.massgeSuccess=local_doc["massgeSuccess"];
     Data.plantIdNumber=(char*)local_doc["plantIdNumber"].as<char*>(); 
     Serial.println(Data.massgeSuccess); 
     Serial.println(Data.plantIdNumber); 
    break;
      case 2://irrigatePlantOption   
    break;
    case 3://reciving sensor data
       Data.moistureStatus=local_doc["moistureStatus"];
       Data.lightStatus=local_doc["lightStatus"]; 
       Serial.println("humidity Status:"+ (String)Data.moistureStatus );
       Serial.println("light Status:"+ (String)Data.lightStatus );  
      break;
    case 4://reciving motor status
        Data.motorState=local_doc["motorState"];
        Data.irrigatePlantWorking=local_doc["irrigatePlantWorking"];
        Data.autoIrrigateState=local_doc["autoIrrigateState"];
        Data.waterState=local_doc["waterState"];
        Data.plantIdNumber=(char*)local_doc["plantIdNumber"].as<char*>();  
        Serial.println("motor state:"+ (String)Data.motorState);
        Serial.println("irrigatePlantWorking state:"+ (String)Data.irrigatePlantWorking ); 
        Serial.println("autoIrrigateState state:"+ (String)Data.autoIrrigateState ); 
        Serial.println("waterState state:"+ (String)Data.waterState ); 
        Serial.println(Data.plantIdNumber); 
      break;
    case 8:
      //checkUpdateProgrem();
    break;
}
}
