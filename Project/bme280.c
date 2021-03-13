#include "bme280.h"

/**
 * @brief Sets the BME280 I2C address to be used for communication.
 */
volatile uint8_t BME280_address = 0x76;

/**
 * @brief Gets the device identification code.
 * @param i2c A pointer to structure of the I2C peripheral to use.
 * @return The identifier code of the device. Must be <i>0x60</i> for BME280.
 */
uint8_t BME280_GetID(I2C_TypeDef *i2c)
{
  uint8_t address = 0xD0;   // id
  uint8_t id;

  if (I2C_Write(i2c, BME280_address, &address, sizeof(address), false) &&
    I2C_Read(i2c, BME280_address, &id, sizeof(id), true))
    return id;

  I2C_SEND_STOP(i2c);
  return 0x00;
}

/**
 * @brief Resets the device.
 * @param i2c A pointer to structure of the I2C peripheral to use.
 * @return <i>true</i> if operation succeeds, otherwise <i>false</i>.
 */
bool BME280_Reset(I2C_TypeDef *i2c)
{
  uint8_t resetData[2] = {0xE0, 0xB6};  // reset = 0xB6
  return I2C_Write(i2c, BME280_address, &resetData[0], 2, true);
}

/**
 * @brief Sets the device configuration.
 * @param i2c A pointer to structure of the I2C peripheral to use.
 * @param config A pointer to the BME280 configuration structure to be set for the device.
 * @return <i>true</i> if operation succeeds, otherwise <i>false</i>.
 */
bool BME280_SetConfig(I2C_TypeDef *i2c, BME280_Config *config)
{
  uint8_t configData[6] = {
    0xF5,   // config
    (config->standbyTime & 0x07) << 5 | (config->filter & 0x07) << 2 | (config->useSPI3WireMode ? 0x01 : 0x00),
    0xF2,   // ctrl_hum
    config->humidityOversampling & 0x07,
    0xF4,   // ctrl_meas
    (config->temperatureOversampling & 0x07) << 5 | (config->pressureOversampling & 0x07) << 2 | (config->mode & 0x03)
  };

  return I2C_Write(i2c, BME280_address, &configData[0], sizeof(configData), true);
}

/**
 * @brief Gets the current device configuration.
 * @param i2c A pointer to structure of the I2C peripheral to use.
 * @param config A pointer to the BME280 configuration structure that will be filled with the data from the device.
 * @return <i>true</i> if operation succeeds, otherwise <i>false</i>.
 */
bool BME280_GetConfig(I2C_TypeDef *i2c, BME280_Config *config)
{
  uint8_t startAddress = 0xF2;  // ctrl_hum
  uint8_t data[4];              // ctrl_hum .. config

  if (!I2C_Write(i2c, BME280_address, &startAddress, sizeof(startAddress), false) ||
    !I2C_Read(i2c, BME280_address, &data[0], sizeof(data), true))
    return false;

  config->humidityOversampling = data[0] & 0x07;
  config->temperatureOversampling = (data[2] & 0xE0) >> 5;
  config->pressureOversampling = (data[2] & 0x1C) >> 2;
  config->mode = data[2] & 0x03;
  config->standbyTime = (data[3] & 0xE0) >> 5;
  config->filter = (data[3] & 0x1C) >> 2;
  config->useSPI3WireMode = data[3] & 0x01;

  return true;
}

/**
 * @brief Gets the current device status.
 * @param i2c A pointer to structure of the I2C peripheral to use.
 * @param status A pointer to the BME280 status structure that will be filled with the data from the device.
 * @return <i>true</i> if operation succeeds, otherwise <i>false</i>.
 */
bool BME280_GetStatus(I2C_TypeDef *i2c, BME280_Status *status)
{
  uint8_t statusAddress = 0xF3;   // status
  uint8_t statusByte;

  if (!I2C_Write(i2c, BME280_address, &statusAddress, sizeof(statusAddress), false) ||
    !I2C_Read(i2c, BME280_address, &statusByte, sizeof(statusByte), true))
    return false;

  status->isMeasuring = statusByte & 0x08;
  status->isMemoryUpdating = statusByte & 0x01;

  return true;
}

/**
 * @brief Gets the device trimming parameters used for device measurements calibration.
 * @param i2c A pointer to structure of the I2C peripheral to use.
 * @param params A pointer to the BME280 trimming parameters structure that will be filled with the data from the device.
 * @return <i>true</i> if operation succeeds, otherwise <i>false</i>.
 */
bool BME280_GetTrimmingParams(I2C_TypeDef *i2c, BME280_TrimmingParams *params)
{
  uint8_t trimmingAddress1 = 0x88;  // calib00
  uint8_t trimmingLength1 = 26;     // calib00 .. calib25
  uint8_t trimmingAddress2 = 0xE1;  // calib26
  uint8_t trimmingLength2 = 16;     // calib26 .. calib41
  uint8_t trimmingData[trimmingLength1 + trimmingLength1];

  if (!I2C_Write(i2c, BME280_address, &trimmingAddress1, sizeof(trimmingAddress1), false) ||
    !I2C_Read(i2c, BME280_address, &trimmingData[0], trimmingLength1, true) ||
    !I2C_Write(i2c, BME280_address, &trimmingAddress2, sizeof(trimmingAddress2), false) ||
    !I2C_Read(i2c, BME280_address, &trimmingData[trimmingLength1], trimmingLength2, true))
    return false;

  params->t[0] = (uint16_t) (trimmingData[0] | trimmingData[1] << 8);
  params->t[1] = (int16_t) (trimmingData[2] | trimmingData[3] << 8);
  params->t[2] = (int16_t) (trimmingData[4] | trimmingData[5] << 8);

  params->p[0] = (uint16_t) (trimmingData[6] | trimmingData[7] << 8);
  params->p[1] = (int16_t) (trimmingData[8] | trimmingData[9] << 8);
  params->p[2] = (int16_t) (trimmingData[10] | trimmingData[11] << 8);
  params->p[3] = (int16_t) (trimmingData[12] | trimmingData[13] << 8);
  params->p[4] = (int16_t) (trimmingData[14] | trimmingData[15] << 8);
  params->p[5] = (int16_t) (trimmingData[16] | trimmingData[17] << 8);
  params->p[6] = (int16_t) (trimmingData[18] | trimmingData[19] << 8);
  params->p[7] = (int16_t) (trimmingData[20] | trimmingData[21] << 8);
  params->p[8] = (int16_t) (trimmingData[22] | trimmingData[23] << 8);

  params->h[0] = (uint8_t) trimmingData[25];
  params->h[1] = (int16_t) (trimmingData[26] | trimmingData[27] << 8);
  params->h[2] = (uint8_t) trimmingData[28];
  params->h[3] = (int16_t) (trimmingData[29] << 4 | trimmingData[30] & 0x0F);
  params->h[4] = (int16_t) (trimmingData[30] >> 4 | trimmingData[31] << 4);
  params->h[5] = (int8_t) trimmingData[32];

  return true;
}

/**
 * @brief Gets the last measured climatic data from the device.
 * @param i2c A pointer to structure of the I2C peripheral to use.
 * @param params A pointer to the BME280 trimming parameters structure that will be used to calibrate the measured data.
 * @param measurement A pointer to the BME280 measurement structure that will be filled with the calculated climatic
 *   data measured by the device.
 * @return <i>true</i> if operation succeeds, otherwise <i>false</i>.
 */
bool BME280_GetMeasurement(I2C_TypeDef *i2c, BME280_TrimmingParams *params, BME280_Measurement *measurement)
{
  uint8_t rawDataAddress = 0xF7;  // press_msb
  uint8_t rawData[8];             // press_msb .. hum_lsb
  if (!I2C_Write(i2c, BME280_address, &rawDataAddress, sizeof(rawDataAddress), false) ||
    !I2C_Read(i2c, BME280_address, &rawData[0], sizeof(rawData), true))
    return false;

  // Collecting the sensor data.
  uint32_t pData = (rawData[0] << 12) | (rawData[1] << 4) | (rawData[2] >> 4);
  uint32_t tData = (rawData[3] << 12) | (rawData[4] << 4) | (rawData[5] >> 4);
  uint32_t hData = (rawData[6] << 8) | rawData[7];

  // Computing the temperature.
  float t1 = (tData / 16384.0F - params->t[0] / 1024.0F) * params->t[1];
  float t2 =
    ((tData / 131072.0F - params->t[0] / 8192.0F) * (tData / 131072.0F - params->t[0] / 8192.0F)) * params->t[2];
  float tFine = t1 + t2;
  measurement->temperature = (t1 + t2) / 5120.0F;

  // Computing the pressure.
  float p1 = tFine / 2.0F - 64000.0F;
  float p2 = p1 * p1 * params->p[5] / 32768.0F;
  p2 = p2 + p1 * params->p[4] * 2.0F;
  p2 = p2 / 4.0F + params->p[3] * 65536.0F;
  p1 = (params->p[2] * p1 * p1 / 524288.0F + params->p[1] * p1) / 524288.0F;
  p1 = (1.0F + p1 / 32768.0F) * params->p[0];
  if (p1 != 0.0)
  {
    float p3 = 1048576.0F - pData;
    p3 = (p3 - p2 / 4096.0F) * 6250.0F / p1;
    p1 = params->p[8] * p3 * p3 / 2147483648.0F;
    p2 = p3 * params->p[7] / 32768.0F;
    measurement->pressure = p3 + (p1 + p2 + params->p[6]) / 16.0F;
  }
  else
    measurement->pressure = 0;

  // Computing the humidity.
  float h = tFine - 76800.0F;
  h = (hData - (params->h[3] * 64.0F + params->h[4] / 16384.0F * h)) *
    (params->h[1] / 65536.0F * (1.0F + params->h[5] / 67108864.0F * h * (1.0F + params->h[2] / 67108864.0F * h)));
  h = h * (1.0F - params->h[0] * h / 524288.0F);
  if (h > 100.0F)
    h = 100.0F;
  else if (h < 0.0F)
    h = 0.0F;
  measurement->humidity = h;

  return true;
}
