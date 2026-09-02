#include "bme280.h"


/* ==========================================================================
 * REGISTRES BME280
 * ========================================================================== */

#define BME280_REG_CHIP_ID          0xD0U
#define BME280_REG_RESET            0xE0U
#define BME280_REG_CTRL_HUM         0xF2U
#define BME280_REG_CTRL_MEAS        0xF4U
#define BME280_REG_CONFIG           0xF5U
#define BME280_REG_PRESS_MSB        0xF7U

#define BME280_RESET_COMMAND        0xB6U

#define BME280_I2C_TIMEOUT          100U


/* ==========================================================================
 * HANDLE I2C
 * ========================================================================== */

static I2C_HandleTypeDef *bme280_i2c = NULL;


/* ==========================================================================
 * MODE SIMULATION RENODE
 * ========================================================================== */

#if BME280_SIMULATION_MODE


/* --------------------------------------------------------------------------
 * Mesures simulées.
 *
 * Elles permettent de tester :
 *
 * NORMAL
 * VENTILATION
 * ALARM
 * -------------------------------------------------------------------------- */

static const BME280_Data_t simulated_data[] =
{
    {25.0f, 55.0f, 1013.2f},
    {28.0f, 60.0f, 1012.8f},
    {32.0f, 68.0f, 1011.9f},
    {35.0f, 72.0f, 1010.5f},
    {27.0f, 58.0f, 1013.0f}
};


/* Index de la mesure actuelle */
static uint8_t simulation_index = 0;


/* Nombre de lectures effectuées */
static uint8_t simulation_read_count = 0;


/* --------------------------------------------------------------------------
 * Une erreur sera générée toutes les 6 lectures.
 *
 * Donc :
 *
 * lecture 1 -> OK
 * lecture 2 -> OK
 * lecture 3 -> OK
 * lecture 4 -> OK
 * lecture 5 -> OK
 * lecture 6 -> HAL_ERROR
 * -------------------------------------------------------------------------- */

#define BME280_SIMULATED_FAULT_PERIOD       6U


#endif /* BME280_SIMULATION_MODE */


/* ==========================================================================
 * PARTIE BME280 REEL
 * ========================================================================== */

#if !BME280_SIMULATION_MODE


/* ==========================================================================
 * COEFFICIENTS DE CALIBRATION
 * ========================================================================== */

typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;

} BME280_Calibration_t;


static BME280_Calibration_t calibration;


/* Valeur utilisée par les calculs de compensation */
static float t_fine = 0.0f;


/* ==========================================================================
 * LECTURE I2C
 * ========================================================================== */

static HAL_StatusTypeDef BME280_ReadRegisters(
    uint8_t reg,
    uint8_t *data,
    uint16_t length)
{
    if ((bme280_i2c == NULL) || (data == NULL))
    {
        return HAL_ERROR;
    }


    return HAL_I2C_Mem_Read(
        bme280_i2c,
        BME280_I2C_ADDRESS,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        data,
        length,
        BME280_I2C_TIMEOUT
    );
}


/* ==========================================================================
 * ECRITURE I2C
 * ========================================================================== */

static HAL_StatusTypeDef BME280_WriteRegister(
    uint8_t reg,
    uint8_t value)
{
    if (bme280_i2c == NULL)
    {
        return HAL_ERROR;
    }


    return HAL_I2C_Mem_Write(
        bme280_i2c,
        BME280_I2C_ADDRESS,
        reg,
        I2C_MEMADD_SIZE_8BIT,
        &value,
        1,
        BME280_I2C_TIMEOUT
    );
}


/* ==========================================================================
 * EXTENSION DU SIGNE SUR 12 BITS
 * ========================================================================== */

static int16_t BME280_SignExtend12(uint16_t value)
{
    if ((value & 0x0800U) != 0U)
    {
        value |= 0xF000U;
    }

    return (int16_t)value;
}


/* ==========================================================================
 * LECTURE DES COEFFICIENTS DE CALIBRATION
 * ========================================================================== */

static HAL_StatusTypeDef BME280_ReadCalibration(void)
{
    uint8_t buffer1[26];
    uint8_t buffer2[7];

    uint16_t h4_raw;
    uint16_t h5_raw;


    /* ----------------------------------------------------------------------
     * Zone de calibration température + pression.
     * ---------------------------------------------------------------------- */

    if (BME280_ReadRegisters(
            0x88U,
            buffer1,
            sizeof(buffer1)) != HAL_OK)
    {
        return HAL_ERROR;
    }


    calibration.dig_T1 =
        (uint16_t)(((uint16_t)buffer1[1] << 8) |
                   buffer1[0]);


    calibration.dig_T2 =
        (int16_t)(((uint16_t)buffer1[3] << 8) |
                  buffer1[2]);


    calibration.dig_T3 =
        (int16_t)(((uint16_t)buffer1[5] << 8) |
                  buffer1[4]);


    calibration.dig_P1 =
        (uint16_t)(((uint16_t)buffer1[7] << 8) |
                   buffer1[6]);


    calibration.dig_P2 =
        (int16_t)(((uint16_t)buffer1[9] << 8) |
                  buffer1[8]);


    calibration.dig_P3 =
        (int16_t)(((uint16_t)buffer1[11] << 8) |
                  buffer1[10]);


    calibration.dig_P4 =
        (int16_t)(((uint16_t)buffer1[13] << 8) |
                  buffer1[12]);


    calibration.dig_P5 =
        (int16_t)(((uint16_t)buffer1[15] << 8) |
                  buffer1[14]);


    calibration.dig_P6 =
        (int16_t)(((uint16_t)buffer1[17] << 8) |
                  buffer1[16]);


    calibration.dig_P7 =
        (int16_t)(((uint16_t)buffer1[19] << 8) |
                  buffer1[18]);


    calibration.dig_P8 =
        (int16_t)(((uint16_t)buffer1[21] << 8) |
                  buffer1[20]);


    calibration.dig_P9 =
        (int16_t)(((uint16_t)buffer1[23] << 8) |
                  buffer1[22]);


    calibration.dig_H1 =
        buffer1[25];


    /* ----------------------------------------------------------------------
     * Zone de calibration humidité.
     * ---------------------------------------------------------------------- */

    if (BME280_ReadRegisters(
            0xE1U,
            buffer2,
            sizeof(buffer2)) != HAL_OK)
    {
        return HAL_ERROR;
    }


    calibration.dig_H2 =
        (int16_t)(((uint16_t)buffer2[1] << 8) |
                  buffer2[0]);


    calibration.dig_H3 =
        buffer2[2];


    h4_raw =
        ((uint16_t)buffer2[3] << 4) |
        (buffer2[4] & 0x0FU);


    calibration.dig_H4 =
        BME280_SignExtend12(h4_raw);


    h5_raw =
        ((uint16_t)buffer2[5] << 4) |
        ((uint16_t)buffer2[4] >> 4);


    calibration.dig_H5 =
        BME280_SignExtend12(h5_raw);


    calibration.dig_H6 =
        (int8_t)buffer2[6];


    return HAL_OK;
}


/* ==========================================================================
 * COMPENSATION TEMPERATURE
 * ========================================================================== */

static float BME280_CompensateTemperature(int32_t adc_T)
{
    float var1;
    float var2;


    var1 =
        (((float)adc_T / 16384.0f) -
         ((float)calibration.dig_T1 / 1024.0f))
        *
        (float)calibration.dig_T2;


    var2 =
        (((float)adc_T / 131072.0f) -
         ((float)calibration.dig_T1 / 8192.0f));


    var2 =
        var2 *
        var2 *
        (float)calibration.dig_T3;


    t_fine =
        var1 + var2;


    return t_fine / 5120.0f;
}


/* ==========================================================================
 * COMPENSATION PRESSION
 *
 * Valeur retournée en Pa.
 * ========================================================================== */

static float BME280_CompensatePressure(int32_t adc_P)
{
    float var1;
    float var2;
    float pressure;


    var1 =
        (t_fine / 2.0f) -
        64000.0f;


    var2 =
        var1 *
        var1 *
        (float)calibration.dig_P6 /
        32768.0f;


    var2 +=
        var1 *
        (float)calibration.dig_P5 *
        2.0f;


    var2 =
        (var2 / 4.0f) +
        ((float)calibration.dig_P4 * 65536.0f);


    var1 =
        (
            ((float)calibration.dig_P3 *
             var1 *
             var1 /
             524288.0f)
            +
            ((float)calibration.dig_P2 *
             var1)
        )
        /
        524288.0f;


    var1 =
        (1.0f +
         (var1 / 32768.0f))
        *
        (float)calibration.dig_P1;


    if (var1 == 0.0f)
    {
        return 0.0f;
    }


    pressure =
        1048576.0f -
        (float)adc_P;


    pressure =
        (pressure -
         (var2 / 4096.0f))
        *
        6250.0f /
        var1;


    var1 =
        (float)calibration.dig_P9 *
        pressure *
        pressure /
        2147483648.0f;


    var2 =
        pressure *
        (float)calibration.dig_P8 /
        32768.0f;


    pressure +=
        (
            var1 +
            var2 +
            (float)calibration.dig_P7
        )
        /
        16.0f;


    return pressure;
}


/* ==========================================================================
 * COMPENSATION HUMIDITE
 * ========================================================================== */

static float BME280_CompensateHumidity(int32_t adc_H)
{
    float humidity;


    humidity =
        t_fine -
        76800.0f;


    humidity =
        (
            (float)adc_H -
            (
                ((float)calibration.dig_H4 * 64.0f)
                +
                (
                    ((float)calibration.dig_H5 /
                     16384.0f)
                    *
                    humidity
                )
            )
        )
        *
        (
            ((float)calibration.dig_H2 /
             65536.0f)
            *
            (
                1.0f
                +
                (
                    ((float)calibration.dig_H6 /
                     67108864.0f)
                    *
                    humidity
                    *
                    (
                        1.0f
                        +
                        (
                            ((float)calibration.dig_H3 /
                             67108864.0f)
                            *
                            humidity
                        )
                    )
                )
            )
        );


    humidity =
        humidity *
        (
            1.0f -
            (
                ((float)calibration.dig_H1 *
                 humidity)
                /
                524288.0f
            )
        );


    if (humidity > 100.0f)
    {
        humidity = 100.0f;
    }


    if (humidity < 0.0f)
    {
        humidity = 0.0f;
    }


    return humidity;
}


#endif /* !BME280_SIMULATION_MODE */


/* ==========================================================================
 * VERIFICATION DU CHIP ID
 * ========================================================================== */

HAL_StatusTypeDef BME280_CheckID(uint8_t *chip_id)
{
    if (chip_id == NULL)
    {
        return HAL_ERROR;
    }


#if BME280_SIMULATION_MODE

    /* Simuler un vrai BME280 */
    *chip_id =
        BME280_CHIP_ID_VALUE;


    return HAL_OK;

#else

    return BME280_ReadRegisters(
        BME280_REG_CHIP_ID,
        chip_id,
        1
    );

#endif
}


/* ==========================================================================
 * INITIALISATION DU DRIVER
 * ========================================================================== */

HAL_StatusTypeDef BME280_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t chip_id = 0;


    if (hi2c == NULL)
    {
        return HAL_ERROR;
    }


    bme280_i2c =
        hi2c;


#if BME280_SIMULATION_MODE


    /* ======================================================================
     * SIMULATION RENODE
     * ====================================================================== */

    simulation_index =
        0;


    simulation_read_count =
        0;


    if (BME280_CheckID(
            &chip_id) != HAL_OK)
    {
        return HAL_ERROR;
    }


    if (chip_id != BME280_CHIP_ID_VALUE)
    {
        return HAL_ERROR;
    }


    return HAL_OK;


#else


    /* ======================================================================
     * BME280 REEL
     * ====================================================================== */


    /* Vérifier la présence du capteur */

    if (HAL_I2C_IsDeviceReady(
            bme280_i2c,
            BME280_I2C_ADDRESS,
            3,
            BME280_I2C_TIMEOUT) != HAL_OK)
    {
        return HAL_ERROR;
    }


    /* Vérifier son Chip ID */

    if (BME280_CheckID(
            &chip_id) != HAL_OK)
    {
        return HAL_ERROR;
    }


    if (chip_id != BME280_CHIP_ID_VALUE)
    {
        return HAL_ERROR;
    }


    /* Reset */

    if (BME280_WriteRegister(
            BME280_REG_RESET,
            BME280_RESET_COMMAND) != HAL_OK)
    {
        return HAL_ERROR;
    }


    HAL_Delay(5);


    /* Lire les coefficients usine */

    if (BME280_ReadCalibration() != HAL_OK)
    {
        return HAL_ERROR;
    }


    /* Humidity oversampling x1 */

    if (BME280_WriteRegister(
            BME280_REG_CTRL_HUM,
            0x01U) != HAL_OK)
    {
        return HAL_ERROR;
    }


    /* Standby + filtre */

    if (BME280_WriteRegister(
            BME280_REG_CONFIG,
            0xA0U) != HAL_OK)
    {
        return HAL_ERROR;
    }


    /* Temp x1 + pression x1 + Normal Mode */

    if (BME280_WriteRegister(
            BME280_REG_CTRL_MEAS,
            0x27U) != HAL_OK)
    {
        return HAL_ERROR;
    }


    HAL_Delay(10);


    return HAL_OK;


#endif
}


/* ==========================================================================
 * LECTURE BME280
 * ========================================================================== */

HAL_StatusTypeDef BME280_Read(BME280_Data_t *data)
{
    if (data == NULL)
    {
        return HAL_ERROR;
    }


    if (bme280_i2c == NULL)
    {
        return HAL_ERROR;
    }


#if BME280_SIMULATION_MODE


    /* ======================================================================
     * SIMULATION D'UNE PANNE
     * ====================================================================== */

    simulation_read_count++;


    /*
     * Toutes les 6 lectures, on simule une erreur capteur.
     */
    if (simulation_read_count >=
        BME280_SIMULATED_FAULT_PERIOD)
    {
        simulation_read_count =
            0;


        return HAL_ERROR;
    }


    /* ======================================================================
     * MESURE NORMALE
     * ====================================================================== */

    *data =
        simulated_data[simulation_index];


    simulation_index++;


    if (simulation_index >=
        (sizeof(simulated_data) /
         sizeof(simulated_data[0])))
    {
        simulation_index =
            0;
    }


    return HAL_OK;


#else


    /* ======================================================================
     * VRAIE LECTURE I2C
     * ====================================================================== */

    uint8_t raw[8];

    int32_t adc_P;
    int32_t adc_T;
    int32_t adc_H;


    if (BME280_ReadRegisters(
            BME280_REG_PRESS_MSB,
            raw,
            sizeof(raw)) != HAL_OK)
    {
        return HAL_ERROR;
    }


    /* Pression RAW 20 bits */

    adc_P =
        ((int32_t)raw[0] << 12) |
        ((int32_t)raw[1] << 4) |
        ((int32_t)raw[2] >> 4);


    /* Température RAW 20 bits */

    adc_T =
        ((int32_t)raw[3] << 12) |
        ((int32_t)raw[4] << 4) |
        ((int32_t)raw[5] >> 4);


    /* Humidité RAW 16 bits */

    adc_H =
        ((int32_t)raw[6] << 8) |
        raw[7];


    /*
     * Température en premier car elle calcule t_fine.
     */

    data->temperature =
        BME280_CompensateTemperature(adc_T);


    data->pressure =
        BME280_CompensatePressure(adc_P)
        /
        100.0f;


    data->humidity =
        BME280_CompensateHumidity(adc_H);


    return HAL_OK;


#endif
}
