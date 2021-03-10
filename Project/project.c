#include "project.h"

/**
 * @brief Stores the BME280 trimming parameters.
 */
BME280_TrimmingParams Project_TrimmingParams;

/**
 * @brief Forces the USB device re-enumeration at the host.
 */
void Project_ReEnumerateUsb()
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
 * @brief Called before peripherals are initialized but after RCC initialization.
 */
void Project_PreInit()
{
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
    .filter = BME280_FILTER_OFF,
    .pressureOversampling = BME280_PRESSURE_OVERSAMPLING_16,
    .temperatureOversampling = BME280_TEMPERATURE_OVERSAMPLING_16,
    .humidityOversampling = BME280_HUMIDITY_OVERSAMPLING_16,
    .standbyTime = BME280_STANDBY_TIME_62ms5,
    .useSPI3WireMode = false
  };

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

  return true;
}

/**
 * @brief Called after peripherals are initialized.
 */
void Project_PostInit()
{
  LL_I2C_Enable(I2C1);

  Project_Bme280Init();
}

/**
 * @brief Called in the main background loop.
 */
void Project_Loop()
{
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
}
