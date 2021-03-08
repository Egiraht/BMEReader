#ifndef USB2BME280_PROJECT_H
#define USB2BME280_PROJECT_H

#include <stdlib.h>
#include <stdbool.h>

#include "usbd_cdc_if.h"
#include "i2c.h"
#include "bme280.h"

/**
 * @brief Defines the maximal length of the command message that can be processed.
 */
#define MAX_COMMAND_MESSAGE_LENGTH 128

/**
 * @brief Defines the maximal length of the response message that must exceed the MAX_COMMAND_MESSAGE_LENGTH value.
 */
#define MAX_RESPONSE_MESSAGE_LENGTH ((MAX_COMMAND_MESSAGE_LENGTH) + 64)

void Project_PreInit();

void Project_PostInit();

void Project_Loop();

bool Project_SendCdcMessage(const char *string, uint16_t length);

void Project_CdcMessageReceived(const char *string, uint16_t length);

#endif //USB2BME280_PROJECT_H
