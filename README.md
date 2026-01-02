# Velux Rolladen Controller

ESP32-basierte Steuerung für 4 Velux-Rollläden mit BTS7960 Motortreibern

## Features

✅ 4 Motoren individuell steuerbar  
✅ Sanftanlauf mit PWM-Synchronisation  
✅ 8 Taster (je 2 pro Motor)  
✅ Strommessung pro Motor (INA219)  
✅ MQTT Integration  
✅ Webinterface  
✅ Position-Tracking (0-100%)  
✅ Automatisches Anlernen der Laufzeiten  
✅ Endlagenerkennung durch Strommessung  

## Hardware

- ESP32 DevKit V1
- 4x BTS7960 (IBT-2) Motortreiber
- 4x INA219 Stromsensoren
- 8x Taster
- Velux 24V DC Netzteil

## Installation

### 1. Software installieren
```bash
# PlatformIO installieren (VSCode Extension)
# Projekt öffnen
cd velux-controller
code .
```

### 2. Konfiguration anpassen
Datei `src/config.h` öffnen und anpassen:
```cpp
#define WIFI_SSID "DeinWiFi"
#define WIFI_PASSWORD "DeinPasswort"
#define MQTT_SERVER "192.168.1.100"
```

### 3. Kompilieren & Flashen
```bash
# In VSCode mit PlatformIO:
# - Build (Ctrl+Alt+B)
# - Upload (Ctrl+Alt+U)
# - Serial Monitor (Ctrl+Alt+S)
```

## Verwendung

### Taster
- **Kurzer Druck**: Motor starten (Auf/Zu)
- **Langer Druck (>1s)**: Motor stoppen

### Webinterface
```
http://velux-controller.local
oder
http://[ESP32-IP]
```

Features:
- Motor-Steuerung (Auf/Zu/Stop)
- Position einstellen (Slider)
- Laufzeiten anlernen
- Live-Status anzeigen

### MQTT Topics

**Steuerung:**
```
velux/motor1/set → "OPEN", "CLOSE", "STOP" oder 0-100
velux/motor2/set
velux/motor3/set
velux/motor4/set
velux/all/set → Alle gleichzeitig
```

**Status (automatisch alle 2s):**
```
velux/motor1/state → {"state":"running","position":50,"current":1234.5}
velux/motor2/state
velux/motor3/state
velux/motor4/state
```

## Erste Inbetriebnahme

1. **Hardware aufbauen** (siehe docs/schaltplan_text.txt)
2. **ESP32 flashen**
3. **Webinterface öffnen**
4. **Motoren kalibrieren:**
   - Motor 1 auf "Öffnungszeit lernen"
   - Motor läuft bis Endlage
   - Wiederholen für "Schließzeit lernen"
   - Für alle 4 Motoren wiederholen
5. **Fertig!** Normale Steuerung möglich

## Sanftanlauf

Der Controller nutzt intelligenten Sanftanlauf:
- Motor 1 startet → PWM rampt 0-2s von 100→255
- Motor 2 startet @ 1s → springt sofort auf aktuelles PWM (177)
- Beide laufen synchron weiter

## Troubleshooting

**Motor läuft nicht:**
- INA219 Adresse prüfen (Serial Monitor)
- BTS7960 Verkabelung prüfen
- Enable-Pins prüfen (sollten HIGH sein)

**WiFi verbindet nicht:**
- SSID/Passwort in config.h prüfen
- Serial Monitor beobachten

**Position stimmt nicht:**
- Motoren neu kalibrieren
- Laufzeiten prüfen (Serial Monitor)

## Schaltpläne

Siehe `docs/`:
- schaltplan_text.txt
- pinbelegung.txt
- stueckliste.txt

# Steuerung
mosquitto_pub -t "velux/motor1/set" -m "OPEN"
mosquitto_pub -t "velux/motor1/set" -m "50"     # 50%
mosquitto_pub -t "velux/all/set" -m "CLOSE"     # Alle

# Status (automatisch alle 2s)
mosquitto_sub -t "velux/motor1/state"
# → {"state":"running","position":75,"current":1234.5}
```
#define WIFI_SSID "DeinWiFi"           // ← Ändern
#define WIFI_PASSWORD "DeinPasswort"   // ← Ändern
#define MQTT_SERVER "192.168.1.100"    // ← Ändern
```

### **4. Kompilieren**
In VSCode:
- **Ctrl+Alt+B** (Build)
- Warten bis "SUCCESS"

### **5. Flashen**
- ESP32 per USB anschließen
- **Ctrl+Alt+U** (Upload)

### **6. Serial Monitor**
- **Ctrl+Alt+S** öffnen
- Ausgabe beobachten:
```
╔═══════════════════════════════════════╗
║  VELUX ROLLADEN CONTROLLER v1.0      ║
╚═══════════════════════════════════════╝

✓ I2C Bus initialisiert
✓ PWM-Controller: Initialisiert
Motor 1: INA219 initialisiert
Motor 2: INA219 initialisiert
...
✓ WiFi verbunden!
IP: 192.168.1.123
Webserver: Gestartet auf Port 80

╔═══════════════════════════════════════╗
║         SYSTEM BEREIT!                ║
╚═══════════════════════════════════════╝

Webinterface: http://192.168.1.123
---

## **📊 Code-Statistik:**
```
Zeilen Code:      ~1200
C++ Files:        9
Header Files:     5
Funktionen:       50+
Kompiliert:       ✓ (getestet)
Größe kompiliert: ~800 KB

## License

MIT



## Autor

Rüdiger Thomas
