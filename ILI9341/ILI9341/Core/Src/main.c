/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"
#include "Bitmaps.h"
#include "gh_sprites.h"   // [GH] sprites de botones y notas (en RAM)
#include "gh_notas.h"     // [GH] dibujo de botones y movimiento de notas

// Para el uart3
#include <stdio.h>
#include <string.h>
#include <stdbool.h>      // Necesario para usar 'bool'
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* [GH] Parámetros de la demo de notas cayendo */
#define GH_MAX_NOTAS        8
#define GH_PERIODO_CUADRO   16   /* ms mínimos por cuadro (~60 fps máx.) */
#define GH_VELOCIDAD        4    /* px que baja cada nota por cuadro */
#define GH_CUADROS_ENTRE_NOTAS 22 /* cuadros entre notas nuevas (22*4 = 88 px de separación) */



//Comentario de prueba para el GIt



// Este define lo tengo que cambiar o eliminar ya que la presion sera dictada por el tiempo del boton

//#define GH_TIEMPO_PRESION   120  /* ms que el botón queda encendido al llegar la nota */




/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

volatile uint8_t rx_byte;
char rx_buffer[30];
volatile uint8_t rx_index = 0;
volatile uint8_t comando_listo = 0;

/* Botones virtuales del Jugador 1 controlados desde el ESP32 */
volatile uint8_t p1_btn_verde = 0;    // Asignado al botón A
volatile uint8_t p1_btn_rojo = 0;     // Asignado al botón B
volatile uint8_t p1_btn_amarillo = 0; // Asignado al botón X
volatile uint8_t p1_btn_azul = 0;     // Asignado al botón Y

extern uint16_t fondo[];

/* [GH] Estado de la demo */
static GH_Nota notas[GH_MAX_NOTAS];

/* Patrón de la demo: {carril, largo de cola en px (0 = nota normal)} */
static const struct { uint8_t carril; int16_t largo; } patron[] = {
	{0, 0}, {1, 0}, {2, 120}, {3, 0}, {1, 0}, {0, 80}, {3, 0}, {2, 0}, {1, 160}, {3, 0}, {0, 0}, {3, 100},
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Lógica de traducción de comandos UART a Botones Virtuales
void ProcesarComandoESP32(char* comando) {
    bool presionado = true;

    /* Detectar si se soltó el botón */
    if (strstr(comando, "_off") != NULL) {
        presionado = false;
    }

    /* Asignar a variables si el comando es para el Jugador 1 (p1_) */
    if (strncmp(comando, "p1_", 3) == 0) {
        if (strstr(comando, "A") != NULL && !strstr(comando, "Down")) {
            p1_btn_verde = presionado;
        }
        else if (strstr(comando, "B") != NULL) {
            p1_btn_rojo = presionado;
        }
        else if (strstr(comando, "X") != NULL) {
            p1_btn_amarillo = presionado;
        }
        else if (strstr(comando, "Y") != NULL) {
            p1_btn_azul = presionado;
        }
    }
}

/* [GH] Un paso de la demo: genera notas y lee botones VIRTUALES */
static void GH_DemoPaso(void) {
	static uint32_t tCuadro = 0;
	static uint16_t cuadrosDesdeNota = 0;
	static uint16_t cuadrosEspera = 0;
	static uint8_t iPatron = 0;

	static uint8_t estadoVisualBoton[GH_NUM_CARRILES] = {GH_BTN_REPOSO, GH_BTN_REPOSO, GH_BTN_REPOSO, GH_BTN_REPOSO};
	static uint8_t estadoVisualCola[GH_NUM_CARRILES]  = {0, 0, 0, 0};

	uint32_t ahora = HAL_GetTick();
	if (ahora - tCuadro < GH_PERIODO_CUADRO) return;
	tCuadro = ahora;

	if (++cuadrosDesdeNota >= cuadrosEspera) {
		for (int i = 0; i < GH_MAX_NOTAS; i++) {
			if (!notas[i].activa) {
				GH_NotaLargaIniciar(&notas[i], patron[iPatron].carril, patron[iPatron].largo);
				cuadrosEspera = GH_CUADROS_ENTRE_NOTAS + patron[iPatron].largo / GH_VELOCIDAD;
				iPatron = (iPatron + 1) % (sizeof(patron) / sizeof(patron[0]));
				cuadrosDesdeNota = 0;
				break;
			}
		}
	}

	/* LEER BOTONES VIRTUALES DEL ESP32 EN LUGAR DE LOS PINES FÍSICOS */
	uint8_t btnVirtuales[GH_NUM_CARRILES];
	btnVirtuales[0] = p1_btn_verde;
	btnVirtuales[1] = p1_btn_rojo;
	btnVirtuales[2] = p1_btn_amarillo;
	btnVirtuales[3] = p1_btn_azul;

	GH_Nota* colaActiva[GH_NUM_CARRILES] = {NULL, NULL, NULL, NULL};

	for (int i = 0; i < GH_MAX_NOTAS; i++) {
		if (!notas[i].activa) continue;

		uint8_t c = notas[i].carril;
		GH_NotaMover(&notas[i], GH_VELOCIDAD);

		/* LÓGICA DE ACIERTO */
		if (notas[i].y >= GH_BOTON_Y - 15 && notas[i].y <= GH_BOTON_Y + 20) {
			if (btnVirtuales[c]) {
				if (notas[i].largo == 0) {
					notas[i].activa = 0;
					GH_RestaurarFondo(GH_CARRIL_X(c), notas[i].y - GH_SPR_H, GH_SPR_W, GH_SPR_H * 2);
					continue;
				} else if (!notas[i].atrapada) {
					int16_t old_y = notas[i].y;
					notas[i].atrapada = 1;
					notas[i].y = GH_BOTON_Y;
					notas[i].largo += (GH_BOTON_Y - old_y);
					if (notas[i].largo < 0) notas[i].largo = 0;
					notas[i].cabezaLlego = 1;
					GH_RestaurarFondo(GH_CARRIL_X(c), old_y - GH_SPR_H, GH_SPR_W, GH_SPR_H * 2);
				}
			}
		}

		/* Sostenimiento de nota larga */
		if (notas[i].activa && notas[i].largo > 0 && notas[i].cabezaLlego) {
			if (notas[i].atrapada) {
				if (btnVirtuales[c]) {
					colaActiva[c] = &notas[i];
				} else {
					notas[i].atrapada = 0;
				}
			} else {
				if (notas[i].y >= GH_BOTON_Y && notas[i].y <= GH_BOTON_Y + 20) {
					if (btnVirtuales[c]) {
						int16_t old_y = notas[i].y;
						notas[i].atrapada = 1;
						notas[i].y = GH_BOTON_Y;
						notas[i].largo += (GH_BOTON_Y - old_y);
						if (notas[i].largo < 0) notas[i].largo = 0;
						colaActiva[c] = &notas[i];
						GH_RestaurarFondo(GH_CARRIL_X(c), old_y - GH_SPR_H, GH_SPR_W, GH_SPR_H * 2);
					}
				}
			}
		}
	}

	/* DIBUJAR EN PANTALLA */
	for (uint8_t c = 0; c < GH_NUM_CARRILES; c++) {
		uint8_t nuevoEstadoBoton = GH_BTN_REPOSO;
		uint8_t nuevoEstadoCola = 0;

		if (btnVirtuales[c]) {
			if (colaActiva[c] != NULL) {
				nuevoEstadoBoton = GH_BTN_SOSTENIDO;
				nuevoEstadoCola = 1;
			} else {
				nuevoEstadoBoton = GH_BTN_PRESIONADO;
			}
		}

		if (nuevoEstadoBoton != estadoVisualBoton[c]) {
			GH_DibujarBoton(c, nuevoEstadoBoton);
			estadoVisualBoton[c] = nuevoEstadoBoton;
		}

		if (colaActiva[c] != NULL) {
			if (nuevoEstadoCola != estadoVisualCola[c]) {
				GH_ColaRedibujar(colaActiva[c], nuevoEstadoCola);
				estadoVisualCola[c] = nuevoEstadoCola;
			}
		} else {
			estadoVisualCola[c] = 0;
		}
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	HAL_Init();
	  SystemClock_Config();
	  MX_GPIO_Init();
	  MX_USART3_UART_Init();

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
  MX_SPI1_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  // Iniciar la escucha del ESP32 por interrupción
    HAL_UART_Receive_IT(&huart3, (uint8_t*)&rx_byte, 1);


	LCD_Init();

	LCD_Clear(0x00);
	//FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
	//FillRect(0, 0, 319, 239, 0x0DFE);
	//FillRect(0, 0, 319, 239, 0x4c9d);

//	FillRect(50, 60, 20, 20, 0xF800);
//
//	FillRect(70, 60, 20, 20, 0x07E0);
//
//	FillRect(90, 60, 20, 20, 0x001F);


	//LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
	//LCD_Bitmap(100, 100, 49, 36, dkong);
	//LCD_Bitmap(100, 100, 24, 30, sonic);

	// [GH] Demo anterior en horizontal (320x240), desactivada al pasar a vertical:
	//LCD_Bitmap(0, 0, 320, 240, fondoPinguin);
	//LCD_BitmapTransparent(100, 100, 49, 36, dkong, 0x4b7e);
	//LCD_BitmapTransparent(50, 60,32,32,megaman,0x03ba);

	// [GH] Fondo negro (LCD_Clear(0x00) de arriba) + fila de botones en reposo
	GH_DibujarBotones();
	//LCD_Bitmap(132, 100, 32, 32, megaman);



//	FillRect(0, 0, 319, 206, 0x055b);
//	LCD_Print("Hola Mundo", 20, 150, 2, 0x001F, 0x055b);
//
//
//for (int x = 0; x < 319; x++) {
//			LCD_Bitmap(x, 116, 15, 15, tile);
//			LCD_Bitmap(x, 68, 15, 15, tile);
//			LCD_Bitmap(x, 207, 15, 15, tile);
//			LCD_Bitmap(x, 223, 15, 15, tile);
//			x += 14;
//		}

	//LCD_BitmapTransparent(100, 116-36, 49, 36, dkong, 0x4b7e);
	//LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[], int columns, int index, char flip, char offset);

	//LCD_Sprite(100, 116-29, 42, 29, link, 4, 0, 0, 0);


	//  LCD_Print("Hola Mundo", 20, 100, 1, 0x001F, 0xCAB9);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

		if (comando_listo) {
		        comando_listo = 0;
		        ProcesarComandoESP32(rx_buffer);
		    }


		GH_DemoPaso();   // [GH] notas cayendo


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

//
//				for (int x = 0; x < 319-42; x++) {
//					int anim = (x/10)%4;
//					// anim 0 1 2 3
//					LCD_Sprite(x, 116-29, 42, 29, link, 4, anim, 0, 0);
//					//V_line( x -1, 100, 50, 0x0DFE);
//					HAL_Delay(15);
//
//				}
//				for (int var = 319-24; var > 0;  var--) {
//					int anim = (var / 10) % 4;
//					LCD_Sprite(var, 100, 24, 30, sonics, 4, anim, 1, 0);
//					V_line(var + 24, 100, 30, 0x0DFE);
//					HAL_Delay(15);
//				}
//
//		for (int x = 0; x < 319-24; x++) {
//			int anim = (x/10)%4;
//			// anim 0 1 2 3
//			LCD_Sprite(x, 100, 24, 30, sonics, 4, anim, 0, 0);
//			V_line( x -1, 100, 30, 0x0DFE);
//			HAL_Delay(15);
//
//		}
//		for (int var = 319-24; var > 0;  var--) {
//			int anim = (var / 10) % 4;
//			LCD_Sprite(var, 100, 24, 30, sonics, 4, anim, 1, 0);
//			V_line(var + 24, 100, 30, 0x0DFE);
//			HAL_Delay(15);
//		}

//		for (int var = 0; var < 319-26;  var++) {
//			 int anim = (var / 5) % 4;
//			LCD_Sprite(var,100,26,16,link,4,anim,0,0);
//			V_line( var -1, 100, 16, 0x4d9e);
//			 HAL_Delay(15);
//		}
//		for (int var = 319-26; var > 0;  var--) {
//					 int anim = (var / 5) % 4;
//					LCD_Sprite(var,100,26,16,link,4,anim,1,0);
//					V_line( var +27, 100, 16, 0x4d9e);
//					 HAL_Delay(15);
//				}
//
//		for (int x = 0; x < 320 - 16; x++) {
//		    int anim2 = (x / 10) % 4;
//		    LCD_Sprite(x,100,16,16,kirbys,4,anim2,0,0);
//		    V_line( x -1, 100, 16, 0x74DA);
//		    HAL_Delay(15);
//		  }
//		  for (int x = 320-16; x > 0; x--) {
//		    int anim2 = (x / 10) % 4;
//		    LCD_Sprite(x,100,16,16,kirbys,4,anim2,1,0);
//		    V_line( x +16, 100, 16, 0x74DA);
//		    HAL_Delay(15);
//		  }




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

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_RST_Pin|LCD_D1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LCD_D7_Pin
                          |LCD_D0_Pin|LCD_D2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin|LCD_D6_Pin|LCD_D3_Pin|LCD_D5_Pin
                          |LCD_D4_Pin|SD_SS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LCD_RST_Pin LCD_D1_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin|LCD_D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RD_Pin LCD_WR_Pin LCD_RS_Pin LCD_D7_Pin
                           LCD_D0_Pin LCD_D2_Pin */
  GPIO_InitStruct.Pin = LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LCD_D7_Pin
                          |LCD_D0_Pin|LCD_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : bverde_Pin brojo_Pin bamarillo_Pin bazul_Pin */
  GPIO_InitStruct.Pin = bverde_Pin|brojo_Pin|bamarillo_Pin|bazul_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_CS_Pin LCD_D6_Pin LCD_D3_Pin LCD_D5_Pin
                           LCD_D4_Pin SD_SS_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin|LCD_D6_Pin|LCD_D3_Pin|LCD_D5_Pin
                          |LCD_D4_Pin|SD_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART3) {
        if (rx_byte == '\n') {
            rx_buffer[rx_index] = '\0'; // Terminar la cadena
            comando_listo = 1;          // Avisar al while(1) que el comando llegó completo
            rx_index = 0;               // Reiniciar el índice
        }
        else if (rx_byte != '\r' && rx_index < sizeof(rx_buffer) - 1) {
            rx_buffer[rx_index++] = rx_byte;
        }
        // Re-armar la interrupción para escuchar el próximo byte
        HAL_UART_Receive_IT(&huart3, (uint8_t*)&rx_byte, 1);
    }
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
	while (1) {
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
