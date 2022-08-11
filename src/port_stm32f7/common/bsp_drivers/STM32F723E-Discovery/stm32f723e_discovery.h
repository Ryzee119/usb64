/**
  ******************************************************************************
  * @file    stm32f723e_discovery.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for STM32F723E-Discovery LEDs,
  *          push-buttons hardware resources.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F723E_DISCOVERY_H
#define __STM32F723E_DISCOVERY_H

#ifdef __cplusplus
 extern "C" {
#endif


 /* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32F723E_DISCOVERY
  * @{
  */

/** @addtogroup STM32F723E_DISCOVERY_LOW_LEVEL
  * @{
  */

/** @defgroup STM32F723E_DISCOVERY_LOW_LEVEL_Exported_Types STM32F723E Discovery Low Level Exported Types
 * @{
 */

/** 
  * @brief  Define for STM32F723E_DISCOVERY board
  */ 
#if !defined (USE_STM32F723E_DISCO)
 #define USE_STM32F723E_DISCO
#endif

/** @brief Led_TypeDef
  *  STM32F723E_Discovery board leds definitions.
  */
typedef enum
{
 LED5 = 0,
 LED_RED = LED5,
 LED6 = 1,
 LED_GREEN = LED6
} Led_TypeDef;

/** @brief Button_TypeDef
  *  STM32F723E_Discovery board Buttons definitions.
  */
typedef enum
{
  BUTTON_WAKEUP = 0,
} Button_TypeDef;

#define BUTTON_USER BUTTON_WAKEUP

/** @brief ButtonMode_TypeDef
  *  STM32F723E_Discovery board Buttons Modes definitions.
  */
typedef enum
{
 BUTTON_MODE_GPIO = 0,
 BUTTON_MODE_EXTI = 1

} ButtonMode_TypeDef;

typedef enum 
{
  PB_SET = 0, 
  PB_RESET = !PB_SET
} ButtonValue_TypeDef;

typedef enum 
{
  COM1 = 0,
}COM_TypeDef;

/** @brief DISCO_Status_TypeDef
  *  STM32F723E_DISCO board Status return possible values.
  */
typedef enum
{
 DISCO_OK    = 0,
 DISCO_ERROR = 1

} DISCO_Status_TypeDef;

/**
  * @}
  */

/** @addtogroup STM32F723E_DISCOVERY_LOW_LEVEL_LED STM32F723E Discovery Low Level Led
  * @{
  */
/* Always four leds for all revisions of Discovery boards */
#define LEDn                             ((uint8_t)2)


/* 2 Leds are connected to MCU directly on PA7 and PB1 */
#define LED5_GPIO_PORT                   ((GPIO_TypeDef*)GPIOA)
#define LED6_GPIO_PORT                   ((GPIO_TypeDef*)GPIOB)

#define LED5_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define LED6_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOB_CLK_ENABLE()

#define LED5_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOA_CLK_DISABLE()
#define LED6_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOB_CLK_DISABLE()


#define LEDx_GPIO_CLK_ENABLE(__INDEX__)  do{if((__INDEX__) == 0) LED5_GPIO_CLK_ENABLE(); else \
                                            if((__INDEX__) == 1) LED6_GPIO_CLK_ENABLE(); \
                                            }while(0)

#define LEDx_GPIO_CLK_DISABLE(__INDEX__)  do{if((__INDEX__) == 0) LED5_GPIO_CLK_DISABLE(); else \
                                             if((__INDEX__) == 1) LED6_GPIO_CLK_DISABLE(); \
                                             }while(0)

#define LED5_PIN                         ((uint32_t)GPIO_PIN_7)
#define LED6_PIN                         ((uint32_t)GPIO_PIN_1)

/**
  * @}
  */

/** @addtogroup STM32F723E_DISCOVERY_LOW_LEVEL_BUTTON STM32F723E Discovery Low Level Button
  * @{
  */
/* Only one User/Wakeup button */
#define BUTTONn                             ((uint8_t)1)

/**
  * @brief Wakeup push-button
  */
#define WAKEUP_BUTTON_PIN                   GPIO_PIN_0
#define WAKEUP_BUTTON_GPIO_PORT             GPIOA
#define WAKEUP_BUTTON_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()
#define WAKEUP_BUTTON_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOA_CLK_DISABLE()
#define WAKEUP_BUTTON_EXTI_IRQn             EXTI0_IRQn

/* Define the USER button as an alias of the Wakeup button */
#define USER_BUTTON_PIN                   WAKEUP_BUTTON_PIN
#define USER_BUTTON_GPIO_PORT             WAKEUP_BUTTON_GPIO_PORT
#define USER_BUTTON_GPIO_CLK_ENABLE()     WAKEUP_BUTTON_GPIO_CLK_ENABLE()
#define USER_BUTTON_GPIO_CLK_DISABLE()    WAKEUP_BUTTON_GPIO_CLK_DISABLE()
#define USER_BUTTON_EXTI_IRQn             WAKEUP_BUTTON_EXTI_IRQn

#define BUTTON_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOA_CLK_ENABLE()

/**
  * @}
  */

/** @defgroup STM32F723E_DISCOVERY_LOW_LEVEL_Exported_Constants LOW_LEVEL Exported Constants
  * @{
  */
/**
  * @brief USB OTG HS Over Current signal
  */
#define OTG_HS_OVER_CURRENT_PIN                  GPIO_PIN_10
#define OTG_HS_OVER_CURRENT_PORT                 GPIOH
#define OTG_HS_OVER_CURRENT_PORT_CLK_ENABLE()    __HAL_RCC_GPIOH_CLK_ENABLE()

/**
  * @brief USB OTG FS Over Current signal
  */
#define OTG_FS_OVER_CURRENT_PIN                  GPIO_PIN_8
#define OTG_FS_OVER_CURRENT_PORT                 GPIOC
#define OTG_FS_OVER_CURRENT_PORT_CLK_ENABLE()    __HAL_RCC_GPIOC_CLK_ENABLE()

/**
  * @brief TS_INT signal from TouchScreen
  */
#define TS_INT_PIN                        ((uint32_t)GPIO_PIN_9)
#define TS_INT_GPIO_PORT                  ((GPIO_TypeDef*)GPIOI)
#define TS_INT_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOI_CLK_ENABLE()
#define TS_INT_GPIO_CLK_DISABLE()         __HAL_RCC_GPIOI_CLK_DISABLE()
#define TS_INT_EXTI_IRQn                  EXTI9_5_IRQn

/**
  * @brief TS RESET pin
  */
#define TS_RESET_PIN                        GPIO_PIN_9
#define TS_RESET_GPIO_PORT                  GPIOH
#define TS_RESET_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOH_CLK_ENABLE()
#define TS_RESET_GPIO_CLK_DISABLE()         __HAL_RCC_GPIOH_CLK_DISABLE()
#define TS_RESET_EXTI_IRQn                  EXTI15_10_IRQn

/**
  * @brief Definition for I2C3 Touchscreen Pins
  * resources (touchescreen).
  * Definition for I2C3 clock resources
  */
#define TS_I2Cx                             I2C3
#define TS_I2Cx_CLK_ENABLE()                __HAL_RCC_I2C3_CLK_ENABLE()
#define TS_I2Cx_SCL_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define TS_I2Cx_SDA_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOH_CLK_ENABLE()

#define TS_I2Cx_FORCE_RESET()               __HAL_RCC_I2C3_FORCE_RESET()
#define TS_I2Cx_RELEASE_RESET()             __HAL_RCC_I2C3_RELEASE_RESET()

/** @brief Definition for Touchscreen Pins
  */
#define TS_I2Cx_SCL_PIN                     GPIO_PIN_8
#define TS_I2Cx_SCL_AF                      GPIO_AF4_I2C3
#define TS_I2Cx_SCL_GPIO_PORT               GPIOA
#define TS_I2Cx_SDA_PIN                     GPIO_PIN_8
#define TS_I2Cx_SDA_AF                      GPIO_AF4_I2C3
#define TS_I2Cx_SDA_GPIO_PORT               GPIOH

#define TS_I2Cx_EV_IRQn                     I2C3_EV_IRQn
#define TS_I2Cx_ER_IRQn                     I2C3_ER_IRQn
    
/**
  * @brief TouchScreen FT6206 Slave I2C address
  */
#define TS_I2C_ADDRESS                   ((uint16_t)0x70)

/**
  * @}
  */

/** @addtogroup STM32F723E_DISCOVERY_LOW_LEVEL_COM STM32F723E DISCOVERY Low Level COM
  * @{
  */
#define COMn                              ((uint8_t)1)

/**
 * @brief Definition for COM port1, connected to USART2
 */ 
#define DISCOVERY_COM1                          USART2
#define DISCOVERY_COM1_CLK_ENABLE()             __HAL_RCC_USART2_CLK_ENABLE()
#define DISCOVERY_COM1_CLK_DISABLE()            __HAL_RCC_USART2_CLK_DISABLE()

#define DISCOVERY_COM1_TX_PIN                   GPIO_PIN_3
#define DISCOVERY_COM1_TX_GPIO_PORT             GPIOA
#define DISCOVERY_COM1_TX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()   
#define DISCOVERY_COM1_TX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOA_CLK_DISABLE()  
#define DISCOVERY_COM1_TX_AF                    GPIO_AF7_USART2

#define DISCOVERY_COM1_RX_PIN                   GPIO_PIN_2
#define DISCOVERY_COM1_RX_GPIO_PORT             GPIOA
#define DISCOVERY_COM1_RX_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOA_CLK_ENABLE()   
#define DISCOVERY_COM1_RX_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOA_CLK_DISABLE()  
#define DISCOVERY_COM1_RX_AF                    GPIO_AF7_USART2

#define DISCOVERY_COM1_IRQn                     USART2_IRQn

#define DISCOVERY_COMx_CLK_ENABLE(__INDEX__)            do { if((__INDEX__) == COM1) {DISCOVERY_COM1_CLK_ENABLE();} } while(0)
#define DISCOVERY_COMx_CLK_DISABLE(__INDEX__)           (((__INDEX__) == 0) ? DISCOVERY_COM1_CLK_DISABLE() : 0)

#define DISCOVERY_COMx_TX_GPIO_CLK_ENABLE(__INDEX__)    do { if((__INDEX__) == COM1) {DISCOVERY_COM1_TX_GPIO_CLK_ENABLE();} } while(0)
#define DISCOVERY_COMx_TX_GPIO_CLK_DISABLE(__INDEX__)   (((__INDEX__) == 0) ? DISCOVERY_COM1_TX_GPIO_CLK_DISABLE() : 0)

#define DISCOVERY_COMx_RX_GPIO_CLK_ENABLE(__INDEX__)    do { if((__INDEX__) == COM1) {DISCOVERY_COM1_RX_GPIO_CLK_ENABLE();} } while(0)
#define DISCOVERY_COMx_RX_GPIO_CLK_DISABLE(__INDEX__)   (((__INDEX__) == 0) ? DISCOVERY_COM1_RX_GPIO_CLK_DISABLE() : 0)


/**
  * @brief Audio I2C Slave address
  */
#define AUDIO_I2C_ADDRESS                ((uint16_t)0x34)

/**
  * @brief User can use this section to tailor I2C1 instance used and associated
  * resources (audio codec).
  * Definition for I2C1 clock resources
  */
#define DISCOVERY_AUDIO_I2Cx                             I2C1
#define DISCOVERY_AUDIO_I2Cx_CLK_ENABLE()                __HAL_RCC_I2C1_CLK_ENABLE()
#define DISCOVERY_AUDIO_I2Cx_SCL_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define DISCOVERY_AUDIO_I2Cx_SDA_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()

#define DISCOVERY_AUDIO_I2Cx_FORCE_RESET()               __HAL_RCC_I2C1_FORCE_RESET()
#define DISCOVERY_AUDIO_I2Cx_RELEASE_RESET()             __HAL_RCC_I2C1_RELEASE_RESET()

/** @brief Definition for I2C1 Pins
  */
#define DISCOVERY_AUDIO_I2Cx_SCL_PIN                     GPIO_PIN_8 /*!< PB8 */
#define DISCOVERY_AUDIO_I2Cx_SCL_AF                      GPIO_AF4_I2C1
#define DISCOVERY_AUDIO_I2Cx_SCL_GPIO_PORT               GPIOB
#define DISCOVERY_AUDIO_I2Cx_SDA_PIN                     GPIO_PIN_9 /*!< PB9 */
#define DISCOVERY_AUDIO_I2Cx_SDA_AF                      GPIO_AF4_I2C1
#define DISCOVERY_AUDIO_I2Cx_SDA_GPIO_PORT               GPIOB
/** @brief Definition of I2C1 interrupt requests
  */
#define DISCOVERY_AUDIO_I2Cx_EV_IRQn                     I2C1_EV_IRQn
#define DISCOVERY_AUDIO_I2Cx_ER_IRQn                     I2C1_ER_IRQn

                                               
/* Definition for external, camera and Arduino connector I2Cx resources */
#define DISCOVERY_EXT_I2Cx                               I2C2
#define DISCOVERY_EXT_I2Cx_CLK_ENABLE()                  __HAL_RCC_I2C2_CLK_ENABLE()
#define DISCOVERY_EXT_DMAx_CLK_ENABLE()                  __HAL_RCC_DMA1_CLK_ENABLE()
#define DISCOVERY_EXT_I2Cx_SCL_SDA_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOH_CLK_ENABLE()

#define DISCOVERY_EXT_I2Cx_FORCE_RESET()                 __HAL_RCC_I2C2_FORCE_RESET()
#define DISCOVERY_EXT_I2Cx_RELEASE_RESET()               __HAL_RCC_I2C2_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define DISCOVERY_EXT_I2Cx_SCL_PIN                       GPIO_PIN_4
#define DISCOVERY_EXT_I2Cx_SCL_SDA_GPIO_PORT             GPIOH
#define DISCOVERY_EXT_I2Cx_SCL_SDA_AF                    GPIO_AF4_I2C2
#define DISCOVERY_EXT_I2Cx_SDA_PIN                       GPIO_PIN_5

/* I2C interrupt requests */
#define DISCOVERY_EXT_I2Cx_EV_IRQn                       I2C2_EV_IRQn
#define DISCOVERY_EXT_I2Cx_ER_IRQn                       I2C2_ER_IRQn
                                               
                                               
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

/** @defgroup STM32F723E_DISCOVERY_LOW_LEVEL_Exported_Macros STM32F723E Discovery Low Level Exported Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup STM32F723E_DISCOVERY_LOW_LEVEL_Exported_Functions STM32F723E Discovery Low Level Exported Functions
  * @{
  */
uint32_t         BSP_GetVersion(void);
void             BSP_LED_Init(Led_TypeDef Led);
void             BSP_LED_DeInit(Led_TypeDef Led);
void             BSP_LED_On(Led_TypeDef Led);
void             BSP_LED_Off(Led_TypeDef Led);
void             BSP_LED_Toggle(Led_TypeDef Led);
void             BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode);
void             BSP_PB_DeInit(Button_TypeDef Button);
uint32_t         BSP_PB_GetState(Button_TypeDef Button);
void BSP_COM_Init(COM_TypeDef COM, UART_HandleTypeDef *huart);
void BSP_COM_DeInit(COM_TypeDef COM, UART_HandleTypeDef *huart);

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

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F723E_DISCOVERY_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
