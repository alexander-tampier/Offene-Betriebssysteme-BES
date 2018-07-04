#include <stdio.h>
#include <unistd.h>
#include "myqueue.h"

/* Funktion print_usage() zur Ausgabe der usage Meldung */
void print_usage(char *program_name) {
    fprintf(stderr, "Usage: %s [A-Z]\n", program_name);
    exit(EXIT_FAILURE);
}

void myhandler(int sig) {
    printf("Killed from server (Sig Nr:%d)\n", sig);
    fflush(stdout);
    exit(0);
}

int main(int argc, char *argv[]) {
    pid_t myPid = getpid();
    message_t msg;    /* Buffer fuer Message */
    int msgid = -1;    /* Message Queue ID */
    char input;
    int inputValid = 0;

    (void) signal(SIGINT, myhandler);

    if ((msgid = msgget(KEY, PERM)) == -1) {
        // error handling
        fprintf(stderr, "%s: Can't access message queue\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Argument Handling */
    if (argc != 2) print_usage(argv[0]);

    msg.mPid = myPid;
    msg.mType = *argv[1] - 64;
    msg.mCardinalPoints = '\0';

    if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        // error handling
        fprintf(stderr, "Can't send message\n");
        return EXIT_FAILURE;
    }

    if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), (*argv[1] - 64) + 26, 0) == -1) {
        /* error handling */
        fprintf(stderr, "Can't receive from message queue\n");
        return EXIT_FAILURE;
    }

    // K .. server sends an okay flag with cardinalPoint K
    if (msg.mCardinalPoints == 'K') {
        printf("Message confirmed for the vehicle: %c\n", *argv[1]);


        while (1) {
            do {
                printf("Please input a cardinal point: ");
                //scanf("%c", &input);
                input = getchar();
                int t;
                while ( (t=getchar())!='\n' && t!=EOF );
                if (input == 'N' || input == 'E' || input == 'S' || input == 'W' || input == 'T') {
                    inputValid = 1;
                } else {
                    printf("Usage [N,E,S,W] to move || [T] to terminate\n");
                }
            } while (inputValid != 1);

                inputValid = 0;

                // Message Queue oeffnen


                // Nachricht verschicken
                msg.mType = *argv[1] - 64;
                msg.mCardinalPoints = input;
                if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
                    // error handling
                    fprintf(stderr, "Can't send message\n");
                    return EXIT_FAILURE;
                }
                printf("Message sent: %c\n", msg.mCardinalPoints);
                input = '\0';
            }

            return EXIT_SUCCESS;
        }

    }