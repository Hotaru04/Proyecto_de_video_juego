/*
 * gh_notas.c
 * Movimiento de notas y dibujo de botones. Ver gh_notas.h
 *
 * Nota normal: al bajar dy px se restaura el fondo en la franja que deja arriba y se redibuja la nota.
 * Nota larga ("estela"):
 *   - Cabeza: en vez de restaurar el fondo, en la franja que deja arriba pinta COLA (mosaico 48x8).
 *   - Fin de cola: es una "nota invisible" que va 'largo' px detrás de la cabeza. No se dibuja,
 *     solo restaura el fondo en la franja que deja atrás. No da ni quita puntos.
 * La zona de botones (y >= GH_BOTON_Y) nunca se toca: ahí solo dibujan los botones.
 */
#include "gh_notas.h"
#include "ili9341.h"

/* Recorta [y, y+h) a la zona de juego [0, GH_BOTON_Y). Devuelve 0 si no queda nada. */
static uint8_t GH_RecortarZona(int16_t *y, int16_t *h) {
	if (*y < 0) { *h += *y; *y = 0; }
	if (*y + *h > GH_BOTON_Y) *h = GH_BOTON_Y - *y;
	return *h > 0;
}

void GH_RestaurarFondo(int16_t x, int16_t y, int16_t w, int16_t h) {
	if (!GH_RecortarZona(&y, &h) || w <= 0) return;
	/* Fondo negro por ahora. Cuando el fondo sea una imagen, aquí se copia ese pedazo de la imagen. */
	FillRectFast(x, y, w, h, GH_COLOR_FONDO);
}

/* Pinta cola en [y, y+h) repitiendo el mosaico de 8 filas (anclado a la pantalla para que no "salte"). */
static void GH_PintarCola(uint8_t carril, int16_t y, int16_t h, uint8_t encendida) {
	if (!GH_RecortarZona(&y, &h)) return;
	const uint8_t *tile = encendida ? gh_hold_tail_on[carril] : gh_hold_tail[carril];
	int16_t x = GH_CARRIL_X(carril);
	while (h > 0) {
		int16_t fila = y % GH_COLA_TILE_H;            /* fila del mosaico donde arrancamos */
		int16_t n = GH_COLA_TILE_H - fila;            /* filas hasta el final del mosaico */
		if (n > h) n = h;
		LCD_BitmapFast(x, y, GH_SPR_W, n, tile + fila * GH_SPR_W * 2);
		y += n; h -= n;
	}
}

/* Dibuja un sprite de 48x48 recortando lo que quede fuera de la pantalla. */
static void GH_SpriteRecortado(int16_t x, int16_t y, const uint8_t *bmp) {
	int16_t fila0 = 0, alto = GH_SPR_H;
	if (y < 0) { fila0 = -y; alto += y; y = 0; }
	if (y + alto > GH_PANTALLA_H) alto = GH_PANTALLA_H - y;
	if (alto <= 0) return;
	LCD_BitmapFast(x, y, GH_SPR_W, alto, bmp + fila0 * GH_SPR_W * 2);
}

void GH_DibujarBoton(uint8_t carril, uint8_t estado) {
	LCD_SpriteFast(GH_CARRIL_X(carril), GH_BOTON_Y, GH_SPR_W, GH_SPR_H,
			gh_btn_sheet[carril], GH_BTN_CUADROS, estado, 0, 0, 0);
}

void GH_DibujarBotones(void) {
	for (uint8_t c = 0; c < GH_NUM_CARRILES; c++) GH_DibujarBoton(c, GH_BTN_REPOSO);
}

void GH_NotaIniciar(GH_Nota *n, uint8_t carril) {
	GH_NotaLargaIniciar(n, carril, 0);
}

void GH_NotaLargaIniciar(GH_Nota *n, uint8_t carril, int16_t largo) {
	n->carril = carril;
	n->y = GH_NOTA_Y_INICIO;
	n->largo = largo;
	n->activa = 1;
	n->cabezaLlego = 0;
}

void GH_ColaRedibujar(const GH_Nota *n, uint8_t encendida) {
	if (!n->activa || n->largo == 0) return;
	int16_t arriba = n->y - n->largo;                         /* fin de cola */
	int16_t abajo = n->cabezaLlego ? GH_BOTON_Y : n->y + GH_SPR_H / 2;
	GH_PintarCola(n->carril, arriba, abajo - arriba, encendida);
}

uint8_t GH_NotaMover(GH_Nota *n, int16_t dy) {
	if (!n->activa) return GH_EV_NADA;
	int16_t x = GH_CARRIL_X(n->carril);
	int16_t y_ant = n->y;
	int16_t y_nueva = y_ant + dy;
	uint8_t evento = GH_EV_NADA;
	n->y = y_nueva;

	/* ---- Cabeza ---- */
	if (!n->cabezaLlego) {
		if (y_nueva >= GH_BOTON_Y) {
			/* Llegó: lo que quedaba arriba del botón se vuelve fondo (normal) o cola (larga) */
			if (n->largo == 0) GH_RestaurarFondo(x, y_ant, GH_SPR_W, GH_BOTON_Y - y_ant);
			else               GH_PintarCola(n->carril, y_ant, GH_BOTON_Y - y_ant, 0);
			n->cabezaLlego = 1;
			evento = GH_EV_LLEGO;
			if (n->largo == 0) { n->activa = 0; return evento; }
		} else {
			if (n->largo == 0) GH_RestaurarFondo(x, y_ant, GH_SPR_W, dy);   /* borra la franja de arriba */
			else               GH_PintarCola(n->carril, y_ant, dy, 0);      /* deja estela */
			GH_SpriteRecortado(x, y_nueva, n->largo ? gh_hold_head[n->carril] : gh_note[n->carril]);
		}
	}

	/* ---- Fin de cola ("nota invisible"): restaura el fondo detrás de la cola ---- */
	if (n->largo > 0) {
		int16_t fin_ant = y_ant - n->largo;
		int16_t fin_nuevo = y_nueva - n->largo;
		if (fin_nuevo >= GH_BOTON_Y) {
			GH_RestaurarFondo(x, fin_ant, GH_SPR_W, GH_BOTON_Y - fin_ant);
			n->activa = 0;
			return GH_EV_FIN_COLA;
		}
		GH_RestaurarFondo(x, fin_ant, GH_SPR_W, dy);
	}
	return evento;
}
