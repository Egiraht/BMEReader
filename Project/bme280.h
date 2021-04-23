/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright © 2020 Maxim Yudin <stibiu@yandex.ru>
 */

#ifndef BME280_H
#define BME280_H

#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#include "main.h"
#include "i2c.h"

/**
 * @brief The BME280 status structure.
 */
typedef struct BME280_Status
{
  bool isMeasuring;
  bool isMemoryUpdating;
} BME280_Status;

/**
 * @brief The BME280 humidity oversampling enumeration.
 */
typedef enum BME280_HumidityOversampling
{
  BME280_HUMIDITY_OVERSAMPLING_SKIPPED = 0x0,
  BME280_HUMIDITY_OVERSAMPLING_1 = 0x1,
  BME280_HUMIDITY_OVERSAMPLING_2 = 0x2,
  BME280_HUMIDITY_OVERSAMPLING_4 = 0x3,
  BME280_HUMIDITY_OVERSAMPLING_8 = 0x4,
  BME280_HUMIDITY_OVERSAMPLING_16 = 0x5
} BME280_HumidityOversampling;

/**
 * @brief The BME280 pressure oversampling enumeration.
 */
typedef enum BME280_PressureOversampling
{
  BME280_PRESSURE_OVERSAMPLING_SKIPPED = 0x0,
  BME280_PRESSURE_OVERSAMPLING_1 = 0x1,
  BME280_PRESSURE_OVERSAMPLING_2 = 0x2,
  BME280_PRESSURE_OVERSAMPLING_4 = 0x3,
  BME280_PRESSURE_OVERSAMPLING_8 = 0x4,
  BME280_PRESSURE_OVERSAMPLING_16 = 0x5
} BME280_PressureOversampling;

/**
 * @brief The BME280 temperature oversampling enumeration.
 */
typedef enum BME280_TemperatureOversampling
{
  BME280_TEMPERATURE_OVERSAMPLING_SKIPPED = 0x0,
  BME280_TEMPERATURE_OVERSAMPLING_1 = 0x1,
  BME280_TEMPERATURE_OVERSAMPLING_2 = 0x2,
  BME280_TEMPERATURE_OVERSAMPLING_4 = 0x3,
  BME280_TEMPERATURE_OVERSAMPLING_8 = 0x4,
  BME280_TEMPERATURE_OVERSAMPLING_16 = 0x5
} BME280_TemperatureOversampling;

/**
 * @brief The BME280 modes enumeration.
 */
typedef enum BME280_Mode
{
  BME280_MODE_SLEEP = 0x0,
  BME280_MODE_FORCED = 0x1,
  BME280_MODE_NORMAL = 0x3
} BME280_Mode;

/**
 * @brief The BME280 standby times enumeration.
 */
typedef enum BME280_StandbyTime
{
  BME280_STANDBY_TIME_0ms5 = 0x0,
  BME280_STANDBY_TIME_62ms5 = 0x1,
  BME280_STANDBY_TIME_125ms = 0x2,
  BME280_STANDBY_TIME_250ms = 0x3,
  BME280_STANDBY_TIME_500ms = 0x4,
  BME280_STANDBY_TIME_1s = 0x5,
  BME280_STANDBY_TIME_10ms = 0x6,
  BME280_STANDBY_TIME_20ms = 0x7
} BME280_StandbyTime;

/**
 * @brief The BME280 filtering factor enumeration.
 */
typedef enum BME280_Filter
{
  BME280_FILTER_OFF = 0x0,
  BME280_FILTER_2 = 0x1,
  BME280_FILTER_4 = 0x2,
  BME280_FILTER_8 = 0x3,
  BME280_FILTER_16 = 0x4
} BME280_Filter;

/**
 * @brief The BME280 configuration structure.
 */
typedef struct BME280_Config
{
  BME280_HumidityOversampling humidityOversampling;
  BME280_PressureOversampling pressureOversampling;
  BME280_TemperatureOversampling temperatureOversampling;
  BME280_Mode mode;
  BME280_StandbyTime standbyTime;
  BME280_Filter filter;
  bool useSPI3WireMode;
} BME280_Config;

/**
 * @brief The BME280 measurement data structure.
 */
typedef struct BME280_Measurement
{
  float temperature;
  float pressure;
  float humidity;
} BME280_Measurement;

/**
 * @brief The BME280 trimming parameters data arrays.
 */
typedef struct BME280_TrimmingParams
{
  float t[3];
  float p[9];
  float h[6];
} BME280_TrimmingParams;

I2C_Result BME280_GetID(I2C_TypeDef *i2c, uint8_t *id);
I2C_Result BME280_Reset(I2C_TypeDef *i2c);
I2C_Result BME280_SetConfig(I2C_TypeDef *i2c, BME280_Config *config);
I2C_Result BME280_GetConfig(I2C_TypeDef *i2c, BME280_Config *config);
I2C_Result BME280_GetStatus(I2C_TypeDef *i2c, BME280_Status *status);
I2C_Result BME280_GetTrimmingParams(I2C_TypeDef *i2c, BME280_TrimmingParams *params);
I2C_Result BME280_GetMeasurement(I2C_TypeDef *i2c, BME280_TrimmingParams *params, BME280_Measurement *measurement);

#endif
