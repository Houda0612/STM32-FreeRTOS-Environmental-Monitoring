/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : STM32 FreeRTOS Environmental Monitoring System
  ******************************************************************************
  */
/* USER CODE END Header */


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>

#include "bme280.h"

/* USER CODE END Includes */


/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


/* ==========================================================================
 * ETATS DU SYSTEME
 * ========================================================================== */

typedef enum
{
    STATE_NORMAL = 0,

    STATE_VENTILATION,

    STATE_ALARM,

    STATE_SENSOR_FAULT

} SystemState_t;


/* ==========================================================================
 * RAISON DE L'ALARME
 * ========================================================================== */

typedef enum
{
    ALARM_REASON_ENVIRONMENT = 0,

    ALARM_REASON_SENSOR,

    ALARM_REASON_MANUAL

} AlarmReason_t;


/* ==========================================================================
 * DONNEES DU CAPTEUR
 * ========================================================================== */

typedef struct
{
    float temperature;

    float humidity;

    float pressure;


    /* 1 = capteur fonctionnel
     * 0 = erreur capteur
     */
    uint8_t sensor_ok;


    /* 1 = le capteur vient d'être récupéré */
    uint8_t sensor_recovered;

} SensorData_t;


/* ==========================================================================
 * ETAT COMPLET DU SYSTEME
 * ========================================================================== */

typedef struct
{
    float temperature;

    float humidity;

    float pressure;


    uint8_t fan_state;


    uint8_t sensor_recovered;


    SystemState_t state;

} SystemStatus_t;


/* USER CODE END PTD */


/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


/* ==========================================================================
 * SEUILS
 * ========================================================================== */

#define TEMPERATURE_FAN_THRESHOLD           30.0f

#define HUMIDITY_FAN_THRESHOLD              65.0f

#define TEMPERATURE_ALARM_THRESHOLD         34.0f

#define HUMIDITY_ALARM_THRESHOLD            75.0f


/* ==========================================================================
 * PERIODES FREERTOS
 *
 * Les valeurs sont exprimées en ticks RTOS.
 * ========================================================================== */

#define SENSOR_READ_PERIOD_TICKS            1000U

#define SENSOR_RECOVERY_RETRY_TICKS         2000U

#define HEALTH_TIMER_PERIOD_TICKS           5000U


/* ==========================================================================
 * GPIO
 * ========================================================================== */

/* PD12 = ventilateur */
#define FAN_Pin                             GPIO_PIN_12
#define FAN_GPIO_Port                       GPIOD


/* PD14 = LED alarme */
#define ALARM_LED_Pin                       GPIO_PIN_14
#define ALARM_LED_GPIO_Port                 GPIOD


/* PA0 = bouton / EXTI0 */
#define ALARM_BUTTON_Pin                    GPIO_PIN_0
#define ALARM_BUTTON_GPIO_Port              GPIOA


/* ==========================================================================
 * EVENT FLAGS
 * ========================================================================== */

/* Capteur opérationnel */
#define EVENT_SENSOR_OK                     (1U << 0)

/* Ventilation active */
#define EVENT_VENTILATION                   (1U << 1)

/* Alarme environnementale */
#define EVENT_ENV_ALARM                     (1U << 2)

/* Panne capteur */
#define EVENT_SENSOR_FAULT                  (1U << 3)

/* Capteur récupéré */
#define EVENT_SENSOR_RECOVERED              (1U << 4)


#define EVENT_ALL_STATES                    \
        (EVENT_SENSOR_OK        |           \
         EVENT_VENTILATION      |           \
         EVENT_ENV_ALARM        |           \
         EVENT_SENSOR_FAULT     |           \
         EVENT_SENSOR_RECOVERED)


/* USER CODE END PD */


/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */


/* Private variables ---------------------------------------------------------*/


/* ==========================================================================
 * UART2
 * ========================================================================== */

UART_HandleTypeDef huart2;


/* ==========================================================================
 * I2C1
 * ========================================================================== */

I2C_HandleTypeDef hi2c1;


/* ==========================================================================
 * SENSOR TASK
 * ========================================================================== */

osThreadId_t SensorTaskHandle;


const osThreadAttr_t SensorTask_attributes =
{
    .name = "SensorTask",

    .stack_size = 128 * 4,

    .priority = (osPriority_t) osPriorityNormal,
};


/* ==========================================================================
 * SENSOR QUEUE
 * ========================================================================== */

osMessageQueueId_t TemperatureQueueHandle;


const osMessageQueueAttr_t TemperatureQueue_attributes =
{
    .name = "TemperatureQueue"
};


/* USER CODE BEGIN PV */


/* ==========================================================================
 * CONTROL TASK
 * ========================================================================== */

osThreadId_t ControlTaskHandle;


const osThreadAttr_t ControlTask_attributes =
{
    .name = "ControlTask",

    .stack_size = 128 * 4,

    .priority = (osPriority_t) osPriorityNormal,
};


/* ==========================================================================
 * UART TASK
 * ========================================================================== */

osThreadId_t UARTTaskHandle;


const osThreadAttr_t UARTTask_attributes =
{
    .name = "UARTTask",

    .stack_size = 256 * 4,

    .priority = (osPriority_t) osPriorityNormal,
};


/* ==========================================================================
 * ALARM TASK
 * ========================================================================== */

osThreadId_t AlarmTaskHandle;


const osThreadAttr_t AlarmTask_attributes =
{
    .name = "AlarmTask",

    .stack_size = 128 * 4,

    .priority = (osPriority_t) osPriorityNormal,
};


/* ==========================================================================
 * STATUS QUEUE
 * ========================================================================== */

osMessageQueueId_t StatusQueueHandle;


const osMessageQueueAttr_t StatusQueue_attributes =
{
    .name = "StatusQueue"
};


/* ==========================================================================
 * ALARM SEMAPHORE
 * ========================================================================== */

osSemaphoreId_t AlarmSemaphoreHandle;


const osSemaphoreAttr_t AlarmSemaphore_attributes =
{
    .name = "AlarmSemaphore"
};


/* ==========================================================================
 * UART MUTEX
 * ========================================================================== */

osMutexId_t UARTMutexHandle;


const osMutexAttr_t UARTMutex_attributes =
{
    .name = "UARTMutex"
};


/* ==========================================================================
 * EVENT FLAGS
 * ========================================================================== */

osEventFlagsId_t SystemEventHandle;


const osEventFlagsAttr_t SystemEvent_attributes =
{
    .name = "SystemEvents"
};


/* ==========================================================================
 * HEALTH SOFTWARE TIMER
 * ========================================================================== */

osTimerId_t HealthTimerHandle;


const osTimerAttr_t HealthTimer_attributes =
{
    .name = "HealthTimer"
};


/* ==========================================================================
 * VARIABLES DE SUPERVISION
 * ========================================================================== */

static volatile AlarmReason_t CurrentAlarmReason =
    ALARM_REASON_MANUAL;


/*
 * Le timer ne fait pas directement de transmission UART.
 *
 * Il prend seulement un snapshot des événements et demande
 * à UARTTask d'afficher le Health Report.
 */
static volatile uint32_t LatestHealthFlags = 0U;

static volatile uint8_t HealthReportPending = 0U;


/* USER CODE END PV */


/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);

static void MX_GPIO_Init(void);

static void MX_USART2_UART_Init(void);

static void MX_I2C1_Init(void);


void StartSensorTask(void *argument);


/* USER CODE BEGIN PFP */

void StartControlTask(void *argument);

void StartUARTTask(void *argument);

void StartAlarmTask(void *argument);


void HealthTimerCallback(void *argument);


static void SystemEvents_Set(uint32_t events);


/* USER CODE END PFP */


/* USER CODE BEGIN 0 */


/* ==========================================================================
 * MISE A JOUR DES EVENT FLAGS
 * ========================================================================== */

static void SystemEvents_Set(uint32_t events)
{
    if (SystemEventHandle != NULL)
    {
        /* Effacer les anciens événements */
        osEventFlagsClear(
            SystemEventHandle,
            EVENT_ALL_STATES
        );


        /* Activer le nouvel état */
        osEventFlagsSet(
            SystemEventHandle,
            events
        );
    }
}


/* ==========================================================================
 * HEALTH TIMER CALLBACK
 *
 * La callback reste volontairement courte.
 *
 * Elle ne fait aucune transmission UART bloquante.
 * ========================================================================== */

void HealthTimerCallback(void *argument)
{
    (void)argument;


    if (SystemEventHandle != NULL)
    {
        /* Sauvegarder l'état actuel */
        LatestHealthFlags =
            osEventFlagsGet(
                SystemEventHandle
            );


        /* Demander à UARTTask d'afficher un rapport */
        HealthReportPending =
            1U;
    }
}


/* USER CODE END 0 */


/* ==========================================================================
 * MAIN
 * ========================================================================== */

int main(void)
{
    HAL_Init();


    SystemClock_Config();


    MX_GPIO_Init();


    MX_USART2_UART_Init();


    MX_I2C1_Init();


    /* USER CODE BEGIN 2 */


    /* ======================================================================
     * MESSAGE DE DEMARRAGE
     * ====================================================================== */

    const char startup_message[] =
        "STM32 Environmental Controller started\r\n";


    HAL_UART_Transmit(
        &huart2,
        (uint8_t *)startup_message,
        sizeof(startup_message) - 1,
        HAL_MAX_DELAY
    );


    /* ======================================================================
     * INITIALISATION BME280
     * ====================================================================== */

    if (BME280_Init(&hi2c1) == HAL_OK)
    {
        const char message[] =
            "BME280 driver initialized\r\n";


        HAL_UART_Transmit(
            &huart2,
            (uint8_t *)message,
            sizeof(message) - 1,
            HAL_MAX_DELAY
        );
    }
    else
    {
        const char message[] =
            "BME280 initialization ERROR\r\n";


        HAL_UART_Transmit(
            &huart2,
            (uint8_t *)message,
            sizeof(message) - 1,
            HAL_MAX_DELAY
        );
    }


    /* USER CODE END 2 */


    /* ======================================================================
     * INITIALISATION FREERTOS
     * ====================================================================== */

    osKernelInitialize();


    /* ======================================================================
     * EVENT FLAGS
     * ====================================================================== */

    SystemEventHandle =
        osEventFlagsNew(
            &SystemEvent_attributes
        );


    if (SystemEventHandle == NULL)
    {
        Error_Handler();
    }


    /* ======================================================================
     * UART MUTEX
     * ====================================================================== */

    UARTMutexHandle =
        osMutexNew(
            &UARTMutex_attributes
        );


    if (UARTMutexHandle == NULL)
    {
        Error_Handler();
    }


    /* ======================================================================
     * SEMAPHORE
     * ====================================================================== */

    AlarmSemaphoreHandle =
        osSemaphoreNew(
            1,
            0,
            &AlarmSemaphore_attributes
        );


    if (AlarmSemaphoreHandle == NULL)
    {
        Error_Handler();
    }


    /* ======================================================================
     * SENSOR QUEUE
     * ====================================================================== */

    TemperatureQueueHandle =
        osMessageQueueNew(
            5,
            sizeof(SensorData_t),
            &TemperatureQueue_attributes
        );


    if (TemperatureQueueHandle == NULL)
    {
        Error_Handler();
    }


    /* ======================================================================
     * STATUS QUEUE
     * ====================================================================== */

    StatusQueueHandle =
        osMessageQueueNew(
            5,
            sizeof(SystemStatus_t),
            &StatusQueue_attributes
        );


    if (StatusQueueHandle == NULL)
    {
        Error_Handler();
    }


    /* ======================================================================
     * HEALTH SOFTWARE TIMER
     * ====================================================================== */

    HealthTimerHandle =
        osTimerNew(
            HealthTimerCallback,
            osTimerPeriodic,
            NULL,
            &HealthTimer_attributes
        );


    if (HealthTimerHandle == NULL)
    {
        Error_Handler();
    }


    /* ======================================================================
     * TASKS
     * ====================================================================== */

    SensorTaskHandle =
        osThreadNew(
            StartSensorTask,
            NULL,
            &SensorTask_attributes
        );


    if (SensorTaskHandle == NULL)
    {
        Error_Handler();
    }


    ControlTaskHandle =
        osThreadNew(
            StartControlTask,
            NULL,
            &ControlTask_attributes
        );


    if (ControlTaskHandle == NULL)
    {
        Error_Handler();
    }


    UARTTaskHandle =
        osThreadNew(
            StartUARTTask,
            NULL,
            &UARTTask_attributes
        );


    if (UARTTaskHandle == NULL)
    {
        Error_Handler();
    }


    AlarmTaskHandle =
        osThreadNew(
            StartAlarmTask,
            NULL,
            &AlarmTask_attributes
        );


    if (AlarmTaskHandle == NULL)
    {
        Error_Handler();
    }


    /* ======================================================================
     * DEMARRAGE HEALTH TIMER
     * ====================================================================== */

    if (osTimerStart(
            HealthTimerHandle,
            HEALTH_TIMER_PERIOD_TICKS) != osOK)
    {
        Error_Handler();
    }


    /* ======================================================================
     * DEMARRAGE FREERTOS
     * ====================================================================== */

    osKernelStart();


    while (1)
    {
    }
}


/* ==========================================================================
 * SYSTEM CLOCK
 * ========================================================================== */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


    __HAL_RCC_PWR_CLK_ENABLE();


    __HAL_PWR_VOLTAGESCALING_CONFIG(
        PWR_REGULATOR_VOLTAGE_SCALE1
    );


    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI;


    RCC_OscInitStruct.HSIState =
        RCC_HSI_ON;


    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;


    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_NONE;


    if (HAL_RCC_OscConfig(
            &RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }


    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;


    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_HSI;


    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;


    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV1;


    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV1;


    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}


/* ==========================================================================
 * I2C1
 * ========================================================================== */

static void MX_I2C1_Init(void)
{
    hi2c1.Instance =
        I2C1;


    hi2c1.Init.ClockSpeed =
        100000;


    hi2c1.Init.DutyCycle =
        I2C_DUTYCYCLE_2;


    hi2c1.Init.OwnAddress1 =
        0;


    hi2c1.Init.AddressingMode =
        I2C_ADDRESSINGMODE_7BIT;


    hi2c1.Init.DualAddressMode =
        I2C_DUALADDRESS_DISABLE;


    hi2c1.Init.OwnAddress2 =
        0;


    hi2c1.Init.GeneralCallMode =
        I2C_GENERALCALL_DISABLE;


    hi2c1.Init.NoStretchMode =
        I2C_NOSTRETCH_DISABLE;


    if (HAL_I2C_Init(
            &hi2c1) != HAL_OK)
    {
        Error_Handler();
    }
}


/* ==========================================================================
 * USART2
 * ========================================================================== */

static void MX_USART2_UART_Init(void)
{
    huart2.Instance =
        USART2;


    huart2.Init.BaudRate =
        115200;


    huart2.Init.WordLength =
        UART_WORDLENGTH_8B;


    huart2.Init.StopBits =
        UART_STOPBITS_1;


    huart2.Init.Parity =
        UART_PARITY_NONE;


    huart2.Init.Mode =
        UART_MODE_TX_RX;


    huart2.Init.HwFlowCtl =
        UART_HWCONTROL_NONE;


    huart2.Init.OverSampling =
        UART_OVERSAMPLING_16;


    if (HAL_UART_Init(
            &huart2) != HAL_OK)
    {
        Error_Handler();
    }
}


/* ==========================================================================
 * GPIO
 * ========================================================================== */

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};


    __HAL_RCC_GPIOA_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();


    /* FAN OFF */
    HAL_GPIO_WritePin(
        FAN_GPIO_Port,
        FAN_Pin,
        GPIO_PIN_RESET
    );


    /* LED ALARM OFF */
    HAL_GPIO_WritePin(
        ALARM_LED_GPIO_Port,
        ALARM_LED_Pin,
        GPIO_PIN_RESET
    );


    /* ======================================================================
     * PD12 : FAN
     * ====================================================================== */

    GPIO_InitStruct.Pin =
        FAN_Pin;


    GPIO_InitStruct.Mode =
        GPIO_MODE_OUTPUT_PP;


    GPIO_InitStruct.Pull =
        GPIO_NOPULL;


    GPIO_InitStruct.Speed =
        GPIO_SPEED_FREQ_LOW;


    HAL_GPIO_Init(
        FAN_GPIO_Port,
        &GPIO_InitStruct
    );


    /* ======================================================================
     * PD14 : LED ALARM
     * ====================================================================== */

    GPIO_InitStruct.Pin =
        ALARM_LED_Pin;


    GPIO_InitStruct.Mode =
        GPIO_MODE_OUTPUT_PP;


    GPIO_InitStruct.Pull =
        GPIO_NOPULL;


    GPIO_InitStruct.Speed =
        GPIO_SPEED_FREQ_LOW;


    HAL_GPIO_Init(
        ALARM_LED_GPIO_Port,
        &GPIO_InitStruct
    );


    /* ======================================================================
     * PA0 : EXTI0
     * ====================================================================== */

    GPIO_InitStruct.Pin =
        ALARM_BUTTON_Pin;


    GPIO_InitStruct.Mode =
        GPIO_MODE_IT_RISING;


    GPIO_InitStruct.Pull =
        GPIO_NOPULL;


    HAL_GPIO_Init(
        ALARM_BUTTON_GPIO_Port,
        &GPIO_InitStruct
    );


    HAL_NVIC_SetPriority(
        EXTI0_IRQn,
        5,
        0
    );


    HAL_NVIC_EnableIRQ(
        EXTI0_IRQn
    );
}


/* USER CODE BEGIN 4 */


/* ==========================================================================
 * INTERRUPTION MANUELLE
 * ========================================================================== */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == ALARM_BUTTON_Pin)
    {
        if (AlarmSemaphoreHandle != NULL)
        {
            CurrentAlarmReason =
                ALARM_REASON_MANUAL;


            osSemaphoreRelease(
                AlarmSemaphoreHandle
            );
        }
    }
}


/* ==========================================================================
 * SENSOR TASK
 *
 * Cette tâche possède maintenant deux modes :
 *
 * 1. MODE NORMAL
 *    -> BME280_Read()
 *
 * 2. MODE RECOVERY
 *    -> BME280_Init()
 *    -> BME280_Read()
 * ========================================================================== */

void StartSensorTask(void *argument)
{
    BME280_Data_t bme_data;

    SensorData_t sensor_data;


    /*
     * 0 = fonctionnement normal
     * 1 = récupération nécessaire
     */
    uint8_t recovery_mode = 0U;


    for (;;)
    {
        /* Valeur par défaut */
        sensor_data.sensor_recovered =
            0U;


        /* ==================================================================
         * MODE NORMAL
         * ================================================================== */

        if (recovery_mode == 0U)
        {
            if (BME280_Read(
                    &bme_data) == HAL_OK)
            {
                /* ==========================================================
                 * LECTURE OK
                 * ========================================================== */

                sensor_data.sensor_ok =
                    1U;


                sensor_data.temperature =
                    bme_data.temperature;


                sensor_data.humidity =
                    bme_data.humidity;


                sensor_data.pressure =
                    bme_data.pressure;
            }
            else
            {
                /* ==========================================================
                 * PANNE DETECTEE
                 * ========================================================== */

                sensor_data.sensor_ok =
                    0U;


                sensor_data.temperature =
                    0.0f;


                sensor_data.humidity =
                    0.0f;


                sensor_data.pressure =
                    0.0f;


                /* Passer en mode récupération */
                recovery_mode =
                    1U;
            }
        }


        /* ==================================================================
         * MODE RECOVERY
         * ================================================================== */

        else
        {
            /*
             * Première étape :
             * réinitialiser le driver / capteur.
             */

            if (BME280_Init(
                    &hi2c1) == HAL_OK)
            {
                /*
                 * Deuxième étape :
                 * vérifier avec une vraie lecture.
                 */

                if (BME280_Read(
                        &bme_data) == HAL_OK)
                {
                    /* ======================================================
                     * RECOVERY REUSSIE
                     * ====================================================== */

                    sensor_data.sensor_ok =
                        1U;


                    sensor_data.sensor_recovered =
                        1U;


                    sensor_data.temperature =
                        bme_data.temperature;


                    sensor_data.humidity =
                        bme_data.humidity;


                    sensor_data.pressure =
                        bme_data.pressure;


                    /* Quitter le mode recovery */
                    recovery_mode =
                        0U;
                }
                else
                {
                    /* ======================================================
                     * RECOVERY ECHOUEE
                     * ====================================================== */

                    sensor_data.sensor_ok =
                        0U;


                    sensor_data.temperature =
                        0.0f;


                    sensor_data.humidity =
                        0.0f;


                    sensor_data.pressure =
                        0.0f;
                }
            }
            else
            {
                /* ==========================================================
                 * REINITIALISATION ECHOUEE
                 * ========================================================== */

                sensor_data.sensor_ok =
                    0U;


                sensor_data.temperature =
                    0.0f;


                sensor_data.humidity =
                    0.0f;


                sensor_data.pressure =
                    0.0f;
            }
        }


        /* ==================================================================
         * ENVOI VERS CONTROL TASK
         * ================================================================== */

        osMessageQueuePut(
            TemperatureQueueHandle,
            &sensor_data,
            0,
            osWaitForever
        );


        /* ==================================================================
         * DELAI
         * ================================================================== */

        if (recovery_mode != 0U)
        {
            /*
             * En panne :
             * attendre avant la prochaine tentative.
             */

            osDelay(
                SENSOR_RECOVERY_RETRY_TICKS
            );
        }
        else
        {
            /* Fonctionnement normal */

            osDelay(
                SENSOR_READ_PERIOD_TICKS
            );
        }
    }
}


/* ==========================================================================
 * CONTROL TASK
 * ========================================================================== */

void StartControlTask(void *argument)
{
    SensorData_t received_data;

    SystemStatus_t status;


    /*
     * Utilisé pour éviter de déclencher l'alarme capteur
     * plusieurs fois pendant la même panne.
     */
    SystemState_t previous_state =
        STATE_NORMAL;


    for (;;)
    {
        if (osMessageQueueGet(
                TemperatureQueueHandle,
                &received_data,
                NULL,
                osWaitForever) == osOK)
        {
            /* Valeur par défaut */
            status.sensor_recovered =
                received_data.sensor_recovered;


            /* ==============================================================
             * SENSOR FAULT
             * ============================================================== */

            if (received_data.sensor_ok == 0U)
            {
                status.temperature =
                    0.0f;


                status.humidity =
                    0.0f;


                status.pressure =
                    0.0f;


                status.state =
                    STATE_SENSOR_FAULT;


                /* ==========================================================
                 * FAIL-SAFE
                 * ========================================================== */

                status.fan_state =
                    1U;


                HAL_GPIO_WritePin(
                    FAN_GPIO_Port,
                    FAN_Pin,
                    GPIO_PIN_SET
                );


                /* ==========================================================
                 * EVENT FLAGS
                 * ========================================================== */

                SystemEvents_Set(
                    EVENT_SENSOR_FAULT
                );


                /* ==========================================================
                 * DECLENCHER L'ALARME UNE SEULE FOIS
                 * ========================================================== */

                if (previous_state !=
                    STATE_SENSOR_FAULT)
                {
                    CurrentAlarmReason =
                        ALARM_REASON_SENSOR;


                    if (AlarmSemaphoreHandle != NULL)
                    {
                        osSemaphoreRelease(
                            AlarmSemaphoreHandle
                        );
                    }
                }


                previous_state =
                    STATE_SENSOR_FAULT;


                /* Envoyer vers UART */
                osMessageQueuePut(
                    StatusQueueHandle,
                    &status,
                    0,
                    osWaitForever
                );


                continue;
            }


            /* ==============================================================
             * MESURES VALIDES
             * ============================================================== */

            status.temperature =
                received_data.temperature;


            status.humidity =
                received_data.humidity;


            status.pressure =
                received_data.pressure;


            /* ==============================================================
             * ALARME ENVIRONNEMENTALE
             * ============================================================== */

            if ((received_data.temperature >
                 TEMPERATURE_ALARM_THRESHOLD) ||

                (received_data.humidity >
                 HUMIDITY_ALARM_THRESHOLD))
            {
                status.state =
                    STATE_ALARM;


                status.fan_state =
                    1U;


                HAL_GPIO_WritePin(
                    FAN_GPIO_Port,
                    FAN_Pin,
                    GPIO_PIN_SET
                );


                if (received_data.sensor_recovered != 0U)
                {
                    SystemEvents_Set(
                        EVENT_SENSOR_OK |
                        EVENT_ENV_ALARM |
                        EVENT_SENSOR_RECOVERED
                    );
                }
                else
                {
                    SystemEvents_Set(
                        EVENT_SENSOR_OK |
                        EVENT_ENV_ALARM
                    );
                }


                CurrentAlarmReason =
                    ALARM_REASON_ENVIRONMENT;


                if (AlarmSemaphoreHandle != NULL)
                {
                    osSemaphoreRelease(
                        AlarmSemaphoreHandle
                    );
                }
            }


            /* ==============================================================
             * VENTILATION
             * ============================================================== */

            else if ((received_data.temperature >
                      TEMPERATURE_FAN_THRESHOLD) ||

                     (received_data.humidity >
                      HUMIDITY_FAN_THRESHOLD))
            {
                status.state =
                    STATE_VENTILATION;


                status.fan_state =
                    1U;


                HAL_GPIO_WritePin(
                    FAN_GPIO_Port,
                    FAN_Pin,
                    GPIO_PIN_SET
                );


                if (received_data.sensor_recovered != 0U)
                {
                    SystemEvents_Set(
                        EVENT_SENSOR_OK |
                        EVENT_VENTILATION |
                        EVENT_SENSOR_RECOVERED
                    );
                }
                else
                {
                    SystemEvents_Set(
                        EVENT_SENSOR_OK |
                        EVENT_VENTILATION
                    );
                }
            }


            /* ==============================================================
             * NORMAL
             * ============================================================== */

            else
            {
                status.state =
                    STATE_NORMAL;


                status.fan_state =
                    0U;


                HAL_GPIO_WritePin(
                    FAN_GPIO_Port,
                    FAN_Pin,
                    GPIO_PIN_RESET
                );


                if (received_data.sensor_recovered != 0U)
                {
                    SystemEvents_Set(
                        EVENT_SENSOR_OK |
                        EVENT_SENSOR_RECOVERED
                    );
                }
                else
                {
                    SystemEvents_Set(
                        EVENT_SENSOR_OK
                    );
                }
            }


            previous_state =
                status.state;


            /* ==============================================================
             * ENVOI VERS UART TASK
             * ============================================================== */

            osMessageQueuePut(
                StatusQueueHandle,
                &status,
                0,
                osWaitForever
            );
        }
    }
}


/* ==========================================================================
 * UART TASK
 * ========================================================================== */

void StartUARTTask(void *argument)
{
    SystemStatus_t received_status;


    char uart_buffer[220];

    char health_buffer[100];


    const char *state_text;

    const char *health_message;


    for (;;)
    {
        if (osMessageQueueGet(
                StatusQueueHandle,
                &received_status,
                NULL,
                osWaitForever) == osOK)
        {
            /* ==============================================================
             * MESSAGE DE RECOVERY
             * ============================================================== */

            if (received_status.sensor_recovered != 0U)
            {
                const char recovery_message[] =
                    "\r\n[RECOVERY] BME280 RECOVERED SUCCESSFULLY\r\n";


                if (osMutexAcquire(
                        UARTMutexHandle,
                        osWaitForever) == osOK)
                {
                    HAL_UART_Transmit(
                        &huart2,
                        (uint8_t *)recovery_message,
                        sizeof(recovery_message) - 1,
                        HAL_MAX_DELAY
                    );


                    osMutexRelease(
                        UARTMutexHandle
                    );
                }
            }


            /* ==============================================================
             * SENSOR FAULT
             * ============================================================== */

            if (received_status.state ==
                STATE_SENSOR_FAULT)
            {
                snprintf(
                    uart_buffer,
                    sizeof(uart_buffer),

                    "SENSOR READ ERROR | FAN = ON | STATE = SENSOR_FAULT\r\n"
                );
            }


            /* ==============================================================
             * MESURES VALIDES
             * ============================================================== */

            else
            {
                int temp_integer;

                int temp_decimal;

                int humidity_integer;

                int humidity_decimal;

                int pressure_integer;

                int pressure_decimal;


                /* Température */

                temp_integer =
                    (int)received_status.temperature;


                temp_decimal =
                    (int)(
                        (received_status.temperature -
                         temp_integer)
                        *
                        10.0f
                    );


                if (temp_decimal < 0)
                {
                    temp_decimal =
                        -temp_decimal;
                }


                /* Humidité */

                humidity_integer =
                    (int)received_status.humidity;


                humidity_decimal =
                    (int)(
                        (received_status.humidity -
                         humidity_integer)
                        *
                        10.0f
                    );


                if (humidity_decimal < 0)
                {
                    humidity_decimal =
                        -humidity_decimal;
                }


                /* Pression */

                pressure_integer =
                    (int)received_status.pressure;


                pressure_decimal =
                    (int)(
                        (received_status.pressure -
                         pressure_integer)
                        *
                        10.0f
                    );


                if (pressure_decimal < 0)
                {
                    pressure_decimal =
                        -pressure_decimal;
                }


                /* ==========================================================
                 * ETAT
                 * ========================================================== */

                switch (received_status.state)
                {
                    case STATE_NORMAL:

                        state_text =
                            "NORMAL";

                        break;


                    case STATE_VENTILATION:

                        state_text =
                            "VENTILATION";

                        break;


                    case STATE_ALARM:

                        state_text =
                            "ALARM";

                        break;


                    case STATE_SENSOR_FAULT:

                        state_text =
                            "SENSOR_FAULT";

                        break;


                    default:

                        state_text =
                            "UNKNOWN";

                        break;
                }


                /* ==========================================================
                 * MESSAGE PRINCIPAL
                 * ========================================================== */

                snprintf(
                    uart_buffer,
                    sizeof(uart_buffer),

                    "T = %d.%d C | H = %d.%d %% | P = %d.%d hPa | FAN = %s | STATE = %s\r\n",

                    temp_integer,
                    temp_decimal,

                    humidity_integer,
                    humidity_decimal,

                    pressure_integer,
                    pressure_decimal,

                    received_status.fan_state ?
                        "ON" :
                        "OFF",

                    state_text
                );
            }


            /* ==============================================================
             * ENVOYER MESSAGE PRINCIPAL
             * ============================================================== */

            if (osMutexAcquire(
                    UARTMutexHandle,
                    osWaitForever) == osOK)
            {
                HAL_UART_Transmit(
                    &huart2,
                    (uint8_t *)uart_buffer,
                    strlen(uart_buffer),
                    HAL_MAX_DELAY
                );


                osMutexRelease(
                    UARTMutexHandle
                );
            }


            /* ==============================================================
             * HEALTH REPORT
             *
             * Le Software Timer a demandé un rapport.
             * ============================================================== */

            if (HealthReportPending != 0U)
            {
                uint32_t flags =
                    LatestHealthFlags;


                HealthReportPending =
                    0U;


                /* ----------------------------------------------------------
                 * Choisir le message.
                 * ---------------------------------------------------------- */

                if ((flags &
                     EVENT_SENSOR_FAULT) != 0U)
                {
                    health_message =
                        "[HEALTH] SENSOR FAULT | FAIL-SAFE ACTIVE\r\n";
                }

                else if ((flags &
                          EVENT_ENV_ALARM) != 0U)
                {
                    health_message =
                        "[HEALTH] ENVIRONMENT ALARM\r\n";
                }

                else if ((flags &
                          EVENT_SENSOR_RECOVERED) != 0U)
                {
                    health_message =
                        "[HEALTH] SENSOR RECOVERED | SYSTEM OPERATIONAL\r\n";
                }

                else if ((flags &
                          EVENT_VENTILATION) != 0U)
                {
                    health_message =
                        "[HEALTH] SYSTEM OK | VENTILATION ACTIVE\r\n";
                }

                else if ((flags &
                          EVENT_SENSOR_OK) != 0U)
                {
                    health_message =
                        "[HEALTH] SYSTEM OK\r\n";
                }

                else
                {
                    health_message =
                        "[HEALTH] WARNING | NO VALID SYSTEM STATE\r\n";
                }


                snprintf(
                    health_buffer,
                    sizeof(health_buffer),
                    "%s",
                    health_message
                );


                /* ----------------------------------------------------------
                 * Transmission UART.
                 * ---------------------------------------------------------- */

                if (osMutexAcquire(
                        UARTMutexHandle,
                        osWaitForever) == osOK)
                {
                    HAL_UART_Transmit(
                        &huart2,
                        (uint8_t *)health_buffer,
                        strlen(health_buffer),
                        HAL_MAX_DELAY
                    );


                    osMutexRelease(
                        UARTMutexHandle
                    );
                }
            }
        }
    }
}


/* ==========================================================================
 * ALARM TASK
 * ========================================================================== */

void StartAlarmTask(void *argument)
{
    const char environmental_alarm[] =
        "\r\n*** ALARM TRIGGERED ***\r\n";


    const char sensor_alarm[] =
        "\r\n*** SENSOR FAULT - FAIL SAFE ACTIVE ***\r\n";


    const char manual_alarm[] =
        "\r\n*** MANUAL ALARM TRIGGERED ***\r\n";


    const char *alarm_message;


    for (;;)
    {
        if (osSemaphoreAcquire(
                AlarmSemaphoreHandle,
                osWaitForever) == osOK)
        {
            /* LED ON */

            HAL_GPIO_WritePin(
                ALARM_LED_GPIO_Port,
                ALARM_LED_Pin,
                GPIO_PIN_SET
            );


            /* Choisir type d'alarme */

            switch (CurrentAlarmReason)
            {
                case ALARM_REASON_SENSOR:

                    alarm_message =
                        sensor_alarm;

                    break;


                case ALARM_REASON_ENVIRONMENT:

                    alarm_message =
                        environmental_alarm;

                    break;


                case ALARM_REASON_MANUAL:

                default:

                    alarm_message =
                        manual_alarm;

                    break;
            }


            /* UART */

            if (osMutexAcquire(
                    UARTMutexHandle,
                    osWaitForever) == osOK)
            {
                HAL_UART_Transmit(
                    &huart2,
                    (uint8_t *)alarm_message,
                    strlen(alarm_message),
                    HAL_MAX_DELAY
                );


                osMutexRelease(
                    UARTMutexHandle
                );
            }


            /* LED active pendant 1000 ticks */

            osDelay(1000);


            /* LED OFF */

            HAL_GPIO_WritePin(
                ALARM_LED_GPIO_Port,
                ALARM_LED_Pin,
                GPIO_PIN_RESET
            );
        }
    }
}


/* USER CODE END 4 */


/* ==========================================================================
 * ERROR HANDLER
 * ========================================================================== */

void Error_Handler(void)
{
    __disable_irq();


    while (1)
    {
    }
}


#ifdef USE_FULL_ASSERT

void assert_failed(
    uint8_t *file,
    uint32_t line)
{
}

#endif /* USE_FULL_ASSERT */
