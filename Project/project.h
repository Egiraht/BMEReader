#ifndef USB2BME280_PROJECT_H
#define USB2BME280_PROJECT_H

#include <stdlib.h>
#include <stdbool.h>

#include "usbd_cdc_if.h"
#include "i2c.h"
#include "bme280.h"

/**
 * @brief Defines the maximal length of the message that can be processed.
 */
#define MAX_MESSAGE_LENGTH 128

void Project_PreInit();

void Project_PostInit();

void Project_Loop();

bool Project_SendCdcMessage(const uint8_t *buffer, uint16_t length);

void Project_CdcMessageReceived(const uint8_t *buffer, uint16_t length);

#endif //USB2BME280_PROJECT_H
