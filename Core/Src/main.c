/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "key.h"
#include "middle_key.h"
#include "shell.h"
#include <stdio.h>
#include "middle_oled.h"
#include "OLED.h"
#include "ring.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
Key_Handle_t g_key1;
Key_Handle_t g_key2;

#define UART1_RX_BUFFER_SIZE 32
uint8_t g_uart1_rx_buffer[UART1_RX_BUFFER_SIZE];

// 定义一个大的循环缓冲区
#define UART1_RING_BUFFER_SIZE 256
uint8_t g_uart1_ring_buffer[UART1_RING_BUFFER_SIZE];
ring_t g_uart1_ring;

// 从缓冲区读出来的数据
uint8_t buf[UART1_RING_BUFFER_SIZE] = {0};

// 1.定义一个shell 的对象  实现必要的两个函数
SHELL_TypeDef shell;
void shellWriteChar(char c)
{
    // 使用您的串口发送函数，例如 HAL_UART_Transmit
    HAL_UART_Transmit(&huart1, (uint8_t *)&c, 1, HAL_MAX_DELAY);
}

signed char shellReadChar(char *ch)
{
    // 尝试从环形缓冲区读取一个字节
    if (ring_read(&g_uart1_ring, (uint8_t *)ch, 1) == 1)
    {
        return 0; // 成功
    }
    return -1; // 失败，缓冲区为空
}

// 2.两个示例函数
void hello(void)
{
    shellPrint(shellGetCurrent(), "Hello from STM32!\r\n");
}
SHELL_EXPORT_CMD(hello, hello, say hello);

void add(int a, int b)
{
    shellPrint(shellGetCurrent(), "%d + %d = %d\r\n", a, b, a + b);
}
// 导出命令
SHELL_EXPORT_CMD(add, add, add two numbers);

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_SPI1_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();
    /* USER CODE BEGIN 2 */
    BSP_Key_Init();
    BSP_OLED_Init();

    // 串口空闲接收
    printf("hello world");
    OLED_Printf(0, 0, OLED_8X16, "Hello, OLED!");
    OLED_Update();

    // 初始化环形缓冲区
    ring_init(&g_uart1_ring, g_uart1_ring_buffer, UART1_RING_BUFFER_SIZE);
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *)g_uart1_rx_buffer, UART1_RX_BUFFER_SIZE);

    shell.write = shellWriteChar;
    shell.read = shellReadChar;
    shellInit(&shell);

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        KeyEvent_t event1 = Key_Scan(&g_key1);
        KeyEvent_t event2 = Key_Scan(&g_key2);

        if (event1 != KEY_EVENT_NONE)
        {
            // 处理按键1事件
            switch (event1)
            {
            case KEY_EVENT_SHORT_PRESS:
                HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
                HAL_UART_Transmit(&huart1, (uint8_t *)"Key1 Short Press\r\n", 18, HAL_MAX_DELAY);
                break;

            default:
                break;
            }
        }

        if (event2 != KEY_EVENT_NONE)
        {
            // 处理按键2事件
            switch (event2)
            {
            case KEY_EVENT_SHORT_PRESS:
                HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
                HAL_UART_Transmit(&huart1, (uint8_t *)"Key2 Short Press\r\n", 18, HAL_MAX_DELAY);
                break;

            default:
                break;
            }
        }

        shellTask(&shell);
        
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */
// 串口中断回调函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1)
    {
        // 将数据推入环形缓冲区
        ring_write(&g_uart1_ring, g_uart1_rx_buffer, Size);
        memset(g_uart1_rx_buffer, 0, UART1_RX_BUFFER_SIZE); // 清空临时接收缓冲区

        // 重新启动空闲接收
        HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *)g_uart1_rx_buffer, UART1_RX_BUFFER_SIZE);
    }
}

int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
