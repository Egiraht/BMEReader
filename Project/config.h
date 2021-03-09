#ifndef USB2BME280_CONFIG_H
#define USB2BME280_CONFIG_H

/**
 * @brief Defines the maximal length of a command message that can be processed.
 */
#define CONFIG_MAX_COMMAND_MESSAGE_LENGTH 64

/**
 * @brief Defines the maximal length of a command name.
 */
#define CONFIG_MAX_COMMAND_NAME_LENGTH 16

/**
 * @brief Defines the maximal length of a command parameter.
 */
#define CONFIG_MAX_COMMAND_PARAM_LENGTH CONFIG_MAX_COMMAND_MESSAGE_LENGTH

/**
 * @brief Defines the maximal length of a command value.
 */
#define CONFIG_MAX_COMMAND_VALUE_LENGTH CONFIG_MAX_COMMAND_MESSAGE_LENGTH

/**
 * @brief Defines the maximal length of the response message.
 */
#define CONFIG_MAX_RESPONSE_MESSAGE_LENGTH ((CONFIG_MAX_COMMAND_MESSAGE_LENGTH) + 64)

#endif //USB2BME280_CONFIG_H
