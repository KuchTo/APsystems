# 🔌 ESP32 APsystems EZ1 Monitoring

Dieses Projekt ermöglicht die direkte Kommunikation mit einem **APsystems EZ1 Balkonwechselrichter** über einen **ESP32**. Es wird eine WLAN-Verbindung zum Wechselrichter aufgebaut, und per HTTP-Requests werden aktuelle Leistungsdaten, Statusinformationen und Alarmmeldungen abgefragt.

## 🧠 Funktionsumfang

- Verbindung zum lokalen WLAN des Wechselrichters
- Abfrage der aktuellen Ausgangsdaten (Leistung, Energie)
- Anzeige von Ein-/Ausschaltstatus
- Anzeige der Geräteeigenschaften (Firmware, IP, SSID, Min/Max Power)
- Auswertung von Alarminformationen (z. B. Kurzschluss, Netzprobleme)
- JSON-Verarbeitung mittels `ArduinoJson`

## 📦 Voraussetzungen

- ESP32 (z. B. DevKitC, NodeMCU ESP32)
- Arduino IDE oder PlatformIO
- WLAN-Zugangsdaten des Wechselrichters

## 📚 Verwendete Bibliotheken

- [`WiFi.h`](https://github.com/espressif/arduino-esp32) – WLAN-Anbindung
- [`HTTPClient.h`](https://github.com/espressif/arduino-esp32) – HTTP-Kommunikation
- [`ESP32Ping.h`](https://github.com/marian-craciunescu/ESP32Ping) – PING-Funktionalität
- [`ArduinoJson`](https://arduinojson.org/) – JSON-Parser
- `esp_wifi.h` – Erweiterte WLAN-Funktionen

> 💡 Tipp: Alle Bibliotheken sind über den Bibliotheksverwalter der Arduino IDE verfügbar.

## 🛠️ Installation

1. Projekt in Arduino IDE oder VSCode/PlatformIO öffnen
2. WLAN-Zugangsdaten und IP des Wechselrichters in den entsprechenden Codeabschnitten eintragen
3. Auf den ESP32 hochladen
4. Serielle Ausgabe (Baudrate 115200) zur Überprüfung öffnen

## 🖥️ Beispielausgabe (seriell)

📶 Verbunden mit AP-EZ1 🔌 Gerätestatus: 🟢 Status: Eingeschaltet ⚡ Aktuelle Output-Daten: 📈 Kanal 1: 142.3 W | e1: 0.153 kWh | te1: 5.521 kWh 📉 Kanal 2: 0.0 W | e2: 0.000 kWh | te2: 0.021 kWh 🚨 Alarminformationen: 🔴 Off Grid Alarm: Normal ⚡ DC 1 Short Circuit Error: Alarm


