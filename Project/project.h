#ifndef BME_READER_PROJECT_H
#define BME_READER_PROJECT_H

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
#define PROJECT_NAME "BMEReader"

/**
 * @brief Defines the project version.
 */
#define PROJECT_VERSION "1.0"

extern BME280_TrimmingParams Project_TrimmingParams;

void Project_SetLedState(bool onState);
void Project_RecoverI2cState();
void Project_PreInit();
bool Project_Bme280Init();
void Project_PostInit();
void Project_Loop();
bool Project_SendCdcMessage(const char *string, uint16_t length);
void Project_CdcMessageReceived(const char *string, uint16_t length);

#endif //BME_READER_PROJECT_H
