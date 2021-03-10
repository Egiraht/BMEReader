#include "command.h"
#include "command_callbacks.h"

#define _TO_STR(s) #s
#define TO_STR(s) _TO_STR(s)

/**
 * @brief Parses the command message.
 * @param command The command message to parse.
 * @param descriptor The pointer to the <i>CommandDescriptor</i> structure where the parsed command information
 *   will be stored.
 * @return The parsed command content structure.
 */
void Command_Parse(const char *command, Command_Descriptor *descriptor)
{
  descriptor->index = COMMAND_INDEX_UNKNOWN;
  sscanf(command,
    "%" TO_STR(CONFIG_MAX_COMMAND_MESSAGE_LENGTH) "s"
    "%" TO_STR(CONFIG_MAX_COMMAND_MESSAGE_LENGTH) "s"
    "%" TO_STR(CONFIG_MAX_COMMAND_MESSAGE_LENGTH) "s",
    descriptor->name, descriptor->param, descriptor->value);

  if (STR_EMPTY(descriptor->name))
    return;

  // Searching for the matching command index.
  for (uint32_t counter = 0; counter < COMMAND_INDICES_COUNT; counter++)
  {
    if (STR_EQUAL(descriptor->name, Command_Names[counter]))
    {
      descriptor->index = counter;
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
  if (STR_EMPTY(commandMessage))
    return false;

  // Parsing the request and invoking the necessary command callback using the parsed command index.
  Command_Descriptor commandDescriptor = {
    .index = COMMAND_INDEX_UNKNOWN,
    .name = "",
    .param = "",
    .value = ""
  };
  Command_Parse(commandMessage, &commandDescriptor);
  Command_Bindings[commandDescriptor.index](&commandDescriptor, responseMessage);
  return true;
}
