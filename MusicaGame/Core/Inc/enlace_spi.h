/*
 * enlace_spi.h
 * SPI2 maestro hacia el STM de la pantalla (ILI9341). Solo transmite. Ver gh_protocolo.h.
 *   PB13 SCK · PB15 MOSI · PB12 CS (salida, activo en bajo)
 *
 * Se configura por registros a propósito: no depende de CubeMX ni del driver HAL de SPI,
 * así que regenerar el .ioc no lo borra. NO actives SPI2 en CubeMX (choca con estos pines).
 */
#ifndef INC_ENLACE_SPI_H_
#define INC_ENLACE_SPI_H_

#include <stdint.h>

void Enlace_Init(void);
/* Arma y manda un paquete de GH_PKT_LEN bytes (agrega SYNC y checksum). Tarda ~60 us. */
void Enlace_Enviar(uint8_t tipo, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4);

extern volatile uint32_t enlace_paquetes;   /* diagnóstico: paquetes enviados (Live Expressions) */

#endif /* INC_ENLACE_SPI_H_ */
