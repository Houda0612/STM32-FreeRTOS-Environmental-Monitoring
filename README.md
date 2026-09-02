<div align="center">



\# 🌡️ STM32 FreeRTOS Environmental Monitoring System



Real-time environmental monitoring and fault-tolerant control system based on \*\*STM32F407VGT6\*\*, \*\*FreeRTOS\*\*, \*\*BME280 abstraction\*\*, and \*\*Renode simulation\*\*.



\*\*STM32F407 · FreeRTOS · Embedded C · I²C · UART · Renode\*\*



</div>



\---



\## 📌 Overview



This project implements a real-time environmental monitoring and control application using STM32 and FreeRTOS.



The system monitors:



\- Temperature

\- Humidity

\- Atmospheric pressure



It automatically manages ventilation, detects critical environmental conditions, handles sensor failures, activates fail-safe behavior, and performs automatic sensor recovery.



\---



\## ⚙️ Key Features



\- FreeRTOS / CMSIS-RTOS2

\- BME280 sensor abstraction

\- I²C communication

\- UART diagnostics

\- SensorTask, ControlTask, UARTTask, AlarmTask

\- Message Queues

\- Mutex

\- Binary Semaphore

\- Event Flags

\- Software Timer

\- Health Monitoring

\- Sensor Fault Detection

\- Fail-Safe Mode

\- Automatic Sensor Recovery

\- Renode Simulation



\---



\## 🧠 System Architecture



```mermaid

flowchart TD



&#x20;   BME\[BME280 / Simulation]

&#x20;   SENSOR\[SensorTask]

&#x20;   QUEUE1\[Sensor Queue]

&#x20;   CONTROL\[ControlTask]

&#x20;   FAN\[FAN]

&#x20;   FLAGS\[Event Flags]

&#x20;   QUEUE2\[Status Queue]

&#x20;   UART\[UARTTask]

&#x20;   SEM\[Semaphore]

&#x20;   ALARM\[AlarmTask]

&#x20;   TIMER\[Software Timer]

&#x20;   HEALTH\[Health Monitoring]

&#x20;   RECOVERY\[Sensor Recovery]



&#x20;   BME --> SENSOR

&#x20;   SENSOR --> QUEUE1

&#x20;   QUEUE1 --> CONTROL



&#x20;   CONTROL --> FAN

&#x20;   CONTROL --> FLAGS

&#x20;   CONTROL --> QUEUE2

&#x20;   QUEUE2 --> UART



&#x20;   CONTROL --> SEM

&#x20;   SEM --> ALARM



&#x20;   TIMER --> HEALTH

&#x20;   FLAGS --> HEALTH



&#x20;   SENSOR --> RECOVERY

&#x20;   RECOVERY --> BME

```



\---



\## 🚦 Operating States



| State | Condition | Fan |

|---|---|:---:|

| `NORMAL` | T ≤ 30 °C and H ≤ 65% | OFF |

| `VENTILATION` | T > 30 °C or H > 65% | ON |

| `ALARM` | T > 34 °C or H > 75% | ON |

| `SENSOR\_FAULT` | BME280 reading failure | ON |



\---



\## 🧵 FreeRTOS Mechanisms



| Mechanism | Role |

|---|---|

| Tasks | Separate application responsibilities |

| Message Queues | Inter-task communication |

| Mutex | UART protection |

| Binary Semaphore | Alarm synchronization |

| Event Flags | Global system state |

| Software Timer | Periodic health monitoring |



\---



\## 🛡️ Fault Handling and Recovery



When a sensor failure occurs:



```text

BME280 Read Error

&#x20;     ↓

SENSOR\_FAULT

&#x20;     ↓

FAN forced ON

&#x20;     ↓

Alarm triggered

&#x20;     ↓

Automatic recovery attempt

```



After successful recovery:



```text

\[RECOVERY] BME280 RECOVERED SUCCESSFULLY

```



The controller then resumes normal monitoring automatically.



\---



\## 🧪 Validation



The system was tested in Renode for the following scenarios:



| Scenario | Result |

|---|:---:|

| Normal operation | ✅ |

| Ventilation activation | ✅ |

| Environmental alarm | ✅ |

| Sensor fault | ✅ |

| Fail-safe behavior | ✅ |

| Automatic recovery | ✅ |

| Health monitoring | ✅ |



\---



\## 📊 Simulation Result



<p align="center">

&#x20; <img src="docs/screenshots/03\_full\_system\_test.png" alt="Renode Full System Test" width="900">

</p>



Example UART messages:



```text

STATE = NORMAL

STATE = VENTILATION

STATE = ALARM



\*\*\* SENSOR FAULT - FAIL SAFE ACTIVE \*\*\*



\[RECOVERY] BME280 RECOVERED SUCCESSFULLY



\[HEALTH] SYSTEM OK

```



\---



\## 📂 Project Structure



```text

STM32\_FreeRTOS\_Project/

├── Core/

│   ├── Inc/

│   │   ├── bme280.h

│   │   └── main.h

│   └── Src/

│       ├── main.c

│       ├── bme280.c

│       └── freertos.c

├── Drivers/

├── Middlewares/

├── Startup/

├── docs/

│   └── screenshots/

│       └── 03\_full\_system\_test.png

├── STM32\_FreeRTOS\_Project.ioc

├── .gitignore

└── README.md

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

\- BME280

\- STM32CubeIDE

\- STM32CubeMX

\- Renode

\- Git / GitHub



\---



\## 🚀 Future Work



\- Physical STM32F407 validation

\- Real BME280 hardware integration

\- Watchdog supervision

\- ESP32 / MQTT connectivity

\- IoT dashboard

\- Data logging

\- TinyML-based anomaly detection



\---



<div align="center">



\*\*STM32 · FreeRTOS · Embedded Systems · Renode\*\*



</div>

