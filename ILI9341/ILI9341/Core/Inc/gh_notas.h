/*
 * gh_notas.h
 * Dibujo de botones y movimiento de notas (normales y largas con estela) del Guitar Hero.
 * Pantalla vertical 240x320. Solo usa funciones del driver: LCD_BitmapFast, LCD_SpriteFast y FillRectFast.
 */
#ifndef INC_GH_NOTAS_H_
#define INC_GH_NOTAS_H_

#include <stdint.h>
#include "gh_sprites.h"

#define GH_PANTALLA_W   240
#define GH_PANTALLA_H   320
#define GH_COLOR_FONDO  0x0000           /* fondo negro (ver GH_RestaurarFondo) */

/* Distribución de la pista: 4 carriles de 52 px de paso, 16 px de margen lateral */
#define GH_CARRIL_X(c)  (18 + (c) * 52)  /* x del sprite del carril c (0..3) */
#define GH_BOTON_Y      262              /* y de la fila de botones */
#define GH_NOTA_Y_INICIO (-GH_SPR_H)     /* las notas nacen justo arriba de la pantalla */

/* Eventos que devuelve GH_NotaMover */
#define GH_EV_NADA      0
#define GH_EV_LLEGO     1   /* la nota (o la cabeza de la nota larga) llegó al botón */
#define GH_EV_FIN_COLA  2   /* el final de la cola de una nota larga llegó al botón */

typedef struct {
	int16_t y;           /* y de la cabeza (sigue avanzando aunque la cabeza ya llegó) */
	int16_t largo;       /* largo de la cola en px; 0 = nota normal */
	uint8_t carril;      /* 0 verde, 1 rojo, 2 amarillo, 3 azul */
	uint8_t activa;      /* 1 = en juego */
	uint8_t cabezaLlego; /* 1 = la cabeza ya llegó al botón (solo notas largas) */
} GH_Nota;

/* Fondo: único punto a cambiar cuando el fondo sea una imagen */
void GH_RestaurarFondo(int16_t x, int16_t y, int16_t w, int16_t h);

/* Botones */
void GH_DibujarBoton(uint8_t carril, uint8_t estado);   /* GH_BTN_REPOSO / PRESIONADO / SOSTENIDO */
void GH_DibujarBotones(void);                           /* los 4 en reposo */

/* Notas */
void    GH_NotaIniciar(GH_Nota *n, uint8_t carril);                     /* nota normal */
void    GH_NotaLargaIniciar(GH_Nota *n, uint8_t carril, int16_t largo); /* nota larga con cola de 'largo' px */
uint8_t GH_NotaMover(GH_Nota *n, int16_t dy);                           /* devuelve GH_EV_* */
void    GH_ColaRedibujar(const GH_Nota *n, uint8_t encendida);          /* repinta la cola visible (p. ej. al sostener) */

#endif /* INC_GH_NOTAS_H_ */
