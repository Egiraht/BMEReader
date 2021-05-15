/**
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright © 2021 Maxim Yudin <stibiu@yandex.ru>
 */

#include "i2c.h"

/**
 * @brief Writes the buffer bytes to the I2C bus.
 * @param i2c The I2C peripheral structure to be used for communication.
 * @param address The 7-bit I2C address with the "read/write" flag bit shifted out.
 * @param buffer A pointer to the buffer where the buffer to be written are stored.
 * @param length The number of buffer bytes to be written from the buffer.
 * @param sendStop Defines if the "stop" condition should be issued after the buffer have been written.
 *   Set to <i>true</i> if further sequential read/write operations will take place after completing the function.
 * @return A value indicating the I2C operation result. See the <i>I2C_Result</i> enumeration.
 */
I2C_Result I2C_Write(I2C_TypeDef *i2c, uint8_t address, uint8_t *buffer, uint16_t length, bool sendStop)
{
  I2C_CLEAR_ALL_FLAGS(i2c);

  // Sending the "(re)start" condition.
  uint16_t attempts = I2C_MAX_ATTEMPTS;
  I2C_SEND_START(i2c);
  while (!I2C_IS_START_OK(i2c) && --attempts);
  if (!attempts)
  {
    I2C_CLEAR_START(i2c);
    I2C_SEND_STOP(i2c);
    return I2C_RESULT_START_FAILED;
  }

  // Sending the address with the "write" flag.
  attempts = I2C_MAX_ATTEMPTS;
  I2C_SEND_ADDRESS_WRITE(i2c, address);
  while (!I2C_IS_ADDRESS_OK(i2c) && !I2C_IS_ACK_FAILED(i2c) && --attempts);
  if (!attempts || I2C_IS_ACK_FAILED(i2c))
  {
    I2C_CLEAR_ACK_FAILED_FLAG(i2c);
    I2C_SEND_STOP(i2c);
    return I2C_RESULT_ADDRESS_FAILED;
  }

  // Writing the buffer bytes.
  I2C_CLEAR_ADDRESS_OK_FLAG(i2c);
  for (uint16_t counter = 0; counter < length; counter++)
  {
    attempts = I2C_MAX_ATTEMPTS;
    I2C_WRITE_BYTE(i2c, buffer[counter]);
    while (!I2C_WRITE_BYTE_OK(i2c) && !I2C_IS_ACK_FAILED(i2c) && --attempts);

    if (!attempts || I2C_IS_ACK_FAILED(i2c))
    {
      I2C_CLEAR_ACK_FAILED_FLAG(i2c);
      I2C_SEND_STOP(i2c);
      return I2C_RESULT_ACK_FAILED;
    }
  }

  // Sending the "stop" condition if necessary.
  if (sendStop)
    I2C_SEND_STOP(i2c);

  return I2C_RESULT_OK;
}

/**
 * @brief Reads the data bytes from the I2C bus.
 * @param i2c The I2C peripheral structure to be used for communication.
 * @param address The 7-bit I2C address with the "read/write" flag bit shifted out.
 * @param buffer A pointer to the buffer where the data to be read will be stored.
 * @param length The number of data bytes to be read to the buffer.
 * @param sendStop Defines if the "stop" condition should be issued after the data have been read.
 *   Set to <i>true</i> if further sequential read/write operations will take place after completing the function.
 * @return A value indicating the I2C operation result. See the <i>I2C_Result</i> enumeration.
 */
I2C_Result I2C_Read(I2C_TypeDef *i2c, uint8_t address, uint8_t *buffer, uint16_t length, bool sendStop)
{
  I2C_CLEAR_ALL_FLAGS(i2c);

  // Sending the "(re)start" condition.
  uint16_t attempts = I2C_MAX_ATTEMPTS;
  I2C_SEND_START(i2c);
  while (!I2C_IS_START_OK(i2c) && --attempts);
  if (!attempts)
  {
    I2C_CLEAR_START(i2c);
    I2C_SEND_STOP(i2c);
    return I2C_RESULT_START_FAILED;
  }

  // Sending the address with the "read" flag.
  attempts = I2C_MAX_ATTEMPTS;
  I2C_SEND_ADDRESS_READ(i2c, address);
  while (!I2C_IS_ADDRESS_OK(i2c) && !I2C_IS_ACK_FAILED(i2c) && --attempts);
  if (!attempts || I2C_IS_ACK_FAILED(i2c))
  {
    I2C_CLEAR_ACK_FAILED_FLAG(i2c);
    I2C_SEND_STOP(i2c);
    return I2C_RESULT_ADDRESS_FAILED;
  }

  // Reading the data bytes.
  I2C_CLEAR_ADDRESS_OK_FLAG(i2c);
  for (uint16_t counter = 0; counter < length; counter++)
  {
    if (counter < length - 1)
      I2C_ACK_NEXT_READ(i2c);
    else
    {
      I2C_NACK_NEXT_READ(i2c);

      if (sendStop)
        I2C_SEND_STOP(i2c);
    }

    attempts = I2C_MAX_ATTEMPTS;
    while (!I2C_IS_BYTE_RECEIVED(i2c) && --attempts);
    if (!attempts)
    {
      I2C_SEND_STOP(i2c);
      return I2C_RESULT_READ_FAILED;
    }

    buffer[counter] = I2C_READ_BYTE(i2c);
  }

  return I2C_RESULT_OK;
}
