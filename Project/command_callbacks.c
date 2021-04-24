/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright © 2020 Maxim Yudin <stibiu@yandex.ru>
 */

#include "command.h"
#include "project.h"

#pragma ide diagnostic ignored "OCUnusedMacroInspection"

/**
 * @brief Returns <i>true</i> when the two strings match each other.
 * @param str1 The first string to compare.
 * @param str2 The second string to compare.
 */
#define STR_EQUAL(str1, str2) (strcasecmp(str1, str2) == 0)

/**
 * @brief Returns <i>true</i> when the string is empty.
 * @param str The string to check.
 */
#define STR_EMPTY(str) (strlen(str) == 0)

/**
 * @brief Defines the OK response string.
 */
#define OK_RESPONSE "OK\n"

/**
 * @brief Defines the formatted OK response string.
 * @param format The text string with optional value-formatting placeholders.
 */
#define OK_RESPONSE_FORMAT(format) "OK; " format "\n"

/**
 * @brief Defines the formatted ERROR response string.
 * @param format The text string with optional value-formatting placeholders.
 */
#define ERROR_RESPONSE_FORMAT(format) "ERROR; " format "\n"

/**
 * @brief Defines the formatted ERROR response string with notification about the invalid command name.
 * @param format The text string with optional value-formatting placeholders.
 */
#define INVALID_COMMAND_RESPONSE_FORMAT(format) ERROR_RESPONSE_FORMAT("Invalid command: " format)

/**
 * @brief Defines the formatted ERROR response string with notification about the invalid command parameter.
 * @param format The text string with optional value-formatting placeholders.
 */
#define INVALID_PARAMETER_RESPONSE_FORMAT(format) ERROR_RESPONSE_FORMAT("Invalid parameter: " format)

/**
 * @brief Defines the formatted ERROR response string with notification about the invalid command parameter.
 *   Also it contains the formatted list of acceptable values.
 * @param param The text string with optional value-formatting placeholders containing the parameter name.
 * @param list The text string with optional value-formatting placeholders containing the acceptable parameter values.
 */
#define INVALID_PARAMETER_LIST_RESPONSE_FORMAT(param, list) INVALID_PARAMETER_RESPONSE_FORMAT(param "; Supported: " list)

/**
 * @brief Defines the formatted ERROR response string with notification about the invalid command parameter value.
 * @param format The text string with optional value-formatting placeholders.
 */
#define INVALID_VALUE_RESPONSE_FORMAT(format) ERROR_RESPONSE_FORMAT("Invalid value: " format)

/**
 * @brief Defines the formatted ERROR response string with notification about the invalid command parameter value.
 *   Also it contains the formatted range of acceptable parameter values.
 * @param param The text string with optional value-formatting placeholders containing the parameter name.
 * @param min The text string with optional value-formatting placeholders containing the minimal parameter value.
 * @param max The text string with optional value-formatting placeholders containing the maximal parameter value.
 */
#define INVALID_VALUE_RANGE_RESPONSE_FORMAT(param, min, max) INVALID_VALUE_RESPONSE_FORMAT(param "; Allowed range: " min "-" max)

/**
 * @brief Gets the message string describing the I2C result.
 * @param result The I2C result value to get the message for.
 * @param resultMessage A string buffer where the message will be put.
 */
static void GetI2cResultMessage(I2C_Result result, char *resultMessage)
{
  switch (result)
  {
    case I2C_RESULT_OK:
    {
      sprintf(resultMessage, OK_RESPONSE);
      return;
    }
    case I2C_RESULT_START_FAILED:
    {
      sprintf(resultMessage, ERROR_RESPONSE_FORMAT("Failed to start an I2C transmission."));
      return;
    }
    case I2C_RESULT_ADDRESS_FAILED:
    {
      sprintf(resultMessage, ERROR_RESPONSE_FORMAT("The BME280 sensor was not detected on the I2C bus."));
      return;
    }
    case I2C_RESULT_ACK_FAILED:
    {
      sprintf(resultMessage, ERROR_RESPONSE_FORMAT("The BME280 sensor failed to acknowledge the data."));
      return;
    }
    case I2C_RESULT_READ_FAILED:
    {
      sprintf(resultMessage, ERROR_RESPONSE_FORMAT("Failed to read data from the BME280 sensor."));
      return;
    }
    default:
    {
      sprintf(resultMessage, ERROR_RESPONSE_FORMAT("An unknown error has occurred."));
      return;
    }
  }
}

/**
 * @brief The default callback for unknown commands.
 * @param commandDescriptor The pointer to the input command descriptor structure.
 * @param response The output response message buffer.
 */
static void UnknownCommand(const Command_Descriptor *commandDescriptor, char *response)
{
  sprintf(response, INVALID_COMMAND_RESPONSE_FORMAT("%s"), commandDescriptor->name);
}

/**
 * @brief The command returning the device identifier string.
 * @param descriptor The pointer to the input command descriptor structure.
 * @param response The output response message buffer.
 * @remarks Command usage:
 *   @code Id
 */
static void IdCommand(__unused const Command_Descriptor *descriptor, char *response)
{
  sprintf(response, OK_RESPONSE_FORMAT("%s; Version: %s; SN: %08lX%08lX%08lX"), PROJECT_NAME, PROJECT_VERSION,
    LL_GetUID_Word2(), LL_GetUID_Word1(), LL_GetUID_Word0());
}

/**
 * @brief The command returning the measured climatic data.
 * @param descriptor The pointer to the input command descriptor structure.
 * @param response The output response message buffer.
 * @remarks Command usage:
 *   @code Measure P|T|H|All
 */
static void MeasureCommand(const Command_Descriptor *descriptor, char *response)
{
  BME280_Config config;
  BME280_Measurement measurement;
  I2C_Result result;

  Project_RecoverI2cState();

  result = BME280_GetConfig(I2C1, &config);
  if (result != I2C_RESULT_OK)
    return GetI2cResultMessage(result, response);

  if (config.mode == BME280_MODE_SLEEP && !Project_Bme280Init())
    return (void) sprintf(response, ERROR_RESPONSE_FORMAT("Failed to initialize the BME280 sensor."));

  result = BME280_GetMeasurement(I2C1, &Project_TrimmingParams, &measurement);
  if (result != I2C_RESULT_OK)
    return GetI2cResultMessage(result, response);

  // Converting Pa to mmHg.
  measurement.pressure *= 0.007500617F;

  if (STR_EQUAL(descriptor->param, "All"))
    sprintf(response, OK_RESPONSE_FORMAT("P = %f mmHg; T = %f degC; H = %f %%"),
      measurement.pressure, measurement.temperature, measurement.humidity);
  else if (STR_EQUAL(descriptor->param, "P"))
    sprintf(response, OK_RESPONSE_FORMAT("%f mmHg"), measurement.pressure);
  else if (STR_EQUAL(descriptor->param, "T"))
    sprintf(response, OK_RESPONSE_FORMAT("%f degC"), measurement.temperature);
  else if (STR_EQUAL(descriptor->param, "H"))
    sprintf(response, OK_RESPONSE_FORMAT("%f %%"), measurement.humidity);
  else
    sprintf(response, INVALID_PARAMETER_LIST_RESPONSE_FORMAT("%s", "%s"), descriptor->param, "P, T, H, All");
}

/**
 * @brief The command that requests a software reset of the MCU and jumps to the bootloader if necessary.
 * @param descriptor The pointer to the input command descriptor structure.
 * @param response The output response message buffer.
 * @remarks Command usage:
 *   @code Reset Normal|Bootloader
 */
static void ResetCommand(__unused const Command_Descriptor *descriptor, __unused char *response)
{
  if (STR_EQUAL(descriptor->param, "Normal"))
    Project_RequestSoftwareReset(false);
  else if (STR_EQUAL(descriptor->param, "Bootloader"))
    Project_RequestSoftwareReset(true);
  else
    return (void) sprintf(response, INVALID_PARAMETER_LIST_RESPONSE_FORMAT("%s", "%s"), descriptor->param,
      "Normal, Bootloader");

  sprintf(response, OK_RESPONSE_FORMAT("Performing a %s software reset shortly..."), descriptor->param);
}

/**
 * @brief The default command callback.
 */
Command_Callback Command_DefaultCallback = UnknownCommand;

/**
 * @brief The command bindings array.
 */
Command_Binding Command_Bindings[] = {
  {
    .commandName = "Id",
    .commandCallback = IdCommand
  },
  {
    .commandName = "Measure",
    .commandCallback = MeasureCommand
  },
  {
    .commandName = "Reset",
    .commandCallback = ResetCommand
  }
};

/**
 * @brief The command bindings count value.
 */
uint32_t Command_BindingsCount = sizeof(Command_Bindings) / sizeof(Command_Binding);
