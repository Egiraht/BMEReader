#ifndef USB2BME280_CONFIG_H
#define USB2BME280_CONFIG_H

/**
 * @brief Defines the maximal length of a command message that can be processed.
 */
#define CONFIG_MAX_COMMAND_MESSAGE_LENGTH 64

/**
 * @brief Defines the maximal length of the response message.
 */
#define CONFIG_MAX_RESPONSE_MESSAGE_LENGTH (2 * (CONFIG_MAX_COMMAND_MESSAGE_LENGTH))

#endif //USB2BME280_CONFIG_H
