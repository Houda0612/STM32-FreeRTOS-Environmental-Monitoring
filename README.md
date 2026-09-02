\# 🌡️ STM32 FreeRTOS Environmental Monitoring \& Control System



Real-time environmental monitoring and control system developed on \*\*STM32F407VGT6\*\* using \*\*FreeRTOS / CMSIS-RTOS2\*\* and validated with \*\*Renode\*\*.



The system monitors temperature, humidity and atmospheric pressure, automatically controls ventilation, detects critical environmental conditions, manages sensor failures using a fail-safe strategy, and performs automatic sensor recovery.



\---



\## 📌 Main Features



\- Environmental monitoring: temperature, humidity and pressure

\- Automatic ventilation control

\- Environmental alarm detection

\- BME280 sensor abstraction with I²C-ready architecture

\- FreeRTOS multitasking

\- Inter-task communication using Message Queues

\- UART protection using Mutex

\- Alarm synchronization using Binary Semaphore

\- Global system status using Event Flags

\- Periodic Health Monitoring using Software Timer

\- Sensor fault detection

\- Fail-safe operation

\- Automatic sensor recovery

\- Functional validation with Renode



\---



\## 🧠 System Architecture



```mermaid

flowchart TD



&#x20;   A\[BME280 / Simulation] --> B\[SensorTask]

&#x20;   B --> C\[Sensor Queue]

&#x20;   C --> D\[ControlTask]



&#x20;   D --> E\[FAN Control]

&#x20;   D --> F\[Event Flags]

&#x20;   D --> G\[Status Queue]

&#x20;   G --> H\[UARTTask]



&#x20;   D --> I\[Alarm Semaphore]

&#x20;   I --> J\[AlarmTask]



&#x20;   K\[Software Timer] --> L\[Health Monitoring]

&#x20;   F --> L



&#x20;   B --> M\[Sensor Recovery]

&#x20;   M --> A

```



\---



\## 🧵 FreeRTOS Architecture



| Component | Role |

|---|---|

| `SensorTask` | Environmental data acquisition |

| `ControlTask` | Decision logic and FAN control |

| `UARTTask` | Diagnostic messages |

| `AlarmTask` | Alarm management |

| Message Queues | Communication between tasks |

| Mutex | USART2 protection |

| Binary Semaphore | Alarm synchronization |

| Event Flags | Global system status |

| Software Timer | Periodic health monitoring |



\---



\## 🚦 Operating States



| State | Condition | Fan |

|---|---|:---:|

| `NORMAL` | T ≤ 30 °C and H ≤ 65% | OFF |

| `VENTILATION` | T > 30 °C or H > 65% | ON |

| `ALARM` | T > 34 °C or H > 75% | ON |

| `SENSOR\_FAULT` | Sensor reading failure | ON |



In `SENSOR\_FAULT`, the FAN is forced ON as a \*\*fail-safe measure\*\*.



\---



\## 🛡️ Fault Handling \& Recovery



```text

BME280 Read Error

&#x20;       ↓

SENSOR\_FAULT

&#x20;       ↓

FAN forced ON

&#x20;       ↓

Alarm triggered

&#x20;       ↓

Automatic recovery attempt

&#x20;       ↓

BME280 recovered

&#x20;       ↓

Resume normal monitoring

```



Example:



```text

\*\*\* SENSOR FAULT - FAIL SAFE ACTIVE \*\*\*



SENSOR READ ERROR | FAN = ON | STATE = SENSOR\_FAULT



\[RECOVERY] BME280 RECOVERED SUCCESSFULLY

```



\---



\## ❤️ Health Monitoring



A periodic FreeRTOS Software Timer supervises the system state.



Example messages:



```text

\[HEALTH] SYSTEM OK

\[HEALTH] SYSTEM OK | VENTILATION ACTIVE

\[HEALTH] ENVIRONMENT ALARM

\[HEALTH] SENSOR FAULT | FAIL-SAFE ACTIVE

\[HEALTH] SENSOR RECOVERED | SYSTEM OPERATIONAL

```



\---



\## 🧪 Validation Results



| Scenario | Result |

|---|:---:|

| Normal operation | ✅ |

| Automatic ventilation | ✅ |

| Environmental alarm | ✅ |

| Sensor fault detection | ✅ |

| Fail-safe operation | ✅ |

| Automatic recovery | ✅ |

| Periodic health monitoring | ✅ |



\---



\## 📊 Renode Simulation Result



The complete application behavior was validated using Renode.



<p align="center">

&#x20; <img src="docs/screenshots/03\_full\_system\_test.png" alt="Renode Full System Test" width="900">

</p>



The UART output demonstrates transitions between:



```text

NORMAL

→ VENTILATION

→ ALARM

→ SENSOR\_FAULT

→ RECOVERY

→ NORMAL

```



\---



\## 🛠️ Technologies



\- STM32F407VGT6

\- ARM Cortex-M4

\- Embedded C

\- FreeRTOS

\- CMSIS-RTOS2

\- STM32 HAL

\- I²C

\- UART

\- BME280 abstraction

\- STM32CubeIDE

\- STM32CubeMX

\- Renode

\- Git / GitHub



\---



\## 📂 Main Project Files



```text

Core/

├── Inc/

│   ├── bme280.h

│   └── main.h

└── Src/

&#x20;   ├── main.c

&#x20;   ├── bme280.c

&#x20;   └── freertos.c



docs/

└── screenshots/

&#x20;   └── 03\_full\_system\_test.png

```



\---



\## 🚀 Future Work



\- Physical STM32F407 validation

\- Real BME280 hardware integration

\- ESP32 / MQTT connectivity

\- IoT dashboard and data logging

\- Watchdog supervision

\- TinyML-based anomaly detection



\---



> \*\*Current status:\*\* The control logic and FreeRTOS architecture have been functionally validated under Renode simulation. Physical hardware validation remains future work.

