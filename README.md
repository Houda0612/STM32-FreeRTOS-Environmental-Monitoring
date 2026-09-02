<div align="center">



\# 🌡️ STM32 FreeRTOS Environmental Monitoring \& Control System



\### Real-Time Embedded Environmental Monitoring, Fault Management and Automatic Recovery



!\[STM32](https://img.shields.io/badge/MCU-STM32F407VGT6-blue)

!\[FreeRTOS](https://img.shields.io/badge/RTOS-FreeRTOS-green)

!\[CMSIS](https://img.shields.io/badge/API-CMSIS--RTOS2-orange)

!\[Language](https://img.shields.io/badge/Language-C-informational)

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



Based on these measurements, the controller automatically manages a simulated ventilation system and detects critical environmental conditions.



The project also implements several reliability mechanisms commonly used in embedded systems:



\- sensor fault detection;

\- fail-safe control;

\- automatic sensor recovery;

\- periodic system health monitoring;

\- inter-task communication and synchronization.



The firmware is currently validated using \*\*Renode simulation\*\*.  

The BME280 layer is implemented using a modular driver abstraction, allowing the same application architecture to be adapted later to a physical BME280 connected through I²C.



\---



\## 🎯 Project Objectives



The project was designed to demonstrate practical implementation of:



| Objective | Implementation |

|---|---|

| Real-time execution | FreeRTOS / CMSIS-RTOS2 |

| Environmental acquisition | BME280 driver abstraction |

| Task communication | Message Queues |

| Resource protection | Mutex |

| Alarm synchronization | Binary Semaphore |

| Global system status | Event Flags |

| Periodic supervision | Software Timer |

| Fault management | Sensor fault detection |

| Safety strategy | Fail-safe ventilation |

| Recovery | Automatic BME280 reinitialization |

| Validation | Renode simulation |



\---



\# 🧠 System Architecture



```mermaid

flowchart TD



&#x20;   BME\[BME280 / Simulation Backend]



&#x20;   SENSOR\[SensorTask]

&#x20;   QUEUE1\[Sensor Message Queue]

&#x20;   CONTROL\[ControlTask]



&#x20;   FAN\[Ventilation / FAN]

&#x20;   FLAGS\[Event Flags]

&#x20;   QUEUE2\[Status Queue]



&#x20;   UART\[UARTTask]

&#x20;   MUTEX\[UART Mutex]



&#x20;   SEM\[Alarm Semaphore]

&#x20;   ALARM\[AlarmTask]



&#x20;   TIMER\[FreeRTOS Software Timer]

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



&#x20;   UART --> MUTEX

&#x20;   ALARM --> MUTEX



&#x20;   TIMER --> HEALTH

&#x20;   FLAGS --> HEALTH



&#x20;   SENSOR --> RECOVERY

&#x20;   RECOVERY --> BME

```



\---



\# ⚙️ Hardware and Software Environment



| Component | Configuration |

|---|---|

| Microcontroller | STM32F407VGT6 |

| CPU | ARM Cortex-M4 |

| RTOS | FreeRTOS |

| RTOS API | CMSIS-RTOS2 |

| Environmental sensor | BME280 abstraction |

| Sensor communication | I²C1 |

| Debug interface | USART2 |

| Ventilation output | PD12 |

| Alarm LED | PD14 |

| External interrupt | PA0 / EXTI0 |

| Development environment | STM32CubeIDE |

| Peripheral configuration | STM32CubeMX |

| Simulation platform | Renode |

| Version control | Git / GitHub |



\---



\# 🧵 FreeRTOS Architecture



The firmware is divided into several independent real-time tasks.



\## `SensorTask`



Responsible for environmental data acquisition.



It obtains:



```text

Temperature

Humidity

Pressure

Sensor status

```



The measurements are transferred to `ControlTask` using a \*\*FreeRTOS Message Queue\*\*.



\---



\## `ControlTask`



Implements the system decision logic.



It analyzes the measurements and determines one of the following states:



```text

NORMAL

VENTILATION

ALARM

SENSOR\_FAULT

```



It is also responsible for:



\- ventilation control;

\- Event Flags management;

\- alarm generation;

\- fail-safe behavior.



\---



\## `UARTTask`



Responsible for diagnostic output through \*\*USART2\*\*.



Example:



```text

T = 25.0 C | H = 55.0 % | P = 1013.2 hPa | FAN = OFF | STATE = NORMAL

```



USART2 is protected using a \*\*FreeRTOS Mutex\*\* because multiple tasks may attempt to use the UART.



\---



\## `AlarmTask`



Responsible for alarm signaling.



The task remains blocked until a \*\*Binary Semaphore\*\* is released.



Supported alarm sources:



```text

Environmental alarm

Sensor fault

Manual EXTI alarm

```



Example:



```text

\*\*\* ALARM TRIGGERED \*\*\*

```



or:



```text

\*\*\* SENSOR FAULT - FAIL SAFE ACTIVE \*\*\*

```



\---



\# 🔄 FreeRTOS Mechanisms Used



| FreeRTOS Mechanism | Purpose |

|---|---|

| Tasks | Separate application responsibilities |

| Message Queue | SensorTask → ControlTask communication |

| Status Queue | ControlTask → UARTTask communication |

| Mutex | Protect USART2 |

| Binary Semaphore | Wake AlarmTask |

| Event Flags | Represent global system state |

| Software Timer | Periodic health supervision |



\---



\# 🚦 Control Strategy



The environmental controller implements four operating states.



| State | Trigger | FAN |

|---|---|---|

| 🟢 `NORMAL` | T ≤ 30 °C and H ≤ 65% | OFF |

| 🟡 `VENTILATION` | T > 30 °C or H > 65% | ON |

| 🔴 `ALARM` | T > 34 °C or H > 75% | ON |

| ⚠️ `SENSOR\_FAULT` | BME280 read failure | ON |



\---



\# 🚩 Event Flags



The global operating status is represented using FreeRTOS Event Flags.



```c

\#define EVENT\_SENSOR\_OK         (1U << 0)

\#define EVENT\_VENTILATION       (1U << 1)

\#define EVENT\_ENV\_ALARM         (1U << 2)

\#define EVENT\_SENSOR\_FAULT      (1U << 3)

\#define EVENT\_SENSOR\_RECOVERED  (1U << 4)

```



\### Example state mapping



| System State | Active Event Flags |

|---|---|

| NORMAL | `SENSOR\_OK` |

| VENTILATION | `SENSOR\_OK + VENTILATION` |

| ALARM | `SENSOR\_OK + ENV\_ALARM` |

| SENSOR\_FAULT | `SENSOR\_FAULT` |

| Recovery | `SENSOR\_OK + SENSOR\_RECOVERED` |



\---



\# 🌡️ BME280 Driver Architecture



The project contains a dedicated sensor abstraction:



```text

Core/Inc/bme280.h

Core/Src/bme280.c

```



Two operating modes are supported by the architecture.



\## Simulation Mode



Used during Renode validation:



```c

\#define BME280\_SIMULATION\_MODE 1

```



A deterministic sequence of environmental measurements is generated:



```text

25 °C / 55 %

28 °C / 60 %

32 °C / 68 %

35 °C / 72 %

27 °C / 58 %

```



These values deliberately exercise the different controller states.



\---



\## Physical Sensor Mode



The driver architecture is also prepared for a physical BME280 using STM32 HAL I²C operations:



```c

HAL\_I2C\_Mem\_Read()

HAL\_I2C\_Mem\_Write()

HAL\_I2C\_IsDeviceReady()

```



BME280 identification:



```text

Register : 0xD0

Expected Chip ID : 0x60

```



> \*\*Note:\*\* Current validation was performed using the simulation backend. Physical BME280 hardware validation is planned as a future extension.



\---



\# ⚠️ Sensor Fault Detection



The simulation periodically generates a BME280 reading failure.



```mermaid

flowchart TD



&#x20;   READ\[BME280\_Read]

&#x20;   CHECK{HAL\_OK ?}



&#x20;   OK\[Continue monitoring]

&#x20;   FAULT\[SENSOR\_FAULT]

&#x20;   FAN\[FAN forced ON]

&#x20;   ALERT\[AlarmTask]

&#x20;   RECOVERY\[Recovery procedure]



&#x20;   READ --> CHECK



&#x20;   CHECK -->|Yes| OK

&#x20;   CHECK -->|No| FAULT



&#x20;   FAULT --> FAN

&#x20;   FAN --> ALERT

&#x20;   ALERT --> RECOVERY

```



Example UART output:



```text

\*\*\* SENSOR FAULT - FAIL SAFE ACTIVE \*\*\*



SENSOR READ ERROR | FAN = ON | STATE = SENSOR\_FAULT

```



\---



\# 🛡️ Fail-Safe Strategy



If the environmental sensor becomes unavailable, the system can no longer reliably determine whether environmental conditions are safe.



The controller therefore applies a conservative safety strategy:



```text

Sensor Failure

&#x20;     ↓

SENSOR\_FAULT

&#x20;     ↓

FAN forced ON

&#x20;     ↓

Alarm triggered

&#x20;     ↓

Automatic recovery attempt

```



This prevents the system from incorrectly assuming safe conditions after loss of sensor data.



\---



\# ♻️ Automatic Sensor Recovery



After a BME280 failure, `SensorTask` automatically enters recovery mode.



```mermaid

flowchart TD



&#x20;   FAULT\[SENSOR\_FAULT]



&#x20;   INIT\[BME280\_Init]

&#x20;   TEST\[BME280\_Read]



&#x20;   CHECK{Reading successful?}



&#x20;   RECOVERED\[SENSOR RECOVERED]

&#x20;   NORMAL\[Resume monitoring]

&#x20;   RETRY\[Wait and retry]



&#x20;   FAULT --> INIT

&#x20;   INIT --> TEST

&#x20;   TEST --> CHECK



&#x20;   CHECK -->|Yes| RECOVERED

&#x20;   RECOVERED --> NORMAL



&#x20;   CHECK -->|No| RETRY

&#x20;   RETRY --> INIT

```



Successful recovery produces:



```text

\[RECOVERY] BME280 RECOVERED SUCCESSFULLY

```



The controller then automatically resumes environmental monitoring.



\---



\# ❤️ Health Monitoring



A \*\*FreeRTOS Software Timer\*\* periodically checks the Event Flags representing the global state.



Possible diagnostic reports include:



```text

\[HEALTH] SYSTEM OK



\[HEALTH] SYSTEM OK | VENTILATION ACTIVE



\[HEALTH] ENVIRONMENT ALARM



\[HEALTH] SENSOR FAULT | FAIL-SAFE ACTIVE



\[HEALTH] SENSOR RECOVERED | SYSTEM OPERATIONAL

```



This provides an independent periodic supervision mechanism.



\---



\# 🧪 Validation Scenarios



| # | Test Scenario | Expected Result | Status |

|---:|---|---|:---:|

| 1 | Normal environment | FAN OFF + NORMAL | ✅ |

| 2 | Elevated temperature/humidity | FAN ON + VENTILATION | ✅ |

| 3 | Critical environment | FAN ON + ALARM | ✅ |

| 4 | Environmental alarm | AlarmTask activated | ✅ |

| 5 | BME280 failure | SENSOR\_FAULT | ✅ |

| 6 | Fail-safe operation | FAN forced ON | ✅ |

| 7 | Sensor recovery | Automatic recovery | ✅ |

| 8 | Health monitoring | Periodic reports | ✅ |

| 9 | UART protection | Mutex synchronization | ✅ |

| 10 | Inter-task communication | Queues operating correctly | ✅ |



\---



\# 🖥️ Renode Simulation



The application was functionally validated using the STM32F4 Discovery platform available in Renode.



Example commands:



```text

mach create "STM32F4"



machine LoadPlatformDescription @platforms/boards/stm32f4\_discovery-kit.repl



sysbus LoadELF @C:/STM32\_Projects/STM32\_FreeRTOS\_Project/Debug/STM32\_FreeRTOS\_Project.elf



showAnalyzer sysbus.usart2



start

```



\---



\# 📊 Simulation Results



\## Full System Validation



The following simulation demonstrates transitions through:



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



<p align="center">

&#x20; <img src="docs/screenshots/03\_full\_system\_test.png" alt="Renode Full System Test" width="900">

</p>



\### Example UART output



```text

T = 25.0 C | H = 55.0 % | P = 1013.2 hPa | FAN = OFF | STATE = NORMAL



T = 32.0 C | H = 68.0 % | P = 1011.9 hPa | FAN = ON | STATE = VENTILATION



\*\*\* ALARM TRIGGERED \*\*\*



T = 35.0 C | H = 72.0 % | P = 1010.5 hPa | FAN = ON | STATE = ALARM



\*\*\* SENSOR FAULT - FAIL SAFE ACTIVE \*\*\*



SENSOR READ ERROR | FAN = ON | STATE = SENSOR\_FAULT



\[RECOVERY] BME280 RECOVERED SUCCESSFULLY



T = 25.0 C | H = 55.0 % | P = 1013.2 hPa | FAN = OFF | STATE = NORMAL

```



\---



\# 📂 Project Structure



```text

STM32\_FreeRTOS\_Project/

│

├── Core/

│   │

│   ├── Inc/

│   │   ├── bme280.h

│   │   ├── main.h

│   │   ├── FreeRTOSConfig.h

│   │   └── ...

│   │

│   └── Src/

│       ├── main.c

│       ├── bme280.c

│       ├── freertos.c

│       ├── stm32f4xx\_it.c

│       ├── stm32f4xx\_hal\_msp.c

│       └── ...

│

├── Drivers/

│

├── Middlewares/

│   └── Third\_Party/

│       └── FreeRTOS/

│

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



\# 🛠️ Technologies \& Tools



<div align="center">



| Embedded | Communication | RTOS / Software | Development |

|---|---|---|---|

| STM32F407 | I²C | FreeRTOS | STM32CubeIDE |

| ARM Cortex-M4 | UART | CMSIS-RTOS2 | STM32CubeMX |

| BME280 | EXTI | STM32 HAL | Renode |

| Embedded C | GPIO | Software Timers | Git / GitHub |



</div>



\---



\# 💡 Skills Demonstrated



This project demonstrates practical experience with:



\*\*Embedded Systems\*\*

\- Embedded C

\- STM32

\- ARM Cortex-M4

\- GPIO

\- Interrupts

\- UART

\- I²C



\*\*Real-Time Systems\*\*

\- FreeRTOS

\- CMSIS-RTOS2

\- Task scheduling

\- Message Queues

\- Mutexes

\- Semaphores

\- Event Flags

\- Software Timers



\*\*Reliable Embedded Design\*\*

\- State-based control

\- Sensor fault detection

\- Fail-safe behavior

\- Health monitoring

\- Automatic recovery



\*\*Development \& Validation\*\*

\- STM32CubeIDE

\- STM32CubeMX

\- Renode

\- Git

\- GitHub



\---



\# ✅ Current Project Status



| Module | Status |

|---|:---:|

| STM32 configuration | ✅ |

| FreeRTOS tasks | ✅ |

| Message queues | ✅ |

| UART diagnostics | ✅ |

| Mutex protection | ✅ |

| Alarm semaphore | ✅ |

| Event Flags | ✅ |

| Software Timer | ✅ |

| Health monitoring | ✅ |

| Sensor fault detection | ✅ |

| Fail-safe behavior | ✅ |

| Automatic recovery | ✅ |

| Renode validation | ✅ |

| Physical BME280 test | ⏳ Future work |



\---



\# 🚀 Future Improvements



Potential extensions include:



1\. Physical STM32F407 hardware validation

2\. Physical BME280 integration

3\. Independent hardware watchdog

4\. ESP32 Wi-Fi connectivity

5\. MQTT communication

6\. Cloud IoT dashboard

7\. Environmental data logging

8\. SD card storage

9\. CAN bus communication

10\. Low-power operating modes

11\. TinyML-based anomaly detection

12\. Predictive environmental control



\---



\# 📚 What This Project Demonstrates



This project goes beyond basic sensor acquisition.



It demonstrates the design of a small \*\*fault-tolerant real-time embedded control architecture\*\* combining:



```text

Acquisition

&#x20;   +

Real-Time Scheduling

&#x20;   +

Inter-task Communication

&#x20;   +

Synchronization

&#x20;   +

Control Logic

&#x20;   +

Fault Detection

&#x20;   +

Fail-Safe Operation

&#x20;   +

Health Monitoring

&#x20;   +

Automatic Recovery

```



\---



<div align="center">



\### ⭐ STM32 · FreeRTOS · Embedded C · Renode



\*\*Real-Time Environmental Monitoring and Control System\*\*



</div>

