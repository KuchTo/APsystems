/*
  ------------------------------------------------------------------------------
  ESP32 APsystems EZ1 Monitoring – by Tobias Kuch
  ------------------------------------------------------------------------------
  Dieses Projekt dient zur Abfrage eines APsystems EZ1 Balkonwechselrichters 
  über einen ESP32. Die Kommunikation erfolgt via HTTP-Requests an das lokale 
  WLAN-Netzwerk des Wechselrichters. Die erhaltenen JSON-Daten werden geparst 
  und aufbereitet ausgegeben.

  Verwendete Bibliotheken:
    - WiFi.h           (für WLAN-Verbindung)
    - HTTPClient.h     (für HTTP-Kommunikation)
    - ESP32Ping.h      (für Erreichbarkeitsprüfung)
    - ArduinoJson.h    (für das Parsen von JSON-Daten)
    - esp_wifi.h       (für WLAN-spezifische Funktionen)

  Autor   : Tobias Kuch
  Lizenz  : 
            Dieses Projekt steht unter einer angepassten Lizenz:

- **Namensnennung erforderlich:** Bei jeglicher Nutzung muss der Name _Tobias Kuch_ genannt und auf dieses Repository verwiesen werden.
- **Keine kommerzielle Nutzung:** Die Nutzung für kommerzielle Zwecke ist **nicht erlaubt**.
- **Keine Bearbeitungen:** Der Code darf **nicht verändert** und als abgewandelte Version weiterverbreitet werden – außer mit ausdrücklicher Zustimmung des Autors.

> Für Sondergenehmigungen oder kommerzielle Nutzung bitte den Autor kontaktieren.
  Datum   : 2025

  GitHub  : https://github.com/kuchto
  ------------------------------------------------------------------------------
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Ping.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>



// Konfigurationsstruktur
struct Config {
  bool debug = true;
};

Config config;

// WLAN-Zugang
const char* ssid = "**************";
const char* password = "**************";

// Inverter-Zugang
const char* InverterIP = "**************";
int InverterPort = 8050;

void debugPrint(const String& text) {
  if (config.debug) Serial.print(text);
}

void debugPrintln(const String& text) {
  if (config.debug) Serial.println(text);
}



bool checkPingToInverter(const char* InverterIP) {
  IPAddress inverterIP;

  if (WiFi.hostByName(InverterIP, inverterIP)) {
    debugPrint("🔁 Versuche Ping an ");
    debugPrintln(inverterIP.toString());

    const int maxRetries = 5;
    int attempts = 0;
    bool success = false;

    while (attempts < maxRetries && !success) {
      attempts++;
      debugPrintln("Ping-Versuch: "+ String(attempts));

      if (Ping.ping(inverterIP)) {
        debugPrintln("✅ Inverter Ping erfolgreich");
        success = true;
      } else {
        debugPrintln("⚠️ Inverter Ping fehlgeschlagen");
        delay(500);
      }
    }

    if (!success) {
      debugPrintln("❌ Alle Ping-Versuche fehlgeschlagen – Verbindung blockiert?");
    }

    return success;

  } else {
    debugPrintln("❌ Hostname konnte nicht aufgelöst werden!");
    return false;
  }
}


// Funktion: HTTP-Anfrage mit Retry-Logik
String sendInverterHttpRequest(String endpoint) {
  String url = String("http://") + InverterIP + ":" + InverterPort + endpoint;
  Serial.print("➡️ Anfrage an: ");
  Serial.println(url);

  WiFiClient client;
  HTTPClient http;
  String payload = "";
  int maxRetries = 5;
  int initialDelay = 100;
  int delayIncrement = 200;

  for (int attempt = 1; attempt <= maxRetries; attempt++) {
    if (http.begin(client, url)) {
      http.addHeader("Host", InverterIP);
      http.addHeader("Connection", "close");
      http.addHeader("Accept", "application/json");
      http.addHeader("User-Agent", "Mozilla/5.0 (ESP32)");
      http.setTimeout(15000);  // 15 Sekunden Timeout
      delay(initialDelay + attempt * delayIncrement);     
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.printf("✅ HTTP Code: %d\n", httpCode);
        payload = http.getString();
        http.end();
        return payload;
      } else {
        debugPrintln("❌ HTTP GET Anfrage fehlgeschlagen: Versuch " + String(attempt) + " von " + String(maxRetries) + " - Fehler: " + http.errorToString(httpCode).c_str());

      }

      http.end();
    } else {
      debugPrintln("❌ Verbindung konnte nicht aufgebaut werden (http.begin)");
    }
    delay(initialDelay + attempt * delayIncrement);  // Wartezeit zwischen Versuchen
    //delay(3000);  // Wartezeit zwischen Versuchen
  }

  return "";  // Keine Daten erhalten
}

// Funktion: /getDeviceInfo
bool get_InverterDeviceInfo(
  String &deviceId,
  String &devVer,
  String &ssid,
  String &ipAddr,
  float &minPower,
  float &maxPower
) {
  String response = sendInverterHttpRequest("/getDeviceInfo");

  if (response == "") {
    debugPrintln("⚠️ Keine Antwort von /getDeviceInfo");
    return false;
  }

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    debugPrint("❌ JSON-Fehler: ");
    debugPrintln(error.c_str());
    return false;
  }

  JsonObject data = doc["data"];

  // Werte extrahieren
  deviceId  = data["deviceId"].as<const char*>();
  devVer    = data["devVer"].as<const char*>();
  ssid      = data["ssid"].as<const char*>();
  ipAddr    = data["ipAddr"].as<const char*>();
  minPower  = data["minPower"].as<float>();
  maxPower  = data["maxPower"].as<float>();

  // Debug-Ausgabe
  debugPrintln("📊 Wechselrichter-Infos:");
  debugPrintln("Geräte-ID: " + deviceId);
  debugPrintln("Firmware  : " + devVer);
  debugPrintln("WLAN-SSID : " + ssid);
  debugPrintln("IP-Adresse: " + ipAddr);
  debugPrintln("Min. Power: " + String(minPower, 1) + " W");
  debugPrintln("Max. Power: " + String(maxPower, 1) + " W");

  return true;
}




bool get_InverterDeviceInfo_old() {
  String response = sendInverterHttpRequest("/getDeviceInfo");
  if (response == "") {
    debugPrintln("⚠️ Keine Antwort von /getDeviceInfo");
    return false;
  }

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    debugPrint("❌ JSON-Fehler: ");
    debugPrintln(error.c_str());
    return false;
  }

  JsonObject data = doc["data"];
debugPrintln("📊 Wechselrichter-Infos:");
debugPrintln("Geräte-ID: " + String(data["deviceId"].as<const char*>()));
debugPrintln("Firmware  : " + String(data["devVer"].as<const char*>()));
debugPrintln("WLAN-SSID : " + String(data["ssid"].as<const char*>()));
debugPrintln("IP-Adresse: " + String(data["ipAddr"].as<const char*>()));
debugPrintln("Min. Power: " + String(data["minPower"].as<float>(), 1) + " W");  // Direkt als float behandeln und formatieren
debugPrintln("Max. Power: " + String(data["maxPower"].as<float>(), 1) + " W");  // Gleiches für maxPower

  return true;
}



// Funktion: /getOutputData
bool getOutputData(float &p1, float &e1, float &te1, float &p2, float &e2, float &te2) {
  String response = sendInverterHttpRequest("/getOutputData");
  
  if (response == "") {
    debugPrintln("⚠️ Keine Antwort von /getOutputData");
    return false;
  }

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, response);
  
  if (error) {
    debugPrint("❌ JSON-Fehler: ");
    debugPrintln(error.c_str());
    return false;
  }

  String deviceId = doc["deviceId"].as<String>();
  String message = doc["message"].as<String>();
  JsonObject data = doc["data"];

  // Werte aus dem JSON ins Funktionsparameter-Feld schreiben
  p1 = data["p1"];
  e1 = data["e1"];
  te1 = data["te1"];
  p2 = data["p2"];
  e2 = data["e2"];
  te2 = data["te2"];

  debugPrintln("⚡ Aktuelle Output-Daten:");
  debugPrintln("📈 Kanal 1: " + String(p1, 1) + " W | e1: " + String(e1, 3) + " kWh | te1: " + String(te1, 3) + " kWh");
  debugPrintln("📉 Kanal 2: " + String(p2, 1) + " W | e2: " + String(e2, 3) + " kWh | te2: " + String(te2, 3) + " kWh");
  debugPrintln("🆔 Gerät-ID: " + deviceId);
  debugPrintln("📬 Status: " + message);

  return true;
}


bool get_InverterMaxPower(int &maxPower) {
  String response = sendInverterHttpRequest("/getMaxPower");
  
  if (response == "") {
    debugPrintln("⚠️ Keine Antwort von /getMaxPower");
    return false;
  }

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, response);
  
  if (error) {
    debugPrint("❌ JSON-Fehler: ");
    debugPrintln(error.c_str());
    return false;
  }

  String deviceId = doc["deviceId"].as<String>();
  String message = doc["message"].as<String>();
  
  // MaxPower als int auslesen
  maxPower = doc["data"]["maxPower"].as<int>();

  debugPrintln("⚡ Maximalleistung des Wechselrichters:");
  debugPrintln("🔋 Max Power: " + String(maxPower) + " W");
  debugPrintln("🆔 Gerät-ID: " + deviceId);
  debugPrintln("📬 Status: " + message);

  return true;
}


bool get_InverterMaxPower_old() {
  String response = sendInverterHttpRequest("/getMaxPower");
  if (response == "") {
    debugPrintln("⚠️ Keine Antwort von /getMaxPower");
    return false;
  }

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    debugPrint("❌ JSON-Fehler: ");
    debugPrintln(error.c_str());
    return false;
  }

  String deviceId = doc["deviceId"].as<String>();
  String message = doc["message"].as<String>();
  String maxPower = doc["data"]["maxPower"].as<String>();

  debugPrintln("⚡ Maximalleistung des Wechselrichters:");
  debugPrintln("🔋 Max Power: "+ maxPower );
  debugPrintln("🆔 Gerät-ID: " + deviceId);
  debugPrintln("📬 Status: " + message);

  return true;
}


bool getOnOff(bool &statusr) {
  String response = sendInverterHttpRequest("/getOnOff");
  
  if (response == "") {
    debugPrintln("⚠️ Keine Antwort von /getOnOff");
    return false;
  }

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, response);
  
  if (error) {
    debugPrint("❌ JSON-Fehler: ");
    debugPrintln(error.c_str());
    return false;
  }

  String deviceId = doc["deviceId"].as<String>();
  String message = doc["message"].as<String>();
  String status = doc["data"]["status"].as<String>();
  String statusText = (status == "0") ? "Eingeschaltet" : "Ausgeschaltet";

  // Umwandlung von "0" -> true und "1" -> false
  statusr = (status == "0");

  debugPrintln("🔌 Gerätestatus:");
  debugPrintln("🟢 Status: " + statusText);
  debugPrintln("🆔 Gerät-ID: " + deviceId);
  debugPrintln("📬 Status: " + message);

  return true;
}


bool get_InverterAlarmInfo(
  String &deviceId,
  String &message,
  bool &og,
  bool &oe,
  bool &isce1,
  bool &isce2
) {
  String response = sendInverterHttpRequest("/getAlarm");
  if (response == "") {
    debugPrintln("⚠️ Keine Antwort von /getAlarm");
    return false;
  }

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    debugPrint("❌ JSON-Fehler: ");
    debugPrintln(error.c_str());
    return false;
  }

  JsonObject data = doc["data"].as<JsonObject>();
  if (!data) {
    debugPrintln("❌ Ungültiges 'data'-Objekt in der Antwort");
    return false;
  }

  // Werte extrahieren
  deviceId = doc["deviceId"].as<String>();
  message  = doc["message"].as<String>();
  og    = data["og"].as<String>() == "1";
  oe    = data["oe"].as<String>() == "1";
  isce1 = data["isce1"].as<String>() == "1";
  isce2 = data["isce2"].as<String>() == "1";

  // Debug-Ausgabe
  debugPrintln("🚨 Alarminformationen:");
  debugPrintln("🔴 Off Grid Alarm: " + String(og    ? "Alarm" : "Normal"));
  debugPrintln("🔴 Off Energy Alarm: " + String(oe   ? "Alarm" : "Normal"));
  debugPrintln("⚡ DC 1 Short Circuit Error: " + String(isce1 ? "Alarm" : "Normal"));
  debugPrintln("⚡ DC 2 Short Circuit Error: " + String(isce2 ? "Alarm" : "Normal"));
  debugPrintln("🆔 Gerät-ID: " + deviceId);
  debugPrintln("📬 Status: " + message);

  return true;
}



bool get_InverterAlarmInfo_old () {
  String response = sendInverterHttpRequest("/getAlarm");
  if (response == "") {
    debugPrintln("⚠️ Keine Antwort von /getAlarm");
    return false;
  }

  // Überprüfen, ob die Antwort im richtigen Format ist
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    debugPrint("❌ JSON-Fehler: ");
    debugPrintln(error.c_str());
    return false;
  }

  // Sicherstellen, dass das "data"-Objekt vorhanden ist
  JsonObject data = doc["data"].as<JsonObject>();
  if (!data) {
    debugPrintln("❌ Ungültiges 'data'-Objekt in der Antwort");
    return false;
  }

  // Abrufen der Alarmdaten
  String deviceId = doc["deviceId"].as<String>();
  String message = doc["message"].as<String>();
  
  // Abrufen und Verarbeiten der Alarmstatuswerte
  String og = data["og"].as<String>();
  String oe = data["oe"].as<String>();
  String isce1 = data["isce1"].as<String>();
  String isce2 = data["isce2"].as<String>();

  // Konvertieren der Alarmstatuswerte
  String ogStatus = (og == "0") ? "Normal" : "Alarm";
  String oeStatus = (oe == "0") ? "Normal" : "Alarm";
  String isce1Status = (isce1 == "0") ? "Normal" : "Alarm";
  String isce2Status = (isce2 == "0") ? "Normal" : "Alarm";

  // Ausgabe der Alarminformationen
  debugPrintln("🚨 Alarminformationen:");
  debugPrintln("🔴 Off Grid Alarm: "+ ogStatus);
  debugPrintln("🔴 Off Energy Alarm: "+ oeStatus);
  debugPrintln("⚡ DC 1 Short Circuit Error: "+ isce1Status);
  debugPrintln("⚡ DC 2 Short Circuit Error: "+ isce2Status);
  debugPrintln("🆔 Gerät-ID: " + deviceId);
  debugPrintln("📬 Status: " + message);

  return true;
}



bool set_InverterDeviceOnOff(bool blstatus) {
  // Status setzen: '0' für Ein, '1' für Aus
  String status = blstatus ? "0" : "1";

  // Überprüfen, ob der Statuswert korrekt ist
  if (status != "0" && status != "1") {
    Serial.println("❌ Ungültiger Statuswert! Verwende '0' für Ein oder '1' für Aus.");
    return false;
  }

  // Baue die URL für die Anfrage
  String url = "/setOnOff?status=" + status;

  // Sende die HTTP-Anfrage
  String response = sendInverterHttpRequest(url);

  // Wenn keine Antwort erhalten wurde
  if (response == "") {
    Serial.println("⚠️ Keine Antwort von /setOnOff");
    return false;
  }

  // JSON-Dokument erstellen und deserialisieren
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.print("❌ JSON-Fehler: ");
    Serial.println(error.c_str());
    return false;
  }

  // Extrahiere Daten aus dem JSON
  String deviceId = doc["deviceId"] | "Unbekannt";
  String message = doc["message"] | "Keine Nachricht";
  String returnedStatus = doc["data"]["status"] | "-1";

  // Prüfe, ob die Operation erfolgreich war
  if (message != "SUCCESS") {
    Serial.println("❌ Gerät konnte nicht umgeschaltet werden.");
    Serial.println("📬 Antwortnachricht: " + message);
    return false;
  }

  // Status-Ausgabe
  String statusText = (returnedStatus == "0") ? "Eingeschaltet" : "Ausgeschaltet";
  Serial.println("✅ Gerätestatus geändert:");
  Serial.printf("🟢 Status: %s\n", statusText.c_str());
  Serial.println("🆔 Gerät-ID: " + deviceId);
  Serial.println("📬 Nachricht: " + message);

  return true;
}


bool set_InverterMaxPower(int maxPower) {
  // Überprüfen, ob der maxPower-Wert im Bereich von 0 bis 3600 liegt
  if (maxPower < 0 || maxPower > 3600) {
    Serial.println("❌ Ungültiger maxPower-Wert! Der Wert muss im Bereich von 0 bis 3600 liegen.");
    return false;
  }

  // Den Wert als String formatieren
  String maxPowerStr = String(maxPower);

  // Baue die URL für die Anfrage
  String url = "/setMaxPower?p=" + maxPowerStr;

  // Sende die HTTP-Anfrage
  String response = sendInverterHttpRequest(url);

  // Wenn keine Antwort erhalten wurde
  if (response == "") {
    Serial.println("⚠️ Keine Antwort von /setMaxPower");
    return false;
  }

  // JSON-Dokument erstellen und deserialisieren
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.print("❌ JSON-Fehler: ");
    Serial.println(error.c_str());
    return false;
  }

  // Werte aus dem JSON extrahieren
  String deviceId = doc["deviceId"] | "Unbekannt";
  String message = doc["message"] | "Keine Nachricht";
  String newMaxPower = doc["data"]["maxPower"] | "-1";

  // Prüfen, ob die Operation erfolgreich war
  if (message != "SUCCESS") {
    Serial.println("❌ Setzen der maximalen Leistung fehlgeschlagen.");
    Serial.println("📬 Antwortnachricht: " + message);
    return false;
  }

  // Ausgabe der neuen maximalen Leistung
  Serial.println("✅ Maximale Leistung geändert:");
  Serial.printf("🔋 Neue maxPower: %s W\n", newMaxPower.c_str());
  Serial.println("🆔 Gerät-ID: " + deviceId);
  Serial.println("📬 Nachricht: " + message);

  return true;
}


void setup() {
  Serial.begin(115200);
  delay(1000);

  uint8_t newMACAddress[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  esp_wifi_set_mac(WIFI_IF_STA, newMACAddress);

  WiFi.begin(ssid, password);
  debugPrint("🔌 Verbinde mit WLAN ");
  debugPrintln(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    debugPrint(".");
  }

  debugPrintln("\n✅ WLAN verbunden!");
  debugPrint("ESP32 IP: ");
  debugPrintln(WiFi.localIP().toString());

  IPAddress inverterIP;


  bool resultd = checkPingToInverter("192.168.1.3");
Serial.println(resultd);
  //delay(5000);

String deviceId, devVer, ssid, ipAddr;
float minPower, maxPowerfl;

if (get_InverterDeviceInfo(deviceId, devVer, ssid, ipAddr, minPower, maxPowerfl)) {
  Serial.println(deviceId);
  // Hier kannst du mit den Werten weiterarbeiten
}




  float p1, e1, te1, p2, e2, te2;

if (getOutputData(p1, e1, te1, p2, e2, te2)) {
  Serial.println("📊 Output-Daten erhalten:");
  Serial.println("📈 Kanal 1:");
  Serial.println("   Leistung      : " + String(p1, 1) + " W");
  Serial.println("   Energie (e1)  : " + String(e1, 3) + " kWh");
  Serial.println("   Gesamt (te1)  : " + String(te1, 3) + " kWh");
  
  Serial.println("📉 Kanal 2:");
  Serial.println("   Leistung      : " + String(p2, 1) + " W");
  Serial.println("   Energie (e2)  : " + String(e2, 3) + " kWh");
  Serial.println("   Gesamt (te2)  : " + String(te2, 3) + " kWh");
} else {
  Serial.println("❌ Fehler beim Abrufen der Output-Daten.");
}





  
  int maxPower;
if (get_InverterMaxPower(maxPower)) {
  Serial.println("Maximale Leistung: " + String(maxPower) + " W");
}
  
  bool status = false;
if (getOnOff(status)) {
  // Verwendet den Wert von 'status', der jetzt entweder true (eingeschaltet) oder false (ausgeschaltet) ist
  if (status) {
    debugPrintln("Auswertung ok! Geraet an");
  } else {
    debugPrintln("Auswertung ok! Geraet aus");
  }
}

String message;
bool og, oe, isce1, isce2;

if (get_InverterAlarmInfo(deviceId, message, og, oe, isce1, isce2)) {
  Serial.println(message);
   Serial.println(og);
   Serial.println(oe);
  // Hier kannst du mit den booleschen Werten weiterarbeiten
}




  set_InverterDeviceOnOff(true);
  set_InverterMaxPower(800);
}


void loop() {
  // Nichts zu tun
}


