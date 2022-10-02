#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#define rxPin 14
#define txPin 12
 
SoftwareSerial jsnSerial(rxPin, txPin);

const char* ssid = "tajna_24";
const char* password = "michalkowokosakowo";
const char* mqtt_server = "192.168.0.100";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
unsigned int distance;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduino", "homeassistant", "naew6uaQuael8roh5Thu3iewei5yeep4ooDoo4Tuig2eidoo4kee8aht3Thoh3ah")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world" );
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void getDistance(){
  
  byte startByte, h_data, l_data, sum = 0;
  byte buf[3];
  
  startByte = (byte)jsnSerial.read();
  if(startByte == 255){
    jsnSerial.readBytes(buf, 3);
    h_data = buf[0];
    l_data = buf[1];
    sum = buf[2];
    distance = (h_data<<8) + l_data;
    if((( h_data + l_data)&0xFF) != sum){
      Serial.println("Invalid result");
    }
    else{
      Serial.print("Distance [mm]: "); 
      Serial.println(distance);
    } 
  } 
  else return;
}

void setup() {
  jsnSerial.begin(9600);
  Serial.begin(9600);
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}
 
void loop() {
    jsnSerial.write(0x01);
    delay(50);
  if(jsnSerial.available()){
    getDistance();
  }
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "%ld", distance);
    Serial.print("Publish message: ");
    Serial.println(msg);
    Serial.print("Odczytana wartość: ");
    Serial.println(distance);
    client.publish("outTopic", msg);
    delay(50);
  }
  ESP.deepSleep(30e6);
}
