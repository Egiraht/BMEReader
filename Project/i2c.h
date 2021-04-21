/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright © 2020 Maxim Yudin <stibiu@yandex.ru>
 */

#ifndef I2C_H
#define I2C_H

#include <stdbool.h>

#include "main.h"

/**
 * @brief Defines the maximal number of I2C successive flag polling attempts before the operation will be timed out.
 */
#define I2C_MAX_ATTEMPTS 1000

/* Hardware control macros. */
#define I2C_SEND_START(i2c)                 (LL_I2C_GenerateStartCondition(i2c))
#define I2C_CLEAR_START(i2c)                (CLEAR_BIT(i2c->CR1, I2C_CR1_START))
#define I2C_IS_START_OK(i2c)                (LL_I2C_IsActiveFlag_SB(i2c))
#define I2C_SEND_STOP(i2c)                  (LL_I2C_GenerateStopCondition(i2c))
#define I2C_SEND_ADDRESS_WRITE(i2c, addr)   (LL_I2C_TransmitData8(i2c, (addr) << 1 | 0))
#define I2C_SEND_ADDRESS_READ(i2c, addr)    (LL_I2C_TransmitData8(i2c, (addr) << 1 | 1))
#define I2C_IS_ADDRESS_OK(i2c)              (LL_I2C_IsActiveFlag_ADDR(i2c))
#define I2C_CLEAR_ADDRESS_OK_FLAG(i2c)      (LL_I2C_ClearFlag_ADDR(i2c))
#define I2C_IS_ACK_FAILED(i2c)              (LL_I2C_IsActiveFlag_AF(i2c))
#define I2C_CLEAR_ACK_FAILED_FLAG(i2c)      (LL_I2C_ClearFlag_AF(i2c))
#define I2C_WRITE_BYTE(i2c, byte)           (LL_I2C_TransmitData8(i2c, (byte)))
#define I2C_WRITE_BYTE_OK(i2c)              (LL_I2C_IsActiveFlag_BTF(i2c))
#define I2C_ACK_NEXT_READ(i2c)              (LL_I2C_AcknowledgeNextData(i2c, LL_I2C_ACK))
#define I2C_NACK_NEXT_READ(i2c)             (LL_I2C_AcknowledgeNextData(i2c, LL_I2C_NACK))
#define I2C_IS_BYTE_RECEIVED(i2c)           (LL_I2C_IsActiveFlag_RXNE(i2c))
#define I2C_READ_BYTE(i2c)                  (LL_I2C_ReceiveData8(i2c))

bool I2C_Write(I2C_TypeDef *i2c, uint8_t address, uint8_t *buffer, uint16_t length, bool sendStop);

bool I2C_Read(I2C_TypeDef *i2c, uint8_t address, uint8_t *buffer, uint16_t length, bool sendStop);

#endif
