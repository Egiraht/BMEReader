#include "command.h"
#include "command_callbacks.h"

#define COMMAND_STR_EQUAL(str1, str2) (strcasecmp(str1, str2) == 0)
#define COMMAND_STR_EMPTY(str) (strlen(str) == 0)
#define _COMMAND_TO_STR(s) #s
#define COMMAND_TO_STR(s) _COMMAND_TO_STR(s)

/**
 * @brief Parses the command message.
 * @param command The command message to parse.
 * @param descriptor The pointer to the <i>CommandDescriptor</i> structure where the parsed command information
 *   will be stored.
 * @return The parsed command content structure.
 */
void Command_Parse(const char *command, Command_Descriptor *descriptor)
{
  descriptor->commandIndex = COMMAND_UNKNOWN;
  sscanf(command,
    "%" COMMAND_TO_STR(CONFIG_MAX_COMMAND_NAME_LENGTH) "s"
    "%" COMMAND_TO_STR(CONFIG_MAX_COMMAND_PARAM_LENGTH) "s"
    "%" COMMAND_TO_STR(CONFIG_MAX_COMMAND_VALUE_LENGTH) "s",
    descriptor->command, descriptor->param, descriptor->value);

  if (COMMAND_STR_EMPTY(descriptor->command))
    return;

  // Searching for the matching command index.
  for (uint32_t counter = 0; counter < COMMANDS_COUNT; counter++)
  {
    if (COMMAND_STR_EQUAL(descriptor->command, Command_Names[counter]))
    {
      descriptor->commandIndex = counter;
      break;
    }
  }
}

/**
 * @brief Processes a command message.
 * @param commandMessage The input command message to process.
 * @param responseMessage The output response message.
 */
bool Command_ProcessMessage(const char *commandMessage, char *responseMessage)
{
  if (COMMAND_STR_EMPTY(commandMessage))
    return false;

  // Parsing the request and invoking the necessary command callback using the parsed command index.
  Command_Descriptor commandDescriptor;
  Command_Parse(commandMessage, &commandDescriptor);
  Command_Bindings[commandDescriptor.commandIndex](&commandDescriptor, responseMessage);
  return true;
}
