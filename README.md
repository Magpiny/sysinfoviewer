# SysInfoViewer

> A lightweight, native Linux system information viewer built with C++ and wxWidgets.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-0.3.0-green.svg)]()
[![C++23](https://img.shields.io/badge/C%2B%2B-23-orange.svg)]()
[![wxWidgets](https://img.shields.io/badge/wxWidgets-3.3-blueviolet.svg)](https://www.wxwidgets.org/)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)]()
[![AUR](https://img.shields.io/aur/version/sysinfoviewer.svg)](https://aur.archlinux.org/packages/sysinfoviewer)

SysInfoViewer provides a clean graphical interface for monitoring your Linux system — CPU, memory, disk, battery, running processes, installed applications, and more. No terminal required.

---

## Screenshots

| System Overview | CPU Info | Installed Apps |
|:-:|:-:|:-:|
| [![Main Window](screenshots/main_window.png)](screenshots/main_window.png) | [![CPU Info](screenshots/cpu_info.png)](screenshots/cpu_info.png) | [![Installed Apps](screenshots/installed_apps.png)](screenshots/installed_apps.png) |

---

## Features

- **System Overview** — OS details, architecture, desktop environment, hostname, and disk usage at a glance
- **Real-time CPU Monitoring** — Live doughnut chart displaying accurate physical core count, logical thread count, and clock speed
- **Memory Usage** — Visual breakdown of used vs. free RAM, updated every second
- **Disk Usage** — Responsive pie chart with dynamically positioned labels that adapt to any panel size
- **Battery Status** — Comprehensive panel sourced from both sysfs and UPower: energy (Wh), power rate (W), voltage (V), health, charge cycles, vendor, model, serial, and time remaining
- **Process List** — Sortable table of running processes with PID, user, CPU%, RAM%, and command
- **Installed Applications** — Browse applications installed on the system
- **Miscellaneous Info** — Sound cards, GPU/display info, and network interfaces

---

## What's New in v0.3.0

### CPU Information — Cores vs. Threads Corrected

Previous versions reported logical processors (threads) as the core count, causing machines such as the Ryzen 7 Pro 7840U to display 16 cores instead of 8. v0.3.0 now reads the `cpu cores` field from `/proc/cpuinfo` separately from the `processor` count, and presents both values correctly.

```
Before:  CPU Count: 16
After:   Cores: 8 | Threads: 16 | Speed: 3200 MHz
```

### Battery Panel — Full UPower-Grade Detail

The battery panel has been fully rewritten. It now reads energy values in watt-hours directly from sysfs `energy_*` nodes (µWh → Wh), derives power rate from `power_now` with a correct `current_now × voltage_now` fallback, and queries `upower` at runtime to retrieve vendor, model, and serial fields that sysfs does not expose. The information displayed now matches `upower -i $(upower -e | grep BAT)` output.

| Field | Source |
|---|---|
| Vendor / Model / Serial | `upower` |
| State | sysfs `status` |
| Percentage | sysfs `capacity` |
| Energy now / full / full-design | sysfs `energy_*` (µWh → Wh) |
| Energy rate | sysfs `power_now` (µW → W) |
| Voltage / Voltage min design | sysfs `voltage_now` / `voltage_min_design` |
| Capacity (health %) | `energy_full ÷ energy_full_design × 100` |
| Charge cycles | sysfs `cycle_count` |
| Time remaining | `energy_now ÷ energy_rate`; overridden by `upower` value when available |
| Technology / Capacity level | sysfs `technology` / `capacity_level` |

The visual has also been updated: a rounded, colour-coded battery bar (green / orange / red) with a terminal nub sits alongside a two-column label/value table with state colour-coding.

### Disk Usage Labels — Responsive Layout

Disk usage labels in the System Overview were previously positioned at hardcoded pixel offsets derived from the pie radius, causing overlap on narrower panels. Labels are now stacked vertically, centred below the pie, and positioned relative to the actual rendered radius so they adapt correctly at any panel size.

### wxWidgets Upgraded to 3.3

The minimum required wxWidgets version is now **3.3**, up from 3.2.8. This release brings improved HiDPI rendering, updated graphics context behaviour, and better font metrics — all of which benefit the battery and CPU chart panels directly.

---

## Installation

### Arch Linux / CachyOS / Manjaro — AUR (easiest)

The package is available on the AUR as `sysinfoviewer`. Install it with your preferred AUR helper:

```bash
# Using paru
paru -S sysinfoviewer

# Using yay
yay -S sysinfoviewer
```

The AUR package handles all dependencies and installs a `.desktop` entry automatically.

---

### Build from Source

#### Prerequisites

**Compiler & build tools**

- GCC 14 or later (C++23 support required)
- CMake 4.2 or later
- Git

**Libraries**

| Library | Purpose | Arch / CachyOS | Debian / Ubuntu |
|---|---|---|---|
| wxWidgets ≥ 3.3 | GUI framework | `sudo pacman -S wxwidgets-gtk3` | `sudo apt install libwxgtk3.2-dev` |
| libcurl | Network interface detection | `sudo pacman -S curl` | `sudo apt install libcurl4-openssl-dev` |
| ALSA | Sound card detection | `sudo pacman -S alsa-lib` | `sudo apt install libasound2-dev` |
| libdrm | GPU / display info | `sudo pacman -S libdrm` | `sudo apt install libdrm-dev` |
| UPower | Battery metadata | `sudo pacman -S upower` | `sudo apt install upower` |

> **Note:** UPower is a **runtime** dependency only. The application detects it via `wxExecute` at startup and degrades gracefully if it is absent — battery vendor, model, and serial fields will show "Unknown".

#### Clone the Repository

```bash
git clone https://github.com/Magpiny/sysinfoviewer.git
cd sysinfoviewer
```

#### Option A — Build Script (recommended)

The included `build.sh` handles CMake configuration, compilation, and AppDir layout in one step.

```bash
# Optimised release build
./build.sh release

# Debug build with symbols and verbose logging
./build.sh debug

# Usage reference
./build.sh --help
```

Run the resulting binary directly without installing:

```bash
./AppDir/bin/sysinfoviewer
```

#### Option B — CMake Manually

```bash
mkdir build && cd build
cmake ..
make
```

#### Installing System-Wide

```bash
sudo make install
```

This installs the `sysinfoviewer` executable and all required resources to the appropriate system paths.

#### Running After Installation

```bash
sysinfoviewer
```

---

## Contributing

Contributions are welcome. Please open an issue to discuss a proposed change before submitting a pull request. Bug reports with system details and steps to reproduce are especially appreciated.

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for full details.

---

## Author

**Wanjare Samuel**
[wanjaresamuel@gmail.com](mailto:wanjaresamuel@gmail.com)

---

## Acknowledgements

- [wxWidgets](https://www.wxwidgets.org/) — cross-platform GUI framework
- [UPower](https://upower.freedesktop.org/) — battery metadata
- All contributors and users of SysInfoViewer
