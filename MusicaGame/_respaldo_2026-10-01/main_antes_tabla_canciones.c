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
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "audio.h"      /* motor de audio: TIM6 -> DAC1 (PA4) <- DMA1 Stream5 */
//#include "song_demo.h"  /* canción de prueba original (incluir en un solo .c) */
// #include "song_b_e_r___the_night_begins_to_shine__wip.h"  /* incluye sus 5 pistas */
//#include "song_mighty_morphin_power_rangers.h"
//#include "song_michael_jackson___beat_it.h"
//#include "song_seven_nation_army.h"
#include "song_b_e_r___the_night_begins_to_shine__wip.h"
#include "song_pokemon_black_and_white_low_hp_midi_version.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* [Prueba mute] El jugador 1 suena solo mientras B1 está presionado */
#define BTN_DEBOUNCE_MS 30

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* [Prueba mute] Pistas del jugador 1 = canal 1 de la canción (lo genera el conversor) */
#define p1_tracks song_pokemon_black_and_white_low_hp_midi_version_canal1
#define P1_N      SONG_POKEMON_BLACK_AND_WHITE_LOW_HP_MIDI_VERSION_CANAL1_N
static uint8_t  p1_muted = 0;
static uint8_t  p1_vol_saved[P1_N];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* [Prueba mute] Silencia / devuelve el volumen de las pistas del jugador 1 */
static void P1_SetMuted(uint8_t mute) {
  if (mute == p1_muted) return;
  const uint8_t *vol = Music_TrackVolumes();
  for (uint32_t i = 0; i < P1_N; i++) {
    if (mute) { p1_vol_saved[i] = vol[p1_tracks[i]]; Music_SetTrackVolume(p1_tracks[i], 0); }
    else      { Music_SetTrackVolume(p1_tracks[i], p1_vol_saved[i]); }
  }
  p1_muted = mute;
}

/* [Prueba mute] B1 (botón azul, activo en bajo) con antirrebote; sin HAL_Delay.
 * Devuelve 1 mientras está presionado. */
static uint8_t B1_Sostenido(void) {
  static uint8_t estable = 0, ultimo = 0;
  static uint32_t tCambio = 0;
  uint8_t ahora = (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET);
  if (ahora != ultimo) { ultimo = ahora; tCambio = HAL_GetTick(); }
  if (HAL_GetTick() - tCambio >= BTN_DEBOUNCE_MS) estable = ahora;
  return estable;
}

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
  MX_DAC_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  Audio_Init();                    /* calcula ARR de TIM6 para 32 kHz y prepara el sintetizador */
  Audio_Start();                   /* desde aquí el audio corre solo por DMA + interrupciones */
  //Audio_PlaySong(&song_demo, 1);
  //Audio_PlaySong(&song_seven_nation_army, 1);
  // Audio_PlaySong(&song_b_e_r___the_night_begins_to_shine__wip, 1);   /* 1 = repetir */
  Audio_SetMasterVolume(160);      /* 16 pistas juntas: con 224 recortaba (distorsión); 160 casi nunca */
  //Audio_PlaySong(&song_seven_nation_army, 1);
  Audio_PlaySong(&song_pokemon_black_and_white_low_hp_midi_version, 1);   /* 1 = repetir */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* [Prueba mute] jugador 1: suena con B1 presionado, muteado al soltarlo */
    P1_SetMuted(!B1_Sostenido());
    /* al repetir la canción el motor restaura los volúmenes: mantener el mute */
    if (p1_muted) {
      for (uint32_t i = 0; i < P1_N; i++)
        if (Music_TrackVolumes()[p1_tracks[i]]) Music_SetTrackVolume(p1_tracks[i], 0);
    }

    /* LD2: parpadea = jugador 1 sonando (y prueba de que el CPU no está bloqueado);
       encendido fijo = jugador 1 muteado */
    static uint32_t tLed = 0;
    if (p1_muted) {
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    } else if (HAL_GetTick() - tLed >= 250) {
      tLed = HAL_GetTick();
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    }
    /* Diagnóstico: ver audio_cpu_load y audio_underruns en Live Expressions */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
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
