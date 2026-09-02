\# STM32 FreeRTOS Environmental Monitoring System



Real-time environmental monitoring and control system based on an STM32F407VGT6 microcontroller and FreeRTOS.



The project monitors environmental parameters, controls a ventilation system, detects abnormal conditions, handles sensor failures and implements automatic recovery mechanisms.



\## Features



\- STM32F407VGT6 microcontroller

\- FreeRTOS / CMSIS-RTOS2

\- BME280 environmental sensor abstraction

\- I2C communication

\- UART monitoring

\- Renode simulation

\- Multi-task real-time architecture

\- Sensor fault detection

\- Fail-safe control

\- Automatic sensor recovery

\- Periodic system health monitoring



\## FreeRTOS Architecture



The application uses several FreeRTOS mechanisms:



\- SensorTask

\- ControlTask

\- UARTTask

\- AlarmTask

\- Message Queues

\- Mutex

\- Binary Semaphore

\- Event Flags

\- Software Timer



\## System Architecture



```text

&#x20;            BME280

&#x20;               |

&#x20;               v

&#x20;          SensorTask

&#x20;               |

&#x20;             Queue

&#x20;               |

&#x20;               v

&#x20;          ControlTask

&#x20;         /     |      \\

&#x20;        /      |       \\

&#x20;      FAN   Event Flags  Status Queue

&#x20;                |            |

&#x20;                |            v

&#x20;                |         UARTTask

&#x20;                |

&#x20;                v

&#x20;            AlarmTask

&#x20;                ^

&#x20;                |

&#x20;            Semaphore





&#x20;      Software Timer

&#x20;            |

&#x20;            v

&#x20;      Health Monitoring

&#x20;            |

&#x20;            v

&#x20;      Sensor Recovery

