# OdyPlan

OdyPlan ist eine modulare C++-Engine zur Beschreibung, Transformation und Ausführung von Datenverarbeitungs-Plänen (logical → physical), aufgeteilt in eine schlanke Kernbibliothek und darauf aufbauende Operator-/IO-Erweiterungen.

## Architektur

Das Projekt ist in klar getrennte Module gegliedert, jedes mit eigenem `include/` (öffentliche API) und `src/` (Implementierung):

```
core/                  Fundament: Plan-Modell, Ports, Edges, Builder
├── shared/            Gemeinsame Basistypen (Type, Identifier, Expression, ...)
├── runtime/           Runtime Utilities (Threading, ...)
├── logical/           Logische Plan-Repräsentation
├── physical/          Physische Plan-Repräsentation & Ausführungsstruktur
└── builder/           Übersetzung logical → physical

basic/                 Konkrete Bausteine, aufbauend auf core
├── io/                IO-Formate (JSON, NDJSON, ...)
├── expression/        Ausdrucks-Bausteine
├── window/            Window-Primitive (Count-/Time-basiert)
└── operators/         Konkrete Operatoren (Source, Sink, Pipe: Select, Merge, Window, ...)
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

Abhängigkeiten (`nlohmann_json`, `spdlog`) werden über `FetchContent`/`_deps` automatisch geladen.

## Qualitätssicherung

- **clang-tidy** läuft über die gesamte Codebasis.
- **Header-Self-Containment-Check**: Jeder öffentliche Header wird isoliert kompiliert (`*_header_check`-Targets), um sicherzustellen, dass er alle benötigten `#include`s selbst mitbringt und nicht von zufälliger Include-Reihenfolge abhängt.

```bash
cmake --build build --target odyplan_operators_header_check
```

## Namensraum

```cpp
sierra::odyplan::v3::core::...
sierra::odyplan::v3::basic::...
```

## Konventionen

- Include-Pfad = Namespace-Pfad (`#include "core/logical/plan/plan.h"` ↔ `core::logical::plan`).
- Neue Module folgen dem Muster `modul/include/`, `modul/src/`, eigenes `CMakeLists.txt`, explizite `target_link_libraries`-Abhängigkeiten gemäß obigem Graph.