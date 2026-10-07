/*
 * song_demo.h
 * Canción de prueba ORIGINAL (no es de ningún artista): La menor, 120 BPM, 8 compases, 4 pistas.
 *
 * Los arreglos son 'static const': viven en FLASH (no gastan RAM).
 * Inclúyelo en UN solo .c (por ejemplo main.c o songs.c).
 */
#ifndef SONG_DEMO_H_
#define SONG_DEMO_H_

#include "music.h"
#include "synth.h"

#define SONG_DEMO_BPM 120

/* Atajos para escribir más corto (opcionales) */
#define SD_CH(n, d)        { (n), (d), 0, NOTE_CHORD }               /* suena junto con la siguiente */
#define SD_PWR(r, f, d)    SD_CH(r, d), { (f), (d) }                    /* power chord: raíz + quinta */
#define SD_K_HH            SD_CH(36, EIGHTH), { 42, EIGHTH }            /* bombo + hi-hat */
#define SD_S_HH            SD_CH(38, EIGHTH), { 42, EIGHTH }            /* caja + hi-hat */
#define SD_HH              { 42, EIGHTH }                            /* hi-hat solo */
#define SD_DRUM_BAR        SD_K_HH, SD_HH, SD_S_HH, SD_HH, SD_K_HH, SD_K_HH, SD_S_HH, SD_HH

/* ---------------- Melodía (seno) ---------------- */
static const Note demo_lead[] = {
	{E5, QUARTER}, {C5, EIGHTH}, {D5, EIGHTH}, {E5, QUARTER}, {A4, QUARTER},
	{F5, DOTTED(QUARTER)}, {E5, EIGHTH}, {D5, QUARTER}, {C5, QUARTER},
	{E5, EIGHTH}, {G5, EIGHTH}, {E5, QUARTER}, {C5, QUARTER}, {REST, EIGHTH}, {D5, EIGHTH},
	{D5, HALF}, {B4, QUARTER}, {REST, QUARTER},
	{A5, QUARTER, 110}, {G5, EIGHTH}, {E5, EIGHTH}, {D5, QUARTER}, {E5, QUARTER},
	{C5, EIGHTH}, {D5, EIGHTH}, {F5, QUARTER}, {A5, QUARTER, 115}, {G5, QUARTER},
	{G5, DOTTED(QUARTER)}, {E5, EIGHTH}, {C5, QUARTER}, {D5, QUARTER},
	{B4, QUARTER}, {D5, QUARTER}, {A4, HALF, 120},
};

/* ---------------- Guitarra rítmica (cuadrada, acordes de 2 notas) ---------------- */
#define SD_GTR_BAR(r, f)   SD_PWR(r, f, QUARTER), SD_PWR(r, f, QUARTER), SD_PWR(r, f, EIGHTH), SD_PWR(r, f, EIGHTH), SD_PWR(r, f, QUARTER)
static const Note demo_guitar[] = {
	SD_GTR_BAR(A3, E4), SD_GTR_BAR(F3, C4), SD_GTR_BAR(C4, G4), SD_GTR_BAR(G3, D4),
	SD_GTR_BAR(A3, E4), SD_GTR_BAR(F3, C4), SD_GTR_BAR(C4, G4),
	SD_PWR(G3, D4, HALF), SD_PWR(A3, E4, HALF),
};

/* ---------------- Bajo (triangular, corcheas) ---------------- */
#define SD_BASS_BAR(n)     {n, EIGHTH}, {n, EIGHTH}, {n, EIGHTH}, {n, EIGHTH}, {n, EIGHTH}, {n, EIGHTH}, {n, EIGHTH}, {n, EIGHTH}
static const Note demo_bass[] = {
	SD_BASS_BAR(A2), SD_BASS_BAR(F2), SD_BASS_BAR(C3), SD_BASS_BAR(G2),
	SD_BASS_BAR(A2), SD_BASS_BAR(F2), SD_BASS_BAR(C3),
	{G2, EIGHTH}, {G2, EIGHTH}, {G2, EIGHTH}, {G2, EIGHTH}, {A2, HALF},
};

/* ---------------- Batería (ruido + bombo) ---------------- */
static const Note demo_drums[] = {
	SD_DRUM_BAR, SD_DRUM_BAR, SD_DRUM_BAR, SD_DRUM_BAR, SD_DRUM_BAR, SD_DRUM_BAR, SD_DRUM_BAR,
	SD_K_HH, SD_HH, SD_S_HH, SD_HH, SD_CH(36, HALF), {49, HALF},     /* final: bombo + platillo */
};

/* ---------------- Canción: pista -> instrumento -> volumen ---------------- */
static const TrackDef demo_tracks[] = {
	TRACK(demo_lead,   INST_LEAD,   VOL(0.40)),   /* pista 0 */
	TRACK(demo_guitar, INST_GUITAR, VOL(0.30)),   /* pista 1 */
	TRACK(demo_bass,   INST_BASS,   VOL(0.35)),   /* pista 2 */
	TRACK(demo_drums,  INST_DRUMS,  VOL(0.25)),   /* pista 3 */
};

static const Song song_demo = { "Demo original", SONG_DEMO_BPM, 4, demo_tracks };

#undef SD_CH
#undef SD_PWR
#undef SD_K_HH
#undef SD_S_HH
#undef SD_HH
#undef SD_DRUM_BAR
#undef SD_GTR_BAR
#undef SD_BASS_BAR

#endif /* SONG_DEMO_H_ */
