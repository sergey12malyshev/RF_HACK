/* USER CODE BEGIN Header */
/*
 * Демонстрация работы с сенсорным экраном (тачскрином) на базе контроллера XPT2046 (HR2046 и т.п.)
 * и дисплеем на базе контроллера ILI9341 (spi)
 *
 * Copyright (C) 2022, VadRov, all right reserved.
 *
 * https://www.youtube.com/@VadRov
 * https://dzen.ru/vadrov
 * https://vk.com/vadrov
 * https://t.me/vadrov_channel
 *
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "display.h"
#include "ili9341.h"
#include "xpt2046.h"
#include "calibrate_touch.h"
#include "demo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint32_t millis = 0;
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
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* Для тех, кто не умеет пользоваться отладчиком или
 * тех, у кого он не работает */
/*
static void convert64bit_to_hex(uint8_t *v, char *b)
{
   b[0] = 0;
  sprintf(&b[strlen(b)], "0x");
   for (int i = 0; i < 8; i++) {
     sprintf(&b[strlen(b)], "%02x", v[7 - i]);
   }
}
*/

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
/* Включаем кэширование инструкций */
#if (INSTRUCTION_CACHE_ENABLE != 0U)
  ((FLASH_TypeDef *) ((0x40000000UL + 0x00020000UL) + 0x3C00UL))->ACR |= (0x1UL << (9U));
#endif

/* Включаем кэширование данных */
#if (DATA_CACHE_ENABLE != 0U)
  ((FLASH_TypeDef *) ((0x40000000UL + 0x00020000UL) + 0x3C00UL))->ACR |= (0x1UL << (10U));
#endif

/* Включаем систему предварительной выборки инструкций */
#if (PREFETCH_ENABLE != 0U)
  ((FLASH_TypeDef *) ((0x40000000UL + 0x00020000UL) + 0x3C00UL))->ACR |= (0x1UL << (8U));
#endif
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* System interrupt init*/
  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  //Настраиваем системный таймер (прерывания 1000 раз в секунду)
  SysTick_Config(SystemCoreClock/1000);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  /* -------------------------------------- Настройка дисплея ------------------------------------------*/
  //Данные DMA
  LCD_DMA_TypeDef dma_tx = { .dma    = DMA2,        //Контроллер DMA
                         .stream = LL_DMA_STREAM_3 };  //Поток контроллера DMA

  //Данные подсветки
  LCD_BackLight_data bkl_data = { .htim_bk        = TIM3,         //Таймер - для подсветки с PWM (изменение яркости подсветки)
                              .channel_htim_bk = LL_TIM_CHANNEL_CH1, //Канал таймера - для подсветки с PWM (изменение яркости подсветки)
                  .blk_port      = 0,           //Порт gpio - подсветка по типу вкл./выкл.
                  .blk_pin        = 0,           //Вывод порта - подсветка по типу вкл./выкл.
                  .bk_percent     = 60  };         //Яркость подсветки, в %

  //Данные подключения
  LCD_SPI_Connected_data spi_con = { .spi      = SPI1,        //�?спользуемый spi
                                 .dma_tx    = dma_tx,        //Данные DMA
                   .reset_port = LCD_RESET_GPIO_Port,  //Порт вывода RES
                   .reset_pin  = LCD_RESET_Pin,    //Пин вывода RES
                   .dc_port    = LCD_DC_GPIO_Port,  //Порт вывода DC
                   .dc_pin    = LCD_DC_Pin,      //Пин вывода DC
                   .cs_port   = LCD_CS_GPIO_Port,  //Порт вывода CS
                   .cs_pin   = LCD_CS_Pin         };//Пин вывода CS

#ifndef  LCD_DYNAMIC_MEM
  LCD_Handler lcd1;
#endif
   //создаем обработчик дисплея ILI9341
   LCD = LCD_DisplayAdd( LCD,
#ifndef  LCD_DYNAMIC_MEM
                         &lcd1,
#endif
                         240,
             320,
             ILI9341_CONTROLLER_WIDTH,
             ILI9341_CONTROLLER_HEIGHT,
             //Задаем смещение по ширине и высоте для нестандартных или бракованных дисплеев:
             0,    //смещение по ширине дисплейной матрицы
             0,    //смещение по высоте дисплейной матрицы
             PAGE_ORIENTATION_PORTRAIT_MIRROR,
             ILI9341_Init,
             ILI9341_SetWindow,
             ILI9341_SleepIn,
             ILI9341_SleepOut,
             &spi_con,
             LCD_DATA_16BIT_BUS,
             bkl_data           );

  LCD_Handler *lcd = LCD;     //Указатель на первый дисплей в списке
  LCD_Init(lcd);         //�?нициализация дисплея
  LCD_Fill(lcd, COLOR_RED);    //Заливка дисплея
  /*---------------------------------------------------------------------------------------------------*/

  /* ----------------------------------- Настройка тачскрина ------------------------------------------*/
  //Будем обмениваться данными с XPT2046 на скорости 2.625 Мбит/с (по спецификации максимум 2.0 Мбит/с).
  XPT2046_ConnectionData cnt_touch = { .spi    = SPI1,      //�?спользуемый spi
                                   .speed    = 4,        //Скорость spi 0...7 (0 - clk/2, 1 - clk/4, ..., 7 - clk/256)
                     .cs_port  = T_CS_GPIO_Port,  //Порт для управления T_CS
                     .cs_pin    = T_CS_Pin,    //Вывод порта для управления T_CS
                     .irq_port = T_IRQ_GPIO_Port,  //Порт для управления T_IRQ
                     .irq_pin  = T_IRQ_Pin,    //Вывод порта для управления T_IRQ
                     .exti_irq = T_IRQ_EXTI_IRQn  //Канал внешнего прерывания
                                     };
  //�?нициализация обработчика XPT2046
  XPT2046_Handler touch1;
  XPT2046_InitTouch(&touch1, 20, &cnt_touch);

/* Самый простой вариант хранения в программе
 * коэффициентов калибровки. Строку XPT2046_CalibrateTouch
 * надо будет закомментировать */
#define CALIBRATE_EN    false
#if !CALIBRATE_EN
  tCoef coef = {.D   = 0x00022b4253626d37,
                .Dx1 = 0xffffd9e9e85d81b6,
                .Dx2 = 0x0000005a555c98ab,
                .Dx3 = 0x022dd7f0419e66b7,
                .Dy1 = 0xffffff6065e10c98,
                .Dy2 = 0x0000343b820dc8bf,
                .Dy3 = 0xff9cc25725238e55 };
  touch1.coef = coef;
#else
  /* Калибровка тачскрина
   * Если коэффициенты калибровки выше определены, то эту строку надо
   * закомментировать */
  XPT2046_CalibrateTouch(&touch1, lcd); //Запускаем процедуру калибровки
#endif
  /* Вывод на дисплей 64 битных коэффициентов калибровки для тех,
   * кто не умеет пользоваться отладчиком или для тех,
   * у кого он не работает. До использования не забудьте раскомментировать
   * функцию convert64bit_to_hex */
  /*
  char b[100];
  convert64bit_to_hex((uint8_t*)(&touch1.coef.D), b);
  LCD_WriteString(lcd, 0, 0, b, &Font_12x20, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
  convert64bit_to_hex((uint8_t*)(&touch1.coef.Dx1), b);
  LCD_WriteString(lcd, 0, 20, b, &Font_12x20, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
  convert64bit_to_hex((uint8_t*)(&touch1.coef.Dx2), b);
  LCD_WriteString(lcd, 0, 40, b, &Font_12x20, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
  convert64bit_to_hex((uint8_t*)(&touch1.coef.Dx3), b);
  LCD_WriteString(lcd, 0, 60, b, &Font_12x20, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
  convert64bit_to_hex((uint8_t*)(&touch1.coef.Dy1), b);
  LCD_WriteString(lcd, 0, 80, b, &Font_12x20, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
  convert64bit_to_hex((uint8_t*)(&touch1.coef.Dy2), b);
  LCD_WriteString(lcd, 0, 100, b, &Font_12x20, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
  convert64bit_to_hex((uint8_t*)(&touch1.coef.Dy3), b);
  LCD_WriteString(lcd, 0, 120, b, &Font_12x20, COLOR_YELLOW, COLOR_BLUE, LCD_SYMBOL_PRINT_FAST);
while(1) { }
  //После того, как перенесете параметры в coef это все "дело" закомментируйте
*/

  /* --------------------------------------------------------------------------------------------------*/

  //----------------------------------------- Запуск демок --------------------------------------------*/
  LCD_Fill(lcd, COLOR_WHITE); //Закрашиваем экран белым цветом
  LCD_Fill(lcd, COLOR_BLACK); 
      LCD_WriteString(lcd, 0, 0, "Test Start",
            &Font_12x20, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  LL_mDelay(5000);
  //Демка для рисования на экране с помощью тачскрина.
  Draw_TouchPenDemo(&touch1, lcd);

  //Демка рисует примитивы, отображает температуру и позволяет перемещать круг по дисплею.
  //При удержании касания окрашивает дисплей случайным цветом.
  RoadCircleDemo(&touch1, lcd);

  /* --------------------------------------------------------------------------------------------------*/

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_2)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_HSE_EnableCSS();
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_25, 168, LL_RCC_PLLP_DIV_2);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_Init1msTick(84000000);
  LL_SetSystemCoreClock(84000000);
  LL_RCC_SetTIMPrescaler(LL_RCC_TIM_PRESCALER_TWICE);
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  LL_SPI_InitTypeDef SPI_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**SPI1 GPIO Configuration
  PA5   ------> SPI1_SCK
  PA7   ------> SPI1_MOSI
  PB4   ------> SPI1_MISO
  */
  GPIO_InitStruct.Pin = LCD_SCK_Pin|LCD_SDI_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = T_OUT_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
  LL_GPIO_Init(T_OUT_GPIO_Port, &GPIO_InitStruct);

  /* SPI1 DMA Init */

  /* SPI1_TX Init */
  LL_DMA_SetChannelSelection(DMA2, LL_DMA_STREAM_3, LL_DMA_CHANNEL_3);

  LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_3, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_3, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA2, LL_DMA_STREAM_3, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_3, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_3, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_3, LL_DMA_PDATAALIGN_HALFWORD);

  LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_3, LL_DMA_MDATAALIGN_HALFWORD);

  LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_3);

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_16BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2;
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 10;
  LL_SPI_Init(SPI1, &SPI_InitStruct);
  LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  TIM_InitStruct.Prescaler = 999;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 209;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM3, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM3);
  LL_TIM_OC_EnablePreload(TIM3, LL_TIM_CHANNEL_CH1);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 104;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM3, LL_TIM_CHANNEL_CH1);
  LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM3);
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**TIM3 GPIO Configuration
  PA6   ------> TIM3_CH1
  */
  GPIO_InitStruct.Pin = LCD_LED_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(LCD_LED_GPIO_Port, &GPIO_InitStruct);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* Init with LL driver */
  /* DMA controller clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

  /* DMA interrupt init */
  /* DMA2_Stream3_IRQn interrupt configuration */
  NVIC_SetPriority(DMA2_Stream3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),3, 0));
  NVIC_EnableIRQ(DMA2_Stream3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

  /**/
  LL_GPIO_SetOutputPin(GPIOA, LCD_DC_Pin|LCD_RESET_Pin|LCD_CS_Pin);

  /**/
  LL_GPIO_SetOutputPin(T_CS_GPIO_Port, T_CS_Pin);

  /**/
  GPIO_InitStruct.Pin = LCD_DC_Pin|LCD_RESET_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LCD_CS_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(LCD_CS_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = T_CS_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(T_CS_GPIO_Port, &GPIO_InitStruct);

  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE0);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_0;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  LL_GPIO_SetPinPull(T_IRQ_GPIO_Port, T_IRQ_Pin, LL_GPIO_PULL_UP);

  /**/
  LL_GPIO_SetPinMode(T_IRQ_GPIO_Port, T_IRQ_Pin, LL_GPIO_MODE_INPUT);

  /* EXTI interrupt init*/
  NVIC_SetPriority(EXTI0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),4, 0));
  NVIC_EnableIRQ(EXTI0_IRQn);

}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
