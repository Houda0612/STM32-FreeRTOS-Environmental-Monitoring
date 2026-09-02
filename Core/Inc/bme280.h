#ifndef BME280_H
#define BME280_H

#include "stm32f4xx_hal.h"


/* ==========================================================================
 * MODE DE FONCTIONNEMENT
 *
 * 1 = simulation pour Renode
 * 0 = véritable BME280 connecté en I2C
 * ========================================================================== */

#define BME280_SIMULATION_MODE          1


/* ==========================================================================
 * CONFIGURATION BME280
 * ========================================================================== */

/* Adresse I2C 7 bits du BME280 = 0x76
 * STM32 HAL attend l'adresse décalée vers la gauche.
 */
#define BME280_I2C_ADDRESS              (0x76U << 1)


/* Chip ID officiel du BME280 */
#define BME280_CHIP_ID_VALUE            0x60U


/* ==========================================================================
 * STRUCTURE DES MESURES
 * ========================================================================== */

typedef struct
{
    float temperature;
    float humidity;
    float pressure;

} BME280_Data_t;


/* ==========================================================================
 * FONCTIONS PUBLIQUES
 * ========================================================================== */

/* Initialise le driver BME280 */
HAL_StatusTypeDef BME280_Init(I2C_HandleTypeDef *hi2c);


/* Vérifie le Chip ID */
HAL_StatusTypeDef BME280_CheckID(uint8_t *chip_id);


/* Lit température, humidité et pression */
HAL_StatusTypeDef BME280_Read(BME280_Data_t *data);


#endif /* BME280_H */
