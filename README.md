\# 🌡️ STM32 FreeRTOS Environmental Monitoring \& Control System



Real-time environmental monitoring and control system developed on \*\*STM32F407VGT6\*\* using \*\*FreeRTOS / CMSIS-RTOS2\*\* and validated with \*\*Renode\*\*.



The system monitors temperature, humidity and atmospheric pressure, automatically controls ventilation, detects critical environmental conditions, manages sensor failures using a fail-safe strategy, and performs automatic sensor recovery.



\---



\## 📌 Main Features



\- Environmental monitoring: temperature, humidity and pressure

\- Automatic ventilation control

\- Environmental alarm detection

\- BME280 sensor abstraction

\- I²C-ready sensor architecture

\- FreeRTOS multitasking

\- Inter-task communication using Message Queues

\- UART protection using Mutex

\- Alarm synchronization using Binary Semaphore

\- Global system state using Event Flags

\- Periodic Health Monitoring using Software Timer

\- Sensor fault detection

\- Fail-safe operation

\- Automatic sensor recovery

\- Functional validation using Renode



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



\## 🔄 System Control Flow



```text

BME280 / Simulation

&#x20;       ↓

&#x20;   SensorTask

&#x20;       ↓

&#x20; Sensor Queue

&#x20;       ↓

&#x20;  ControlTask

&#x20;   ↓    ↓    ↓

&#x20;  FAN  Events Status Queue

&#x20;              ↓

&#x20;           UARTTask



ControlTask

&#x20;   ↓

Semaphore

&#x20;   ↓

AlarmTask



Software Timer

&#x20;   ↓

Health Monitoring

```



\---



\## 🛡️ Fault Handling and Recovery



When a sensor failure occurs:



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

```



Example UART output:



```text

\*\*\* SENSOR FAULT - FAIL SAFE ACTIVE \*\*\*



SENSOR READ ERROR | FAN = ON | STATE = SENSOR\_FAULT

```



After successful recovery:



```text

\[RECOVERY] BME280 RECOVERED SUCCESSFULLY

```



The controller then resumes environmental monitoring automatically.



\---



\## ❤️ Health Monitoring



A periodic FreeRTOS Software Timer supervises the global system state.



Example messages:



```text

\[HEALTH] SYSTEM OK

\[HEALTH] SYSTEM OK | VENTILATION ACTIVE

\[HEALTH] ENVIRONMENT ALARM

\[HEALTH] SENSOR FAULT | FAIL-SAFE ACTIVE

\[HEALTH] SENSOR RECOVERED | SYSTEM OPERATIONAL

```



\---



\## 🚩 Event Flags



The application uses Event Flags to represent the global system state.



```c

\#define EVENT\_SENSOR\_OK         (1U << 0)

\#define EVENT\_VENTILATION       (1U << 1)

\#define EVENT\_ENV\_ALARM         (1U << 2)

\#define EVENT\_SENSOR\_FAULT      (1U << 3)

\#define EVENT\_SENSOR\_RECOVERED  (1U << 4)

```



| System State | Active Event Flags |

|---|---|

| `NORMAL` | `EVENT\_SENSOR\_OK` |

| `VENTILATION` | `EVENT\_SENSOR\_OK` + `EVENT\_VENTILATION` |

| `ALARM` | `EVENT\_SENSOR\_OK` + `EVENT\_ENV\_ALARM` |

| `SENSOR\_FAULT` | `EVENT\_SENSOR\_FAULT` |

| Recovery | `EVENT\_SENSOR\_OK` + `EVENT\_SENSOR\_RECOVERED` |



\---



\## 🌡️ BME280 Driver



The project includes a dedicated BME280 abstraction layer:



```text

Core/Inc/bme280.h

Core/Src/bme280.c

```



The current Renode validation uses:



```c

\#define BME280\_SIMULATION\_MODE 1

```



The simulation backend generates deterministic environmental values such as:



```text

25 °C / 55 %

28 °C / 60 %

32 °C / 68 %

35 °C / 72 %

27 °C / 58 %

```



These values allow the different system states to be tested automatically.



The driver architecture is also prepared for future physical BME280 integration using STM32 HAL I²C functions:



```c

HAL\_I2C\_Mem\_Read()

HAL\_I2C\_Mem\_Write()

HAL\_I2C\_IsDeviceReady()

```



> Current validation uses the simulated BME280 backend. Physical hardware validation remains future work.



\---



\## 🧪 Validation Results



| Scenario | Expected Behavior | Result |

|---|---|:---:|

| Normal operation | FAN OFF + `NORMAL` | ✅ |

| Automatic ventilation | FAN ON + `VENTILATION` | ✅ |

| Environmental alarm | FAN ON + `ALARM` | ✅ |

| Sensor fault detection | `SENSOR\_FAULT` detected | ✅ |

| Fail-safe operation | FAN forced ON | ✅ |

| Automatic recovery | Sensor operation restored | ✅ |

| Health monitoring | Periodic reports generated | ✅ |

| UART synchronization | Mutex protection active | ✅ |

| Inter-task communication | Message Queues operational | ✅ |



\---



\## 📊 Renode Simulation Result



The complete application behavior was validated using Renode.



The UART output demonstrates the following sequence:



```text

NORMAL

&#x20;  ↓

VENTILATION

&#x20;  ↓

ALARM

&#x20;  ↓

SENSOR\_FAULT

&#x20;  ↓

FAIL-SAFE

&#x20;  ↓

SENSOR RECOVERY

&#x20;  ↓

NORMAL OPERATION

```



\### Full System Test



<p align="center">

&#x20; <img src="docs/screenshots/03\_full\_system\_test.png" alt="Renode Full System Test" width="900">

</p>



Example UART output:



```text

T = 25.0 C | H = 55.0 % | P = 1013.2 hPa | FAN = OFF | STATE = NORMAL



T = 32.0 C | H = 68.0 % | P = 1011.9 hPa | FAN = ON | STATE = VENTILATION



\*\*\* ALARM TRIGGERED \*\*\*



T = 35.0 C | H = 72.0 % | P = 1010.5 hPa | FAN = ON | STATE = ALARM



\*\*\* SENSOR FAULT - FAIL SAFE ACTIVE \*\*\*



SENSOR READ ERROR | FAN = ON | STATE = SENSOR\_FAULT



\[RECOVERY] BME280 RECOVERED SUCCESSFULLY



\[HEALTH] SYSTEM OK

```



\---



\## 🛠️ Technologies



| Category | Technology |

|---|---|

| Microcontroller | STM32F407VGT6 |

| Processor | ARM Cortex-M4 |

| Programming Language | Embedded C |

| RTOS | FreeRTOS |

| RTOS API | CMSIS-RTOS2 |

| Hardware Abstraction | STM32 HAL |

| Sensor Interface | I²C |

| Diagnostics | UART |

| Sensor | BME280 abstraction |

| Development IDE | STM32CubeIDE |

| Configuration Tool | STM32CubeMX |

| Simulation | Renode |

| Version Control | Git / GitHub |



\---



\## 📂 Main Project Files



```text

STM32\_FreeRTOS\_Project/

│

├── Core/

│   ├── Inc/

│   │   ├── bme280.h

│   │   └── main.h

│   │

│   └── Src/

│       ├── main.c

│       ├── bme280.c

│       └── freertos.c

│

├── Drivers/

├── Middlewares/

├── Startup/

│

├── docs/

│   └── screenshots/

│       └── 03\_full\_system\_test.png

│

├── STM32\_FreeRTOS\_Project.ioc

├── .gitignore

└── README.md

```



\---



\## 🚀 Future Work



\- Physical STM32F407 hardware validation

\- Real BME280 hardware integration

\- Independent watchdog supervision

\- ESP32 / MQTT connectivity

\- IoT dashboard

\- Environmental data logging

\- CAN communication

\- Low-power operating modes

\- TinyML-based anomaly detection



\---



\## ✅ Project Status



The \*\*FreeRTOS architecture, environmental control logic, fault detection, fail-safe behavior, health monitoring and automatic recovery\*\* have been functionally validated using Renode.



Physical STM32F407 and BME280 hardware validation is planned as future work.

