#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

void handleMyFind(char *searchPath[], char *filesVar[], int recursiveFlag, int ignoreCapitalization, int n);

char* convertCapitalLetters(char toLower[]);

int main(int argc, char **argv) {
    int c;
    int recursive = 0;
    int ignoreCapitalization = 0;
    char *searchPath[100];

    while ((c = getopt(argc, argv, "iR")) != -1)
        switch (c) {
            case 'R':
                recursive = 1;
                break;
            case 'i':
                ignoreCapitalization = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-R] [-i] searchpath filename1 [filename2] ...[filenamen]\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }

    *searchPath = argv[optind];

    char *files[argc - (optind + 1)];

    int index = optind + 1;
    int n = sizeof(files) / sizeof(files[0]);
    for (int i = 0; i < n; i++) {
        files[i] = argv[index];
        index++;
    }

    handleMyFind(&searchPath, &files, recursive, ignoreCapitalization, n);

    pid_t pid = getpid();

    printf("PID before fork(): %d\n", pid);
    pid = fork();

    switch(pid) {
        case -1:
            printf("Fehler beim Erzeugen eines Kindprozesses.\n");
            return 1;
            break;
        case 0: // Kindprozess
            sleep(1);
            //printf("Kindprozess pid: %d\n", pid);
            printf("Kindprozess getpid(): %d\n", getpid());
            printf("-----------\n");
            fflush(stdout);
            exit (0);
            break;
        default: // Elternprozess
            sleep(1);
            //printf("Elternprozess pid: %d\n", pid);
            printf("Elternprozess getpid(): %d\n", getpid());

            pid = fork();

            if (pid == 0) {
                //printf("Kindprozess pid: %d\n", pid);
                printf("Kindprozess getpid(): %d\n", getpid());
                exit (0);
            }
            else if (pid == -1) {
                return 1;
            }
            else {
                //printf("Elternprozess pid: %d\n", pid);
                while(wait(NULL)>0);
                printf("Elternrozess getpid(): %d\n", getpid());

                printf("WAITED\n");
            }

            printf("-----------\n");
            fflush(stdout);
            break;
    }

    return 0;
}

void handleMyFind(char *searchPath[], char *filesVar[], int recursiveFlag, int ignoreCapitalization, int n) {
    if (ignoreCapitalization != 0) {
        char testString[] = "CapitALleTTERtest";
        *testString = convertCapitalLetters(testString);
    }
    /*
    printf("Searchpath: %s\n", *searchPath);

    for (int i = 0; i < n; i++) {
        printf("%s\n", filesVar[i]);
    }
    */
}

char* convertCapitalLetters(char toLower[]) {
    for (int i = 0; i < strlen(toLower); i++) {
        unsigned int asciiNum = toLower[i];
        if (asciiNum >= 65 && asciiNum <= 90) {
            toLower[i] += (97 - 65);
        }
    }

    return toLower;
}
