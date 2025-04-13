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

🔧 Projektstruktur und zukünftige Planung
Der enthaltene Sketch APSystemsInverter_ESP32_Standalone.ino ist derzeit eigenständig nutzbar, ohne zusätzliche externe Bibliothek.
Er wurde so konzipiert, dass er direkt auf einem ESP32 ausgeführt werden kann und sämtliche Funktionen zur Kommunikation mit einem APsystems EZ1 Wechselrichter enthält.

📚 Zukünftige Entwicklung:
Eine eigenständige, modulare C++-Bibliothek für den ESP32 zur Integration und Wiederverwendung der Funktionen befindet sich in Planung.
Sobald diese Library entwickelt und getestet wurde, wird sie in diesem Repository veröffentlicht und dokumentiert, um die Wiederverwendung in anderen Projekten zu erleichtern.


## 🖥️ Beispielausgabe (seriell)



![AP Systems EZ1 Inverter ESP32 Data](https://github.com/user-attachments/assets/78b90a2f-85b0-4af4-b196-a36d445a7924)

