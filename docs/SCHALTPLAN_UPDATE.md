# Schaltplan Update - Übersicht der Änderungen

## 🆕 Neue Komponenten hinzugefügt

### 1. **Relais für Motorstromversorgung** (GPIO 23)
```
12V Netzteil (+) ──→ [Relais NO] ──→ BTS7960 B+ (alle 4)
                          │
                          └─ Steuerung: ESP32 GPIO 23
```

**Funktion:**
- Schaltet Motorstromversorgung nur bei Bedarf ein
- **300ms Voreinschaltung** vor Motorstart
- **20 Sekunden Nachlauf** nach letztem Motor
- Spart Energie und reduziert BTS7960-Standby-Verbrauch

**Komponente:**
- 5V Relaismodul (LOW = EIN)
- Schaltkontakt: min. 10A @ 12V DC
- Mit Freilaufdiode

---

### 2. **433 MHz RF-Empfänger** (GPIO 35)
```
RF-Empfänger Module:
├─ VCC  → ESP32 3.3V (oder 5V)
├─ DATA → ESP32 GPIO 35
└─ GND  → ESP32 GND
```

**Funktion:**
- Empfängt 433 MHz Funkfernbedienungen
- 16 anlernbare RF-Codes
- Speicherung im Flash-Speicher
- Kompatibel mit PT2262, EV1527, HT6P20B Sendern

**Empfohlene Module:**
- **Superheterodyne**: RXB6, RXB8, RXB14 (besser)
- **Superregenerative**: RX480E, XY-MK-5V (günstiger)
- Antenne: ~17.3 cm Draht (λ/4)

---

### 3. **Tastenbelegung geändert**

#### ✅ Neue Belegung (ergonomischer):
```
┌────────────────────────────────────┐
│  [1↑]  [2↑]  [3↑]  [4↑]    Reihe 1│
│  [1■]  [2■]  [3■]  [4■]    Reihe 2│
│  [1↓]  [2↓]  [3↓]  [4↓]    Reihe 3│
│  [ALL↑] [ALL↓]  [ ]  [ ]   Reihe 4│
└────────────────────────────────────┘

0-3:  Motoren HOCH   (unverändert)
4-7:  Motoren STOP   (🔄 war RUNTER)
8-11: Motoren RUNTER (🔄 war STOP)
12:   Alle HOCH      (unverändert)
13:   Alle RUNTER    (unverändert)
```

**Vorteil:**
- STOP in der Mitte zwischen HOCH und RUNTER
- Intuitivere Matrix-Anordnung
- Schnellerer Zugriff auf STOP-Funktion

---

## 📌 Aktualisierte Pin-Belegung

| Pin | Alt (vorher) | Neu (jetzt) | Beschreibung |
|-----|-------------|------------|--------------|
| GPIO 21 | *(frei)* | I2C SDA | INA219 Stromsensoren |
| GPIO 22 | *(frei)* | I2C SCL | INA219 Stromsensoren |
| GPIO 23 | *(frei)* | **Relais** | Motorstromversorgung |
| GPIO 34 | Keypad | Keypad | Analoges Keypad (unverändert) |
| GPIO 35 | *(frei)* | **RF-Empfänger** | 433 MHz DATA |

---

## 🔌 Stromversorgung mit Relais

### Ohne Relais (alt):
```
12V Netzteil ──→ BTS7960 (4x) ──→ Motor
                    └─ Standby ~20mA pro Modul = 80mA
```

### Mit Relais (neu):
```
12V Netzteil ──→ [Relais] ──→ BTS7960 (4x) ──→ Motor
                    ↑
                 GPIO 23
                    
Standby: 0mA (Relais aus)
Betrieb: Relais schaltet 300ms vor Motorstart
         Relais aus 20s nach letztem Motor
```

**Energieeinsparung:**
- Standby-Verbrauch: ~80mA → 0mA
- Nur aktiv bei Motorbetrieb
- Konfigurierbare Zeitparameter

---

## 📡 RF-Fernbedienung Integration

### Workflow:
1. **Lernmodus aktivieren**: Via MQTT/Web/Serial
   ```
   mosquitto_pub -t "velux/rf/learn" -m "0"
   ```

2. **Fernbedienungstaste drücken**: Innerhalb 30s
   ```
   ✓✓✓ RF-Code für Taste 0 angelernt: 12345678
   ```

3. **Betrieb**: Fernbedienung wird automatisch erkannt
   ```
   RF empfangen: Code 12345678 -> Taste 0
   Keypad: Motor 1 AUF
   ```

### RF-Code Management:
- 16 Codes speicherbar (Tasten 0-15)
- Persistent im Flash (überleben Neustart)
- Einzeln oder alle löschbar
- Via MQTT/Web/Serial verwaltbar

---

## 🛠️ Benötigte zusätzliche Hardware

### 1. Relaismodul
- **Typ**: 5V Relaismodul mit Optokoppler
- **Schaltleistung**: Min. 10A @ 12V DC
- **Anschluss**:
  - VCC → ESP32 5V (VIN)
  - GND → ESP32 GND
  - IN → ESP32 GPIO 23
  - COM → 12V Netzteil (+)
  - NO → BTS7960 B+ (alle 4)

### 2. RF-Empfängermodul
- **Typ**: 433 MHz Superheterodyne (RXB6/8/14)
- **Anschluss**:
  - VCC → ESP32 3.3V (oder 5V)
  - DATA → ESP32 GPIO 35
  - GND → ESP32 GND
  - Antenne → ~17.3cm Draht

---

## ✅ Änderungen in der Software

### Geänderte Dateien:
1. **config.h**
   - RF-Empfänger Pin (GPIO 35)
   - Relais Pin (GPIO 23)
   - Timing-Parameter

2. **button_handler.h/cpp**
   - RFReceiver Klasse
   - Neue Tastenbelegung
   - RF-Code Verwaltung

3. **motor_controller.h/cpp**
   - PWMController mit Relais-Steuerung
   - Timing-Logik (Pre-on, Post-off)

4. **platformio.ini**
   - RCSwitch Library hinzugefügt

### Neue Features:
- ✅ Relais-Steuerung mit konfigurierbaren Delays
- ✅ RF-Empfänger mit 16 anlernbaren Codes
- ✅ Flash-Speicherung der RF-Codes
- ✅ Geänderte Tastenbelegung (ergonomischer)

---

## 📖 Dokumentation

Neue Dokumentationsdateien:
- **docs/RF_CODES.md** - RF-Code Übersicht
- **docs/RF_VERKABELUNG.md** - RF-Hardware Anschluss
- **docs/TASTENBELEGUNG.md** - Neue Tastenbelegung
- **docs/pinbelegung.txt** - Aktualisiert
- **docs/schaltplan_komplett.txt** - Aktualisiert

---

## 🎯 Zusammenfassung

**Vorteile der Änderungen:**
1. **Energieeffizienz**: Motorstrom nur bei Bedarf
2. **Fernbedienung**: 16 Funkfernbedienungen anlernbar
3. **Ergonomie**: Verbesserte Tastenbelegung
4. **Flexibilität**: Anpassbare Timing-Parameter

**Kompatibilität:**
- ✅ Abwärtskompatibel (bestehende Konfiguration bleibt)
- ✅ Alle bisherigen Funktionen erhalten
- ✅ Zusätzliche Features optional nutzbar
