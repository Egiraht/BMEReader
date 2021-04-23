/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright © 2020 Maxim Yudin <stibiu@yandex.ru>
 */

#include "command.h"

#define _TO_STR(s) #s
#define TO_STR(s) _TO_STR(s)

/**
 * @brief The default empty default command callback. Returns an empty string as a command response.
 */
void Command_EmptyDefaultCallback(__unused const Command_Descriptor *descriptor, __unused char *response)
{
  response = "";
}

/**
 * @brief A pointer to the default callback function that will handle commands that do not match any bindings in
 *   the <i>Command_Bindings</i> array.
 * @note This callback must be set or overridden in external code before any command processing is performed.
 *   By default it returns an empty string as a response.
 */
__weak_symbol Command_Callback Command_DefaultCallback = Command_EmptyDefaultCallback;

/**
 * @brief An array that defines bindings between command names and corresponding command callback functions.
 * @note This array must be set or overridden in external code before any command processing is performed.
 *   By default the array is empty.
 */
__weak_symbol Command_Binding Command_Bindings[] = {};

/**
 * @brief A constant that must provide the size of the <i>Command_Bindings</i> array.
 * @note This value must be set or overridden in external code before any command processing is performed.
 *   By default it is set to 0.
 */
__weak_symbol uint32_t Command_BindingsCount = 0;

/**
 * @brief Processes a command message.
 * @param commandMessage A string containing the command message to process.
 * @param responseMessage A pointer to the buffer that will be provided a response string returned by the processing
 *   callback method.
 */
void Command_ProcessMessage(const char *commandMessage, char *responseMessage)
{
  Command_Descriptor descriptor = {
    .name = "",
    .param = "",
    .value = ""
  };

  sscanf(commandMessage,
    "%" TO_STR(CONFIG_MAX_COMMAND_MESSAGE_LENGTH) "s"
    "%" TO_STR(CONFIG_MAX_COMMAND_MESSAGE_LENGTH) "s"
    "%" TO_STR(CONFIG_MAX_COMMAND_MESSAGE_LENGTH) "s",
    descriptor.name, descriptor.param, descriptor.value);

  for (uint32_t index = 0; index < Command_BindingsCount; index++)
  {
    if (strcasecmp(descriptor.name, Command_Bindings[index].commandName) == 0)
    {
      Command_Bindings[index].commandCallback(&descriptor, responseMessage);
      return;
    }
  }

  Command_DefaultCallback(&descriptor, responseMessage);
}
