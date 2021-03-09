#ifndef USB2BME280_COMMAND_CALLBACKS_H
#define USB2BME280_COMMAND_CALLBACKS_H

#include "project.h"
#include "command.h"

/**
 * @brief The enumeration of the command indices defining the actions to be performed.
 * @see Command_Names
 * @see Command_Bindings
 */
typedef enum Command_Index
{
  COMMAND_UNKNOWN,
  COMMAND_ID,
  COMMAND_MEASURE,

  /**
   * @brief The total command indices count.
   */
  COMMANDS_COUNT
} Command_Index;

#endif //USB2BME280_COMMAND_CALLBACKS_H
