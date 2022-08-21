#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <DHT.h>
#include <DHT_U.h>

//Variables
int SENSOR = D2;
DHT dht(SENSOR, DHT22);

//Configuración de red
const char* ssid = "RedCasa1";
const char* password = "clavecasa123g";

//Datos MQTT Broker
const char* mqtt_server = "192.168.31.166";

const char *mqtt_user = "Gary";
const char *mqtt_pass = "grupo6iot";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[25];
int value = 0;

float temperature = 0;
char tempCadena[10];

void setup(){
  Serial.begin(115200);
  dht.begin();//Inicio del sensor

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi(){
  delay(10);
  //
  Serial.println();
  Serial.print("Conectando a:");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while(WiFi.status() !=  WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado!");
  Serial.println("Dirección IP:");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length){
  Serial.print("Mensaje recibido al tópico");
  Serial.print(topic);
  Serial.print("Mensaje:");
  String messageTemp;

  for(int i = 0; i < length; i++){
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  //Cambie el estado
  if(String(topic)=="ESP8266/output"){
    Serial.print("Cambiando el output a:");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(SENSOR, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(SENSOR, LOW);
    }
  }
}

void reconnect(){
  //Loop
  while(!client.connected()){
    Serial.print("Attempting MQTT connection...");
    //Se intenta la conexión
    if(client.connect("ESP8266Client",mqtt_user, mqtt_pass)){
      Serial.println("connected");
      //Suscribe
      client.subscribe("ESP8266/output");
    } else{
      Serial.print("fallido, por razón rc=");
      Serial.print(client.state());
      Serial.print("Intente nuevamente en 5 segundos");
      delay(5000);
    }
  }
}

void loop(){
  if(!client.connected()){
    reconnect();
  }
  client.loop();

  long now=millis();
  if(now - lastMsg > 5000){
    lastMsg = now;

    temperature = dht.readTemperature();//Lee datos
    sprintf(tempCadena, "%.2f", temperature);
    Serial.print("\nTemperatura: ");
    Serial.print(temperature);
    client.publish("ESP8266/Temperatura", tempCadena);
  }
}
