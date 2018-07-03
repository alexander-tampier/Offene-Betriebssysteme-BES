#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "myqueue.h"
#include "driver.h"
#include "grid.h"

/*
 * PIPE
 */
//pipe creation fifo filedirectory for communications with other process
char *myfifo = "/tmp/myfifo";

/*
 * MESSAGE QUEUE
 */
//global for sigkill (message Queue) hanlder
int msgid = -1;    /* Message Queue ID */

/*
 * DRIVER
 */
const int DRIVER_SIZE = 26;
driver *drivers;

/*
 * GRID
 */
grid *myGrid;

/*
 * PROGRAM
 */
/* globale Variable fuer den Programmnamen */
char *program_name = NULL;


//signal handler
void myhandler(int sig) {
    printf("CTRL+C (Sig Nr:%d)ignored...\n", sig);
    fflush(stdout);
    // TODO - send SigKILL to all vehicles

    //driver *killDriver;

    for (int i = 0; i < DRIVER_SIZE; i++) {
        //*killDriver = drivers[i];
        if (drivers[i].name >= 65 && drivers[i].name <= (65 + 26)) {
            kill(drivers[i].processId, SIGKILL);
        }
    }

    //kill message Queue
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        printf("could not kill message Queue!?");
    } else {
        printf("Queue killed!");
    };

    //free/dealloc Grid
    for (int i = 0; i < myGrid->width; i++) {
        free(myGrid->grid[i]);
    }
    free(myGrid);
    free(drivers);

    //kill FIFO (named pipe) to display
    remove(myfifo);

    exit(0);
}

grid *initializeGrid(int width, int height) {
    grid *finalGrid = malloc(sizeof(grid));
    finalGrid->height = height;
    finalGrid->width = width;

    finalGrid->grid = malloc(finalGrid->width * sizeof(char *));
    for (int i = 0; i < finalGrid->height; i++) {
        finalGrid->grid[i] = (char *) malloc(finalGrid->width * sizeof(char));
    }

    for (int i = 0; i < finalGrid->height; i++) {
        for (int j = 0; j < finalGrid->width; j++) {
            if (i == 0 || j == 0) {
                finalGrid->grid[i][j] = '#';
            } else if (i == (finalGrid->height - 1) || j == (finalGrid->width - 1)) {
                finalGrid->grid[i][j] = '#';
            } else {
                finalGrid->grid[i][j] = '.';
            }
        }
    }
    return finalGrid;
};

void printGrid(grid *finalGrid) {
    for (int i = 0; i < finalGrid->width; i++) {
        for (int j = 0; j < finalGrid->height; j++) {
            printf("%c", finalGrid->grid[i][j]);
        }
        printf("\n");
    }
}

void printToDisplay(grid *finalGrid, FILE *fp) {
    //writing to display through named pipe to griddisplay program
    if ((fp = fopen(myfifo, "w")) != NULL) {
        for (int j = 0; j < finalGrid->height; j++) {
            fprintf(fp, "%s\n", finalGrid->grid[j]);
        }
    }
    fclose(fp);

    sleep(2);
}

//initialize the driver array
driver *initializeDrivers(driver *drivers) {

    drivers = malloc(DRIVER_SIZE * sizeof(*drivers));

    return drivers;
}

/* Funktion print_usage() zur Ausgabe der usage Meldung */
void print_usage() {
    fprintf(stderr, "Usage: %s [-x width] [-y height]\n", program_name);
    exit(EXIT_FAILURE);
}


/* main Funktion mit Argumentbehandlung */
int main(int argc, char *argv[]) {
    int c;
    int width;
    int height;
    int error = 0;
    int optionx = 0;
    int optiony = 0;
    int stopNestedFor = 0;
    program_name = argv[0];


    //message Queue initialization
    message_t msg;    /* Buffer fuer Message */

    drivers = initializeDrivers(drivers);

    /* Message Queue neu anlegen */
    if ((msgid = msgget(KEY, PERM | IPC_CREAT | IPC_EXCL)) == -1) {
        /* error handling */
        fprintf(stderr, "%s: Error creating message queue\n", argv[0]);
        return EXIT_FAILURE;
    }



    //Signal handler call!
    (void) signal(SIGINT, myhandler);

    while ((c = getopt(argc, argv, "x:y:")) != EOF) {
        switch (c) {
            case 'x': /* Option mit Argument */
                if (optionx) /* mehrmalige Verwendung? */
                {
                    error = 1;
                    break;
                }
                optionx = 1;
                width = atoi(optarg);
                break;
            case 'y': /* Option mit Argument */
                if (optiony) /* mehrmalige Verwendung? */
                {
                    error = 1;
                    break;
                }
                optiony = 1;
                height = atoi(optarg); /* optarg zeigt auf Optionsargument */
                break;
            case '?': /* unguelgtiges Argument */
                error = 1;
                break;
            default: /* unmoegliech */
                assert(0);
        }
    }
    if (error) /* Optionen fehlerhaft ? */
    {
        print_usage();
    }
    if (argv[optind] || optind < 5) {
        print_usage();
    }
    // +2 # for the outer border
    myGrid = initializeGrid(width + 2, height + 2);

    FILE *fp;

    if (mkfifo(myfifo, 0666) == -1) {
        fprintf(stderr, "myfifo: Error creating fifo\n");
        return EXIT_FAILURE;
    }

    printToDisplay(myGrid, fp);

    while (1) {
        /*
         * Read messages from gridclient
         */
        if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), -26, 0) == -1) {
            /* error handling */
            fprintf(stderr, "%s: Can't receive from message queue\n", argv[0]);
            return EXIT_FAILURE;
        } else {
            // CardinalPoint is empty -> therefore register the current vehicle from client
            if (msg.mCardinalPoints == '\0') {
                // check if driver has already been created
                for (int i = 0; i < DRIVER_SIZE; i++) {
                    if (drivers[i].name == (char) (msg.mType + 64)) {
                        kill(msg.mPid, SIGKILL);
                        msg.mType = -1;
                        break;
                    }
                }

                // new driver wants to register
                if (msg.mType != -1) {
                    driver localDriver;
                    localDriver.name = (char) (msg.mType + 64);
                    localDriver.processId = msg.mPid;
                    //look for empty position on the grid
                    for (int i = 0; i < myGrid->width; i++) {
                        for (int j = 0; j < myGrid->height; j++) {
                            if (myGrid->grid[i][j] == '.') {
                                localDriver.x = i;
                                localDriver.y = j;
                                myGrid->grid[i][j] = localDriver.name;
                                stopNestedFor = 1;
                                break;
                            }
                        }
                        if (stopNestedFor) {
                            stopNestedFor = 0;
                            break;
                        }
                    }

                    drivers[msg.mType - 1] = localDriver;
                    printToDisplay(myGrid, fp);
                }
            } else {
                int oldPositionX;
                int newPositionX;
                int oldPositionY;
                int newPositionY;
                driver driver;

                switch (msg.mCardinalPoints) {
                    case 'N':

                        break;
                    case 'E':
                        break;
                    case 'S':
                        oldPositionX = drivers[msg.mType - 1].x;
                        newPositionX = oldPositionX;
                        oldPositionY = drivers[(msg.mType - 1)].y;
                        newPositionY = drivers[(msg.mType - 1)].y += 1;

                        //# or driver
                        if (myGrid->grid[oldPositionX][newPositionY] != '.') {
                            kill(msg.mPid, SIGKILL);
                            drivers[msg.mType - 1].name = '\0';
                            myGrid->grid[oldPositionX][oldPositionY] = '.';

                            //only when driver
                            if (myGrid->grid[newPositionX][newPositionY] != '#') {
                                driver = drivers[myGrid->grid[oldPositionX][oldPositionY] - 65];
                                kill(driver.processId, SIGKILL);
                                driver.name = '\0';
                                myGrid->grid[oldPositionX][oldPositionY] = '.';
                            }
                        } else {
                            myGrid->grid[newPositionX][newPositionY] = (char) (msg.mType + 64);
                            myGrid->grid[oldPositionX][oldPositionY] = '.';
                            printf("x: %d | y: %d - New Position in grid %c", newPositionX, newPositionY,
                                   myGrid->grid[newPositionX][newPositionY]);
                        }


                        break;
                    case 'W':
                        break;
                    case 'T':
                        break;
                }
                printToDisplay(myGrid, fp);
            }

            printf("Our first awesome Client: %c\n", (char) (msg.mType + 64));


            //CHANNEL RECEIVED AND SEND ACK FLAG
            if (msg.mType != -1) {
                msg.mType += 26;
                msg.mCardinalPoints = 'K';
                if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
                    /* error handling */
                    fprintf(stderr, "%s: Can't send message\n", argv[0]);
                    return EXIT_FAILURE;
                }
            }
        }

    }
}