/*
 * song_b_e_r___the_night_begins_to_shine__wip.h
 * Generado por midi_to_c.py desde B.E.R - The Night Begins to Shine (WIP).mid
 * Incluir en UN solo .c. Los arreglos son static const (flash).
 */
#ifndef SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_H_
#define SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_H_

#include "music.h"
#include "synth.h"

#define SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_BPM 130

#include "song_b_e_r___the_night_begins_to_shine__wip_canal1.h"
#include "song_b_e_r___the_night_begins_to_shine__wip_canal2.h"
#include "song_b_e_r___the_night_begins_to_shine__wip_canal3.h"

static const TrackDef b_e_r___the_night_begins_to_shine__wip_tracks[] = {
	/* canal 1 */
	TRACK(b_e_r___the_night_begins_to_shine__wip_distortion_guitar, INST_GUITAR, VOL(0.30)),   /* pista 0 */
	TRACK(b_e_r___the_night_begins_to_shine__wip_electric_guitar, INST_GUITAR, VOL(0.30)),   /* pista 1 */
	/* canal 2 */
	TRACK(b_e_r___the_night_begins_to_shine__wip_bass_guitar, INST_BASS, VOL(0.30)),   /* pista 2 */
	TRACK(b_e_r___the_night_begins_to_shine__wip_electric_drum_kit, INST_DRUMS, VOL(0.30)),   /* pista 3 */
	/* canal 3 */
	TRACK(b_e_r___the_night_begins_to_shine__wip_synth_pluck, INST_SYNTH, VOL(0.30)),   /* pista 4 */
};

static const Song song_b_e_r___the_night_begins_to_shine__wip = { "b_e_r___the_night_begins_to_shine__wip", SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_BPM, 5, b_e_r___the_night_begins_to_shine__wip_tracks };

/* Canales: índices de pista (en b_e_r___the_night_begins_to_shine__wip_tracks[]) que pertenecen a cada canal.
 * Ej.: for (i = 0; i < SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_CANAL1_N; i++) Music_SetTrackVolume(b_e_r___the_night_begins_to_shine__wip_canal1[i], 0); */
#define SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_NUM_CANALES 3
#define SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_CANAL1_N 2
static const uint8_t b_e_r___the_night_begins_to_shine__wip_canal1[] __attribute__((unused)) = { 0, 1 };
#define SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_CANAL2_N 2
static const uint8_t b_e_r___the_night_begins_to_shine__wip_canal2[] __attribute__((unused)) = { 2, 3 };
#define SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_CANAL3_N 1
static const uint8_t b_e_r___the_night_begins_to_shine__wip_canal3[] __attribute__((unused)) = { 4 };

#endif /* SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_H_ */
