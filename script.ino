//Bibliothek einbinden
#include <Adafruit_NeoPixel.h> //Bibliothek für LED Ring (benötigt)
#include <ArduinoJson.h> //Bibliothek für JSON (Nachrichten & MQTT) (benötigt)
#include <WiFi.h> //(benötigt)
#include <HTTPClient.h> //(benötigt)
#include <ESPAsyncWebServer.h> //Bibliothek für WebServer zur Einsicht der Wertungen
#include <WebSerial.h> //Bibliothek für Webserver Oberfläche zur Einsicht der Wertungen
#include <PubSubClient.h> //Bibliothek für MQTT

// Allgemeine Einstellungen vornhemen

// LEDs und Steuerungspin festlegen
int leds = 16; //Anzahl der LEDs
int ledPin = 15; //GPIO Pin, an dem der NeoPixel auf dem ESP32 Board angeschlossen ist

//Farben festlegen (Immer zwei Farben sind zusammengefasst. So ensteht aktuell eine 7er Skala. Alternativ kann auch eine 11er Skala gewählt werden. Dazu müssen die Farbwerte angepasst werden
int minus5[] = {168, 0, 0};
int minus4[] = {251, 100, 0}; //11er Skala: 255, 80, 0
int minus3[] = {251, 100, 0};
int minus2[] = {255, 196, 0};
int minus1[] = {255, 196, 0}; //11er Skala: 0, 255, 230
int neutral[] = {98, 186, 39};
int plus1[] = {51, 66, 196}; //11er Skala: 0, 100, 255
int plus2[] = {51, 66, 196};
int plus3[] = {147, 98, 196}; //11er Skala: 150, 0, 255
int plus4[] = {147, 98, 196}; //11er Skala: 220, 0, 255
int plus5[] = {255, 0, 128};

//NeoPixel als "pixels" instanziieren
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(leds, ledPin, NEO_GRB + NEO_KHZ800);

//WLAN Einstellungen vornehmen
const char* ssid = "WLAN-SSID";
const char* password =  "WLAN-PASSWORD";

//WiFiClienten instanziieren
WiFiClient wifiClient;

//Netwerkschnittelle für MQTT auf Wifi festlegen
PubSubClient mqttClient(wifiClient);

//MQTT Verbindungseinstellungen vornehmen
const char* clientID = "Moodlicht";
const char* mqtt_server = "MQTT-IP";
const int mqtt_port = 1883;
const char* mqtt_user = "MQTT_USER";
const char* mqtt_password = "MQTT-PASSWORD";

//MQTT Nachricht Zwischenspeichergröße definieren
char out[384];

//Grundwert der Zustandsanalyse festlegen
String article;
int sentiment = 0;

//Webserver für Einsicht der Bewertungen starten
AsyncWebServer server(80);

}

//Funktion MQTT Serververbindung (WERTE anpassen). Wird später im Loop aufgerufen.
void connectToMQTT() {
 mqttClient.setServer("MQTT-IP", 1883);//MQTT Server, - Port
  mqttClient.setBufferSize(512);
  if (mqttClient.connect("Moodlicht" , "MQTT-USER", "MQTT-PASSWORD")) {
    Serial.println("MQTT verbunden");
    WebSerial.println("MQTT verbunden");
  }

//Funktion für die Bewertung der Nachrichten. Wird später im Loop aufgerufen.
void getSentiment() {
  sentiment = 0;
  HTTPClient http;
  WiFiClient clientt;
  http.begin(clientt, "http://api.meaningcloud.com/sentiment-2.1&key=API-TOKEN&of=json&lang=de&txt=" + article);


  int httpCode = http.GET();

  if (httpCode == 200) {

    String payload = http.getString();

    const size_t capacity = 2*JSON_ARRAY_SIZE(0) + 7*JSON_ARRAY_SIZE(1) + 5*JSON_ARRAY_SIZE(2) + 2*JSON_ARRAY_SIZE(3) + JSON_ARRAY_SIZE(4) + 2*JSON_ARRAY_SIZE(5) + 2*JSON_ARRAY_SIZE(6) + JSON_ARRAY_SIZE(7) + 25*JSON_OBJECT_SIZE(4) + 4*JSON_OBJECT_SIZE(6) + 22*JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(8) + 2*JSON_OBJECT_SIZE(9) + 3*JSON_OBJECT_SIZE(10) + JSON_OBJECT_SIZE(11) + 3500;

    DynamicJsonDocument doc(capacity);

    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::NestingLimit(11));

    if (error) {
      Serial.print(F("deserializeJson() Error: "));
      Serial.println(error.c_str());
      return;
    }

    JsonObject status = doc["status"];
    String score_tag = doc["score_tag"];

    Serial.println(score_tag);
    WebSerial.println("Analyse: " + score_tag);
   
//Punkte für Bewertungen festlegen
    if (score_tag == "P+") {
      sentiment += 3;
    }
    else if (score_tag == "P") {
      sentiment += 2;
    }
    else if (score_tag == "N") {
      sentiment -= 1;
    }
    else if (score_tag == "N+") {
      sentiment -= 2;
    }
  }
  
  else {
    Serial.println("Bad HTTP Request");
  }

  http.end();
}

//Allgemeine Einstellungen anwenden
void setup() {
 
  pixels.begin();
  pixels.setBrightness(255);

  pinMode (15, OUTPUT); //LED Pin
 
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Verbinde...");
  }
  Serial.println("Verbunden!");
  	
  WebSerial.begin(&server);
  server.begin();

 //MQTT Nachricht definieren
  StaticJsonDocument<384> doc;

  doc["name"] = "Zustand der Welt";
  doc["state_topic"] = "Moodlicht/zustandderwelt";
  doc["force_update"] = true;
  doc["availability_topic"] = "Moodlicht/status";
  doc["payload_available"] = "online";
  doc["payload_not_available"] = "offline";
  doc["unique_id"] = "moodlicht1";

  JsonObject device = doc.createNestedObject("device");
  device["manufacturer"] = "Simon Luthe";
  device["model"] = "Moodlicht";

  JsonArray device_connections_0 = device["connections"].createNestedArray();
  device_connections_0.add("mac");
  device_connections_0.add("7C9EBD371680");
  
  serializeJson(doc, out);
  Serial.println(out);
}

 //Wiederkehrende Funktion. Alles was hier steht wird alle 30 Minuten durchgeführt.
void loop() {

  if ((WiFi.status() == WL_CONNECTED)) {

    HTTPClient http;
    WiFiClient clientt;

  http.begin(clientt, "http://newsapi.org/v2/top-headlines?country=de&apiKey=API_TOKEN");

    int httpCode = http.GET();

    if (httpCode == 200) {
      
      String payload = http.getString();

      const size_t capacity = JSON_ARRAY_SIZE(20) + 20 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 20 * JSON_OBJECT_SIZE(8) + 16600;
      DynamicJsonDocument doc(capacity);

      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print(F("deserializeJson() Error: "));
        Serial.println(error.c_str());
        return;
      }

      //Anzahl der Überschriften speichern
      int noHeadlines = doc["totalResults"];

      //Durch Überschriften loopen und Wertung abrufen
      for (int i = 0; i < 15; i++) {        
        //Überschriften von NewsAPI holen
        JsonArray articles = doc["articles"];

        JsonObject articles_number = articles[i];

        const char* articles_number_title = articles_number["title"];

        article = String(articles_number_title);
        Serial.println(article);
        WebSerial.println(article);
        
        article.replace(" ", "%20");
        Serial.println(article);
        getSentiment();
      }
      Serial.println("Weltanalyse beendet.");
      WebSerial.println("Weltanalyse beendet.");
      Serial.println(sentiment);
      WebSerial.println("Zustand der Welt:");
      WebSerial.println(sentiment);
     
      //MQTT Geräte verbinden und Standardwerte schicken
      connectToMQTT();
      mqttClient.publish("Moodlicht/status", "online", true);
      mqttClient.publish("homeassistant/sensor/Moodlicht/zustandderwelt/config", out, true);
      
      //Ergebnis der Analyse via LED und MQTT ausgeben
      if (sentiment <= -5) {
        for (int i = 0; i < 16; i++) {
          pixels.setPixelColor(i, minus5[0], minus5[1], minus5[2]);
          pixels.setBrightness(255);
          pixels.show();
          //MQTT Zustand der Welt Wert schicken
          mqttClient.publish("Moodlicht/zustandderwelt", "-5", true);
        }
      } else if (sentiment >= 5) {
        for (int i = 0; i < 16; i++) {
          pixels.setPixelColor(i, plus5[0], plus5[1], plus5[2]);
          pixels.setBrightness(255);
          pixels.show();
         //MQTT Zustand der Welt Wert schicken
          mqttClient.publish("Moodlicht/zustandderwelt", "5", true);
        }
      }

      switch (sentiment) {
        case -4:
          for (int i = 0; i < 16; i++) {
            pixels.setPixelColor(i, minus4[0], minus4[1], minus4[2]);
            pixels.setBrightness(255);
            pixels.show();
           //MQTT Zustand der Welt Wert schicken
            mqttClient.publish("Moodlicht/zustandderwelt", "-4", true);
          }
          break;
        case -3:
          for (int i = 0; i < 16; i++) {
            pixels.setPixelColor(i, minus3[0], minus3[1], minus3[2]);
            pixels.show();
           //MQTT Zustand der Welt Wert schicken
            mqttClient.publish("Moodlicht/zustandderwelt", "-3", true);
          }
          break;
        case -2:
          for (int i = 0; i < 16; i++) {
            pixels.setPixelColor(i, minus2[0], minus2[1], minus2[2]);
            pixels.setBrightness(255);
            pixels.show();
           //MQTT Zustand der Welt Wert schicken
            mqttClient.publish("Moodlicht/zustandderwelt", "-2", true);
          }
          break;
        case -1:
          for (int i = 0; i < 16; i++) {
            pixels.setPixelColor(i, minus1[0], minus1[1], minus1[2]);
            pixels.setBrightness(255);
            pixels.show();
           //MQTT Zustand der Welt Wert schicken
            mqttClient.publish("Moodlicht/zustandderwelt", "-1", true);
          }
          break;
        case 0:
          for (int i = 0; i < 16; i++) {
            pixels.setPixelColor(i, neutral[0], neutral[1], neutral[2]);
            pixels.setBrightness(255);
            pixels.show();
           //MQTT Zustand der Welt Wert schicken
            mqttClient.publish("Moodlicht/zustandderwelt", "0", true);
          }
          break;
        case 1:
          for (int i = 0; i < 16; i++) {
            pixels.setPixelColor(i, plus1[0], plus1[1], plus1[2]);
            pixels.setBrightness(255);
            pixels.show();
           //MQTT Zustand der Welt Wert schicken
            mqttClient.publish("Moodlicht/zustandderwelt", "1", true);
          }
          break;
        case 2:
          for (int i = 0; i < 16; i++) {
            pixels.setPixelColor(i, plus2[0], plus2[1], plus2[2]);
            pixels.setBrightness(255);
            pixels.show();
           //MQTT Zustand der Welt Wert schicken
            mqttClient.publish("Moodlicht/zustandderwelt", "2", true);
          }
          break;
        case 3:
          for (int i = 0; i < 16; i++) {
            pixels.setPixelColor(i, plus3[0], plus3[1], plus3[2]);
            pixels.setBrightness(255);
            pixels.show();
           //MQTT Zustand der Welt Wert schicken
            mqttClient.publish("Moodlicht/zustandderwelt", "3", true);
          }
          break;
        case 4:
          for (int i = 0; i < 16; i++) {
            pixels.setPixelColor(i, plus4[0], plus4[1], plus4[2]);
            pixels.setBrightness(255);
            pixels.show();
           //MQTT Zustand der Welt Wert schicken
            mqttClient.publish("Moodlicht/zustandderwelt", "4"), true;
          }
          break;
      }
    }

    else {
      Serial.println("Bad HTTP Request");
      WebSerial.println("Bad HTTP Request");
    }

    http.end();
    Serial.println("Connection closed.");
    WebSerial.println("Verbindung beendet.");
  }
  //30 Minuten warten und neu holen
  delay(1800000);
}
