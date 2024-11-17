/* USER CODE BEGIN Header */

/*
 * RF_HACK project 2024
 * Malyshev Sergey
 * 
 * https://github.com/sergey12malyshev/RF_HACK
 * 
 * GPL-3.0 license
 */

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "iwdg.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


#include "workStates.h"
#include "configFile.h"

#include "display.h"
#include "ili9341.h"
#include "xpt2046.h"
#include "calibrate_touch.h"
#include "demo.h"
#include "displayInit.h"

#include "button_Thread.h"
#include "gps_Thread.h"

#include "runBootloader.h"
#include "buzzer_driver.h"

#include "gps.h"
#include "cc1101.h"

#include "cli_driver.h"
#include "cli_thread.h"

#define LC_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "encoderDriver.h"
#include "application_Thread.h"
#include "button_Thread.h"
#include "subGHz_RX_Thread.h"
#include "subGHz_TX_Thread.h"
#include "spectrumScan_Thread.h"
#include "jammer_Thread.h"
#include "configFile.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
static struct pt application_pt, cli_pt, rf_pt, sub_tx_pt, button_pt, specrum_pt, jammer_pt, gps_pt;

uint32_t millis = 0;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define GCC_VERSION ((__GNUC__ * 100) + (__GNUC_MINOR__ * 10) + ( __GNUC_PATCHLEVEL__ ))

// Firmware builds require at least GCC 5.4.1
#if (GCC_VERSION < 541)
	#error "GCC compiler >= 5.4.1 required"
  #pragma message("GCC is " STR(__GNUC__)"."STR(__GNUC_MINOR__)"."STR(__GNUC_PATCHLEVEL__))
#endif
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void scheduler(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void checkResetSourse(void)
{
  if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
  {    
    DEBUG_PRINT(CLI_SYS"PORRST"CLI_NEW_LINE);
  }   
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) != RESET)    
  {
    DEBUG_PRINT(CLI_SYS"SFTRST"CLI_NEW_LINE);
  } 
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET)    
  {
    DEBUG_PRINT(CLI_SYS"IWDGRST"CLI_NEW_LINE);
  }  
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)    
  {
    DEBUG_PRINT(CLI_SYS"PINRST"CLI_NEW_LINE);
  }
  __HAL_RCC_CLEAR_RESET_FLAGS();
}

void initProtothreads(void)
{
  PT_INIT(&application_pt);
  PT_INIT(&cli_pt);
  PT_INIT(&rf_pt);
  PT_INIT(&sub_tx_pt);
  PT_INIT(&button_pt);
  PT_INIT(&specrum_pt);
  PT_INIT(&jammer_pt);
  PT_INIT(&gps_pt);
}

bool CC1101_reinit(void)
{
  return TI_init(&hspi2, NSS_CS_GPIO_Port, NSS_CS_Pin); // CS
}

static void stm32_cacheEnable(void)
{
#if (INSTRUCTION_CACHE_ENABLE != 0U) /* Включаем кэширование инструкций */
  ((FLASH_TypeDef *) ((0x40000000UL + 0x00020000UL) + 0x3C00UL))->ACR |= (0x1UL << (9U));
#endif

#if (DATA_CACHE_ENABLE != 0U) /* Включаем кэширование данных */
  ((FLASH_TypeDef *) ((0x40000000UL + 0x00020000UL) + 0x3C00UL))->ACR |= (0x1UL << (10U));
#endif

#if (PREFETCH_ENABLE != 0U) /* Включаем систему предварительной выборки инструкций */
  ((FLASH_TypeDef *) ((0x40000000UL + 0x00020000UL) + 0x3C00UL))->ACR |= (0x1UL << (8U));
#endif
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  stm32_cacheEnable();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  
  SysTick_Config(SystemCoreClock/1000); //Настраиваем системный таймер (прерывания 1000 раз в секунду)
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  MX_SPI2_Init();
  MX_ADC1_Init();
  MX_IWDG_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  /* Настройка дисплея */
  LCD_SPI_Connected_data spi_con =     
  { 
    .spi        = SPI1,
    .dma_tx     = dma_tx_1,        // Данные DMA
    .reset_port = LCD_RESET_GPIO_Port,
    .reset_pin  = LCD_RESET_Pin,
    .dc_port    = LCD_DC_GPIO_Port,
    .dc_pin     = LCD_DC_Pin,
    .cs_port    = LCD_CS_GPIO_Port,
    .cs_pin     = LCD_CS_Pin
  };

#ifndef  LCD_DYNAMIC_MEM
  LCD_Handler lcd1;
#endif
   //Cоздаем обработчик дисплея ILI9341
   LCD = LCD_DisplayAdd(LCD,
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
             PAGE_ORIENTATION_PORTRAIT,
             ILI9341_Init,
             ILI9341_SetWindow,
             ILI9341_SleepIn,
             ILI9341_SleepOut,
             &spi_con,
             LCD_DATA_16BIT_BUS,
             bkl_data);

  lcd = LCD;     //Указатель на первый дисплей в списке
  LCD_Init(lcd);
  LCD_Fill(lcd, COLOR_RED);


  XPT2046_InitTouch(&touch1, 20, &cnt_touch);   // инициализация обработчика XPT2046

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
calibrateTouchEnable();
#endif

#if RUN_DEMO
  LCD_Fill(lcd, COLOR_WHITE);
  Draw_TouchPenDemo(&touch1, lcd); //Демка для рисования на экране с помощью тачскрина.
  RoadCircleDemo(&touch1, lcd);    //Демка рисует примитивы, отображает температуру и позволяет перемещать круг по дисплею.
#endif

  LCD_Fill(lcd, COLOR_BLACK);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  buzzer_init(BUZZER_GPIO_Port, BUZZER_Pin);
  buzzer_enable();
  LL_mDelay(5);
  buzzer_disable();

  adc_enable();

  LCD_WriteString(lcd, 5, 25, "CC1101 int...",
            &Font_8x13, COLOR_WHITE, COLOR_BLACK, LCD_SYMBOL_PRINT_FAST);

  CC1101_customSetCSpin(&hspi2, NSS_CS_GPIO_Port, NSS_CS_Pin);

  bool error_state = CC1101_power_up_reset();

  if(error_state)
  {
    LCD_WriteString(lcd, 5, 55, "CC1101 not found!",
            &Font_8x13, COLOR_WHITE, COLOR_RED, LCD_SYMBOL_PRINT_FAST);
    while (1)
    {
      IWDG_reload();
    }
  }

#if CUSTOM_OLD_CONFIG
  TI_setCarrierFreq(CFREQ_433);
  TI_setDevAddress(1); 
#endif
  error_state = CC1101_reinit();

  encoder_init();
  
  initProtothreads();

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    if(getTxButtonState() || TX_MODE_ALWAYS)
    {
      if(getWorkState() != TX_MODE)
      {
        PT_INIT(&sub_tx_pt);
        setWorkSate(TX_MODE);
        debugPrintf("TX Mode"CLI_NEW_LINE);
      }
    }
    else if(getjammButtonState())
    {
      if(getWorkState() != JAMMER_MODE)
      {
        PT_INIT(&jammer_pt);
        setWorkSate(JAMMER_MODE);
        debugPrintf("JAMMER Mode"CLI_NEW_LINE);
      }
    }
    else if(getScanButtonState())
    {
      if(getWorkState() != SCAN_MODE)
      {
        PT_INIT(&specrum_pt);
        setWorkSate(SCAN_MODE);
        debugPrintf("SCAN Mode"CLI_NEW_LINE);
      }
    }
    else if(getGpsButtonState())
    {
      if(getWorkState() != GPS_MODE)
      {
        PT_INIT(&gps_pt);
        setWorkSate(GPS_MODE);
        debugPrintf("GPS Mode"CLI_NEW_LINE);
      }
    }
    else
    {
      if(getWorkState() != RX_MODE)
      {
        PT_INIT(&rf_pt);
        setWorkSate(RX_MODE);
        debugPrintf("RX Mode"CLI_NEW_LINE);
      }
    }

    if(getBootButtonState())
    {
      runBootloader();
    }
    
    scheduler();

    IWDG_reload();
  }
  /* USER CODE END 3 */
}

static void scheduler(void)
{
  Application_Thread(&application_pt);
  Button_Thread(&button_pt);
#if(CLI_ENABLE)
    CLI_Thread(&cli_pt);
#endif

  if (getBootingScreenMode())
  {
    return;
  }

  switch (getWorkState())
  {
    case TX_MODE:
      subGHz_TX_Thread(&sub_tx_pt);
      break;
    case RX_MODE:
      subGHz_RX_Thread(&rf_pt);
      break;
    case SCAN_MODE:
      spectrumScan_Thread(&specrum_pt);
      break;
    case JAMMER_MODE:
      jammer_Thread(&jammer_pt);
      break;
    case GPS_MODE:
      gps_Thread(&gps_pt);
      break;
    default:
      assert_param(0U);
      break;
  }
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
  LL_RCC_LSI_Enable();

   /* Wait till LSI is ready */
  while(LL_RCC_LSI_IsReady() != 1)
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
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_8);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_SetSystemCoreClock(84000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
  LL_RCC_SetTIMPrescaler(LL_RCC_TIM_PRESCALER_TWICE);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();

  debugPrintf("HAL_ERROR!\r\n");
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
  debugPrintf("Wrong parameters value: file %s on line %d\r\n", file, line);
  while(1);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
