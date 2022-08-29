#include <ESP8266WiFi.h> //Libreria que hace la conexion de la placa a Wifi
#include <PubSubClient.h> //Libreria que hace que nuestra placa se comporte como un cliente MQTT
#include <Wire.h> //Libreria que permite comunicarse con dispositivos por bus I2C
#include <DHT.h> //Libreria para el uso del sensor DHT22
#include <DHT_U.h>

//Variables
int SENSOR = D2; //Declaración del pin digital 2 donde estará conectada la salida del sensor
DHT dht(SENSOR, DHT22); //Funcion necesaria para establecer el modelo de sensor DHT22
int relay = D4; //Declaración del pin digital 4 donde estará conectada la salida del módulo relé

//Configuración de red
//Datos de red Wi-Fi
const char* ssid = "RedCasa1"; //Nombre de la red
const char* password = "clavecasa123g"; //Contraseña de la red

//Datos MQTT Broker
const char* mqtt_server = "192.168.31.166"; //Puerto por donde se implementa el protocolo MQTT

//Datos user placa en Node-RED
const char *mqtt_user = "Gary"; //Usuario establecido para la placa ESP8266 usado en Node-RED
const char *mqtt_pass = "grupo6iot"; //Contraseña establecida para la placa ESP8266 usado en Node-RED

WiFiClient espClient; //Crea una clase WiFiClient para conectarse al servidor MQTT
PubSubClient client(espClient); //Crea una instancia de espClient parcialmente incializada
long lastMsg = 0;
char msg[25];
int value = 0;

float temperature = 0; //Variable para capturar el valor de la temperatura
int humedad = 0; //Variable para capturar el valor de la humedad

//Variables char creadas para enviar la información al topic en Node-RED
char tempCadena[10]; //Variable para mostrar el valor de la temperatura
char humeCadena[10]; //Variable para mostrar el valor de la humedad
char humeSuCadena[10]; //Variable para mostrar el valor del porcentaje de humedad en el suelo


void setup(){
  Serial.begin(115200); //Conexion a 115200 baudios respecto a la comunicacion de la placa
  pinMode(relay, OUTPUT); // Configurar relay como salida o OUTPUT
  dht.begin();//Inicio del sensor
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

//Función encargada de la comprobación de la conexión a Wi-Fi
void setup_wifi(){
  delay(10); //Espera una centesima de segundo
  //
  Serial.println(); //Imprime un espacion en blanco
  Serial.print("Conectando a:"); //Imprime el texto "Conectando a:"
  Serial.println(ssid); //Imprime el valor de la SSID (nombre de red a la cual se ha conectado la placa)

  WiFi.begin(ssid, password); //Conectar la placa a la red Wifi

  while(WiFi.status() !=  WL_CONNECTED){ //Bucle respecto al estado de la conexion
    delay(500); //Espera de medio segundo
    Serial.print("."); //Mensaje a consola
  }

  Serial.println(""); //Imprime un espacion en blanco
  Serial.println("WiFi conectado!"); //Imprime el texto "WiFi conectado!"
  Serial.println("Dirección IP:"); //Imprime el texto "Direccion IP:"
  Serial.println(WiFi.localIP()); //Imprime el valor de la direccion IP utilizando una funcion WiFi

}

void callback(char* topic, byte* message, unsigned int length){ //Esta función se llama cuando llegan nuevos mensajes al cliente
  Serial.print("Mensaje recibido al tópico"); //Imprime el texto "Mensaje recibido al topico"
  Serial.print(topic);
  Serial.print("Mensaje:"); //Imprime el texto "Mensaje: "
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
      digitalWrite(SENSOR, HIGH); //Activa el pin de salida
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(SENSOR, LOW); //Desactiva el pin de salida
    }
  }
}

void reconnect(){
  //Loop
  while(!client.connected()){ //Bucle respecto al estado de conexion del cliente
    Serial.print("Attempting MQTT connection..."); //Mensaje haciendo referencia a la conexion MQTT
    //Se intenta la conexión
    if(client.connect("ESP8266Client",mqtt_user, mqtt_pass)){ //Verifica si la conexion es exitosa
      Serial.println("connected"); //Imprime el texto "connected"
      //Suscribe
      client.subscribe("ESP8266/output");
    } else{
      Serial.print("fallido, por razón rc="); //Mensaje haciendo referencia a la conexion fallida
      Serial.print(client.state()); //Se llama al metodo state que devolvera un codigo con informacion sobre por que fallo la conexio
      Serial.print("Intente nuevamente en 5 segundos"); //Mensaje para intentar conectarse nuevamente
      delay(5000); //Pausa de 5 segundos
    }
  }
}

void loop(){
  if(!client.connected()){ //Verifica que el cliente este conectado
    reconnect(); //Llama a la funcion reconnect() si es que el cliente no esta conectado
  }
  client.loop(); //Funcion que sirve para mantener la comunicación
  
  digitalWrite(relay, HIGH); // envia señal alta al relay
  Serial.println("Relay accionado");
  delay(600);           // 0,6 segundos
  
  digitalWrite(relay, LOW);  // envia señal baja al relay
  Serial.println("Relay no accionado");
  delay(1000);           // 1 segundo
  
  long now=millis();//Declara la variable now, la cual almacena el numero de milisegundos desde que la placa Arduino empezo a ejecutar
  if(now - lastMsg > 5000){  //Verifica que tiempo de ejecucion hasta el momento sea mayor a 5 segundos
    lastMsg = now;
    
    //Valores del sensor DHT22
    temperature = dht.readTemperature();//Lectura de temperatura por DHT22
    humedad = dht.readHumidity(); //Lectura de humedad por DHT22
    
    //Valor del sensor de humedad de suelo
    int lectura = analogRead(A0); //Lectura por parte del ping analogico 0(sensor de humedad de suelo)
    int lecturaPorcentaje = map(lectura, 1023, 0, 0, 100); //Transforma el valor a porcentaje

    if(temperature>22 || lecturaPorcentaje<60){ //Condicional que establece cuándo prender el relay, en casos
      //la temperatura sea mayor de 22 grados o el sensor de humedad de suelo marque menos del 60% de HR suelo
      
      digitalWrite(relay, HIGH); // envia señal alta al relay
      Serial.println("Relay accionado");
      delay(600);           // 0,6 segundos
      
      digitalWrite(relay, LOW);  // envia señal baja al relay
      Serial.println("Relay no accionado");
      delay(1000);           // 1 segundo
      
    }

    //Transformar valores de los sensores a char
    sprintf(tempCadena, "%.2f", temperature); //Transforma el dato float temperature a cadena
    sprintf(humeCadena, "%d", humedad); //Transforma el dato int humedad a cadena
    sprintf(humeSuCadena, "%d", lecturaPorcentaje); //Transforma el dato int lecturaPorcentaje a cadena

    //Mostrar mensajes en consola
    Serial.print("\nTemperatura: "); //Imprime el texto "Temperatura: "
    Serial.println(temperature); //Muestra el valor de la temperatura
    Serial.print("Humedad: "); //Imprime el texto "Humedad: "
    Serial.println(humedad); //Muestra el valor de la humedad
    Serial.print("Humedad de suelo: "); //Imprime el texto "Humedad de suelo: "
    Serial.print(lecturaPorcentaje); //Muestra el valor en porcentaje de la humedad en el suelo
    Serial.println("%");

    //Enviar y publicar valores a NodeRed
    client.publish("ESP8266/Temperatura", tempCadena); //Publica el mensaje tempCadena en el topico ESP8266/Temperatura
    client.publish("ESP8266/Humedad", humeCadena); //Publica el mensaje humeCadena en el topico ESP8266/Humedad
    client.publish("ESP8266/HumedadSuelo", humeSuCadena); //Publica el mensaje humeSuCadena en el topico ESP8266/HumedadSuelo
    
  }
}
