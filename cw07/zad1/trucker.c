#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/types.h> 
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include "belt.h"

int BELT_CAP = 10
int BELT_WGHT_LMT = 100
int TRUCK_LMT = 200

int main(int argc, char **argv){
    key_t belt_cap_key = ftok(getenv("HOME"), CAP_SEED);
    key_t belt_wght_key = ftok(getenv("HOME"), WGTH_LMT_SEED);
    key_t shm_key = ftok(getenv("HOME"), SHM_SEED);

}