#ifndef USB2BME280_COMMAND_CALLBACKS_H
#define USB2BME280_COMMAND_CALLBACKS_H

#include "project.h"
#include "command.h"

/**
 * @brief The enumeration of the command indices defining the actions to be performed.
 * @see <i>Command_Names</i>
 * @see <i>Command_Bindings</i>
 */
typedef enum Command_Index
{
  COMMAND_INDEX_UNKNOWN,
  COMMAND_INDEX_ID,
  COMMAND_INDEX_MEASURE,

  /* Put new command indices above. */

  /**
   * @brief The total command indices count.
   */
  COMMAND_INDICES_COUNT
} Command_Index;

#endif //USB2BME280_COMMAND_CALLBACKS_H
