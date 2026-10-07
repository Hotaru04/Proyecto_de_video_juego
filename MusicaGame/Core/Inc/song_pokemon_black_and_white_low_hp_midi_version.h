/*
 * song_pokemon_black_and_white_low_hp_midi_version.h
 * Generado por midi_to_c.py desde Pokemon_black_and_white_low_hp_midi_version.mid
 * Incluir en UN solo .c. Los arreglos son static const (flash).
 */
#ifndef SONG_POKEMON_BLACK_AND_WHITE_LOW_HP_MIDI_VERSION_H_
#define SONG_POKEMON_BLACK_AND_WHITE_LOW_HP_MIDI_VERSION_H_

#include "music.h"
#include "synth.h"

#define SONG_POKEMON_BLACK_AND_WHITE_LOW_HP_MIDI_VERSION_BPM 175

#include "song_pokemon_black_and_white_low_hp_midi_version_canal1.h"
#include "song_pokemon_black_and_white_low_hp_midi_version_canal2.h"
#include "song_pokemon_black_and_white_low_hp_midi_version_canal3.h"

static const TrackDef pokemon_black_and_white_low_hp_midi_version_tracks[] = {
	/* canal 1 */
	TRACK(pokemon_black_and_white_low_hp_midi_version_orchestra_hit, INST_LEAD, VOL(0.40)),   /* pista 0 */
	TRACK(pokemon_black_and_white_low_hp_midi_version_percussion, INST_DRUMS, VOL(0.30)),   /* pista 1 */
	TRACK(pokemon_black_and_white_low_hp_midi_version_electric_piano, INST_LEAD, VOL(0.40)),   /* pista 2 */
	/* canal 2 */
	TRACK(pokemon_black_and_white_low_hp_midi_version_reed_organ, INST_LEAD, VOL(0.40)),   /* pista 3 */
	TRACK(pokemon_black_and_white_low_hp_midi_version_drawbar_organ, INST_LEAD, VOL(0.40)),   /* pista 4 */
	TRACK(pokemon_black_and_white_low_hp_midi_version_strings, INST_LEAD, VOL(0.40)),   /* pista 5 */
	TRACK(pokemon_black_and_white_low_hp_midi_version_synth_bass, INST_BASS, VOL(0.30)),   /* pista 6 */
	/* canal 3 */
	TRACK(pokemon_black_and_white_low_hp_midi_version_square, INST_SYNTH, VOL(0.30)),   /* pista 7 */
};

static const Song song_pokemon_black_and_white_low_hp_midi_version = { "pokemon_black_and_white_low_hp_midi_version", SONG_POKEMON_BLACK_AND_WHITE_LOW_HP_MIDI_VERSION_BPM, 8, pokemon_black_and_white_low_hp_midi_version_tracks };

/* Canales: índices de pista (en pokemon_black_and_white_low_hp_midi_version_tracks[]) que pertenecen a cada canal.
 * Ej.: for (i = 0; i < SONG_POKEMON_BLACK_AND_WHITE_LOW_HP_MIDI_VERSION_CANAL1_N; i++) Music_SetTrackVolume(pokemon_black_and_white_low_hp_midi_version_canal1[i], 0); */
#define SONG_POKEMON_BLACK_AND_WHITE_LOW_HP_MIDI_VERSION_NUM_CANALES 3
#define SONG_POKEMON_BLACK_AND_WHITE_LOW_HP_MIDI_VERSION_CANAL1_N 3
static const uint8_t pokemon_black_and_white_low_hp_midi_version_canal1[] __attribute__((unused)) = { 0, 1, 2 };
#define SONG_POKEMON_BLACK_AND_WHITE_LOW_HP_MIDI_VERSION_CANAL2_N 4
static const uint8_t pokemon_black_and_white_low_hp_midi_version_canal2[] __attribute__((unused)) = { 3, 4, 5, 6 };
#define SONG_POKEMON_BLACK_AND_WHITE_LOW_HP_MIDI_VERSION_CANAL3_N 1
static const uint8_t pokemon_black_and_white_low_hp_midi_version_canal3[] __attribute__((unused)) = { 7 };

#endif /* SONG_POKEMON_BLACK_AND_WHITE_LOW_HP_MIDI_VERSION_H_ */
