#ifndef USB2BME280_PROJECT_H
#define USB2BME280_PROJECT_H

#include <stdlib.h>
#include <stdbool.h>

#include "config.h"
#include "usbd_cdc_if.h"
#include "i2c.h"
#include "bme280.h"
#include "command.h"

/**
 * @brief Defines the project name.
 */
#define PROJECT_NAME "USB2BME280"

/**
 * @brief Defines the project version.
 */
#define PROJECT_VERSION "1.0"

extern BME280_TrimmingParams Project_TrimmingParams;
extern bool Project_IsBme280Initialized;

void Project_ReEnumerateUsb();
void Project_PreInit();
bool Project_Bme280Init();
void Project_PostInit();
void Project_Loop();
bool Project_SendCdcMessage(const char *string, uint16_t length);
void Project_CdcMessageReceived(const char *string, uint16_t length);

#endif //USB2BME280_PROJECT_H
