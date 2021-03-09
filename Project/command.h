#ifndef USB2BME280_COMMAND_H
#define USB2BME280_COMMAND_H

#include <stdint.h>
#include <stdbool.h>

#include "config.h"

/**
 * @brief Defines the parsed command descriptor structure.
 */
typedef struct Command_Descriptor
{
  /**
   * @brief The found index of the command.
   * @see <i>CommandIndex</i> enumeration.
   */
  uint32_t commandIndex;

  /**
   * @brief The command name string.
   */
  char command[CONFIG_MAX_COMMAND_NAME_LENGTH + 1];

  /**
   * @brief The command parameter string.
   */
  char param[CONFIG_MAX_COMMAND_PARAM_LENGTH + 1];

  /**
   * @brief The command value string.
   */
  char value[CONFIG_MAX_COMMAND_VALUE_LENGTH + 1];
} Command_Descriptor;

/**
 * @brief The common function pointer type used for the command callbacks.
 */
typedef void (*Command_Callback)(const Command_Descriptor *commandDescriptor, char *response);

extern char *Command_Names[];
extern Command_Callback Command_Bindings[];

bool Command_ProcessMessage(const char *commandMessage, char *responseMessage);

#endif //USB2BME280_COMMAND_H
