//
// Created by Alexander Tampier on 03.07.18.
//

#ifndef GRIDSERVER_DRIVER_H
#define GRIDSERVER_DRIVER_H

#include <ntsid.h>

typedef struct {
    // name of the driver
    char name;
    // x position of grid
    int x;
    // y position of grid
    int y;
    // processId of the vehicle
    pid_t processId;
} driver;

#endif //GRIDSERVER_DRIVER_H
