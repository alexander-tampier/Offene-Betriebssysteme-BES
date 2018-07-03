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

//pipe creation fifo filedirectory for communications with other process
char *myfifo = "/tmp/myfifo";

//global for sigkill (message Queue) hanlder
int msgid = -1;    /* Message Queue ID */

//signal handler
void myhandler(int sig) {
    printf("CTRL+C (Sig Nr:%d)ignored...\n", sig);
    fflush(stdout);
    // TODO - send SigKILL to all vehicles

    //kill message Queue
    if(msgctl(msgid, IPC_RMID, NULL)==-1){
        printf("could not kill message Queue!?");
    };

    //kill FIFO (named pipe) to display
    remove(myfifo);

    exit(0);
}


/* globale Variable fuer den Programmnamen */
char *program_name = NULL;

typedef struct grid {
    int width;
    int height;
    char **grid;

} grid;

struct grid *initializeGrid(int width, int height) {
    struct grid *finalGrid = malloc(sizeof(struct grid));
    finalGrid->height = height;
    finalGrid->width = width;

    finalGrid->grid = malloc(finalGrid->height * sizeof(char *));
    for (int i = 0; i < finalGrid->width; i++) {
        finalGrid->grid[i] = (char *) malloc(finalGrid->height * sizeof(char));
    }

    for (int i = 0; i < finalGrid->width; i++) {
        for (int j = 0; j < finalGrid->height; j++) {
            if (i == 0 || j == 0) {
                finalGrid->grid[i][j] = '#';
            } else if (i == (finalGrid->width - 1) || j == (finalGrid->height - 1)) {
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

void printToDisplay(grid *finalGrid, FILE *file) {
    for (int i = 0; i < finalGrid->width; i++) {
        for (int j = 0; j < finalGrid->height; j++) {
            fprintf(file, "%c", finalGrid->grid[i][j]);
        }
        fprintf(file, "\n");
    }
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
    program_name = argv[0];

    //message Queue initialization
    message_t msg;    /* Buffer fuer Message */


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
    grid *myGrid = initializeGrid(width + 1, height + 1);
    //printGrid(myGrid);

    FILE *fp;

    if (mkfifo(myfifo, 0666) == -1) {
        fprintf(stderr, "myfifo: Error creating fifo\n");
        return EXIT_FAILURE;
    }


    while (1) {
        /*
         * Read messages from gridclient
         */
        if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), 0, 0) == -1) {
            /* error handling */
            fprintf(stderr, "%s: Can't receive from message queue\n", argv[0]);
            return EXIT_FAILURE;
        }
        printf("Our first awesome Client: %ld\n", msg.mType);

        //writing to display through named pipe to griddisplay program
        if ((fp = fopen(myfifo, "w")) != NULL) {
            for (int j = 0; j < myGrid->height; j++) {
                fprintf(fp, "%s\n", myGrid->grid[j]);
            }
        }
        fclose(fp);

        sleep(2);
    }
}