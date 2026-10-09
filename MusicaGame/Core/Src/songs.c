/*
 * songs.c
 * Aquí (y solo aquí) se incluyen los .h de las canciones: así cada canción existe una sola vez en flash.
 */
#include "songs.h"
#include "synth.h"

#include "song_demo.h"
#include "song_seven_nation_army.h"
#include "song_b_e_r___the_night_begins_to_shine__wip.h"
#include "song_mighty_morphin_power_rangers.h"
#include "song_michael_jackson___beat_it.h"
#include "song_pokemon_black_and_white_low_hp_midi_version.h"

/* Fila de una canción exportada CON canales (el conversor genera prefijo_canalN y su _N):
 * canal 1 = jugador 1, canal 2 = jugador 2 */
#define SONG_CON_P1(song, canal1, n1, canal2, n2, master)  { &(song), (canal1), (n1), (master), (canal2), (n2) }
/* Fila de una canción SIN canales (no se mutea nada y no hay notas para la pantalla) */
#define SONG_SIN_P1(song, master)                          { &(song), 0, 0, (master), 0, 0 }

const SongEntry song_table[] = {
	/* 0 */ SONG_SIN_P1(song_demo, 0),
	/* 1 */ SONG_CON_P1(song_seven_nation_army,
	                    seven_nation_army_canal1, SONG_SEVEN_NATION_ARMY_CANAL1_N,
	                    seven_nation_army_canal2, SONG_SEVEN_NATION_ARMY_CANAL2_N, 0),
	/* 2 */ SONG_CON_P1(song_b_e_r___the_night_begins_to_shine__wip,
	                    b_e_r___the_night_begins_to_shine__wip_canal1,
	                    SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_CANAL1_N,
	                    b_e_r___the_night_begins_to_shine__wip_canal2,
	                    SONG_B_E_R___THE_NIGHT_BEGINS_TO_SHINE__WIP_CANAL2_N, 0),
	/* 3 */ SONG_SIN_P1(song_powerrangers, 0),
	/* 4 */ SONG_CON_P1(song_michael_jackson___beat_it,
	                    michael_jackson___beat_it_canal1, SONG_MICHAEL_JACKSON___BEAT_IT_CANAL1_N,
	                    michael_jackson___beat_it_canal2, SONG_MICHAEL_JACKSON___BEAT_IT_CANAL2_N,
	                    160),   /* 16 pistas: con 224 recorta */
	/* 5 */ SONG_CON_P1(song_pokemon_black_and_white_low_hp_midi_version,
	                    pokemon_black_and_white_low_hp_midi_version_canal1,
	                    SONG_POKEMON_BLACK_AND_WHITE_LOW_HP_MIDI_VERSION_CANAL1_N,
	                    pokemon_black_and_white_low_hp_midi_version_canal2,
	                    SONG_POKEMON_BLACK_AND_WHITE_LOW_HP_MIDI_VERSION_CANAL2_N, 0),
};

const uint8_t SONG_COUNT = sizeof(song_table) / sizeof(song_table[0]);
