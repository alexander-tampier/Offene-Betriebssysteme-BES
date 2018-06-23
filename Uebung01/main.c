#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    int c;
    int recursive = 0;
    int ignoreCapitalization = 0;
    char *searchPath[100];

    while ((c = getopt(argc, argv, "iR")) != EOF)
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

    pid_t pid = getpid();

    printf("PID before fork(): %d\n", pid);
    pid = fork();

    if (pid == 0) {
        /* Kindprozess
         * wenn fork() eine 0 zurückgibt, befinden wir uns im Kindprozess
         */
        char myLocalSearchPath[100];
        char myLocalFileToFind[100];

        struct dirent *direntp;
        DIR *dirp;

        strcpy(myLocalSearchPath, *searchPath);
        strcpy(myLocalFileToFind, files[0]);

        if ((dirp = opendir(*searchPath)) == NULL) {
            perror("Failed to open directory");
            return 1;
        }

        while ((direntp = readdir(dirp)) != NULL){
            if(ignoreCapitalization!=0){
                if(strcasecmp(direntp->d_name, myLocalFileToFind) == 0)
                    printf("++Process: %d -> Found File directory: %s %s\n", getpid(), myLocalSearchPath, direntp->d_name);
            }else if(strcmp(myLocalFileToFind, direntp->d_name) == 0){
                printf("%s\n", direntp->d_name);

            }
        }
        while ((closedir(dirp) == -1) && (errno == EINTR));

        sleep(1);
        //printf("Kindprozess pid: %d\n", pid);
        printf("Kindprozess getpid(): %d\n", getpid());
        printf("-----------\n");
        fflush(stdout);
        exit(0);
    } else if (pid > 0) {
        /* Elternprozess
         * Gibt fork() einen Wert größer 0 zurück, befinden wir uns im Elternprozess
         * in pid steht die ID des Kindprozesses
         * getpid() gibt die eigene PID zurück
         */
        printf("Elternprozess getpid(): %d\n", getpid());
        sleep(1);
        printf("-----------\n");
        //wait for all children - asynchron
        while (wait(NULL) > 0) {}
        printf("WAITED");
        fflush(stdout);
    }

    return 0;
}
