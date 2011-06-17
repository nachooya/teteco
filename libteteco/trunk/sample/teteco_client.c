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
                                     11111,
                                     30000,
                                     "127.0.0.1",
                                     devices_index[0],
                                     devices_index[0],
                                     TETECO_AUDIO_RECEIVER,
                                     TETECO_SPEEX_NB,
                                     7,
                                     "/home/nacho/teteco");


    while (1) {
        sleep (2);
        teteco_chat_send (teteco, "hola");
    }

    return 0;
}