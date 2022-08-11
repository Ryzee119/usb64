// Copyright 2020, Ryan Wendland, usb64
// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include "common.h"
#include "memory.h"
#include "fileio.h"
#include "memory.h"

UART_HandleTypeDef huart;
CRC_HandleTypeDef hcrc;

static void SystemClock_Config(void);

//usb64 passes a pin number to the backend (See port_conf.h). We need to convert this usb64 pin number to a
//port and device pin number.
typedef struct
{
    uint16_t pin;
    GPIO_TypeDef *port;
} dev_gpio_t;

static dev_gpio_t _dev_gpio[USB64_PIN_MAX];
static dev_gpio_t *n64hal_pin_to_gpio(usb64_pin_t pin)
{
    if (pin == -1 || pin >= USB64_PIN_MAX)
    {
        return NULL;
    }
    return &_dev_gpio[pin];
}

//n64 controller interrupt handles
void (*n64_1)(void) = NULL;
void (*n64_2)(void) = NULL;
void (*n64_3)(void) = NULL;
void (*n64_4)(void) = NULL;

void n64hal_system_init()
{
    SCB_EnableICache();
    SCB_EnableDCache();

    HAL_Init();
    SystemClock_Config();

    __disable_irq();
    //Move the interrupt vector table from Flash to RAM. Should have better interrupt perf and consistency
    void *vtor = (void *)RAMDTCM_BASE;
    memcpy(vtor, (void *)SCB->VTOR, 0x200);
    SCB->VTOR = (uint32_t)vtor;
    __enable_irq();

    __HAL_RCC_CRC_CLK_ENABLE();
    hcrc.Instance = CRC;
    hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
    hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;
    hcrc.Init.GeneratingPolynomial = 0x85;
    hcrc.Init.CRCLength = CRC_POLYLENGTH_8B;
    hcrc.Init.InitValue = 0;
    hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
    hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
    HAL_CRC_Init(&hcrc);
}

void n64hal_debug_init()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    huart.Instance = USART1;
    huart.Init.BaudRate = 115200;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    HAL_UART_Init(&huart);
}

void n64hal_gpio_init()
{
    dev_gpio_t *pin;
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOJ_CLK_ENABLE();

    for (int i = 0; i < USB64_PIN_MAX; i++)
    {
        switch (i)
        {
        case N64_CONSOLE_SENSE_PIN:
            _dev_gpio[i].pin = GPIO_PIN_6; //D3 (PF6)
            _dev_gpio[i].port = GPIOF;
            break;
        case N64_CONTROLLER_1_PIN:
            _dev_gpio[i].pin = GPIO_PIN_1; //D2 (PJ1)
            _dev_gpio[i].port = GPIOJ;
            break;
        case N64_CONTROLLER_2_PIN:
            _dev_gpio[i].pin = GPIO_PIN_0; //D4 (PJ0)
            _dev_gpio[i].port = GPIOJ;
            break;
        case N64_CONTROLLER_3_PIN:
            _dev_gpio[i].pin = GPIO_PIN_3; //D7 (PJ3)
            _dev_gpio[i].port = GPIOJ;
            break;
        case N64_CONTROLLER_4_PIN:
            _dev_gpio[i].pin = GPIO_PIN_4; //D8 (PJ4)
            _dev_gpio[i].port = GPIOJ;
            break;
        }
    }

    pin = n64hal_pin_to_gpio(N64_CONSOLE_SENSE_PIN);
    GPIO_InitStruct.Pin = pin->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(pin->port, &GPIO_InitStruct);

    pin = n64hal_pin_to_gpio(N64_CONTROLLER_1_PIN);
    HAL_GPIO_WritePin(pin->port, pin->pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = pin->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(pin->port, &GPIO_InitStruct);

    pin = n64hal_pin_to_gpio(N64_CONTROLLER_2_PIN);
    HAL_GPIO_WritePin(pin->port, pin->pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = pin->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(pin->port, &GPIO_InitStruct);

    pin = n64hal_pin_to_gpio(N64_CONTROLLER_3_PIN);
    HAL_GPIO_WritePin(pin->port, pin->pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = pin->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(pin->port, &GPIO_InitStruct);

    pin = n64hal_pin_to_gpio(N64_CONTROLLER_4_PIN);
    HAL_GPIO_WritePin(pin->port, pin->pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = pin->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(pin->port, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

    HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);

    HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);

    NVIC_SetPriority(SysTick_IRQn, 15);
}

void n64hal_debug_write(char c)
{
    HAL_UART_Transmit(&huart, (uint8_t *)&c, 1, 5000);
}

void n64hal_disable_interrupts()
{
    //Disable the controller input interrupts
    HAL_NVIC_DisableIRQ(EXTI0_IRQn);
    HAL_NVIC_DisableIRQ(EXTI1_IRQn);
    HAL_NVIC_DisableIRQ(EXTI3_IRQn);
    HAL_NVIC_DisableIRQ(EXTI4_IRQn);
    HAL_NVIC_DisableIRQ(OTG_HS_IRQn);
}

void n64hal_enable_interrupts()
{
    //Disable the controller input interrupts
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
}

void n64hal_attach_interrupt(usb64_pin_t pin, void (*handler)(void), int mode)
{
    (void)mode; //Should always be falling. Not used currently
    switch (pin)
    {
    case N64_CONTROLLER_1_PIN:
        n64_1 = handler;
        break;
    case N64_CONTROLLER_2_PIN:
        n64_2 = handler;
        break;
    case N64_CONTROLLER_3_PIN:
        n64_3 = handler;
        break;
    case N64_CONTROLLER_4_PIN:
        n64_4 = handler;
        break;
    default:
        break; //Not required
    }
}

void n64hal_detach_interrupt(usb64_pin_t pin)
{
    switch (pin)
    {
    case N64_CONTROLLER_1_PIN:
        n64_1 = NULL;
        break;
    case N64_CONTROLLER_2_PIN:
        n64_2 = NULL;
        break;
    case N64_CONTROLLER_3_PIN:
        n64_3 = NULL;
        break;
    case N64_CONTROLLER_4_PIN:
        n64_4 = NULL;
        break;
    default:
        break; //Not required
    }
}

void n64hal_rtc_read(uint8_t *day_high, uint8_t *day_low, uint8_t *h, uint8_t *m, uint8_t *s)
{
    //Not implemented
}

void n64hal_rtc_write(uint8_t *day_high, uint8_t *day_low, uint8_t *h, uint8_t *m, uint8_t *s)
{
    //Not implemented
}

void n64hal_hs_tick_init()
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->LAR = 0xC5ACCE55;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t n64hal_hs_tick_get_speed()
{
    return SystemCoreClock;
}

uint32_t n64hal_hs_tick_get()
{
    return DWT->CYCCNT;
}

uint32_t n64hal_millis()
{
    return HAL_GetTick();
}

void n64hal_input_swap(usb64_pin_t pin, uint8_t val)
{
    static uint16_t gpio_pin_offset[USB64_PIN_MAX];
    static uint8_t initialised[USB64_PIN_MAX] = {0};
    uint32_t mode = (val == N64_OUTPUT) ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;

    //Get the backend gpio from the pin number
    dev_gpio_t *gpio = n64hal_pin_to_gpio(pin);
    if (gpio == NULL)
    {
        return;
    }

    //Get the pin offset of the gpio. ie. GPIO6 = 6. This only happens on the first call
    //for efficiency
    if (initialised[pin] == 0)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            if (gpio->pin & (1UL << i))
                gpio_pin_offset[pin] = i;
        }
        initialised[pin] = 1;
    }
    //Change the pin mode from input/outputpp
    //When output, the line is driven low, when input is it floating (pulled up by resistor)
    uint32_t temp = gpio->port->MODER;
    uint32_t offset = gpio_pin_offset[pin];
    temp &= ~(GPIO_MODER_MODER0 << (offset * 2U));
    temp |= ((mode & GPIO_MODER_MODER0) << (offset * 2U));
    gpio->port->MODER = temp;
}

uint8_t n64hal_input_read(usb64_pin_t pin)
{
    dev_gpio_t *gpio = n64hal_pin_to_gpio(pin);
    if (gpio != NULL)
    {
        return HAL_GPIO_ReadPin(gpio->port, gpio->pin);
    }
    return 0;
}

void n64hal_output_set(usb64_pin_t pin, uint8_t level)
{
    dev_gpio_t *gpio = n64hal_pin_to_gpio(pin);
    if (gpio != NULL)
    {
        HAL_GPIO_WritePin(gpio->port, gpio->pin, (GPIO_PinState)level);
    }
    return;
}

void n64hal_read_extram(void *rx_buff, void *src, uint32_t offset, uint32_t len)
{
    //External ram is just memory mapped. memcpy is fine
    memcpy(rx_buff, (void *)((uintptr_t)src + offset), len);
}

void n64hal_write_extram(void *tx_buff, void *dst, uint32_t offset, uint32_t len)
{
    //External ram is just memory mapped. memcpy is fine
    memcpy((void *)((uintptr_t)dst + offset), tx_buff, len);
    memory_mark_dirty(dst);
}

void *n64hal_malloc(uint32_t len)
{
    return memory_dev_malloc(len);
}

void n64hal_free(void *addr)
{
    memory_dev_free(addr);
}

uint32_t n64hal_list_gb_roms(char **gb_list, uint32_t max)
{
    //Retrieve full directory list
    char *file_list[256];
    uint32_t num_files = fileio_list_directory(file_list, 256);

    //Find only files with .gb or gbc extensions to populate rom list.
    uint32_t rom_count = 0;
    for (uint32_t i = 0; i < num_files; i++)
    {
        if (file_list[i] == NULL)
            continue;

        if (strstr(file_list[i], ".GB\0") != NULL || strstr(file_list[i], ".GBC\0") != NULL ||
            strstr(file_list[i], ".gb\0") != NULL || strstr(file_list[i], ".gbc\0") != NULL)
        {
            if (rom_count < max)
            {
                gb_list[rom_count] = (char *)memory_dev_malloc(strlen(file_list[i]) + 1);
                strcpy(gb_list[rom_count], file_list[i]);
                rom_count++;
            }
        }
        //Free file list as we go
        memory_dev_free(file_list[i]);
    }
    return rom_count;
}

void n64hal_read_storage(char *name, uint32_t file_offset, uint8_t *data, uint32_t len)
{
    fileio_read_from_file(name, file_offset, data, len);
}

void Error_Handler(void)
{
    HAL_UART_Transmit(&huart, (uint8_t *)"Error_Handler\n", 14, 5000);
    __disable_irq();
    while (1)
    {
    }
}

//STM32 specifics

//STM32F7 has a powerful, configurable CRC unit. Use it instead of my software one which needs a fast CPU
extern "C" uint8_t n64_get_crc(uint8_t *data)
{
    return HAL_CRC_Calculate(&hcrc, (uint32_t *)data, 32);
}

extern "C" void EXTI4_IRQHandler(void)
{
    if (n64_4)
    {
        n64_4();
    }
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
    HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
}

extern "C" void EXTI3_IRQHandler(void)
{
    if (n64_3)
    {
        n64_3();
    }
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
    HAL_NVIC_ClearPendingIRQ(EXTI3_IRQn);
}

extern "C" void EXTI0_IRQHandler(void)
{
    if (n64_2)
    {
        n64_2();
    }
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
    HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
}

extern "C" void EXTI1_IRQHandler(void)
{
    if (n64_1)
    {
        n64_1();
    }
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
    HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Configure LSE Drive Capability
  */
    HAL_PWR_EnableBkUpAccess();
    /** Configure the main internal regulator output voltage
  */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 400;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    RCC_OscInitStruct.PLL.PLLR = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Activate the Over-Drive mode
  */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB buses clocks
  */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPDIFRX | RCC_PERIPHCLK_LTDC | 
                                               RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_USART1 | 
                                               RCC_PERIPHCLK_USART6 | RCC_PERIPHCLK_UART5 | 
                                               RCC_PERIPHCLK_SAI1 | RCC_PERIPHCLK_SAI2 | 
                                               RCC_PERIPHCLK_I2C1 | RCC_PERIPHCLK_I2C4 | 
                                               RCC_PERIPHCLK_SDMMC2 | RCC_PERIPHCLK_CLK48 | 
                                               RCC_PERIPHCLK_CEC;
    PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
    PeriphClkInitStruct.PLLI2S.PLLI2SP = RCC_PLLP_DIV2;
    PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
    PeriphClkInitStruct.PLLI2S.PLLI2SQ = 2;
    PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
    PeriphClkInitStruct.PLLSAI.PLLSAIR = 2;
    PeriphClkInitStruct.PLLSAI.PLLSAIQ = 3;
    PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV4;
    PeriphClkInitStruct.PLLI2SDivQ = 1;
    PeriphClkInitStruct.PLLSAIDivQ = 1;
    PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLLSAI;
    PeriphClkInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLSAI;
    PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInitStruct.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
    PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK2;
    PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_PCLK1;
    PeriphClkInitStruct.CecClockSelection = RCC_CECCLKSOURCE_HSI;
    PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
    PeriphClkInitStruct.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_CLK48;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
}
