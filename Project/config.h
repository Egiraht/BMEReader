#ifndef BME_READER_CONFIG_H
#define BME_READER_CONFIG_H

/**
 * @brief Defines the maximal length of a command message that can be processed.
 */
#define CONFIG_MAX_COMMAND_MESSAGE_LENGTH 64

/**
 * @brief Defines the maximal length of the response message.
 */
#define CONFIG_MAX_RESPONSE_MESSAGE_LENGTH (2 * (CONFIG_MAX_COMMAND_MESSAGE_LENGTH))

/**
 * @brief Defines the BME280 sensor pressure oversampling factor.
 * @see <i>BME280_PressureOversampling</i> enumeration values.
 */
#define CONFIG_PRESSURE_OVERSAMPLING_FACTOR BME280_PRESSURE_OVERSAMPLING_16

/**
 * @brief Defines the BME280 sensor temperature oversampling factor.
 * @see <i>BME280_TemperatureOversampling</i> enumeration values.
 */
#define CONFIG_TEMPERATURE_OVERSAMPLING_FACTOR BME280_TEMPERATURE_OVERSAMPLING_16

/**
 * @brief Defines the BME280 sensor temperature oversampling factor.
 * @see <i>BME280_TemperatureOversampling</i> enumeration values.
 */
#define CONFIG_HUMIDITY_OVERSAMPLING_FACTOR BME280_HUMIDITY_OVERSAMPLING_16

/**
 * @brief Defines the BME280 sensor filtering factor.
 * @see <i>BME280_Filter</i> enumeration values.
 */
#define CONFIG_FILTER_FACTOR BME280_FILTER_OFF

/**
 * @brief Defines the BME280 sensor standby time.
 * @see <i>BME280_StandbyTime</i> enumeration values.
 */
#define CONFIG_STANDBY_TIME BME280_STANDBY_TIME_62ms5

#endif //BME_READER_CONFIG_H
