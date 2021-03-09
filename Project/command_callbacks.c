#include "command_callbacks.h"

#pragma ide diagnostic ignored "OCUnusedMacroInspection"

#define OK_RESPONSE "OK\n"
#define OK_RESPONSE_FORMAT(format) "OK; " format "\n"
#define ERROR_RESPONSE_FORMAT(format) "ERROR; " format "\n"
#define INVALID_COMMAND_RESPONSE_FORMAT(format) ERROR_RESPONSE_FORMAT("Invalid command: " format)
#define INVALID_PARAMETER_RESPONSE_FORMAT(format) ERROR_RESPONSE_FORMAT("Invalid parameter: " format)
#define INVALID_PARAMETER_LIST_RESPONSE_FORMAT(param, list) INVALID_PARAMETER_RESPONSE_FORMAT(param "; Supported: " list)
#define INVALID_VALUE_RESPONSE_FORMAT(format) ERROR_RESPONSE_FORMAT("Invalid value: " format)
#define INVALID_VALUE_RANGE_RESPONSE_FORMAT(param, min, max) INVALID_VALUE_RESPONSE_FORMAT(param "; Allowed range: " min "-" max)

/**
 * @brief The array of available command names that should be matched by the request.
 *   The names are case insensitive.
 * @see Command_Index
 * @see Command_Bindings
 */
char *Command_Names[COMMANDS_COUNT] = {
  [COMMAND_ID] = "id",
  [COMMAND_MEASURE] = "meas"
};

/**
 * @brief The default callback for unknown commands.
 * @param commandDescriptor The pointer to the input command descriptor structure.
 * @param response The output response message buffer.
 */
void CommandCallback_Unknown(const Command_Descriptor *commandDescriptor, char *response)
{
  sprintf(response, INVALID_COMMAND_RESPONSE_FORMAT("%s"), commandDescriptor->command);
}

/**
 * @brief The command returning the device identifier string.
 * @param descriptor The pointer to the input command descriptor structure.
 * @param response The output response message buffer.
 */
void CommandCallback_Id(__unused const Command_Descriptor *descriptor, char *response)
{
  sprintf(response, OK_RESPONSE_FORMAT("%s; Version: %s; SN: %08lX%08lX%08lX"), PROJECT_NAME, PROJECT_VERSION,
    LL_GetUID_Word2(), LL_GetUID_Word1(), LL_GetUID_Word0());
}

/**
 * @brief The command returning the measured climatic data.
 * @param descriptor The pointer to the input command descriptor structure.
 * @param response The output response message buffer.
 */
void CommandCallback_Measure(__unused const Command_Descriptor *descriptor, char *response)
{
  if (!Project_IsBme280Initialized)
  {
    if (!Project_Bme280Init())
      sprintf(response, ERROR_RESPONSE_FORMAT("Failed to initialize the BME280"));
  }

  BME280_Measurement measurement;
  if (!BME280_GetMeasurement(I2C1, &Project_TrimmingParams, &measurement))
    sprintf(response, ERROR_RESPONSE_FORMAT("Failed to communicate with the BME280"));

  // Converting Pa to mmHg.
  measurement.pressure *= 0.007500617F;

  sprintf(response, OK_RESPONSE_FORMAT("P = %f mmHg; T = %f degC; H = %f%%"),
    measurement.pressure, measurement.temperature, measurement.humidity);
}

/**
 * @brief The array that establishes the bindings between the commands indices and the corresponding command callback
 *   pointers of type <i>CommandCallback</i> to be invoked on the corresponding command request.
 * @see Command_Index
 * @see Command_Names
 */
Command_Callback Command_Bindings[COMMANDS_COUNT] = {
  [COMMAND_UNKNOWN] = CommandCallback_Unknown,
  [COMMAND_ID] = CommandCallback_Id,
  [COMMAND_MEASURE] = CommandCallback_Measure
};
