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
#include "songs.h"      /* tabla de canciones (los .h de canciones se incluyen en songs.c) */

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
/* ===== Canción seleccionada =====
 * Índice en song_table[] (ver songs.c):
 *   0 demo · 1 Seven Nation Army · 2 The Night Begins to Shine · 3 Power Rangers · 4 Beat It · 5 Pokémon
 * Por ahora se cambia aquí. Después la escribirá la comunicación (UART/SPI/I2C): al cambiar su valor,
 * el while(1) detecta el cambio y arranca la nueva canción. */
volatile uint8_t song_index = 5;

static uint8_t  song_playing = 0x00;
static const SongEntry *cur = 0;          /* fila de la canción actual */

/* [Prueba mute] estado del jugador 1 */
static uint8_t  p1_muted = 0;
static uint8_t  p1_vol_saved[MUSIC_MAX_TRACKS];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Arranca la canción 'idx' de la tabla (ignora índices fuera de rango) */
static void Song_Start(uint8_t idx) {
  if (idx >= SONG_COUNT) return;
  cur = &song_table[idx];
  p1_muted = 0;                                           /* volúmenes nuevos: nada muteado */
  Audio_SetMasterVolume(cur->master ? cur->master : AUDIO_MASTER_VOLUME);
  Audio_PlaySong(cur->song, 1);                           /* 1 = repetir */
  song_playing = idx;
}

/* [Prueba mute] Silencia / devuelve el volumen de las pistas del jugador 1 */
static void P1_SetMuted(uint8_t mute) {
  if (mute == p1_muted || cur == 0 || cur->p1_tracks == 0) return;
  const uint8_t *vol = Music_TrackVolumes();
  for (uint32_t i = 0; i < cur->p1_n; i++) {
    uint8_t t = cur->p1_tracks[i];
    if (mute) { p1_vol_saved[i] = vol[t]; Music_SetTrackVolume(t, 0); }
    else      { Music_SetTrackVolume(t, p1_vol_saved[i]); }
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
  Song_Start(song_index);          /* canción inicial */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* Cambio de canción: cuando song_index cambia (hoy en código, luego por comunicación) */
    if (song_index != song_playing) {
      Song_Start(song_index);
    }

    /* [Prueba mute] jugador 1: suena con B1 presionado, muteado al soltarlo */
    P1_SetMuted(!B1_Sostenido());

    /* al repetir la canción el motor restaura los volúmenes: mantener el mute */
    if (p1_muted && cur && cur->p1_tracks) {
      for (uint32_t i = 0; i < cur->p1_n; i++)
        if (Music_TrackVolumes()[cur->p1_tracks[i]]) Music_SetTrackVolume(cur->p1_tracks[i], 0);
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
