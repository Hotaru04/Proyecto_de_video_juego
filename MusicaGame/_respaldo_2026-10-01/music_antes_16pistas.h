/*
 * music.h
 * Motor de música (secuenciador): lee las pistas de una canción y dispara notas en el sintetizador.
 *
 * Tiempo: todo se mide en TICKS musicales (MUSIC_PPQ ticks por negra). Un solo reloj global de ticks
 * avanza todas las pistas a la vez -> nunca se desincronizan (son enteros, no hay deriva).
 * El reloj de ticks lo mueve audio.c contando MUESTRAS, así que el tiempo es exacto a la muestra.
 */
#ifndef INC_MUSIC_H_
#define INC_MUSIC_H_

#include <stdint.h>
#include "notes.h"

/* ---------- Duraciones musicales (en ticks) ---------- */
#define MUSIC_PPQ      48                 /* ticks por negra: divisible entre 2, 3, 4, 8, 16 */
#define WHOLE          (MUSIC_PPQ * 4)
#define HALF           (MUSIC_PPQ * 2)
#define QUARTER        (MUSIC_PPQ)
#define EIGHTH         (MUSIC_PPQ / 2)
#define SIXTEENTH      (MUSIC_PPQ / 4)
#define THIRTYSECOND   (MUSIC_PPQ / 8)
#define DOTTED(d)      ((d) * 3 / 2)      /* con puntillo */
#define TRIPLET(d)     ((d) * 2 / 3)      /* tresillo */

/* ---------- Nota ----------
 * {nota, duración}              -> suena 'dur' ticks y la siguiente empieza al terminar
 * {nota, duración, velocidad}   -> velocidad 1..127 (0 = valor por defecto MUSIC_DEFAULT_VEL)
 * {nota, duración, vel, NOTE_CHORD} -> la SIGUIENTE nota empieza al mismo tiempo (acordes / notas encimadas)
 * {REST, duración}              -> silencio: no suena, solo avanza el tiempo
 * Ocupa 6 bytes en flash.
 */
#define NOTE_CHORD           0x01
#define MUSIC_DEFAULT_VEL    100

typedef struct {
	uint8_t  note;    /* número MIDI (C4, A4...) o REST */
	uint16_t dur;     /* duración en ticks (QUARTER, EIGHTH...) */
	uint8_t  vel;     /* velocidad 1..127, 0 = por defecto */
	uint8_t  flags;   /* NOTE_CHORD */
} Note;

/* ---------- Definición de pista y canción (const, en flash) ---------- */
typedef struct {
	const Note *notes;
	uint16_t    length;      /* cantidad de notas */
	uint8_t     instrument;  /* INST_* de synth.h */
	uint8_t     volume;      /* 0..255, usar VOL(0.35) */
} TrackDef;

typedef struct {
	const char     *name;
	uint16_t        bpm;
	uint8_t         num_tracks;
	const TrackDef *tracks;
} Song;

#define VOL(x)               ((uint8_t)((x) * 255.0f + 0.5f))   /* 0.0..1.0 -> 0..255 (se calcula al compilar) */
#define NOTES_LEN(arr)       ((uint16_t)(sizeof(arr) / sizeof((arr)[0])))
#define TRACK(arr, inst, vol) { (arr), NOTES_LEN(arr), (inst), (vol) }

/* ---------- Estado de reproducción de una pista (RAM) ---------- */
typedef enum { TRACK_STOPPED = 0, TRACK_PLAYING, TRACK_FINISHED } TrackState;

typedef struct {
	const TrackDef *def;
	uint16_t index;      /* nota actual */
	uint16_t wait;       /* ticks que faltan para la siguiente nota */
	uint8_t  volume;     /* volumen actual (se puede cambiar en vivo) */
	uint8_t  state;      /* TrackState */
} Track;

#define MUSIC_MAX_TRACKS 8

/* ---------- API ---------- */
void     Music_Init(uint32_t sample_rate);
void     Music_Play(const Song *song, uint8_t loop);   /* todas las pistas arrancan en t = 0 */
void     Music_Stop(void);
uint8_t  Music_IsPlaying(void);
void     Music_SetTrackVolume(uint8_t track, uint8_t volume);   /* p. ej. silenciar guitarra al fallar */
uint32_t Music_GetTick(void);                                   /* ticks desde el inicio de la canción */
const uint8_t *Music_TrackVolumes(void);                        /* usado por el mezclador */

/* Usadas por audio.c para mover el reloj (no llamar desde la aplicación) */
uint32_t Music_SamplesUntilTick(void);
void     Music_AdvanceSamples(uint32_t n);

/* ---------- Ganchos (se pueden redefinir en otro .c; por defecto no hacen nada) ----------
 * OJO: se llaman desde la interrupción del DMA. Deben ser cortos (p. ej. meter un dato en una cola). */
void Music_OnNoteStart(uint8_t track, uint8_t note, uint8_t vel, uint16_t dur_ticks); /* futuro: banderas SPI */
void Music_OnSongEnd(void);

#endif /* INC_MUSIC_H_ */
