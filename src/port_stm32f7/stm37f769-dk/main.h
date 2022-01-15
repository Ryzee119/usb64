/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_B0_Pin GPIO_PIN_4
#define LCD_B0_GPIO_Port GPIOE
#define OTG_HS_OverCurrent_Pin GPIO_PIN_4 //Updated.
#define OTG_HS_OverCurrent_GPIO_Port GPIOD //Updated.
#define QSPI_D2_Pin GPIO_PIN_2 //Checked.
#define QSPI_D2_GPIO_Port GPIOE //Checked.
#define RMII_TXD1_Pin GPIO_PIN_14 //Checked.
#define RMII_TXD1_GPIO_Port GPIOG //Checked.
#define FMC_NBL1_Pin GPIO_PIN_1 //Checked.
#define FMC_NBL1_GPIO_Port GPIOE //Checked.
#define FMC_NBL0_Pin GPIO_PIN_0 //Checked.
#define FMC_NBL0_GPIO_Port GPIOE //Checked.
#define ARDUINO_SCL_D15_Pin GPIO_PIN_8
#define ARDUINO_SCL_D15_GPIO_Port GPIOB
#define ARDUINO_D3_Pin GPIO_PIN_4
#define ARDUINO_D3_GPIO_Port GPIOB
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define SPDIF_RX0_Pin GPIO_PIN_11 //Updated.
#define SPDIF_RX0_GPIO_Port GPIOD //Checked.
#define SDMMC_CK_Pin GPIO_PIN_6 //Updated.
#define SDMMC_CK_GPIO_Port GPIOD //Updated.
#define ARDUINO_PWM_D9_Pin GPIO_PIN_15
#define ARDUINO_PWM_D9_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14 //Checked.
#define SWCLK_GPIO_Port GPIOA //Checked.
#define SWDIO_Pin GPIO_PIN_13 //Checked.
#define SWDIO_GPIO_Port GPIOA //Checked.
#define DCMI_D6_Pin GPIO_PIN_5
#define DCMI_D6_GPIO_Port GPIOE
#define DCMI_D7_Pin GPIO_PIN_6
#define DCMI_D7_GPIO_Port GPIOE
#define RMII_TXD0_Pin GPIO_PIN_13 //Checked.
#define RMII_TXD0_GPIO_Port GPIOG //Checked.
#define ARDUINO_SDA_D14_Pin GPIO_PIN_9
#define ARDUINO_SDA_D14_GPIO_Port GPIOB
#define VCP_RX_Pin GPIO_PIN_7
#define VCP_RX_GPIO_Port GPIOB
#define QSPI_NCS_Pin GPIO_PIN_6 //Checked.
#define QSPI_NCS_GPIO_Port GPIOB //Checked.
#define FMC_SDNCAS_Pin GPIO_PIN_15 //Checked.
#define FMC_SDNCAS_GPIO_Port GPIOG //Checked.
#define RMII_TX_EN_Pin GPIO_PIN_11 //Checked.
#define RMII_TX_EN_GPIO_Port GPIOG //Checked.
#define LCD_B1_Pin GPIO_PIN_13
#define LCD_B1_GPIO_Port GPIOJ
// #define OTG_FS_VBUS_Pin GPIO_PIN_12
// #define OTG_FS_VBUS_GPIO_Port GPIOJ
#define FMC_D2_Pin GPIO_PIN_0 //Checked.
#define FMC_D2_GPIO_Port GPIOD //Checked.
#define SDMMC_D3_Pin GPIO_PIN_4 //Updated.
#define SDMMC_D3_GPIO_Port GPIOB //Updated.
#define SDMMC_D2_Pin GPIO_PIN_3 //Updated.
#define SDMMC_D2_GPIO_Port GPIOB //Updated.
// #define OTG_FS_P_Pin GPIO_PIN_12
// #define OTG_FS_P_GPIO_Port GPIOA
#define NC1_Pin GPIO_PIN_8
#define NC1_GPIO_Port GPIOI
#define SAI1_MCLKA_Pin GPIO_PIN_7 //Updated.
#define SAI1_MCLKA_GPIO_Port GPIOG //Updated.
#define LCD_DE_Pin GPIO_PIN_7
#define LCD_DE_GPIO_Port GPIOK
#define LCD_B7_Pin GPIO_PIN_6
#define LCD_B7_GPIO_Port GPIOK
#define LCD_B6_Pin GPIO_PIN_5
#define LCD_B6_GPIO_Port GPIOK
#define LCD_B4_Pin GPIO_PIN_12
#define LCD_B4_GPIO_Port GPIOG
#define SAI1_SDB_Pin GPIO_PIN_3 //Updated.
#define SAI1_SDB_GPIO_Port GPIOE//Updated.
#define LCD_B2_Pin GPIO_PIN_14
#define LCD_B2_GPIO_Port GPIOJ
// #define OTG_FS_PowerSwitchOn_Pin GPIO_PIN_5
// #define OTG_FS_PowerSwitchOn_GPIO_Port GPIOD
#define DCMI_D5_Pin GPIO_PIN_3
#define DCMI_D5_GPIO_Port GPIOD
#define FMC_D3_Pin GPIO_PIN_1 //Checked.
#define FMC_D3_GPIO_Port GPIOD //Checked.
#define ARDUINO_D7_Pin GPIO_PIN_3
#define ARDUINO_D7_GPIO_Port GPIOI
#define ARDUINO_D7_EXTI_IRQn EXTI3_IRQn
#define ARDUINO_D8_Pin GPIO_PIN_2
#define ARDUINO_D8_GPIO_Port GPIOI
#define ARDUINO_D8_EXTI_IRQn EXTI2_IRQn
// #define OTG_FS_N_Pin GPIO_PIN_11
// #define OTG_FS_N_GPIO_Port GPIOA
#define uSD_Detect_Pin GPIO_PIN_15 //Updated.
#define uSD_Detect_GPIO_Port GPIOI //Updated.
#define FMC_A0_Pin GPIO_PIN_0 //Checked.
#define FMC_A0_GPIO_Port GPIOF //Checked.
#define SAI1_SCKA_Pin GPIO_PIN_5 //Updated.
#define SAI1_SCKA_GPIO_Port GPIOE //Updated.
#define SAI1_FSA_Pin GPIO_PIN_4 //Updated.
#define SAI1_FSA_GPIO_Port GPIOE //Updated.
#define LCD_HSYNC_Pin GPIO_PIN_10
#define LCD_HSYNC_GPIO_Port GPIOI
#define SAI1_SDA_Pin GPIO_PIN_6 //Updated.
#define SAI1_SDA_GPIO_Port GPIOE //Updated.
#define LCD_B5_Pin GPIO_PIN_4
#define LCD_B5_GPIO_Port GPIOK
#define LCD_BL_CTRL_Pin GPIO_PIN_3
#define LCD_BL_CTRL_GPIO_Port GPIOK
#define DCMI_VSYNC_Pin GPIO_PIN_9
#define DCMI_VSYNC_GPIO_Port GPIOG
#define LCD_B3_Pin GPIO_PIN_15
#define LCD_B3_GPIO_Port GPIOJ
// #define OTG_FS_OverCurrent_Pin GPIO_PIN_4
// #define OTG_FS_OverCurrent_GPIO_Port GPIOD
#define SDMMC_CMD_Pin GPIO_PIN_7 //Updated.
#define SDMMC_CMD_GPIO_Port GPIOD //Checked.
#define TP3_Pin GPIO_PIN_15
#define TP3_GPIO_Port GPIOH
#define ARDUINO_SCK_D13_Pin GPIO_PIN_1
#define ARDUINO_SCK_D13_GPIO_Port GPIOI
// #define OTG_FS_ID_Pin GPIO_PIN_10
// #define OTG_FS_ID_GPIO_Port GPIOA
#define RCC_OSC32_IN_Pin GPIO_PIN_14
#define RCC_OSC32_IN_GPIO_Port GPIOC
#define FMC_A1_Pin GPIO_PIN_1 //Checked.
#define FMC_A1_GPIO_Port GPIOF //Checked.
#define LCD_DISP_Pin GPIO_PIN_12
#define LCD_DISP_GPIO_Port GPIOI
#define LCD_VSYNC_Pin GPIO_PIN_9
#define LCD_VSYNC_GPIO_Port GPIOI
#define DCMI_PWR_EN_Pin GPIO_PIN_13
#define DCMI_PWR_EN_GPIO_Port GPIOH
#define DCMI_D4_Pin GPIO_PIN_14
#define DCMI_D4_GPIO_Port GPIOH
#define ARDUINO_PWM_CS_D5_Pin GPIO_PIN_0
#define ARDUINO_PWM_CS_D5_GPIO_Port GPIOI
#define VCP_TX_Pin GPIO_PIN_9
#define VCP_TX_GPIO_Port GPIOA
#define RCC_OSC32_OUT_Pin GPIO_PIN_15
#define RCC_OSC32_OUT_GPIO_Port GPIOC
#define LCD_G6_Pin GPIO_PIN_1
#define LCD_G6_GPIO_Port GPIOK
#define LCD_G7_Pin GPIO_PIN_2
#define LCD_G7_GPIO_Port GPIOK
#define ARDUINO_PWM_D10_Pin GPIO_PIN_8
#define ARDUINO_PWM_D10_GPIO_Port GPIOA
#define OSC_25M_Pin GPIO_PIN_0  //Checked. - IN
#define OSC_25M_GPIO_Port GPIOH //Checked. - IN
#define FMC_A2_Pin GPIO_PIN_2 //Checked.
#define FMC_A2_GPIO_Port GPIOF //Checked.
#define LCD_INT_Pin GPIO_PIN_13
#define LCD_INT_GPIO_Port GPIOI
#define LCD_R0_Pin GPIO_PIN_15
#define LCD_R0_GPIO_Port GPIOI
#define LCD_G4_Pin GPIO_PIN_11
#define LCD_G4_GPIO_Port GPIOJ
#define LCD_G5_Pin GPIO_PIN_0
#define LCD_G5_GPIO_Port GPIOK
#define ARDUINO_RX_D0_Pin GPIO_PIN_7
#define ARDUINO_RX_D0_GPIO_Port GPIOC
#define FMC_A3_Pin GPIO_PIN_3 //Checked.
#define FMC_A3_GPIO_Port GPIOF //Checked.
#define LCD_CLK_Pin GPIO_PIN_14
#define LCD_CLK_GPIO_Port GPIOI
#define LCD_G1_Pin GPIO_PIN_8
#define LCD_G1_GPIO_Port GPIOJ
#define LCD_G3_Pin GPIO_PIN_10
#define LCD_G3_GPIO_Port GPIOJ
#define FMC_SDCLK_Pin GPIO_PIN_8 //Checked.
#define FMC_SDCLK_GPIO_Port GPIOG //Checked.
#define ARDUINO_TX_D1_Pin GPIO_PIN_6
#define ARDUINO_TX_D1_GPIO_Port GPIOC
#define FMC_A4_Pin GPIO_PIN_4 //Checked.
#define FMC_A4_GPIO_Port GPIOF //Checked.
#define FMC_SDNME_Pin GPIO_PIN_5 //Checked.
#define FMC_SDNME_GPIO_Port GPIOH //Checked.
#define FMC_SDNE0_Pin GPIO_PIN_3 //Checked.
#define FMC_SDNE0_GPIO_Port GPIOH //Checked.
#define LCD_G0_Pin GPIO_PIN_7
#define LCD_G0_GPIO_Port GPIOJ
#define LCD_G2_Pin GPIO_PIN_9
#define LCD_G2_GPIO_Port GPIOJ
#define ARDUINO_D4_Pin GPIO_PIN_7
#define ARDUINO_D4_GPIO_Port GPIOG
#define ARDUINO_D4_EXTI_IRQn EXTI9_5_IRQn
#define ARDUINO_D2_Pin GPIO_PIN_6
#define ARDUINO_D2_GPIO_Port GPIOG
#define ARDUINO_D2_EXTI_IRQn EXTI9_5_IRQn
#define ARDUINO_A4_Pin GPIO_PIN_7
#define ARDUINO_A4_GPIO_Port GPIOF
#define ARDUINO_A5_Pin GPIO_PIN_6
#define ARDUINO_A5_GPIO_Port GPIOF
#define FMC_A5_Pin GPIO_PIN_5 //Checked.
#define FMC_A5_GPIO_Port GPIOF //Checked.
#define NC2_Pin GPIO_PIN_2
#define NC2_GPIO_Port GPIOH
#define LCD_R7_Pin GPIO_PIN_6
#define LCD_R7_GPIO_Port GPIOJ
#define FMC_D1_Pin GPIO_PIN_15 //Checked.
#define FMC_D1_GPIO_Port GPIOD //Checked.
#define FMC_D15_Pin GPIO_PIN_10 //Checked.
#define FMC_D15_GPIO_Port GPIOD //Checked.
#define ARDUINO_A1_Pin GPIO_PIN_10
#define ARDUINO_A1_GPIO_Port GPIOF
#define ARDUINO_A2_Pin GPIO_PIN_9
#define ARDUINO_A2_GPIO_Port GPIOF
#define ARDUINO_A3_Pin GPIO_PIN_8
#define ARDUINO_A3_GPIO_Port GPIOF
#define FMC_SDCKE0_Pin GPIO_PIN_2 //Updated.
#define FMC_SDCKE0_GPIO_Port GPIOH //Updated.
#define FMC_D0_Pin GPIO_PIN_14 //Checked.
#define FMC_D0_GPIO_Port GPIOD //Checked.
#define FMC_D14_Pin GPIO_PIN_9 //Checked.
#define FMC_D14_GPIO_Port GPIOD //Checked.
#define FMC_D13_Pin GPIO_PIN_8 //Checked.
#define FMC_D13_GPIO_Port GPIOD //Checked.
#define RMII_MDC_Pin GPIO_PIN_1 //Checked.
#define RMII_MDC_GPIO_Port GPIOC //Checked.
#define FMC_A6_Pin GPIO_PIN_12 //Checked.
#define FMC_A6_GPIO_Port GPIOF //Checked.
#define FMC_A11_Pin GPIO_PIN_1 //Checked.
#define FMC_A11_GPIO_Port GPIOG //Checked.
#define FMC_A9_Pin GPIO_PIN_15 //Checked.
#define FMC_A9_GPIO_Port GPIOF //Checked.
#define LCD_R5_Pin GPIO_PIN_4
#define LCD_R5_GPIO_Port GPIOJ
#define QSPI_D1_Pin GPIO_PIN_10 //Updated.
#define QSPI_D1_GPIO_Port GPIOC //Updated.
#define QSPI_D3_Pin GPIO_PIN_13 //Checked.
#define QSPI_D3_GPIO_Port GPIOD //Checked.
#define EXT_RST_Pin GPIO_PIN_7 //Updated.
#define EXT_RST_GPIO_Port GPIOH //Updated.
#define RMII_RXER_Pin GPIO_PIN_5 //Updated.
#define RMII_RXER_GPIO_Port GPIOD //Updated.
#define LCD_R6_Pin GPIO_PIN_5
#define LCD_R6_GPIO_Port GPIOJ
#define DCMI_D3_Pin GPIO_PIN_12
#define DCMI_D3_GPIO_Port GPIOH
#define RMII_REF_CLK_Pin GPIO_PIN_1 //Checked.
#define RMII_REF_CLK_GPIO_Port GPIOA //Checked.
#define ARDUINO_A0_Pin GPIO_PIN_0
#define ARDUINO_A0_GPIO_Port GPIOA
#define DCMI_HSYNC_Pin GPIO_PIN_4
#define DCMI_HSYNC_GPIO_Port GPIOA
#define RMII_RXD0_Pin GPIO_PIN_4 //Checked.
#define RMII_RXD0_GPIO_Port GPIOC //Checked.
#define FMC_A7_Pin GPIO_PIN_13 //Checked.
#define FMC_A7_GPIO_Port GPIOF //Checked.
#define FMC_A10_Pin GPIO_PIN_0 //Checked.
#define FMC_A10_GPIO_Port GPIOG //Checked.
#define LCD_R4_Pin GPIO_PIN_3
#define LCD_R4_GPIO_Port GPIOJ
#define FMC_D5_Pin GPIO_PIN_8 //Checked.
#define FMC_D5_GPIO_Port GPIOE //Checked.
#define QSPI_D0_Pin GPIO_PIN_9 //Updated.
#define QSPI_D0_GPIO_Port GPIOC //Updated.
#define FMC_BA1_Pin GPIO_PIN_5 //Checked.
#define FMC_BA1_GPIO_Port GPIOG //Checked.
#define FMC_BA0_Pin GPIO_PIN_4 //Checked.
#define FMC_BA0_GPIO_Port GPIOG //Checked.
#define LCD_SCL_Pin GPIO_PIN_7
#define LCD_SCL_GPIO_Port GPIOH
#define DCMI_D0_Pin GPIO_PIN_9
#define DCMI_D0_GPIO_Port GPIOH
#define DCMI_D2_Pin GPIO_PIN_11
#define DCMI_D2_GPIO_Port GPIOH
#define RMII_MDIO_Pin GPIO_PIN_2 //Checked.
#define RMII_MDIO_GPIO_Port GPIOA //Checked.
#define RMII_RXD1_Pin GPIO_PIN_5 //Checked.
#define RMII_RXD1_GPIO_Port GPIOC //Checked.
#define FMC_A8_Pin GPIO_PIN_14 //Checked.
#define FMC_A8_GPIO_Port GPIOF //Checked.
#define LCD_R3_Pin GPIO_PIN_2
#define LCD_R3_GPIO_Port GPIOJ
#define FMC_SDNRAS_Pin GPIO_PIN_11 //Checked.
#define FMC_SDNRAS_GPIO_Port GPIOF //Checked.
#define FMC_D6_Pin GPIO_PIN_9 //Checked.
#define FMC_D6_GPIO_Port GPIOE //Checked.
#define FMC_D8_Pin GPIO_PIN_11 //Checked.
#define FMC_D8_GPIO_Port GPIOE //Checked.
#define FMC_D11_Pin GPIO_PIN_14 //Checked.
#define FMC_D11_GPIO_Port GPIOE //Checked.
#define ULPI_D3_Pin GPIO_PIN_10 //Checked.
#define ULPI_D3_GPIO_Port GPIOB //Checked.
#define ARDUINO_PWM_D6_Pin GPIO_PIN_6
#define ARDUINO_PWM_D6_GPIO_Port GPIOH
#define LCD_SDA_Pin GPIO_PIN_8
#define LCD_SDA_GPIO_Port GPIOH
#define DCMI_D1_Pin GPIO_PIN_10
#define DCMI_D1_GPIO_Port GPIOH
#define RMII_CRS_DV_Pin GPIO_PIN_7 //Checked.
#define RMII_CRS_DV_GPIO_Port GPIOA //Checked.
#define LCD_R1_Pin GPIO_PIN_0
#define LCD_R1_GPIO_Port GPIOJ
#define LCD_R2_Pin GPIO_PIN_1
#define LCD_R2_GPIO_Port GPIOJ
#define FMC_D4_Pin GPIO_PIN_7 //Checked.
#define FMC_D4_GPIO_Port GPIOE //Checked.
#define FMC_D7_Pin GPIO_PIN_10 //Checked.
#define FMC_D7_GPIO_Port GPIOE //Checked.
#define FMC_D9_Pin GPIO_PIN_12 //Checked.
#define FMC_D9_GPIO_Port GPIOE //Checked.
#define FMC_D12_Pin GPIO_PIN_15 //Checked.
#define FMC_D12_GPIO_Port GPIOE //Checked.
#define FMC_D10_Pin GPIO_PIN_13 //Checked.
#define FMC_D10_GPIO_Port GPIOE //Checked.
#define ARDUINO_MISO_D12_Pin GPIO_PIN_14
#define ARDUINO_MISO_D12_GPIO_Port GPIOB
#define ARDUINO_MOSI_PWM_D11_Pin GPIO_PIN_15
#define ARDUINO_MOSI_PWM_D11_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
