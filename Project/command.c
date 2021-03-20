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
