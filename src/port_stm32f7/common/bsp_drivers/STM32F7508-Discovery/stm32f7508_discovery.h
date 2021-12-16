/**
  ******************************************************************************
  * @file    stm32f7508_discovery.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for STM32F7508_DISCOVERY's LEDs,
  *          push-buttons and COM ports hardware resources.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */ 
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F7508_DISCOVERY_H
#define __STM32F7508_DISCOVERY_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
   
/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32F7508_DISCOVERY
  * @{
  */
      
/** @addtogroup STM32F7508_DISCOVERY_LOW_LEVEL
  * @{
  */ 

/** @defgroup STM32F7508_DISCOVERY_LOW_LEVEL_Exported_Types STM32F7508_DISCOVERY_LOW_LEVEL Exported Types
  * @{
  */
typedef enum 
{
LED1 = 0,
LED_GREEN = LED1,
}Led_TypeDef;

typedef enum 
{  
  BUTTON_WAKEUP = 0,
  BUTTON_TAMPER = 1,
  BUTTON_KEY = 2
}Button_TypeDef;

typedef enum 
{  
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
}ButtonMode_TypeDef;

typedef enum 
{
  COM1 = 0,
  COM2 = 1
}COM_TypeDef;
/**
  * @}
  */ 

/** @defgroup STM32F7508_DISCOVERY_LOW_LEVEL_Exported_Constants STM32F7508_DISCOVERY_LOW_LEVEL Exported Constants
  * @{
  */ 

/** 
  * @brief  Define for STM32F7508_DISCOVERY board
  */ 
#if !defined (USE_STM32F7508_DISCO)
 #define USE_STM32F7508_DISCO
#endif

/** @addtogroup STM32F7508_DISCOVERY_LOW_LEVEL_LED
  * @{
  */

#define LEDn                             ((uint8_t)1)

#define LED1_GPIO_PORT                   GPIOI
#define LED1_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOI_CLK_ENABLE()
#define LED1_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOI_CLK_DISABLE()
#define LED1_PIN                         GPIO_PIN_1

/**
  * @}
  */

/** @addtogroup STM32F7508_DISCOVERY_LOW_LEVEL_BUTTON
  * @{
  */ 
#define BUTTONn                             ((uint8_t)3) 

/**
  * @brief Wakeup push-button
  */
#define WAKEUP_BUTTON_PIN                   GPIO_PIN_11
#define WAKEUP_BUTTON_GPIO_PORT             GPIOI
#define WAKEUP_BUTTON_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOI_CLK_ENABLE()
#define WAKEUP_BUTTON_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOI_CLK_DISABLE()
#define WAKEUP_BUTTON_EXTI_IRQn             EXTI15_10_IRQn 

/**
  * @brief Tamper push-button
  */
#define TAMPER_BUTTON_PIN                    GPIO_PIN_11
#define TAMPER_BUTTON_GPIO_PORT              GPIOI
#define TAMPER_BUTTON_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOI_CLK_ENABLE()
#define TAMPER_BUTTON_GPIO_CLK_DISABLE()     __HAL_RCC_GPIOI_CLK_DISABLE()
#define TAMPER_BUTTON_EXTI_IRQn              EXTI15_10_IRQn

/**
  * @brief Key push-button
  */
#define KEY_BUTTON_PIN                       GPIO_PIN_11
#define KEY_BUTTON_GPIO_PORT                 GPIOI
#define KEY_BUTTON_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOI_CLK_ENABLE()
#define KEY_BUTTON_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOI_CLK_DISABLE()
#define KEY_BUTTON_EXTI_IRQn                 EXTI15_10_IRQn

#define BUTTONx_GPIO_CLK_ENABLE(__INDEX__)    do { if((__INDEX__) == 0) WAKEUP_BUTTON_GPIO_CLK_ENABLE(); else\
                                                   if((__INDEX__) == 1) TAMPER_BUTTON_GPIO_CLK_ENABLE(); else\
												                        KEY_BUTTON_GPIO_CLK_ENABLE(); } while(0)											   

#define BUTTONx_GPIO_CLK_DISABLE(__INDEX__)    (((__INDEX__) == 0) ? WAKEUP_BUTTON_GPIO_CLK_DISABLE() :\
                                                ((__INDEX__) == 1) ? TAMPER_BUTTON_GPIO_CLK_DISABLE() : KEY_BUTTON_GPIO_CLK_DISABLE())

/**
  * @}
  */

/** @addtogroup STM32F7508_DISCOVERY_LOW_LEVEL_SIGNAL
  * @{
  */
#define SIGNALn                             ((uint8_t)1)

/**
  * @brief SD-detect signal
  */
#define SD_DETECT_PIN                        GPIO_PIN_13
#define SD_DETECT_GPIO_PORT                  GPIOC
#define SD_DETECT_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOC_CLK_ENABLE()
#define SD_DETECT_GPIO_CLK_DISABLE()         __HAL_RCC_GPIOC_CLK_DISABLE()
#define SD_DETECT_EXTI_IRQn                  EXTI15_10_IRQn

/**
  * @brief Touch screen interrupt signal
  */
#define TS_INT_PIN                           GPIO_PIN_13
#define TS_INT_GPIO_PORT                     GPIOI
#define TS_INT_GPIO_CLK_ENABLE()             __HAL_RCC_GPIOI_CLK_ENABLE()
#define TS_INT_GPIO_CLK_DISABLE()            __HAL_RCC_GPIOI_CLK_DISABLE()
#define TS_INT_EXTI_IRQn                     EXTI15_10_IRQn

/**
  * @}
  */ 

/** @addtogroup STM32F7508_DISCOVERY_LOW_LEVEL_COM
  * @{
  */
#define COMn                             ((uint8_t)1)

/**
 * @brief Definition for COM port1, connected to USART1
 */ 
#define DISCOVERY_COM1                          USART1
#define DISCOVERY_COM1_CLK_ENABLE()             __HAL_RCC_USART1_CLK_ENABLE()
#define DISCOVERY_COM1_CLK_DISABLE()            __HAL_RCC_USART1_CLK_DISABLE()

#define DISCOVERY_COM1_TX_PIN                   GPIO_PIN_9
#define DISCOVERY_COM1_TX_GPIO_PORT             GPIOA
#define DISCOVERY_COM1_TX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define DISCOVERY_COM1_TX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOA_CLK_DISABLE()
#define DISCOVERY_COM1_TX_AF                    GPIO_AF7_USART1

#define DISCOVERY_COM1_RX_PIN                   GPIO_PIN_7
#define DISCOVERY_COM1_RX_GPIO_PORT             GPIOB
#define DISCOVERY_COM1_RX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOB_CLK_ENABLE()
#define DISCOVERY_COM1_RX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOB_CLK_DISABLE()
#define DISCOVERY_COM1_RX_AF                    GPIO_AF7_USART1

#define DISCOVERY_COM1_IRQn                     USART1_IRQn

#define DISCOVERY_COMx_CLK_ENABLE(__INDEX__)            do { if((__INDEX__) == COM1) DISCOVERY_COM1_CLK_ENABLE(); } while(0)
#define DISCOVERY_COMx_CLK_DISABLE(__INDEX__)           (((__INDEX__) == 0) ? DISCOVERY_COM1_CLK_DISABLE() : 0)

#define DISCOVERY_COMx_TX_GPIO_CLK_ENABLE(__INDEX__)    do { if((__INDEX__) == COM1) DISCOVERY_COM1_TX_GPIO_CLK_ENABLE(); } while(0)
#define DISCOVERY_COMx_TX_GPIO_CLK_DISABLE(__INDEX__)   (((__INDEX__) == 0) ? DISCOVERY_COM1_TX_GPIO_CLK_DISABLE() : 0)

#define DISCOVERY_COMx_RX_GPIO_CLK_ENABLE(__INDEX__)    do { if((__INDEX__) == COM1) DISCOVERY_COM1_RX_GPIO_CLK_ENABLE(); } while(0)
#define DISCOVERY_COMx_RX_GPIO_CLK_DISABLE(__INDEX__)   (((__INDEX__) == 0) ? DISCOVERY_COM1_RX_GPIO_CLK_DISABLE() : 0)

/* Exported constant IO ------------------------------------------------------*/

#define LCD_I2C_ADDRESS                  ((uint16_t)0x70)
#define CAMERA_I2C_ADDRESS               ((uint16_t)0x60)
#define AUDIO_I2C_ADDRESS                ((uint16_t)0x34)
#define EEPROM_I2C_ADDRESS_A01           ((uint16_t)0xA0)
#define EEPROM_I2C_ADDRESS_A02           ((uint16_t)0xA6)
#define TS_I2C_ADDRESS                   ((uint16_t)0x70)

/* I2C clock speed configuration (in Hz) 
   WARNING: 
   Make sure that this define is not already declared in other files (ie. 
   stm32f7508_discovery.h file). It can be used in parallel by other modules. */
#ifndef I2C_SPEED
 #define I2C_SPEED                       ((uint32_t)100000)
#endif /* I2C_SPEED */

/* User can use this section to tailor I2Cx/I2Cx instance used and associated 
   resources */
/* Definition for AUDIO and LCD I2Cx resources */
#define DISCOVERY_AUDIO_I2Cx                             I2C3
#define DISCOVERY_AUDIO_I2Cx_CLK_ENABLE()                __HAL_RCC_I2C3_CLK_ENABLE()
#define DISCOVERY_AUDIO_DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define DISCOVERY_AUDIO_I2Cx_SCL_SDA_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOH_CLK_ENABLE()

#define DISCOVERY_AUDIO_I2Cx_FORCE_RESET()               __HAL_RCC_I2C3_FORCE_RESET()
#define DISCOVERY_AUDIO_I2Cx_RELEASE_RESET()             __HAL_RCC_I2C3_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define DISCOVERY_AUDIO_I2Cx_SCL_PIN                     GPIO_PIN_7
#define DISCOVERY_AUDIO_I2Cx_SCL_SDA_GPIO_PORT           GPIOH
#define DISCOVERY_AUDIO_I2Cx_SCL_SDA_AF                  GPIO_AF4_I2C3
#define DISCOVERY_AUDIO_I2Cx_SDA_PIN                     GPIO_PIN_8

/* I2C interrupt requests */
#define DISCOVERY_AUDIO_I2Cx_EV_IRQn                     I2C3_EV_IRQn
#define DISCOVERY_AUDIO_I2Cx_ER_IRQn                     I2C3_ER_IRQn

/* Definition for external, camera and Arduino connector I2Cx resources */
#define DISCOVERY_EXT_I2Cx                               I2C1
#define DISCOVERY_EXT_I2Cx_CLK_ENABLE()                  __HAL_RCC_I2C1_CLK_ENABLE()
#define DISCOVERY_EXT_DMAx_CLK_ENABLE()                  __HAL_RCC_DMA1_CLK_ENABLE()
#define DISCOVERY_EXT_I2Cx_SCL_SDA_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOB_CLK_ENABLE()

#define DISCOVERY_EXT_I2Cx_FORCE_RESET()                 __HAL_RCC_I2C1_FORCE_RESET()
#define DISCOVERY_EXT_I2Cx_RELEASE_RESET()               __HAL_RCC_I2C1_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define DISCOVERY_EXT_I2Cx_SCL_PIN                       GPIO_PIN_8
#define DISCOVERY_EXT_I2Cx_SCL_SDA_GPIO_PORT             GPIOB
#define DISCOVERY_EXT_I2Cx_SCL_SDA_AF                    GPIO_AF4_I2C1
#define DISCOVERY_EXT_I2Cx_SDA_PIN                       GPIO_PIN_9

/* I2C interrupt requests */
#define DISCOVERY_EXT_I2Cx_EV_IRQn                       I2C1_EV_IRQn
#define DISCOVERY_EXT_I2Cx_ER_IRQn                       I2C1_ER_IRQn

/* I2C TIMING Register define when I2C clock source is SYSCLK */
/* I2C TIMING is calculated from APB1 source clock = 50 MHz */
/* Due to the big MOFSET capacity for adapting the camera level the rising time is very large (>1us) */
/* 0x40912732 takes in account the big rising and aims a clock of 100khz */
#ifndef DISCOVERY_I2Cx_TIMING  
#define DISCOVERY_I2Cx_TIMING                      ((uint32_t)0x40912732)  
#endif /* DISCOVERY_I2Cx_TIMING */

/**
  * @}
  */ 

/**
  * @}
  */ 
  
/** @defgroup STM32F7508_DISCOVERY_LOW_LEVEL_Exported_Macros STM32F7508_DISCOVERY_LOW_LEVEL Exported Macros
  * @{
  */  
/**
  * @}
  */ 

/** @addtogroup STM32F7508_DISCOVERY_LOW_LEVEL_Exported_Functions
  * @{
  */
uint32_t  BSP_GetVersion(void);
void      BSP_LED_Init(Led_TypeDef Led);
void      BSP_LED_DeInit(Led_TypeDef Led);
void      BSP_LED_On(Led_TypeDef Led);
void      BSP_LED_Off(Led_TypeDef Led);
void      BSP_LED_Toggle(Led_TypeDef Led);
void      BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode);
void      BSP_PB_DeInit(Button_TypeDef Button);
uint32_t  BSP_PB_GetState(Button_TypeDef Button);
void      BSP_COM_Init(COM_TypeDef COM, UART_HandleTypeDef *husart);
void      BSP_COM_DeInit(COM_TypeDef COM, UART_HandleTypeDef *huart);

/**
  * @}
  */

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F7508_DISCOVERY_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
