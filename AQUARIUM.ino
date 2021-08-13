  /*
   * 
   * Bandar Lampung, Januari 2021
  
  */
  
  #include <ESP8266WiFi.h>
  #include <PubSubClient.h>
  #include <DallasTemperature.h>
  #include <OneWire.h>
  #include <ESP8266HTTPClient.h>
  #include <dummy.h>
  #define ONE_WIRE_BUS 4
  OneWire oneWire(ONE_WIRE_BUS);
  DallasTemperature sensors(&oneWire); 
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <ArduinoJson.h>
  #include <FS.h> 
  #include <Ticker.h>
  #include <EasyNTPClient.h>
  String suhu;
  String Status_Lampu;
  String Status_Kipas;
  const int kipas  = D1;
  const int lampu  = D5; 
  WiFiClient client;
  String request_string;
  const char* host ="192.168.43.222";
  String path      = "/Api/lampu.json";  
  String path1     = "/Api/kipas.json";  
  String  urlkipas ="http://192.168.43.222/Api/kipas.json";
  String  urllampu ="http://192.168.43.222/Api/lampu.json";
  HTTPClient http;
  StaticJsonBuffer<200> jsonBuffer;
  const char* ssid = "DaBol";
  const char* password = "05050505";
  const char* mqtt_server = "167.205.7.226";
  const char* mqtt_user = "/iotpertanian:iot_pertanian";
  const char* mqtt_password = "iotpertanian";
  const int   mqttPort = 1883;
   
  WiFiClient espClient;
  PubSubClient Client(espClient);
  void setup() {
  Serial.println("Data");
  Serial.begin(9600);
  pinMode(kipas,OUTPUT);
  pinMode(lampu,OUTPUT);
  digitalWrite(kipas,LOW);
  digitalWrite(lampu,LOW);
   WiFi.disconnect();
     WiFi.begin(ssid,password);
     int wifi_ctr =10;
      while ((!(WiFi.status() == WL_CONNECTED))){
        delay(1000);
      }
       Serial.println("Konek Ke Wi-Fi");
       Client.setServer(mqtt_server, mqttPort);
   
    while (!Client.connected()) {
      Serial.println("Konek ke MQTT...");
      if (Client.connect("Aquarium...", mqtt_user, mqtt_password )) {
        Serial.println("Konek");
         publish_data();
      } else {
        Serial.print("Gagal dong :(");
        Serial.print(Client.state());
        delay(1000);
      }
    }
    Client.setServer(mqtt_server, mqttPort);
  }
   
  void loop() {
    Client.setServer(mqtt_server, mqttPort);
//      if(Status_Lampu="on"){
//        digitalWrite(lampu,LOW);
//      }else if (Status_Lampu="off"){
//         digitalWrite(lampu,HIGH);
//      }else if (Status_Kipas="on"){
//        digitalWrite(lampu,LOW);
//      }else if (Status_Kipas="off"){
//        digitalWrite(lampu,HIGH);
//      }
//    if(sensors.getTempCByIndex(0)< 27){
////      digitalWrite(lampu, LOW);
////      Serial.println("Lampu Hidup");
////      digitalWrite(kipas, HIGH);
////      Serial.println("Kipas Mati");
//      Status_Lampu="on";
//      Status_Kipas="off";
//    }else  if(sensors.getTempCByIndex(0)> 31){
////      digitalWrite(lampu, HIGH);
////      Serial.println("Lampu Mati");
////      digitalWrite(kipas, LOW);
////      Serial.println("Kipas Hidup");
//      Status_Lampu="off";
//      Status_Kipas="on";
//    }
    publish_data();  
    Client.loop();
    getsuhu();
    kirimdata();
//    Triger();
    kontrol_kipas();
    kontrol_lampu();
    
    
  }
  
  void kirimdata(){
      delay(2000);
      if (!client.connect(host,80)) {
        Serial.println("Gagal Konek :(");
        return;
      }
      request_string = "/Api/input_sensor.php?data=";
      request_string +=suhu;
      Serial.print("requesting URL: ");
      Serial.println(request_string);
      client.print(String("GET ") + request_string + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (client.available() == 10) {
        if (millis() - timeout > 500) {
        Serial.println(">>> Client Timeout !");
//        client.stop();
        return;
      }
    } 
   }
    void getsuhu(){
      sensors.requestTemperatures();  
      suhu=sensors.getTempCByIndex(0);    
      Serial.println("Suhu:");
      Serial.println(sensors.getTempCByIndex(0));  
      delay(500);
    }
    void publish_data(){
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        root["mac"] = "1234567890";
        root["suhu"] =sensors.getTempCByIndex(0);
        root["kipas"] =Status_Kipas;
        root["lampu"] =Status_Lampu;
        String pubmsg;
        root.printTo(pubmsg);
        Serial.println("Kirim ke RMQ");
        Client.publish("dani", pubmsg.c_str());
     
      }
   void kontrol_lampu(){
//     HTTPClient http;  
    http.begin(urllampu);
    int httpCode = http.GET();                                                                 
    if (httpCode > 0) {
      const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(http.getString());
      String datalampu= root["lampu"]; 
      Serial.print("Lampu:"); 
      Serial.println(datalampu);
      if(datalampu=="on"){
        digitalWrite(lampu,LOW);
        Status_Lampu="on";
        Serial.print("Lampu HIDUP"); 
         publish_data();
        }else{
        digitalWrite(lampu,HIGH);
        Status_Lampu="off";
        Serial.print("Lampu MATI");
         publish_data();
        }
    }else{
    http.end();
  }
  delay(1000);
   }
    void kontrol_kipas(){
    HTTPClient http;  
    http.begin(urlkipas);
    int httpCode = http.GET();                                                                 
    if (httpCode > 0) {
      const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(http.getString());
      String datakipas= root["kipas"]; 
      Serial.print("kipas:"); 
      Serial.println(datakipas);
      if(datakipas=="on"){
        digitalWrite(kipas,LOW);
        Status_Kipas="on";
        Serial.print("kipas HIDUP"); 
         publish_data();
        }else{
        digitalWrite(kipas,HIGH);
        Status_Kipas="off";
        Serial.print("kipas MATI");
         publish_data();
        }
    }else{
    http.end();
  }
  delay(1000);
   }



   
   void Triger(){
    if(sensors.getTempCByIndex(0)< 27){
      digitalWrite(lampu, LOW);
      Serial.println("Lampu Hidup");
      digitalWrite(kipas, HIGH);
      Serial.println("Kipas Mati");
      Status_Lampu="on";
      Status_Kipas="off";
    }else  if(sensors.getTempCByIndex(0)> 31){
      digitalWrite(lampu, HIGH);
      Serial.println("Lampu Mati");
      digitalWrite(kipas, LOW);
      Serial.println("Kipas Hidup");
      Status_Lampu="off";
      Status_Kipas="on";
    }
   }
