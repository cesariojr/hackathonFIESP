/*
   19/07/2016
   Jose Maria Cesario Jr
   Exemplo IBM Watson IoT Platform
   Hardware: new NodeMCU LoLin V3 + BMP180

   Logica:
   1. efetua conexao com a rede WiFi
   2. obtem as grandezas de temperatura, pressao e altitude do sensor BMP180
   3. conecta no servidor MQTT quickstart do IBM Watson IoT Platform
   4. publica a JSON string para o topico quickstart:MAC_ADDRESS

   referencias Bluemix e IoTF:
   Author: Ant Elder
   https://developer.ibm.com/recipes/tutorials/connect-an-esp8266-with-the-arduino-sdk-to-the-ibm-iot-foundation/

   referencias conversao float para string
   http://www.hobbytronics.co.uk/arduino-float-vars
   http://forum.carriots.com/index.php/topic/61-wireless-gardening-with-arduino-cc3000-wifi-modules/page-2

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>


//atualize SSID e senha WiFi
const char* ssid = "HACKATHON";
const char* password = "hackathon@2016";

//d1 5 d2 4
#define TRIGGER 5
#define ECHO    4

//d5 14 d6 12
#define TRIGGER2 14
#define ECHO2    12

//D4 only for Lolin board
#define LED_BUILTIN D4

//Atualize os valores de Org, device type, device id e token
#define ORG "7axzsw"
#define DEVICE_TYPE "porta"
#define DEVICE_ID "linha_0001-onibus_0001-porta_01"
#define TOKEN "linha_0001-onibus_0001-porta_01"

//broker messagesight server
char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char topic[] = "iot-2/evt/status/fmt/json";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

WiFiClient wifiClient;
PubSubClient client(server, 1883, NULL, wifiClient);

int estado = 0;
int limit = 30;
boolean sensor01Acionado = false;
boolean sensor02Acionado = false;

int fora = 0;
int entrandoFora = 1;
int entrandoDentro = 2;
int saindoDentro = 3;
int saindoFora = 4;
int dentro = 0;
const String bus_id = "11394";
const String line_id = "35050";

String direcao;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Iniciando...");

  Serial.print("Conectando na rede WiFi "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("[INFO] Conectado WiFi IP: ");
  Serial.println(WiFi.localIP());

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(TRIGGER2, OUTPUT);
  pinMode(ECHO2, INPUT);

  if (!!!client.connected()) {
    Serial.print("Reconnecting client to ");
    Serial.println(server);
    while (!!!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    Serial.println();
  }

}

void loop() {

  int length = 0;
      
  if (lerSensor01() < limit) {
    sensor01Acionado = true;
  }
  else sensor01Acionado = false;
  //Serial.print (lerSensor01());

  if (lerSensor02() < limit) {
    sensor02Acionado = true;
  }
  else  sensor02Acionado = false;
  //Serial.print (lerSensor02());

  //---------------------------------------------------------------
  if (estado == fora) {
    if (sensor01Acionado and sensor02Acionado) {
      Serial.println("Sensores bloqueados fora");
    }

    else if (sensor01Acionado and !sensor02Acionado) {
      Serial.println("entrando fora");
      estado = entrandoFora;
    }

    else if (!sensor01Acionado and sensor02Acionado) {
      Serial.println("saindo dentro");
      estado = saindoDentro;
    }

    else if (!sensor01Acionado and !sensor02Acionado) {
      //Serial.println("sensores nao acionados");
      estado = fora;
    }
  }

  else if (estado == entrandoFora) {
    if (sensor01Acionado and sensor02Acionado) {
      Serial.println("Sensores bloqueados entrando fora");
    }

    else if (sensor01Acionado and !sensor02Acionado) {
      Serial.println("entrando fora");
      estado = entrandoFora;
    }

    else if (!sensor01Acionado and sensor02Acionado) {
      Serial.println("entrando dentro");
      estado = entrandoDentro;
    }

    else if (!sensor01Acionado and !sensor02Acionado) {
      //Serial.println("sensores nao acionados");
      estado = 0;
    }
  }

  //---------------------------------------------------------------
  if (estado == entrandoDentro) {
    if (sensor01Acionado and sensor02Acionado) {
      Serial.println("entrando fora");
      estado = entrandoFora;
    }

    else if (sensor01Acionado and !sensor02Acionado) {
      Serial.println("entrando fora");
      estado = entrandoFora;
    }

    else if (!sensor01Acionado and sensor02Acionado) {
      Serial.println("entrando dentro");
      estado = entrandoDentro;
    }

    else if (!sensor01Acionado and !sensor02Acionado) {
      Serial.println("****************** DENTRO *************************");
      Serial.println("****************** DENTRO *************************");
      Serial.println("****************** DENTRO *************************");
      estado = dentro;
      direcao = "entrando";
      
      // Prepara JSON para IOT Platform

      String payload =  "{\"d\":{\"direcao\": \"" + String(direcao) + "\",\"bus_id\":\"" + String(bus_id) + "\",\"line_id\":\"" + String(line_id) + "\"}}";

      length = payload.length();
      Serial.print(F("\nData length "));
      Serial.println(length);

      Serial.print("Sending payload: ");
      Serial.println(payload);

      if (client.publish(topic, (char*) payload.c_str())) {
        Serial.println("Publish ok");
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
      } else {
        Serial.println("Publish failed");
      }
    }
  }

  //---------------------------------------------------------------
  //---------------------------------------------------------------
  if (estado == saindoDentro) {
    if (sensor01Acionado and sensor02Acionado) {
      Serial.println("Sensores bloqueados estado 0");
    }

    else if (!sensor01Acionado and sensor02Acionado) {
      Serial.println("saindo dentro");
      estado = saindoDentro;
    }

    else if (sensor01Acionado and !sensor02Acionado) {
      Serial.println("saindo fora");
      estado = saindoFora;
    }

    else if (!sensor01Acionado and !sensor02Acionado) {
      Serial.println("sensores nao acionados - voltou p/ dentro do bus");
      estado = dentro;
    }
  }

  //---------------------------------------------------------------
  //---------------------------------------------------------------
  //---------------------------------------------------------------
  if (estado == saindoFora) {
    if (sensor01Acionado and sensor02Acionado) {
      Serial.println("saindo dentro");
      estado = saindoDentro;
    }

    else if (!sensor01Acionado and sensor02Acionado) {
      Serial.println("saindo dentro");
      estado = saindoDentro;
    }

    else if (sensor01Acionado and !sensor02Acionado) {
      Serial.println("saindo fora");
      estado = saindoFora;
    }

    else if (!sensor01Acionado and !sensor02Acionado) {
      Serial.println("*********************  SAIU DO BUS ********************");
      Serial.println("*********************  SAIU DO BUS ********************");
      Serial.println("*********************  SAIU DO BUS ********************");
      estado = fora;
      direcao = "saindo";
            // Prepara JSON para IOT Platform

      String payload =  "{\"d\":{\"direcao\": \"" + String(direcao) + "\",\"bus_id\":\"" + String(bus_id) + "\",\"line_id\":\"" + String(line_id) + "\"}}";

      length = payload.length();
      Serial.print(F("\nData length "));
      Serial.println(length);

      Serial.print("Sending payload: ");
      Serial.println(payload);

      if (client.publish(topic, (char*) payload.c_str())) {
        Serial.println("Publish ok");
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
      } else {
        Serial.println("Publish failed");
      }
    }
  }



}

int lerSensor01() {
  //SENSOR 01 ******************************************************
  long duration, distance;
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIGGER, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = (duration / 2) / 29.1;

  return distance;
}

int lerSensor02() {
  //SENSOR 02 ******************************************************
  long duration, distance;
  digitalWrite(TRIGGER2, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER2, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIGGER2, LOW);
  duration = pulseIn(ECHO2, HIGH);
  distance = (duration / 2) / 29.1;

  return distance;
}


