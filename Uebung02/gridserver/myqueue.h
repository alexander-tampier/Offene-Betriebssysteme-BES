//
// Created by Alexander Tampier on 03.07.18.
//

#ifndef GRIDSERVER_MYQUEUE_H
#define GRIDSERVER_MYQUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define KEY 420257991		/* eindeutiger Key z.B. Matrikelnummer */
#define PERM 0660

typedef struct
{
    //vehicle type A-Z (1-26) as Signaltype and (27-52) as response from server
    long mType;
    // North, East, South, West, Terminate
    char mCardinalPoints;
    //process ID of client
    pid_t mPid;

} message_t;

#endif //GRIDSERVER_MYQUEUE_H
