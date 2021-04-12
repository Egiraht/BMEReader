/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright © 2020 Maxim Yudin <stibiu@yandex.ru>
 */

#ifndef BME_READER_COMMAND_H
#define BME_READER_COMMAND_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "config.h"

/**
 * @brief Defines the parsed command descriptor structure.
 */
typedef struct Command_Descriptor
{
  /**
   * @brief The command name string.
   */
  char name[CONFIG_MAX_COMMAND_MESSAGE_LENGTH + 1];

  /**
   * @brief The command parameter string.
   */
  char param[CONFIG_MAX_COMMAND_MESSAGE_LENGTH + 1];

  /**
   * @brief The command value string.
   */
  char value[CONFIG_MAX_COMMAND_MESSAGE_LENGTH + 1];
} Command_Descriptor;

/**
 * @brief The function pointer type for command callbacks.
 */
typedef void (*Command_Callback)(const Command_Descriptor *commandDescriptor, char *response);

/**
 * @brief The structure for binding command names and corresponding callback functions.
 */
typedef struct Command_Binding
{
  /**
   * @brief The command name used for invoking the bound <i>callback</i> function.
   */
  char *commandName;

  /**
   * @brief The callback function that will be invoked when the command name matches the bound <i>commandName</i>
   *   value.
   */
  Command_Callback commandCallback;
} Command_Binding;

extern Command_Callback Command_DefaultCallback;
extern Command_Binding Command_Bindings[];
extern uint32_t Command_BindingsCount;

void Command_ProcessMessage(const char *commandMessage, char *responseMessage);

#endif //BME_READER_COMMAND_H
