#include <WiFi.h>
#include "config_wifi.h"
#include <ArduinoJson.h>
#include <SocketIoClient.h>
//#include "EOTAUpdate.h"
#include <HardwareSerial.h>
#include <ESP32httpUpdate.h>


#define EEPROM_SSID 0
#define EEPROM_PASS 100
#define EEPROM_ID 200

//Config_OTA
//const char * const   VERSION_STRING = "0.1";
  const unsigned short VERSION_NUMBER = 1;
  //String URL_SERVER="aqueous-river-62632.herokuapp.com";
  //String const URL_SERVER="http://a16f-87-68-203-193.ngrok.io";
  //String const URL_SERVER="192.168.1.185:5000";
  //const char * const   UPDATE_URL = "http://a16f-87-68-203-193.ngrok.io/hubmaster.txt";
  //const char * const   UPDATE_URL = "https://aqueous-river-62632.herokuapp.com/hubmaster.txt";
  //EOTAUpdate updater(UPDATE_URL, VERSION_NUMBER);

////socket.io init////
//   char host[34] = "aqueous-river-62632.herokuapp.com"; // Socket.IO Server Address
//   int port=80; // Socket.IO Port Address
   char host[34] = "192.168.1.185";
   int port=5000; // Socket.IO Port Addres

///socket funnctions///   
  char path[39] = "/socket.io/?EIO=3&transport=websocket"; // Socket.IO Base Path
  SocketIoClient  webSocket;
  
//socket funnctions
  void socket_Connected(const char * payload, size_t length);
  void socket_task(const char * payload, size_t length);
  void socket_Eleven_Update_Progrem_Hub(const char * payload, size_t length);
  void socket_Eight_Update_Progrem_Palnt(const char * payload, size_t length);
  void socket_event(const char * payload, size_t length);
  void send_version(const char * payload, size_t length);


//Config_wifi
  Config_wifi wifi;
  int wifiLed=4;
  
// product ID
  String id;
   
//struct of veribales that are received from plant
typedef struct DataStruct{ 
  int task;
  String productCatNumber;
  int batteryStatus;
  int moistureStatus;
  int lightStatus;
  bool motorState;
  bool waterState;
  int motorCurrentSub;
  int irrigatePlantOption;
  bool autoIrrigateState;
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
  pinMode(wifiLed,OUTPUT);
  wifi.wifiSetupNew();
    digitalWrite(wifiLed, HIGH);

  Serial2.begin(4500000);
  delay(50);
  Serial.println("i am your fater");
  //wifi.saveSsidPass("","");
  id="\""+wifi.readStringEEPROM(EEPROM_ID)+"\"";
  Serial.println((String)id);
  
//    Setup 'on' listen events
    webSocket.on("connect", socket_Connected);
    webSocket.on("task", socket_task);
//    webSocket.on("Plant_Initialization", socket_One_Plant_Initialization);
//    webSocket.on("Irrigate_Plant_Option", socket_Two_Irrigate_Plant_Option);
//    webSocket.on("Sends_Sensors", socket_Three_Sends_Sensors);// send data to the server with the sensore average reading
//    webSocket.on("Auto_Irrigate_State", socket_Four_Auto_Irrigate_State);
//    webSocket.on("Motor_Stop_Start", socket_Six_Motor_Stop_Start);// change the motor state
    webSocket.on("Update_Progrem_plant", socket_Eight_Update_Progrem_Palnt); // see if there is update_progrem 
    webSocket.on("Update_Progrem_hub", socket_Eleven_Update_Progrem_Hub); // see if there is update_progrem   
    webSocket.on("event", socket_event);
//    webSocket.on("send_progrem_Version", send_version);
    webSocket.begin(host, port, path);
}

void loop() { 
  webSocket.loop();  
  delay(30);   
  boolean messageReady = false;
  String message = "";
    if(Serial2.available()) {
      Serial.println("recived new massage");//delete before prduction
      message = Serial2.readString();
      messageReady = true;
    }
    if (messageReady){
      DynamicJsonDocument doc2(2048);  
        // Attempt to deserialize the JSON-formatted message
      DeserializationError error = deserializeJson(doc2,message);
      if(error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
      //  return;
      }
      if ((messageReady)&&(!error)){
      Data.task = doc2["task"];
      //delay(50);
      Serial.println("task recive:"+ (String)Data.task);
      swithTaskSlave(Data.task,  doc2);
      }
    }
}

/// socket functions
void socket_Connected(const char * payload, size_t length) {
  webSocket.emit("storeClientInfo", { customId: const_cast<char*>(id.c_str()) });
  Serial.println("Socket.IO Connected!");
}

void socket_event(const char * payload, size_t length) {
  Serial.print("got message: ");
  Serial.println(payload);
}

void socket_task(const char * payload, size_t length){
  DynamicJsonDocument doc1(2048);
  DeserializationError error = deserializeJson(doc1,(String)payload);
      if(error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }  
//  StaticJsonDocument<200> doc;
//  doc["task"] = 1;
//  doc["motorCurrentSub"]=doc1["motorCurrentSub"];
//  doc["productCatNumber"]=doc1["productCatNumber"];
//  doc["macAddress"]=doc1["macAddress"];
  serializeJson(doc1,Serial2); 
}

void socket_Eight_Update_Progrem_Palnt(const char * payload, size_t length){
  DynamicJsonDocument doc1(2048);
  DeserializationError error = deserializeJson(doc1,(String)payload);
      if(error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      } 
    doc1["ssid"]=wifi.readStringEEPROM(EEPROM_SSID);
    doc1["pass"]=wifi.readStringEEPROM(EEPROM_PASS);
    serializeJson(doc1,Serial2); 
  }

void socket_Eleven_Update_Progrem_Hub(const char * payload, size_t length){
    DynamicJsonDocument doc1(2048);
    DeserializationError error = deserializeJson(doc1,(String)payload);
        if(error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.c_str());
          return;
        }

    if(doc1["type"].as<String>()=="hub"){
      Serial.println(doc1["VERSION_NUMBER"].as<int>());
      if(doc1["VERSION_NUMBER"].as<int>()>VERSION_NUMBER){
        Serial.println(doc1["UPDATE_URL"].as<String>());           
           t_httpUpdate_return ret = ESPhttpUpdate.update(doc1["UPDATE_URL"].as<String>());
      }
      else {
        String resultsData="\"{hubId:"+id+", versionNumber:"+String(VERSION_NUMBER)+",type:"+"hub"+"}\"";
        char* resultsDataC; 
        if(resultsData.length()!=0){
               resultsDataC = const_cast<char*>(resultsData.c_str());}
        Serial.println(resultsDataC);
        webSocket.emit("Update_Progrem_hub", {resultsDataC});       
      }
    }
    if(doc1["type"].as<String>()=="slave"){
              Serial.println(wifi.readStringEEPROM(EEPROM_SSID));
          doc1["ssid"]=wifi.readStringEEPROM(EEPROM_SSID);
          doc1["pass"]=wifi.readStringEEPROM(EEPROM_PASS);
          serializeJson(doc1,Serial2); 
      }    
}

void swithTaskSlave(int taskReceive,const JsonDocument& local_doc){//task recived from Slave 
  String resultsData;
  char* resultsDataC; 
  Serial.println("\'"+local_doc["productCatNumber"].as<String>()+"\'");   
  String productCatNumberString="\'"+local_doc["productCatNumber"].as<String>()+"\'";
  
  switch(taskReceive) {
    case 1://plantInitialization
     Serial.println(local_doc["massgeSuccess"].as<String>());   
     resultsData="\"{productCatNumber:"+productCatNumberString+",massgeSuccess:"+local_doc["massgeSuccess"].as<String>()+"}\"";
     if(resultsData.length()!=0)
     resultsDataC = const_cast<char*>(resultsData.c_str());
     Serial.println(resultsDataC);
     webSocket.emit("plantInitialization", resultsDataC);
    break;
      case 2://irrigatePlantOption   
    break;
    case 3://reciving sensor data
       resultsData="\"{productCatNumber:"+productCatNumberString+",moistureStatus:"+local_doc["moistureStatus"].as<String>()+",lightStatus:"+local_doc["lightStatus"].as<String>()+"}\"";
       if(resultsData.length()!=0)
       resultsDataC = const_cast<char*>(resultsData.c_str());
       Serial.println(resultsDataC);
       webSocket.emit("resultsdata", resultsDataC);
      break;
    case 4://reciving motor status
       resultsData="\"{productCatNumber:"+productCatNumberString+",irrigatePlantWorking:"+local_doc["irrigatePlantWorking"].as<String>()+",autoIrrigateState:"+local_doc["autoIrrigateState"].as<String>()+",waterState:"+local_doc["waterState"].as<String>()+",motorState:"+local_doc["motorState"].as<String>()+"}\"";
       if(resultsData.length()!=0)
       resultsDataC = const_cast<char*>(resultsData.c_str());
       Serial.println(resultsDataC);
       webSocket.emit("irrigatedata", resultsDataC);  
      break;
    case 8:
     resultsData="\"{productCatNumber:"+productCatNumberString+",versionNumber:"+local_doc["VERSION_NUMBER"].as<String>()+",massgeSuccess:"+local_doc["massgeSuccess"].as<String>()+",wifiWorked:"+local_doc["wifiWorked"].as<String>()+"}\"";
    if(resultsData.length()!=0)
       resultsDataC = const_cast<char*>(resultsData.c_str());
    Serial.println(resultsDataC);
    webSocket.emit("updatingSuccessed", resultsDataC);  
      //checkUpdateProgrem();
    break;
    case 11:
     resultsData="\"{hubId:"+id+",,versionNumber:"+local_doc["VERSION_NUMBER"].as<String>()+",massgeSuccess:"+local_doc["massgeSuccess"].as<String>()+"}\"";
    if(resultsData.length()!=0)
       resultsDataC = const_cast<char*>(resultsData.c_str());
    Serial.println(resultsDataC);
    webSocket.emit("Update_Progrem_hub", resultsDataC);  
      //checkUpdateProgrem();
    break;
}
}
