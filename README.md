<div align="center">



\# 🌡️ STM32 FreeRTOS Environmental Monitoring \& Control System



\### Real-Time Environmental Monitoring, Fault Management and Automatic Recovery



!\[STM32](https://img.shields.io/badge/MCU-STM32F407VGT6-blue)

!\[FreeRTOS](https://img.shields.io/badge/RTOS-FreeRTOS-green)

!\[CMSIS](https://img.shields.io/badge/API-CMSIS--RTOS2-orange)

!\[Language](https://img.shields.io/badge/Language-Embedded%20C-informational)

!\[Simulation](https://img.shields.io/badge/Simulation-Renode-purple)

!\[Status](https://img.shields.io/badge/Status-Functional%20Prototype-success)



\*\*STM32F407 · FreeRTOS · CMSIS-RTOS2 · BME280 · I²C · UART · Renode\*\*



</div>



\---



\## 📌 Overview



This project implements a \*\*real-time environmental monitoring and control system\*\* based on the \*\*STM32F407VGT6\*\* microcontroller and \*\*FreeRTOS\*\*.



The application monitors:



\- 🌡️ Temperature

\- 💧 Relative humidity

\- 🌬️ Atmospheric pressure



Based on the environmental measurements, the controller automatically manages ventilation and detects abnormal operating conditions.



The project also integrates:



\- multitasking with FreeRTOS;

\- inter-task communication;

\- alarm synchronization;

\- global system state management;

\- sensor fault detection;

\- fail-safe operation;

\- periodic health monitoring;

\- automatic sensor recovery.



The current implementation is functionally validated using \*\*Renode simulation\*\*.



A modular BME280 driver abstraction is used so that the application can later be connected to a physical BME280 sensor through I²C without redesigning the application architecture.



\---



\## ⚙️ Key Features



\- STM32F407VGT6 microcontroller

\- ARM Cortex-M4

\- Embedded C

\- FreeRTOS / CMSIS-RTOS2

\- BME280 sensor abstraction

\- I²C communication

\- USART2 diagnostics

\- Multi-task real-time architecture

\- Message Queues

\- Mutex

\- Binary Semaphore

\- Event Flags

\- Software Timer

\- Environmental control logic

\- Sensor fault detection

\- Fail-safe ventilation

\- Periodic health monitoring

\- Automatic sensor recovery

\- Renode simulation and validation



\---



\## 🧠 System Architecture



```mermaid

flowchart TD



&#x20;   A\[BME280 / Simulation Backend] --> B\[SensorTask]

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



The application separates acquisition, control, communication, alarm handling and supervision into independent software components.



\---



\## 🚦 Operating States



| State | Condition | Fan | Action |

|---|---|:---:|---|

| 🟢 `NORMAL` | T ≤ 30 °C and H ≤ 65% | OFF | Normal monitoring |

| 🟡 `VENTILATION` | T > 30 °C or H > 65% | ON | Automatic ventilation |

| 🔴 `ALARM` | T > 34 °C or H > 75% | ON | Environmental alarm |

| ⚠️ `SENSOR\_FAULT` | BME280 reading failure | ON | Fail-safe mode |



\---



\## 🧵 FreeRTOS Mechanisms



| Mechanism | Role |

|---|---|

| Tasks | Separate application responsibilities |

| Sensor Queue | Transfers measurements from `SensorTask` to `ControlTask` |

| Status Queue | Transfers processed state from `ControlTask` to `UARTTask` |

| Mutex | Protects USART2 against concurrent access |

| Binary Semaphore | Synchronizes alarm activation with `AlarmTask` |

| Event Flags | Represents the global system state |

| Software Timer | Performs periodic health monitoring |



\---



\## 🔄 Main Tasks



\### SensorTask



`SensorTask` is responsible for acquiring environmental information from the BME280 driver abstraction.



It handles:



```text

Temperature

Humidity

Pressure

Sensor status

```



The acquired information is transmitted to `ControlTask` through a FreeRTOS Message Queue.



\### ControlTask



`ControlTask` analyzes the environmental measurements and determines the operating state:



```text

NORMAL

VENTILATION

ALARM

SENSOR\_FAULT

```



It also manages:



\- ventilation output;

\- Event Flags;

\- environmental alarm generation;

\- fail-safe operation.



\### UARTTask



`UARTTask` receives processed system information and transmits diagnostic messages through USART2.



Example:



```text

T = 25.0 C | H = 55.0 % | P = 1013.2 hPa | FAN = OFF | STATE = NORMAL

```



USART2 access is protected using a FreeRTOS Mutex.



\### AlarmTask



`AlarmTask` waits on a Binary Semaphore and handles alarm notifications.



The system supports:



```text

Environmental alarm

Sensor fault alarm

Manual alarm

```



\---



\## 🚩 Event Flags



The system uses Event Flags to represent its global status.



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



The project contains a dedicated BME280 abstraction layer:



```text

Core/Inc/bme280.h

Core/Src/bme280.c

```



Two modes are considered by the driver architecture.



\### Simulation Mode



Used for Renode functional validation.



```c

\#define BME280\_SIMULATION\_MODE 1

```



The simulation backend provides deterministic environmental measurements such as:



```text

25 °C / 55 %

28 °C / 60 %

32 °C / 68 %

35 °C / 72 %

27 °C / 58 %

```



These values allow the different control states to be tested automatically.



\### Hardware Mode



The driver structure is prepared for future communication with a physical BME280 using STM32 HAL I²C functions such as:



```c

HAL\_I2C\_Mem\_Read()

HAL\_I2C\_Mem\_Write()

HAL\_I2C\_IsDeviceReady()

```



BME280 identification uses:



```text

Chip ID register : 0xD0

Expected value   : 0x60

```



> Current validation uses the simulated BME280 backend. Physical BME280 hardware validation is planned as future work.



\---



\## 🛡️ Fault Handling and Recovery



When a sensor reading fails, the controller enters a fail-safe state.



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



The fail-safe strategy forces ventilation ON because environmental conditions cannot be considered reliable while the sensor is unavailable.



\---



\## ♻️ Automatic Sensor Recovery



After detecting a sensor failure, the application attempts to restore the sensor automatically.



```mermaid

flowchart TD



&#x20;   A\[SENSOR\_FAULT] --> B\[BME280\_Init]

&#x20;   B --> C\[BME280\_Read]

&#x20;   C --> D{Reading successful?}



&#x20;   D -->|Yes| E\[SENSOR RECOVERED]

&#x20;   E --> F\[Resume monitoring]



&#x20;   D -->|No| G\[Wait before retry]

&#x20;   G --> B

```



Successful recovery generates:



```text

\[RECOVERY] BME280 RECOVERED SUCCESSFULLY

```



The controller then automatically resumes normal environmental monitoring.



\---



\## ❤️ Health Monitoring



A FreeRTOS Software Timer periodically supervises the global system state through Event Flags.



Example reports:



```text

\[HEALTH] SYSTEM OK

```



```text

\[HEALTH] SYSTEM OK | VENTILATION ACTIVE

```



```text

\[HEALTH] ENVIRONMENT ALARM

```



```text

\[HEALTH] SENSOR FAULT | FAIL-SAFE ACTIVE

```



```text

\[HEALTH] SENSOR RECOVERED | SYSTEM OPERATIONAL

```



\---



\## 🧪 Validation



The application was tested under Renode using several operating scenarios.



| Scenario | Expected Behavior | Result |

|---|---|:---:|

| Normal operation | FAN OFF + `NORMAL` | ✅ |

| Elevated temperature/humidity | FAN ON + `VENTILATION` | ✅ |

| Critical environment | FAN ON + `ALARM` | ✅ |

| Environmental alarm | `AlarmTask` activated | ✅ |

| Sensor fault | `SENSOR\_FAULT` detected | ✅ |

| Fail-safe mode | FAN forced ON | ✅ |

| Automatic recovery | Sensor operation restored | ✅ |

| Health monitoring | Periodic reports generated | ✅ |

| UART synchronization | Mutex protection active | ✅ |

| Inter-task communication | Queues operational | ✅ |



\---



\## 📊 Simulation Result



The following Renode UART output demonstrates the complete control sequence:



```text

NORMAL

&#x20;    ↓

VENTILATION

&#x20;    ↓

ALARM

&#x20;    ↓

SENSOR\_FAULT

&#x20;    ↓

FAIL-SAFE

&#x20;    ↓

SENSOR RECOVERY

&#x20;    ↓

NORMAL OPERATION

```



\### Full System Test



<p align="center">

&#x20; <img src="docs/screenshots/03\_full\_system\_test.png" alt="Renode Full System Test" width="900">

</p>



Example UART messages:



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



\## 🖥️ Renode Simulation



Example Renode commands used to execute the firmware:



```text

mach create "STM32F4"



machine LoadPlatformDescription @platforms/boards/stm32f4\_discovery-kit.repl



sysbus LoadELF @C:/STM32\_Projects/STM32\_FreeRTOS\_Project/Debug/STM32\_FreeRTOS\_Project.elf



showAnalyzer sysbus.usart2



start

```



\---



\## 📂 Project Structure



```text

STM32\_FreeRTOS\_Project/

│

├── Core/

│   ├── Inc/

│   │   ├── bme280.h

│   │   ├── main.h

│   │   └── FreeRTOSConfig.h

│   │

│   └── Src/

│       ├── main.c

│       ├── bme280.c

│       ├── freertos.c

│       ├── stm32f4xx\_it.c

│       └── stm32f4xx\_hal\_msp.c

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

├── STM32F407VGTX\_FLASH.ld

├── STM32F407VGTX\_RAM.ld

├── .gitignore

└── README.md

```



\---



\## 🛠️ Technologies



| Category | Technologies |

|---|---|

| MCU | STM32F407VGT6 |

| Processor | ARM Cortex-M4 |

| Programming | Embedded C |

| RTOS | FreeRTOS |

| API | CMSIS-RTOS2 |

| Hardware Abstraction | STM32 HAL |

| Communication | I²C, UART |

| Sensor | BME280 abstraction |

| IDE | STM32CubeIDE |

| Configuration | STM32CubeMX |

| Simulation | Renode |

| Version Control | Git / GitHub |



\---



\## 🚀 Future Work



Possible extensions include:



\- Physical STM32F407 hardware validation

\- Real BME280 hardware integration

\- Independent watchdog supervision

\- ESP32 Wi-Fi connectivity

\- MQTT communication

\- IoT dashboard

\- Environmental data logging

\- SD card storage

\- CAN communication

\- Low-power operation

\- TinyML-based anomaly detection

\- Predictive environmental control



\---



\## ✅ Current Status



The real-time application architecture and control logic have been \*\*functionally validated under Renode simulation\*\*.



The current version demonstrates:



\- environmental acquisition;

\- real-time task execution;

\- automatic ventilation control;

\- environmental alarm handling;

\- sensor fault detection;

\- fail-safe behavior;

\- health monitoring;

\- automatic sensor recovery.



Physical hardware validation remains a future extension.



\---



<div align="center">



\### STM32 · FreeRTOS · Embedded C · Renode



\*\*Real-Time Environmental Monitoring and Fault-Tolerant Control\*\*



</div>

