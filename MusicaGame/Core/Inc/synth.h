/*
 * synth.h
 * Sintetizador: banco de VOCES (osciladores DDS + envolvente ADSR) que se suman en un buffer de mezcla.
 * No sabe nada de canciones ni del DAC: recibe NoteOn y produce muestras.
 */
#ifndef INC_SYNTH_H_
#define INC_SYNTH_H_

#include <stdint.h>

#define SYNTH_MAX_VOICES  12     /* polifonía total (todas las pistas juntas) */

typedef enum { WAVE_SINE = 0, WAVE_SQUARE, WAVE_TRIANGLE, WAVE_SAW, WAVE_NOISE } Waveform;

/* Instrumentos disponibles (índice para TrackDef.instrument) */
enum {
	INST_LEAD = 0,   /* seno, melodía */
	INST_GUITAR,     /* cuadrada, ataque rápido */
	INST_BASS,       /* triangular */
	INST_SYNTH,      /* diente de sierra */
	INST_DRUMS,      /* ruido + bombo; la nota MIDI elige el golpe (36 bombo, 38 caja, 42 hi-hat...) */
	INST_COUNT
};

typedef struct {
	Waveform wave;
	uint16_t attack_ms;
	uint16_t decay_ms;
	uint8_t  sustain;      /* 0..255 (nivel relativo) */
	uint16_t release_ms;
	uint8_t  amplitude;    /* 0..255: compensa que cuadrada/sierra suenan más fuerte que el seno */
} Instrument;

extern const Instrument synth_instruments[INST_COUNT];

void    Synth_Init(uint32_t sample_rate);
void    Synth_NoteOn(uint8_t track, uint8_t inst, uint8_t midi_note, uint8_t vel, uint16_t dur_ticks);
void    Synth_Tick(void);       /* llamar una vez por tick musical: termina notas -> release */
void    Synth_AllOff(void);
uint8_t Synth_ActiveVoices(void);

/* Suma 'n' muestras de todas las voces en mix[] (int32, se debe poner en 0 antes).
 * track_vol[] = volumen 0..255 de cada pista. */
void    Synth_Render(int32_t *mix, uint32_t n, const uint8_t *track_vol);

#endif /* INC_SYNTH_H_ */
