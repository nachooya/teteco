#include <stdio.h>
#include <unistd.h>
#include "../include/teteco.h"


int main (int argc, char *argv[]) {

    teteco_init ();

    int*   devices_index = NULL;
    char** devices = NULL;
    char*  message = "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";

    int devices_num = teteco_get_in_devices (&devices_index, &devices);

    int i;
    for (i = 0; i < devices_num; i++) {
        printf ("Device %d [%d]:%s\n", i, devices_index[i], devices[i]);
    }

    teteco_t* teteco = teteco_start (TETECO_NET_SERVER,
                                     30000,
                                     11111,
                                     "127.0.0.1",
                                     devices_index[0],
                                     devices_index[0],
                                     TETECO_AUDIO_SENDER,
                                     TETECO_SPEEX_NB,
                                     7,
                                     "/home/nacho/teteco");

    while (1) {
        sleep (5);
        teteco_chat_send (teteco, message);
    }

    return 0;
}