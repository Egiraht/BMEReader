#include "project.h"

/**
 * @brief Forces the USB device re-enumeration at the host.
 */
static inline void Project_ReEnumerateUsb()
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
 * @brief Called after peripherals are initialized.
 */
void Project_PostInit()
{

}

/**
 * @brief Called in the main background loop.
 */
void Project_Loop()
{

}

/**
 * @brief Sends a message over USB CDC interface.
 * @param buffer A pointer to the buffer containing the message to be sent.
 * @param length Length of the message in the buffer.
 * @return <i>true</i> if the message has been sent successfully, otherwise <i>false</i>.
 */
inline bool Project_SendCdcMessage(const uint8_t *buffer, uint16_t length)
{
  return CDC_Transmit_FS((uint8_t *) buffer, length) == USBD_OK;
}

/**
 * @brief Processes the provided command message.
 * @param command The zero-terminated string containing the command message to process.
 */
static inline void Project_ProcessCommand(const char *command)
{
  uint16_t length = strlen(command);
  uint8_t buffer[MAX_MESSAGE_LENGTH + 1];
  memcpy(buffer, command, length);
  buffer[length] = 0x0A;
  Project_SendCdcMessage(buffer, length + 1);
}

/**
 * @brief The callback to be processed on USB CDC message reception.
 * @param buffer A pointer to the buffer containing the received message.
 * @param length Length of the message in the buffer.
 */
void Project_CdcMessageReceived(const uint8_t *buffer, uint16_t length)
{
  static char commandBuffer[MAX_MESSAGE_LENGTH + 1];
  static int16_t commandBufferIndex = 0;

  for (uint16_t index = 0; index < length; index++)
  {
    if (buffer[index] == 0x0A)
    {
      commandBuffer[commandBufferIndex] = 0x00;
      commandBufferIndex = 0;
      Project_ProcessCommand(&commandBuffer[0]);
    }
    else if (commandBufferIndex <= MAX_MESSAGE_LENGTH)
      commandBuffer[commandBufferIndex++] = buffer[index];
  }
}
