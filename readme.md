> [!IMPORTANT]\
> **This repository is archived**\
> OdyPlan is no longer developed here. Active development has moved to:
>
> **https://gitlab.example.com/sierra/odyplan**
>
> Please open issues, merge requests, and questions there. This repository is kept read-only for reference.
> The last release published here is [**v0.6.4**](https://github.com/SlepiK/OdyPlan/releases/tag/v0.6.4).

---

# OdyPlan

![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform: Linux x64](https://img.shields.io/badge/platform-Linux%20x64-lightgrey.svg)
![Status: Archived](https://img.shields.io/badge/status-archived-red.svg)

**OdyPlan** is a C++/LLVM-based **Data Stream Management System (DSMS)** test engine, built for researching and experimenting with stream-processing mechanisms.

> ⚠️ **Research prototype.**\
> OdyPlan is intended for experimentation and exploration, not for production use.

## Versions & Releases

| Version         | Status | Notes                                                                                                                                                        |
|-----------------|--------|--------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **v3**          | ✅ Current | The maintained generation of OdyPlan. Everything in this README, the `odyplan/v3` namespaces, and `examples/v3/` refer to it.                                |
| **v1** & **v2** | ⚠️ Deprecated | Earlier iterations and experimental test attempts. Kept for historical reference only. They are unsupported, may not build, and are not recommended for use. |

The last release published on this repository is [**v0.6.4**](https://github.com/SlepiK/OdyPlan/releases/tag/v0.6.4). For anything newer, see the GitLab repository linked above.


## Platform Support

OdyPlan has so far only been built and tested on **Linux (x86-64)**. Other operating systems and architectures (Windows, macOS, ARM) are untested and may require changes to build or run.

## Compilation Modes

OdyPlan leverages LLVM to support two compilation strategies for query plans:

| Mode | Description | Examples |
|------|-------------|----------|
| **AOT** (ahead-of-time) | Plans are compiled before execution. | [`examples/v3/aot`](examples/v3/aot) |
| **JIT** (just-in-time) | Plans are compiled at runtime, right before execution. | [`examples/v3/jit`](examples/v3/jit) |

See the linked example directories for working demonstrations of each mode.


## Architecture

The project is split into clearly separated modules. Each module has its own `include/` (public API) and `src/` (implementation):

```
core/                  Plan models, ports, segments, builders, etc.
├── shared/            Common base types (Type, Identifier, Expression, ...)
├── runtime/           Runtime utilities (threading, ...)
├── logical/           Logical plan representation
├── physical/          Physical plan representation & execution structure
└── builder/           Translation from logical to physical plans

basic/                 Concrete building blocks on top of core
├── io/                I/O formats (JSON, NDJSON, ...)
├── expression/        Expression building blocks
├── storage/           storage primitives (low-level storage class)
└── operators/         Concrete operators (Source, Sink, Pipe: Select, Join, ...)
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

**Dependencies:** `nlohmann_json`, `spdlog`, `llvm`, `zstd`

## Namespaces

```cpp
sierra::odyplan::v3::core::...
sierra::odyplan::v3::basic::...
```

## License

This project is licensed under the [MIT License](LICENSE).