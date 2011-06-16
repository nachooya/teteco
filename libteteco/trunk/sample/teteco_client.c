#include <stdio.h>
#include <unistd.h>
#include "../include/teteco.h"


int main (int argc, char *argv[]) {

    teteco_init ();

    int*   devices_index = NULL;
    char** devices = NULL;

    int devices_num = teteco_get_out_devices (&devices_index, &devices);

    int i;
    for (i = 0; i < devices_num; i++) {
        printf ("Device %d [%d]:%s\n", i, devices_index[i], devices[i]);
    }

    teteco_t* teteco = teteco_start (TETECO_NET_CLIENT,
                                     22223,
                                     22222,
                                     "127.0.0.1",
                                     0,
                                     devices_index[0],
                                     TETECO_AUDIO_RECEIVER,
                                     TETECO_SPEEX_NB,
                                     7,
                                     "/home/nacho/teteco");


    pause ();

    return 0;
}