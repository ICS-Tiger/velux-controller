# 433 MHz RF-Empfänger - Tastenbelegung

## Hardware
- **RF-Empfänger-Pin**: GPIO 21
- **Library**: RCSwitch (sui77/rc-switch)
- **Frequenz**: 433 MHz

## Tastenbelegung (16 RF-Codes)

### Motor 1-4 HOCH (Codes 0-3)
- **Code 0**: Motor 1 HOCH
- **Code 1**: Motor 2 HOCH
- **Code 2**: Motor 3 HOCH
- **Code 3**: Motor 4 HOCH

### Motor 1-4 STOP (Codes 4-7)
- **Code 4**: Motor 1 STOP
- **Code 5**: Motor 2 STOP
- **Code 6**: Motor 3 STOP
- **Code 7**: Motor 4 STOP

### Motor 1-4 RUNTER (Codes 8-11)
- **Code 8**: Motor 1 RUNTER
- **Code 9**: Motor 2 RUNTER
- **Code 10**: Motor 3 RUNTER
- **Code 11**: Motor 4 RUNTER

### Spezialfunktionen (Codes 12-15)
- **Code 12**: ALLE HOCH
- **Code 13**: ALLE RUNTER
- **Code 14**: Reserve
- **Code 15**: Reserve

## RF-Codes anlernen

### Über Serial Monitor
```
1. RF-Lernmodus starten (siehe MQTT oder Webinterface)
2. Innerhalb von 30 Sekunden die gewünschte Fernbedienungstaste drücken
3. Code wird automatisch gespeichert
```

### Über MQTT
```bash
# RF-Code für Taste 0 (Motor 1 HOCH) anlernen
mosquitto_pub -h 192.168.0.22 -t "velux/rf/learn" -m "0"

# RF-Code für Taste 5 (Motor 2 STOP) anlernen
mosquitto_pub -h 192.168.0.22 -t "velux/rf/learn" -m "5"

# Einzelnen RF-Code löschen
mosquitto_pub -h 192.168.0.22 -t "velux/rf/clear" -m "5"

# ALLE RF-Codes löschen
mosquitto_pub -h 192.168.0.22 -t "velux/rf/clear" -m "-1"
```

### Über Webinterface
```
1. Webinterface öffnen (http://[ESP32-IP])
2. Zu "RF-Fernbedienung" navigieren
3. Taste auswählen und "Anlernen" klicken
4. Fernbedienungstaste innerhalb von 30 Sekunden drücken
```

## Speicherung
- RF-Codes werden im Flash-Speicher (Preferences) gespeichert
- Namespace: "rf_codes"
- Keys: "code_0" bis "code_15"
- Codes bleiben nach Neustart erhalten

## Funktionsweise
1. **Normal-Modus**: Empfangene RF-Codes werden mit gespeicherten Codes verglichen
2. **Lern-Modus**: Nächster empfangener Code wird für die gewählte Taste gespeichert
3. **Timeout**: Lern-Modus wird nach 30 Sekunden automatisch beendet

## Kompatible Fernbedienungen
- Alle 433 MHz Sender (ASK/OOK Modulation)
- Typische Fernbedienungen: PT2262, EV1527, HT6P20B
- Garagen-, Tor- und Universal-Fernbedienungen

## Debugging
Im Serial Monitor werden empfangene RF-Codes angezeigt:
```
RF empfangen: Code 12345678 -> Taste 0
RF empfangen: Unbekannter Code 87654321
>>> RF-Lernmodus für Taste 5 gestartet (30s)
✓✓✓ RF-Code für Taste 5 angelernt: 12345678
```
