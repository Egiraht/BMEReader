/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright © 2020 Maxim Yudin <stibiu@yandex.ru>
 */

#include "project.h"

/**
 * @brief Defines the MCU bootloader start address.
 */
#define PROJECT_BOOTLOADER_START_ADDRESS 0x1FFF0000

/**
 * @brief Defines the key value that indicates that a jump to the bootloader has been requested.
 */
#define PROJECT_BOOTLOADER_KEY 0x12345678

/**
 * @brief The simple action callback definition.
 */
typedef void (*Project_Action)();

/**
 * @brief Stores the BME280 trimming parameters.
 */
BME280_TrimmingParams Project_TrimmingParams;

/**
 * @brief Requests a jump to the bootloader after performing a software reset. This functions does not return.
 */
__NO_RETURN void Project_RequestJumpToBootloader()
{
  // Setting the bootloader key value to the backup register.
  LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, PROJECT_BOOTLOADER_KEY);

  // Requesting a software reset for the MCU.
  NVIC_SystemReset();
}

/**
 * @brief Checks if the bootloader jump has been requested, and jumps to it if true.
 */
void Project_JumpToBootloaderIfRequested()
{
  // Checking if the bootloader key value has been set in the backup register.
  if (LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR0) != PROJECT_BOOTLOADER_KEY)
    return;

  // Clearing the key value from the backup register.
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_EnableBkUpAccess();
  LL_RCC_EnableRTC();
  LL_RTC_BAK_SetRegister(RTC, LL_RTC_BKP_DR0, 0);

  // Setting the bootloader stack pointer and jumping to the bootloader entry point.
  uint32_t bootloaderStackPointer = *((uint32_t *) PROJECT_BOOTLOADER_START_ADDRESS);
  Project_Action bootloaderEntryPoint = (Project_Action) *((uint32_t *) (PROJECT_BOOTLOADER_START_ADDRESS + 4));
  __set_MSP(bootloaderStackPointer);
  bootloaderEntryPoint();
}

/**
 * @brief Sets the LED state.
 * @param onState If <i>true</i> turns the LED on, otherwise turns the LED off.
 */
inline void Project_SetLedState(bool onState)
{
  return onState ? LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin) : LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin);
}

/**
 * @brief Recovers the I2C bus from possible stuck states. This function must be called before any I2C communication is
 *   performed.
 */
void Project_RecoverI2cState()
{
  // Recover only if the I2C bus BUSY flag is set.
  if (!LL_I2C_IsActiveFlag_BUSY(I2C1))
    return;

  // Forcing release of the SDA line if a slave is holding it low.
  uint16_t attempts = 20;
  LL_I2C_Disable(I2C1);
  LL_GPIO_SetPinMode(SCL_GPIO_Port, SCL_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(SDA_GPIO_Port, SDA_Pin, LL_GPIO_MODE_INPUT);
  while (!LL_GPIO_IsInputPinSet(SDA_GPIO_Port, SDA_Pin) && --attempts)
  {
    LL_GPIO_ResetOutputPin(SCL_GPIO_Port, SCL_Pin);
    LL_mDelay(0);
    LL_GPIO_SetOutputPin(SCL_GPIO_Port, SCL_Pin);
    LL_mDelay(0);
  }
  LL_GPIO_SetPinMode(SCL_GPIO_Port, SCL_Pin, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinMode(SDA_GPIO_Port, SDA_Pin, LL_GPIO_MODE_ALTERNATE);

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

  if (BME280_GetID(I2C1) != 0x60 || !BME280_Reset(I2C1))
    return false;

  uint16_t attempts = 100;
  BME280_Status status;
  do
  {
    if (!BME280_GetStatus(I2C1, &status))
      return false;
  }
  while (status.isMemoryUpdating && --attempts);
  if (!attempts)
    return false;

  if (!BME280_GetTrimmingParams(I2C1, &Project_TrimmingParams) || !BME280_SetConfig(I2C1, &config))
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
