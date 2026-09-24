<div align="center">
  
# GPU-X (РУССКИЙ)

Лёгкая нативная Windows-утилита для мониторинга видеокарты.

Показывает информацию об адаптере: частоты, температуру, напряжение, обороты вентиляторов и поддерживаемые графические API. Работает с **NVIDIA**, **AMD** и **Intel**.

[![Platform](https://img.shields.io/badge/platform-Windows%207%2F8%2F10%2F11-0078D6?logo=windows&logoColor=white)](#требования)
[![Language](https://img.shields.io/badge/language-C%2B%2B-00599C?logo=cplusplus&logoColor=white)](#)
[![Build](https://img.shields.io/badge/build-Visual%20Studio%202022-5C2D91?logo=visualstudio&logoColor=white)](#сборка)
[![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)](./LICENSE.txt)

</div>

## Возможности

- Определяет все установленные видеоадаптеры через DXGI
- Общая информация: название, Device ID, ревизия, версия и дата драйвера, версия BIOS, объём и тип видеопамяти, текущая/максимальная скорость шины PCI Express, максимальный поддерживаемый Feature Level DirectX
- Проверка поддержки API и технологий:
  - DirectX (до Feature Level 12.2), DirectCompute, DirectML
  - Vulkan
  - OpenCL
  - OpenGL 4.6
  - Resizable BAR
  - CUDA и PhysX (только для NVIDIA)
- Вкладка «Датчики» с показаниями в реальном времени и короткой историей на графиках:
  - температура ядра
  - напряжение ядра
  - обороты вентилятора(ов) в об/мин
- Несколько видеокарт - переключение через выпадающий список внизу окна
- Локализация: русский, английский, немецкий (выбирается по языку системы)
- Поддержка High DPI
- Компактный тёмный интерфейс с кастомными контролами (без стандартного Win32-вида)
- Нет внешних зависимостей во время выполнения. NVAPI / ADL / Intel Graphics Control Library подгружаются динамически - отсутствие одного драйвера не мешает работе с остальными GPU

## Скриншоты

| Информация | Датчики |
|------------|---------|
| ![Info](https://github.com/user-attachments/assets/84385073-3255-4f03-bb44-9ce4344e74f8) | ![Sensors](https://github.com/user-attachments/assets/4460e538-a4a2-42a5-95e6-c8e255684882) |

## Архитектура

В основе абстрактный класс `GPU`. Общая логика (перечисление адаптеров через DXGI, проверка DirectX / Vulkan / OpenCL / OpenGL / Resizable BAR) живёт в базовом классе. Вендор-специфичные данные (частоты, температуры, напряжение, кулеры, BIOS) берутся через приватные API производителей:

```
main.cpp          – точка входа, создание окна и общих контролов
GPUFactory        – перечисляет DXGI-адаптеры, создаёт VkInstance и нужные объекты GPU
GPU (базовый)     – DXGI + D3D9-12 + DirectML + Vulkan + OpenCL + OpenGL + Resizable BAR + PCI-локация
├── NVIDIA_GPU    – обёртка над NVAPI
├── AMD_GPU       – ADL + Overdrive 5/6/7/8
└── Intel_GPU     – Intel Graphics Control Library (igcl / ctl_*)
MainWindow        – отрисовка UI, таймер обновления
Button / ComboBox – кастомные контролы
DPIManager        – масштабирование под текущий DPI
StringManager     – загрузка локализованных строк из ресурсов
Themes.h          – цветовая схема
GPU-X.rc          – таблицы строк (RU / EN / DE) + иконка
```

### Используемые технологии

| Технология                  | Назначение                                              |
|-----------------------------|---------------------------------------------------------|
| Win32 API                   | окно, отрисовка, ресурсы                                |
| DXGI / Direct3D 9–12        | список адаптеров, Feature Level, объём памяти           |
| DirectML                    | проверка поддержки ML-инференса                         |
| Vulkan                      | поддержка Vulkan (динамическая загрузка `vulkan-1.dll`) |
| Заголовки OpenCL            | проверка OpenCL (уже лежат в репозитории)               |
| SetupAPI / CfgMgr32         | свойства устройства из диспетчера устройств             |
| NVAPI                       | NVIDIA: частоты, температура, напряжение, вентиляторы, BIOS, CUDA |
| ADL + Overdrive 5–8         | AMD: частоты, температура, напряжение, вентиляторы, BIOS |
| Intel Graphics Control Lib  | Intel: частоты, температура, память, шина               |

## Требования

### Для запуска

- Windows 7 и новее (рекомендуется Windows 11)
- Официальный драйвер для той видеокарты, для которой хотите видеть датчики:
  - NVIDIA - драйвер с NVAPI
  - AMD - драйвер с поддержкой ADL
  - Intel - драйвер с Intel Graphics Control Library

### Для сборки

- Visual Studio 2022 с компонентом «Разработка классических приложений на C++»
- Windows 10/11 SDK
- Vulkan SDK (заголовки должны быть доступны компилятору)
- Заголовки OpenCL уже включены (`OpenCL-SDK-v2026.05.29-Win-x64/include`) - отдельно ставить ничего не нужно

## Сборка

1. Установите Visual Studio 2022
2. Убедитесь, что заголовки Vulkan видны компилятору
3. Откройте `GPU-X.sln`
4. Выберите конфигурацию **Release**
5. Соберите решение (`Ctrl+Shift+B`)

Проект линкуется только с системными библиотеками (`dxgi`, `setupapi`, `opengl32`, `cfgmgr32`, `Dwmapi`, `Shcore`). Вендорские SDK подгружаются через `LoadLibrary` во время работы, поэтому их отсутствие не ломает ни сборку, ни запуск - соответствующие функции останутся недоступны.

## Запуск

Запускаете `GPU-X.exe`. Дальше программа автоматически:

1. Перечисляет адаптеры через DXGI
2. По Vendor ID создаёт нужный объект (`NVIDIA_GPU` / `AMD_GPU` / `Intel_GPU`)
3. Собирает статическую информацию (название, память, шина, BIOS, поддерживаемые API)
4. Открывает окно с вкладками «Информация» и «Датчики»
5. Начинает опрашивать датчики и обновлять графики

Если видеокарт несколько - переключайтесь через выпадающий список внизу.

## Структура репозитория

```
GPU-X/
├── GPU-X.sln
├── icon.ico
├── LICENSE.txt
├── OpenCL-SDK-v2026.05.29-Win-x64/   # заголовки OpenCL для сборки
└── GPU-X/
    ├── main.cpp
    ├── GPU.h / GPU.cpp              # базовый класс
    ├── GPUFactory.h / .cpp
    ├── NVIDIA GPU.h / .cpp
    ├── AMD GPU.h / .cpp
    ├── Intel GPU.h / .cpp
    ├── Overdrive*.h / .cpp          # поколения AMD Overdrive 5–8
    ├── MainWindow.h / .cpp
    ├── Button.h / .cpp
    ├── ComboBox.h / .cpp
    ├── DPIManager.h / DPI.cpp
    ├── StringManager.h / .cpp
    ├── Themes.h
    ├── Globals.h / .cpp
    ├── WindowHelpers.h / .cpp
    ├── VersionHelper.h
    ├── CommonID.h / resource.h
    ├── GPU-X.rc                     # таблицы строк RU / EN / DE + иконка
    ├── GPU-X.manifest
    ├── AMD/  NVIDIA/  Intel/        # приватные заголовки вендоров
    └── x64/                         
```

## Локализация

Строки лежат в `GPU-X.rc` в отдельных `STRINGTABLE` для каждого `LANGUAGE`. Сейчас есть:

- русский
- английский
- немецкий

Язык выбирается автоматически по локали системы.

Чтобы добавить новый язык:

1. Скопируйте блок `STRINGTABLE` и укажите нужный `LANGUAGE` / `SUBLANG`
2. Переведите все строки `IDS_*`
3. Пересоберите проект

## Лицензия

GNU General Public License v3.0 - полный текст в [LICENSE.txt](./LICENSE.txt).

## Дисклеймер

NVIDIA, AMD, Intel и их API (NVAPI, ADL, Intel Graphics Control Library) — торговые марки соответствующих компаний. Заголовочные файлы в репозитории нужны только для общения с официальными драйверами и не попадают под GPL-лицензию самого проекта.

---

<div align="center">

# GPU-X (ENGLISH)

Lightweight native Windows utility for monitoring your graphics card.

Shows adapter info, clocks, temperature, voltage, fan speeds and which graphics APIs are supported. Works with **NVIDIA**, **AMD** and **Intel**.

[![Platform](https://img.shields.io/badge/platform-Windows%207%2F8%2F10%2F11-0078D6?logo=windows&logoColor=white)](#requirements)
[![Language](https://img.shields.io/badge/language-C%2B%2B-00599C?logo=cplusplus&logoColor=white)](#)
[![Build](https://img.shields.io/badge/build-Visual%20Studio%202022-5C2D91?logo=visualstudio&logoColor=white)](#building)
[![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)](./LICENSE.txt)

</div>

## What it does

- Detects every installed GPU through DXGI
- Basic adapter info: name, Device ID, revision, driver version & date, BIOS version, VRAM size and type, PCIe bus speed (current / max), highest supported DirectX feature level
- Checks which APIs and features are available:
  - DirectX (up to Feature Level 12.2), DirectCompute, DirectML
  - Vulkan
  - OpenCL
  - OpenGL 4.6
  - Resizable BAR
  - CUDA and PhysX (NVIDIA only)
- Sensors tab with live values and short history graphs:
  - GPU core temperature
  - Core voltage
  - Fan speed(s) in RPM
- Multiple GPUs? Just pick one from the dropdown at the bottom
- UI in Russian, English or German - picks the language from the system locale
- High-DPI aware
- Small dark theme UI with custom owner-drawn controls (no stock Win32 look)
- No runtime dependencies on vendor SDKs - NVAPI / ADL / Intel Graphics Control Library are loaded dynamically. Missing one driver doesn’t break the others

## Screenshots

| Info tab | Sensors tab |
|----------|-------------|
| <img width="400" height="500" alt="image" src="https://github.com/user-attachments/assets/7d03bb40-a7c0-47a3-9c0b-b3cc04c88858" /> | <img width="400" height="500" alt="image" src="https://github.com/user-attachments/assets/7262f092-868f-4c36-b3ff-7169e98b5445" /> |

## How it’s put together

Everything goes through an abstract `GPU` base class. DXGI handles the common stuff (adapter enumeration, DirectX / Vulkan / OpenCL / OpenGL / Resizable BAR checks). Vendor-specific data (clocks, temps, voltage, fans, BIOS) comes from the private APIs:

```
main.cpp          – entry point, creates the window and shared controls
GPUFactory        – walks DXGI adapters, creates a VkInstance, instantiates the right GPU subclass
GPU (base)        – DXGI + D3D9-12 + DirectML + Vulkan + OpenCL + OpenGL + Resizable BAR + PCI location
├── NVIDIA_GPU    – thin wrapper around NVAPI
├── AMD_GPU       – ADL + Overdrive 5/6/7/8
└── Intel_GPU     – Intel Graphics Control Library (igcl / ctl_*)
MainWindow        – draws the UI, runs the update timer
Button / ComboBox – custom owner-drawn controls
DPIManager        – scales everything to the current screen DPI
StringManager     – loads localized strings from the resource file
Themes.h          – colour scheme
GPU-X.rc          – string tables (RU / EN / DE) + icon
```

### Libraries & SDKs used at build time

| What                        | What for                                              |
|-----------------------------|-------------------------------------------------------|
| Win32 API                   | window, painting, resources                           |
| DXGI / Direct3D 9–12        | adapter list, feature levels, memory size             |
| DirectML                    | ML inference support check                            |
| Vulkan                      | Vulkan support (loads `vulkan-1.dll` at runtime)      |
| OpenCL headers              | OpenCL support check (headers are already in the repo)|
| SetupAPI / CfgMgr32         | device properties from Device Manager                 |
| NVAPI                       | NVIDIA clocks / temp / voltage / fans / BIOS / CUDA   |
| ADL + Overdrive 5–8         | AMD clocks / temp / voltage / fans / BIOS             |
| Intel Graphics Control Lib  | Intel clocks / temp / memory / bus                    |

## Requirements

### Running the app

- Windows 7 or newer (Windows 11 recommended)
- Official driver for the GPU you want sensors from:
  - NVIDIA -> driver that includes NVAPI
  - AMD -> driver with ADL support
  - Intel -> driver with Intel Graphics Control Library

### Building

- Visual Studio 2022
- Windows 10/11 SDK
- Vulkan SDK (headers must be on the include path, or set `$(VULKAN_SDK)` / adjust Additional Include Directories)
- OpenCL headers are already shipped in `OpenCL-SDK-v2026.05.29-Win-x64/include` - nothing extra to install

## Building

1. Install Visual Studio 2022
2. Make sure Vulkan headers are visible (install the official Vulkan SDK or point the project at your own copy)
3. Open `GPU-X.sln`
4. Select **Release**
5. Build Solution (`Ctrl+Shift+B`)

The project only links against system libs (`dxgi`, `setupapi`, `opengl32`, `cfgmgr32`, `Dwmapi`, `Shcore`). Vendor SDKs are loaded with `LoadLibrary` at runtime, so the absence of NVAPI / ADL / igcl does not break the build or the launch - the corresponding features simply stay unavailable.

## Running

Just start `GPU-X.exe`. It will:

1. Enumerate DXGI adapters
2. Create the matching `NVIDIA_GPU` / `AMD_GPU` / `Intel_GPU` object from the Vendor ID
3. Fill in static info (name, memory, bus, BIOS, supported APIs)
4. Open the main window with the “Graphics Card” and “Sensors” tabs
5. Start polling sensors and updating the history graphs

If you have more than one GPU, use the combo box at the bottom to switch between them.

## Repo layout

```
GPU-X/
├── GPU-X.sln
├── icon.ico
├── LICENSE.txt
├── OpenCL-SDK-v2026.05.29-Win-x64/   # OpenCL headers used at build time
└── GPU-X/
    ├── main.cpp
    ├── GPU.h / GPU.cpp              # base class
    ├── GPUFactory.h / .cpp
    ├── NVIDIA GPU.h / .cpp
    ├── AMD GPU.h / .cpp
    ├── Intel GPU.h / .cpp
    ├── Overdrive*.h / .cpp          # AMD Overdrive generations 5–8
    ├── MainWindow.h / .cpp
    ├── Button.h / .cpp
    ├── ComboBox.h / .cpp
    ├── DPIManager.h / DPI.cpp
    ├── StringManager.h / .cpp
    ├── Themes.h
    ├── Globals.h / .cpp
    ├── WindowHelpers.h / .cpp
    ├── VersionHelper.h
    ├── CommonID.h / resource.h
    ├── GPU-X.rc                     # RU / EN / DE string tables + icon
    ├── GPU-X.manifest
    ├── AMD/  NVIDIA/  Intel/        # private vendor headers
    └── x64/                         
```

## Localization

Strings live in `GPU-X.rc` as separate `STRINGTABLE` blocks for each `LANGUAGE`. Currently:

- Russian
- English
- German

Language is chosen automatically from the system locale.

To add another language:

1. Copy a `STRINGTABLE` block and set the proper `LANGUAGE` / `SUBLANG`
2. Translate every `IDS_*` string
3. Rebuild

## License

GNU General Public License v3.0 - see [LICENSE.txt](./LICENSE.txt).

## Disclaimer

NVIDIA, AMD, Intel and their respective APIs (NVAPI, ADL, Intel Graphics Control Library) are trademarks of their owners. The header files included in this repository are used only to talk to the official drivers and are not covered by the project’s GPL license.

