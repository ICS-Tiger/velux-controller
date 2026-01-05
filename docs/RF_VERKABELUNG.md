# Verkabelung 433 MHz RF-Empfänger

## RF-Empfänger Modul Anschluss

### Typisches 433 MHz Empfänger-Modul
```
 RF-EMPFÄNGER MODUL
┌─────────────────┐
│                 │
│   [Antenne]     │
│                 │
│  VCC  DATA  GND │
└──┬────┬────┬───┘
   │    │    │
   │    │    └─── GND (ESP32 GND)
   │    └──────── GPIO 21 (ESP32)
   └───────────── 5V oder 3.3V (ESP32 VCC)
```

## Pin-Belegung

| RF-Modul | ESP32 Pin | Beschreibung |
|----------|-----------|--------------|
| VCC | 3.3V oder 5V | Stromversorgung (je nach Modul) |
| DATA | GPIO 35 | Datensignal |
| GND | GND | Masse |

## Hinweise zur Stromversorgung

### 3.3V Module (empfohlen für ESP32)
- Direkt an ESP32 3.3V anschließen
- Keine Level-Shifter nötig
- Reichweite: ca. 20-50m

### 5V Module (höhere Reichweite)
- An ESP32 5V (VIN) anschließen
- DATA-Signal ist meist auch 3.3V-kompatibel
- Bei Problemen: Level-Shifter verwenden
- Reichweite: ca. 50-100m

## Antenne

### Drahtantenne (optimal)
```
Länge = 300 / (Frequenz in MHz) / 4
Für 433 MHz: 300 / 433 / 4 = ~17.3 cm

┌─────────────────┐
│   RF-MODUL      │
│                 │
└────┬────────────┘
     │
     │ ~17.3 cm Draht
     │ (vertikal)
     │
```

### Spiralfeder-Antenne
- Oft im Lieferumfang
- Einfach aufstecken
- Geringere Reichweite als Drahtantenne

## Empfohlene Module

### Superheterodyne Empfänger (besser)
- Modell: RXB6, RXB8, RXB14
- Höhere Empfindlichkeit
- Bessere Störfestigkeit
- Preis: ~2-4 EUR

### Superregenerative Empfänger (günstiger)
- Modell: RX480E, XY-MK-5V
- Ausreichend für kurze Distanzen
- Anfälliger für Störungen
- Preis: ~1-2 EUR

## Montage-Tipps

1. **Abstand zu Metall**: Mind. 5cm Abstand zu Metallgehäusen
2. **Abstand zum ESP32**: Mind. 2cm (RF-Störungen)
3. **Antenne ausrichten**: Vertikal für beste Reichweite
4. **Kabelführung**: DATA-Kabel kurz halten (<10cm)

## Integration ins Velux-Controller System

### Schaltplan-Ergänzung
```
ESP32 Pin-Belegung:
├─ GPIO 25: RPWM (Motoren vorwärts)
├─ GPIO 26: LPWM (Motoren rückwärts)
├─ GPIO 23: Relais Motorstromversorgung [NEU]
├─ GPIO 34: Analoges Keypad (16 Tasten)
├─ GPIO 35: RF-Empfänger DATA [NEU]
├─ GPIO 21: I2C SDA (INA219 Stromsensoren)
└─ GPIO 22: I2C SCL (INA219 Stromsensoren)
```

## Test nach Installation

1. **Upload der Software**
2. **Serial Monitor öffnen** (115200 Baud)
3. **RF-Code anlernen**:
   ```
   >>> RF-Lernmodus für Taste 0 gestartet (30s)
   ```
4. **Fernbedienung drücken**:
   ```
   ✓✓✓ RF-Code für Taste 0 angelernt: 12345678
   ```
5. **Test**: Fernbedienung erneut drücken:
   ```
   RF empfangen: Code 12345678 -> Taste 0
   Keypad: Motor 1 AUF
   ```

## Fehlersuche

### Kein Empfang
- VCC/GND prüfen (Multimeter)
- Antennenlänge prüfen
- Anderen GPIO-Pin testen
- Fernbedienung Batterie prüfen

### Sporadischer Empfang
- Antenne verlängern/kürzen
- Superheterodyne-Modul verwenden
- Abstand zu ESP32 vergrößern

### Falscher Code
- RF-Code neu anlernen
- Andere Fernbedienungstaste verwenden
