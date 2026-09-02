\# 🌡️ STM32 FreeRTOS Environmental Monitoring \& Control System



Real-time environmental monitoring and control system based on \*\*STM32F407VGT6\*\*, \*\*FreeRTOS / CMSIS-RTOS2\*\*, BME280 abstraction and \*\*Renode simulation\*\*.



The system monitors temperature, humidity and pressure, controls ventilation, detects critical conditions, handles sensor failures using a fail-safe strategy and performs automatic recovery.



\## ⚙️ Main Features



\- STM32F407VGT6 + FreeRTOS

\- Temperature, humidity and pressure monitoring

\- Automatic ventilation control

\- Environmental alarm detection

\- BME280 / I²C architecture

\- UART diagnostics

\- Message Queues, Mutex and Binary Semaphore

\- Event Flags and Software Timer

\- Sensor fault detection

\- Fail-safe mode

\- Automatic sensor recovery

\- Renode validation



\## 🧠 Architecture



```mermaid

flowchart TD

&#x20;   A\[BME280 / Simulation] --> B\[SensorTask]

&#x20;   B --> C\[Sensor Queue]

&#x20;   C --> D\[ControlTask]



&#x20;   D --> E\[FAN]

&#x20;   D --> F\[Event Flags]

&#x20;   D --> G\[Status Queue]

&#x20;   G --> H\[UARTTask]



&#x20;   D --> I\[Semaphore]

&#x20;   I --> J\[AlarmTask]



&#x20;   K\[Software Timer] --> L\[Health Monitoring]

&#x20;   F --> L

```



\## 🚦 Operating States



| State | Condition | Fan |

|---|---|:---:|

| `NORMAL` | T ≤ 30 °C and H ≤ 65% | OFF |

| `VENTILATION` | T > 30 °C or H > 65% | ON |

| `ALARM` | T > 34 °C or H > 75% | ON |

| `SENSOR\_FAULT` | Sensor reading failure | ON |



\## 🧵 FreeRTOS



| Mechanism | Role |

|---|---|

| Tasks | Application functions |

| Message Queues | Inter-task communication |

| Mutex | UART protection |

| Binary Semaphore | Alarm synchronization |

| Event Flags | System state |

| Software Timer | Health monitoring |



\## 🛡️ Fault Handling



```text

Sensor Error

&#x20;   ↓

SENSOR\_FAULT

&#x20;   ↓

FAN forced ON

&#x20;   ↓

Alarm

&#x20;   ↓

Automatic Recovery

```



Successful recovery:



```text

\[RECOVERY] BME280 RECOVERED SUCCESSFULLY

```



\## 🧪 Renode Validation



| Scenario | Result |

|---|:---:|

| Normal operation | ✅ |

| Ventilation | ✅ |

| Environmental alarm | ✅ |

| Sensor fault | ✅ |

| Fail-safe mode | ✅ |

| Automatic recovery | ✅ |

| Health monitoring | ✅ |



\### Simulation Result



<p align="center">

&#x20; <img src="docs/screenshots/03\_full\_system\_test.png" alt="Renode Full System Test" width="900">

</p>



\## 🛠️ Technologies



\*\*STM32F407 · ARM Cortex-M4 · Embedded C · FreeRTOS · CMSIS-RTOS2 · STM32 HAL · I²C · UART · BME280 · STM32CubeIDE · Renode · Git/GitHub\*\*



\## 🚀 Future Work



\- Physical STM32F407 + BME280 validation

\- ESP32 / MQTT connectivity

\- IoT dashboard

\- Watchdog supervision

\- TinyML anomaly detection

