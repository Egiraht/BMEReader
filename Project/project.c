/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright © 2020 Maxim Yudin <stibiu@yandex.ru>
 */

#include "project.h"

/**
 * @brief Stores the BME280 trimming parameters.
 */
BME280_TrimmingParams Project_TrimmingParams;

/**
 * @brief Sets the LED state.
 * @param onState If <i>true</i> turns the LED on, otherwise turns the LED off.
 */
inline void Project_SetLedState(bool onState)
{
  return onState ? LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin) : LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin);
}

/**
 * @brief Forces the USB device re-enumeration at the host. Must be called before USB peripheral initialization.
 */
static void Project_ReEnumerateUsb()
{
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  LL_GPIO_ResetOutputPin(DM_GPIO_Port, DM_Pin);
  LL_GPIO_ResetOutputPin(DP_GPIO_Port, DP_Pin);
  LL_GPIO_SetPinOutputType(DM_GPIO_Port, DM_Pin, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinOutputType(DM_GPIO_Port, DP_Pin, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinMode(DM_GPIO_Port, DM_Pin, LL_GPIO_MODE_OUTPUT_50MHz);
  LL_GPIO_SetPinMode(DP_GPIO_Port, DP_Pin, LL_GPIO_MODE_OUTPUT_50MHz);
  LL_mDelay(10);
  LL_GPIO_SetOutputPin(DP_GPIO_Port, DP_Pin);
  LL_GPIO_SetPinMode(DM_GPIO_Port, DM_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(DP_GPIO_Port, DP_Pin, LL_GPIO_MODE_INPUT);
}

/**
 * @brief Recovers the I2C bus from possible stuck states. This function must be called before any I2C communication is
 *   performed.
 */
void Project_RecoverI2cState()
{
  // Recover only if the I2C peripheral BUSY flag is set.
  if (!LL_I2C_IsActiveFlag_BUSY(I2C1))
    return;

  LL_I2C_Disable(I2C1);

  // Forcing release of the SDA line if a slave is holding it low.
  if (!LL_GPIO_IsInputPinSet(SDA_GPIO_Port, SDA_Pin))
  {
    // Disconnecting the SCL and SDA lines from the I2C peripheral and setting them as open-drain outputs.
    LL_GPIO_SetOutputPin(SCL_GPIO_Port, SCL_Pin);
    LL_GPIO_SetOutputPin(SDA_GPIO_Port, SDA_Pin);
    LL_GPIO_SetPinMode(SCL_GPIO_Port, SCL_Pin, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(SDA_GPIO_Port, SDA_Pin, LL_GPIO_MODE_OUTPUT);

    // Cycling the SCL line until the SDA line is released.
    uint16_t attempts = 20;
    while (!LL_GPIO_IsInputPinSet(SDA_GPIO_Port, SDA_Pin) && --attempts)
    {
      LL_GPIO_ResetOutputPin(SCL_GPIO_Port, SCL_Pin);
      LL_mDelay(0);
      LL_GPIO_SetOutputPin(SCL_GPIO_Port, SCL_Pin);
      LL_mDelay(0);
    }

    // Generating sequential START and STOP conditions.
    LL_GPIO_ResetOutputPin(SDA_GPIO_Port, SDA_Pin);
    LL_mDelay(0);
    LL_GPIO_ResetOutputPin(SCL_GPIO_Port, SCL_Pin);
    LL_mDelay(0);
    LL_GPIO_SetOutputPin(SCL_GPIO_Port, SCL_Pin);
    LL_mDelay(0);
    LL_GPIO_SetOutputPin(SDA_GPIO_Port, SDA_Pin);
    LL_mDelay(0);

    // Connecting the SCL and SDA lines back to the I2C peripheral.
    LL_GPIO_SetPinMode(SCL_GPIO_Port, SCL_Pin, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinMode(SDA_GPIO_Port, SDA_Pin, LL_GPIO_MODE_ALTERNATE);
  }

  // Reinitializing the I2C peripheral.
  LL_I2C_EnableReset(I2C1);
  LL_I2C_DisableReset(I2C1);
  MX_I2C1_Init();
  LL_I2C_Enable(I2C1);
}

/**
 * @brief Called before peripherals are initialized but after RCC initialization.
 */
void Project_PreInit()
{
  Project_SetLedState(true);
  Project_ReEnumerateUsb();
}

/**
 * @brief Initializes the BME280 sensor.
 * @return <i>true</i> on successful device initialization, otherwise <i>false</i>.
 */
bool Project_Bme280Init()
{
  BME280_Config config = {
    .mode = BME280_MODE_NORMAL,
    .filter = CONFIG_FILTER_FACTOR,
    .pressureOversampling = CONFIG_PRESSURE_OVERSAMPLING_FACTOR,
    .temperatureOversampling = CONFIG_TEMPERATURE_OVERSAMPLING_FACTOR,
    .humidityOversampling = CONFIG_HUMIDITY_OVERSAMPLING_FACTOR,
    .standbyTime = CONFIG_STANDBY_TIME,
    .useSPI3WireMode = false
  };

  Project_RecoverI2cState();

  uint8_t id;
  if (BME280_GetID(I2C1, &id) != I2C_RESULT_OK || id != 0x60 || BME280_Reset(I2C1) != I2C_RESULT_OK)
    return false;

  uint16_t attempts = 100;
  BME280_Status status;
  do
  {
    if (BME280_GetStatus(I2C1, &status) != I2C_RESULT_OK)
      return false;
  }
  while (status.isMemoryUpdating && --attempts);
  if (!attempts)
    return false;

  if (BME280_GetTrimmingParams(I2C1, &Project_TrimmingParams) != I2C_RESULT_OK ||
    BME280_SetConfig(I2C1, &config) != I2C_RESULT_OK)
    return false;

  LL_mDelay(100);

  return true;
}

/**
 * @brief Called after peripherals are initialized.
 */
void Project_PostInit()
{
  Project_Bme280Init();
  Project_SetLedState(false);
}

/**
 * @brief Sends a message over USB CDC interface.
 * @param string A pointer to the string containing the message to be sent.
 * @param length Length of the message in the string.
 * @return <i>true</i> if the message has been sent successfully, otherwise <i>false</i>.
 */
inline bool Project_SendCdcMessage(const char *string, uint16_t length)
{
  return CDC_Transmit_FS((uint8_t *) string, length) == USBD_OK;
}

/**
 * @brief Processes the provided command message.
 * @param command A pointer to the string containing the command message to process.
 */
static void Project_ProcessCommand(const char *command)
{
  char response[CONFIG_MAX_RESPONSE_MESSAGE_LENGTH + 1];
  Command_ProcessMessage(command, &response[0]);
  Project_SendCdcMessage(&response[0], strlen(response));
}

/**
 * @brief The callback to be processed on USB CDC message reception.
 * @param string A pointer to the string containing the received message.
 * @param length Length of the message in the string.
 */
void Project_CdcMessageReceived(const char *string, uint16_t length)
{
  static char commandBuffer[CONFIG_MAX_COMMAND_MESSAGE_LENGTH + 1];
  static int16_t commandBufferIndex = 0;

  Project_SetLedState(true);

  for (uint16_t index = 0; index < length; index++)
  {
    if (string[index] == 0x0A)
    {
      commandBuffer[commandBufferIndex] = 0x00;
      commandBufferIndex = 0;
      Project_ProcessCommand(&commandBuffer[0]);
    }
    else if (commandBufferIndex <= CONFIG_MAX_COMMAND_MESSAGE_LENGTH)
      commandBuffer[commandBufferIndex++] = string[index];
  }

  Project_SetLedState(false);
}

/**
 * @brief Called in the main background loop.
 */
void Project_Loop()
{
}
