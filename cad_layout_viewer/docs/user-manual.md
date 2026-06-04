# CAD Layout Viewer & Simulator — Benutzerhandbuch

## Installation

**Linux:**
```bash
cmake -S cad_layout_viewer -B build-cad -DCMAKE_BUILD_TYPE=Release
cmake --build build-cad --parallel
./build-cad/cad_layout_viewer
```

**Windows (MSVC):**
```
cmake -S cad_layout_viewer -B build-cad -DCMAKE_BUILD_TYPE=Release
cmake --build build-cad --config Release
windeployqt build-cad\Release\cad_layout_viewer.exe
build-cad\Release\cad_layout_viewer.exe
```

## Erste Schritte

1. **Datei → Öffnen** — JSON-Layout-Datei auswählen.  
2. Layout erscheint im Hauptfenster. Zoom: Mausrad. Pan: mittlere Maustaste.  
3. Im **Layer-Panel** links Lagen ein-/ausblenden.

## Layout-Dateiformat

```json
{
  "layers": [
    {
      "layer_id": 1,
      "name": "Metal1",
      "polygons": [
        { "x": [0.0, 100.0, 100.0, 0.0],
          "y": [0.0,   0.0,  20.0, 20.0] }
      ]
    }
  ]
}
```

Koordinaten in Nanometern (nm). Beliebig viele Polygone pro Layer.

## Simulation starten

1. Layout öffnen.  
2. Im **Eigenschaften-Panel** Δt und Schritte einstellen.  
   - Faustregel: Δt < dx²/(4·α_max) (CFL-Bedingung).  
   - Für Si (α ≈ 8e-5 m²/s) und dx = 1 µm: Δt < 3 ms.  
3. **Simulation → Starten**.  
4. Heatmap erscheint über dem Layout (blau=kalt, rot=heiß).

## Thermische Leitfähigkeiten

| Material    | κ (W/m·K) |
|-------------|-----------|
| Kupfer      | 385       |
| Polysilizium| 30        |
| SiO₂        | 1.4       |
| Luft        | 0.025     |

## Hotspot-Export

Tabelle in **Eigenschaften-Panel** zeigt Hotspots mit Koordinaten und T_max.  
**Datei → Exportieren** speichert die Tabelle als CSV.

## FAQ

**Warum friert die Anwendung nicht ein?**  
Die Simulation läuft in einem eigenen QThread. Der UI-Thread bleibt reaktiv.

**Was bedeutet die CFL-Warnung?**  
Bei Δt > dx²/(4·α_max) wird der explizite Euler-Solver numerisch instabil.  
Das UI zeigt eine Warnung an und bricht die Simulation ab.
