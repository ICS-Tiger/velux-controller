# Tastenbelegung - Analoges Keypad & RF-Fernbedienung

## Übersicht - NEUE Belegung

### Tasten 0-3: Motoren HOCH ⬆️
| Taste | Funktion | Beschreibung |
|-------|----------|--------------|
| **0** | Motor 1 HOCH | Fenster 1 öffnen |
| **1** | Motor 2 HOCH | Fenster 2 öffnen |
| **2** | Motor 3 HOCH | Fenster 3 öffnen |
| **3** | Motor 4 HOCH | Fenster 4 öffnen |

### Tasten 4-7: Motoren STOP ⏹️
| Taste | Funktion | Beschreibung |
|-------|----------|--------------|
| **4** | Motor 1 STOP | Fenster 1 stoppen |
| **5** | Motor 2 STOP | Fenster 2 stoppen |
| **6** | Motor 3 STOP | Fenster 3 stoppen |
| **7** | Motor 4 STOP | Fenster 4 stoppen |

### Tasten 8-11: Motoren RUNTER ⬇️
| Taste | Funktion | Beschreibung |
|-------|----------|--------------|
| **8** | Motor 1 RUNTER | Fenster 1 schließen |
| **9** | Motor 2 RUNTER | Fenster 2 schließen |
| **10** | Motor 3 RUNTER | Fenster 3 schließen |
| **11** | Motor 4 RUNTER | Fenster 4 schließen |

### Tasten 12-15: Spezialfunktionen ⚡
| Taste | Funktion | Beschreibung |
|-------|----------|--------------|
| **12** | ALLE HOCH | Alle Fenster öffnen |
| **13** | ALLE RUNTER | Alle Fenster schließen |
| **14** | Reserve | Nicht belegt |
| **15** | Reserve | Nicht belegt |

## Änderungen gegenüber alter Belegung

### ALT (vorher):
```
0-3: Motoren HOCH
4-7: Motoren RUNTER
8-11: Motoren STOP
12: Alle HOCH
13: Alle RUNTER
```

### NEU (jetzt):
```
0-3: Motoren HOCH      ✓ unverändert
4-7: Motoren STOP      🔄 GEÄNDERT (war RUNTER)
8-11: Motoren RUNTER   🔄 GEÄNDERT (war STOP)
12: Alle HOCH          ✓ unverändert
13: Alle RUNTER        ✓ unverändert
```

## Ergonomie-Vorteile der neuen Belegung

1. **Logische Gruppierung**: 
   - Obere Reihe (0-3): HOCH
   - Mittlere Reihe (4-7): STOP
   - Untere Reihe (8-11): RUNTER

2. **Intuitive Bedienung**:
   - STOP in der Mitte zwischen HOCH und RUNTER
   - Schneller Zugriff auf STOP-Funktion

3. **Matrix-Layout kompatibel**:
   ```
   [1↑] [2↑] [3↑] [4↑]
   [1■] [2■] [3■] [4■]
   [1↓] [2↓] [3↓] [4↓]
   [All↑] [All↓] [ ] [ ]
   ```

## Anschluss Analoges Keypad

- **Pin**: GPIO 34 (ADC)
- **Spannungsteiler**: 16 Tasten über Widerstandsnetzwerk
- **ADC-Bereich**: 0-3.3V (12-bit Auflösung)
- **Debounce**: 50ms

## Code-Referenz

Siehe [config.h](../src/config.h#L60-L72) für Pin-Definition und Tastenbelegung.
Siehe [button_handler.cpp](../src/button_handler.cpp) für Implementierung.
