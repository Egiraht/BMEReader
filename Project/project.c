#include "project.h"

/**
 * @brief Stores the BME280 trimming parameters.
 */
static BME280_TrimmingParams trimmingParams;

/**
 * @brief Indicates if the BME280 device has been successfully initialized.
 */
static bool Project_IsBme280Initialized = false;

/**
 * @brief Forces the USB device re-enumeration at the host.
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
static bool Project_Bme280Init()
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

  LL_mDelay(1);

  if (!BME280_GetTrimmingParams(I2C1, &trimmingParams) || !BME280_SetConfig(I2C1, &config))
    return false;

  Project_IsBme280Initialized = true;
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
 * @param command The zero-terminated string containing the command message to process.
 */
static void Project_ProcessCommand(const char *command)
{
  char response[MAX_RESPONSE_MESSAGE_LENGTH + 1];
  BME280_Measurement measurement;

  if (strcasecmp(command, "get") == 0)
  {
    if (!Project_IsBme280Initialized)
      sprintf(response, "ERROR; The BME280 device failed to initialize\n");
    else
    {
      BME280_GetMeasurement(I2C1, &trimmingParams, &measurement);

      // Converting Pa to mmHg.
      measurement.pressure *= 0.007500617F;

      sprintf(response, "OK; Last measurement: P = %f mmHg, T = %f degC, H = %f %%\n",
        measurement.pressure, measurement.temperature, measurement.humidity);
    }
  }
  else
    sprintf(response, "ERROR; Unknown command: %s\n", command);

  Project_SendCdcMessage(&response[0], strlen(response));
}

/**
 * @brief The callback to be processed on USB CDC message reception.
 * @param string A pointer to the string containing the received message.
 * @param length Length of the message in the string.
 */
void Project_CdcMessageReceived(const char *string, uint16_t length)
{
  static char commandBuffer[MAX_COMMAND_MESSAGE_LENGTH + 1];
  static int16_t commandBufferIndex = 0;

  for (uint16_t index = 0; index < length; index++)
  {
    if (string[index] == 0x0A)
    {
      commandBuffer[commandBufferIndex] = 0x00;
      commandBufferIndex = 0;
      Project_ProcessCommand(&commandBuffer[0]);
    }
    else if (commandBufferIndex <= MAX_COMMAND_MESSAGE_LENGTH)
      commandBuffer[commandBufferIndex++] = string[index];
  }
}
