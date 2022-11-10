# Segenszustand der Welt
Die Lampe zeigt den Segenszustand der Welt.<br />
Wir glauben, dass Sie dabei helfen kann aufmerksam zu werden für die guten Nachrichten die viel zu häufig keine Beachtung finden. Dabei bleibt es den Nutzer:innen verschlossen welche Nachrichten auf welche Weise bewertet wurde. Lediglich die Farbe gibt einen Hinweis auf den aktuellen Zustand. Es ist den Nutzer:innen freigestellt selber nach Nachrichten zu suchen oder nicht. Die Lampe ist als ein andauernder Begleiter konzipiert.<br />
Segen wird so sichtbar ohne explizit zu machen wann, wo und wie etwas Gutes oder Schlechtes passiert ist.

## Funktionsweise
Die Lampe ruft via [NewsAPI.org](newsapi.org) die neusten *Schlagzeile* aus Deutschland hab. [NewsAPI.org](newsapi.org) greift dabei insgesamt auf mehr als 50 Webseiten zu und ließt nie aktuellste *Schlagzeile* jeder Seite aus. Die erste *25 Schlagzeilen* werden mittels Sentimentanalyse via [meaningcloud.com](https://www.meaningcloud.com/developer/sentiment-analysis) bewertet. Den *Schlagzeilen* werden dabei die Werte von -2, -1, 0, 1, 2 zugewiesen. Diese Werte werden miteinander addiert und so der aktuelle Wert für den Segenszustand der Welt berechnet. Der so errechnete Wert wird via 7er oder 11er Farbskala an den LED Ring ausgegeben. Dieser Vorgang wiederholt sich alle 30 Minuten. <br />
Um den Segenszustand der Welt als Statistik ausgegeben zu können wird der Wert via MQTT ausgegeben. Dazu muss ein MQTT Broker (https://mosquitto.org/) im Netzwerk verfügbar sein. Dieser Wert kann dann via Home Assistant(https://www.home-assistant.io/) ausgegeben werden.<br />
Um die *Schlagzeilen* und den Vorgang der Bewertung einsehen zu können sind die Log Dateien via [WebSerial](https://www.arduino.cc/reference/en/libraries/webserial/) im Browser auslesbar. Auslesen ist nur möglich im Moment der Datenabfrage.

# Installation
## Skripte
Es sind vier Skripte für die Lampe mit unterschiedlichem Funktionsumfang vorhanden:
1. [Lampe inkl. MQTT und Webserial](/lampe-mqtt-webserial.ino)
2. [Lampe inkl. Webserial](/lampe-webserial.ino)
3. [Lampe inkl. MQTT](/lampe-mqtt.ino)
4. [Lampe](/lampe.ino)

## Hardware
* ESP32
* NeoPixel Ring
* Mini USB Kabel und Stecker

## Zusammenbau der Hardware
* Verbinden Sie den NeoPixel Ring mit dem ESP32 Board
* Schließen Sie den 5V Anschluss an VIN an
* Schließen Sie GND an GND an
* Schließen Sie den DIN Anschluss an GPIO 15 an. Wenn Sie einen anderen GPIO Pin verwenden müssen Sie dies im Code anpassen

## Schaltplan
![Schaltplan](docs/schaltplan.png)

## Installation der Software
*  [Arduino IDE](https://www.arduino.cc/en/software) installieren
* Verbinden Sie das ESP32 Board via USB mit Ihrem Computer
* Starten Sie die Arduino IDE
* Wählen Sie das Board aus:
	* Klicken Sie oben in der Leiste auf die Auswahlbox und wählen: `Select other board an port...`
	* Suchen Sie im Bereich Boards nach FireBeetle-ESP32
	* Wählen Sie im Bereich Ports das angeschlossene Board aus
	* Bestätigen Sie mit OK.
* Das Board ist nun mit Arduino IDE verbunden.
* Löschen Sie alles was aktuell im Skript steht.
* Kopieren Sie den Inhalt des von ihnen gewünschten Skriptes von GitHub und fügen es ein.
	* Passen Sie das Skript an:
		* WLAN (notwendig)
		* NewsAPI und Meaningcloud API Keys (notwendig)
		* LED Anzahl anpassen (notwendig)
		* MQTT (wenn gewünscht)
* Drücken Sie oben links auf Upload
* Damit der Upload startet halten Sie am ESP32 Board den Knopf `boot` gedrückt bis der Vorgang auf dem Rechner startet.
* Wenn der Upload erfolgreich war startet der Vorgang der Analyse.
	* Diesen können Sie im unteren Drittel unter dem Reiter `Serial Log` einsehen.

## Anpassungen vornehmen
### Anzahl LEDs
Sofern Ihr LED Ring nicht aus 16 LEDs besteht müssen Sie diesen Wert im Skript anpassen. <br />
* Suchen Sie nach: `int leds = 16;`.<br />
* Passen Sie diesen Wert an die Anzahl der LEDs an.

### GPIO Anschluss
* Suchen Sie nach: `int ledPin = 15; //GPIO Pin`.<br />
* Tragen Sie hier die Nummer des GPIO Pin ein an den Sie den NeoPixel Ring angelschlossen haben.

### WLAN Einstellungen
* Suchen Sie nach:
`const char* ssid = "WLAN-SSID";
const char* password =  "WLAN-PASSWORD";`
* Tragen Sie den Namen Ihres WLAN und das Passwort ein.

### Meaningcloud API
* Erstellen Sie einen [kostenlosen Account](https://www.meaningcloud.com/developer/create-account) bei meaningcloud.com oder loggen Sie sich mit einem [bestehenden Account](https://www.meaningcloud.com/developer/login) ein.
* Loggen Sie sich ein und holen Sie sich ihren API Schlüssel z.B. über diesen Link. Sie finden Ihren API Schlüssel im Feld Key. (https://learn.meaningcloud.com/developer/sentiment-analysis/2.1/console)
* Suchen Sie im Skript nach: `http.begin(clientt, "http://api.meaningcloud.com/sentiment-2.1&key=API-TOKEN&of=json&lang=de&txt=" + article);`
* Ersetzen Sie API-TOKEN mit Ihrem Key.

### newsAPI.org API
* Erstellen Sie einen [kostenlosen Account](https://newsapi.org/register) bei newsapi.org.
* Loggen Sie sich ein und holen Sie sich ihren API Schlüssel unter (https://newsapi.org/account). Sie finden den Schlüssel unter API key.
* Suchen Sie im Skript nach: `http.begin(clientt, "http://newsapi.org/v2/top-headlines?country=de&apiKey=API_TOKEN");`
* Ersetzen Sie API-TOKEN mit Ihrem API key.

## Option: Farben anpassen
* Suchen Sie nach: `int minus5[] = {168, 0, 0};`
* Hier können Sie die Farben für die jeweiligen Werte anpassen.
* Sie müssen die Werte als RGB Werte eintragen. Dafür können Sie diesen Farbwähler verwenden und tragen Sie die Werte im Format: `{ROT, GELB, GRÜN}` ein. (https://colorpicker.fr/app/)

### Option: MQTT
#### MQTT Broker verbinden
* Suchen Sie im Code nach: `//MQTT Verbindungseinstellungen vornehmen`
* Passen Sie nachfolgend die Werte für: IP, PORT, USER, PASSWORT an.
* Suchen Sie im Code nach: `//Funktion MQTT Serververbindung (WERTE anpassen). Wird später im Loop aufgerufen.
`
* Passen Sie nachfolgend die Werte für: IP, PORT, USER, PASSWORT an.

#### MQTT Nachricht Anpassungen
Folgende Informationen werden an MQTT geschickt und können angepasst werden.

```
{
  "name": "Segenszustand der Welt",
  "state_topic": "Moodlicht/segenszustandderwelt",
  "force_update": true,
  "availability_topic": "Moodlicht/status",
  "payload_available": "online",
  "payload_not_available": "offline",
  "unique_id": "moodlicht1",
  "device": {
    "manufacturer": "Simon Luthe",
    "model": "Moodlicht",
    "connections": [
      [
        "mac",
        "7C9EBD371680"
      ]
    ]
  }
}
```

Wenn Sie Änderungen an dieser MQTT Nachricht vornehmen muss diese ins JSON Format serialisiert werden. Dabei hilft diese Seite.

https://arduinojson.org/v6/assistant/#/step1

- Im ersten Schritt muss Prozessor, Mode und Output type definiert werden.
	- Prozessor: `ESP32`
	- Mode: `Serialize`
	- Output: `char[N]`

- Im zweiten Schritt muss die angepasste Nachricht eingefügt werden.
- Im dritten Schritt wird die erforderliche Größe der Nachricht in bytes angegeben. Dieser Wert ist notwendig und beträgt bei der vorgeschlagene Nachricht 384.
- Im vierten Schritt wird der Programmcode für die MQTT Nachricht ausgegeben und kann an der richtigen Stelle im Dokument eingefügt werden.
- Suchen Sie im Code nach: `char out[384];`
- Passen Sie diesen Wert an den Wert aus Schritt 3 an.
- Suchen Sie im Code nach ` //MQTT Nachricht definieren`
- Ersetzten Sie den nachfolgenden Code durch den Wert aus Schritt 4

## Option: WebSerial
Mittels IP-Adresse kann die Webansicht der Analyse im Browser aufgerufen werden.
