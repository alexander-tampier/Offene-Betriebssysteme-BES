#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int main(int argc, char **argv) {
    pthread_mutex_t mutex;

    int c;
    int recursive = 0;
    int ignoreCapitalization = 0;
    char *searchPath[100];

    //read the parameter with a minus form command
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

        //optind is int after the parameters (when other arguments start)
    *searchPath = argv[optind];
//files to search (argc is all option parameters incl programm name)
    char *files[argc - (optind + 1)];

    //initialize array of files with given parameter(files)
    int index = optind + 1;
    int n = sizeof(files) / sizeof(files[0]);
    for (int i = 0; i < n; i++) {
        files[i] = argv[index];
        index++;
    }

    //procesess...
    pid_t pid;

    //iterate through files and start X child processes
    for(int i=0;i<n;i++) {
        pid = fork();

        if (pid == 0) {
            /* Kindprozess
             * wenn fork() eine 0 zurückgibt, befinden wir uns im Kindprozess
             */
            char myLocalSearchPath[100];
            char myLocalFileToFind[100];

            //struct for directory operations
            struct dirent *direntp;
            DIR *dirp;

            //mutexes locked and unlocked while children could try to access same
            //resources at the SAME time, so they save them locally
            pthread_mutex_lock(&mutex);
            strcpy(myLocalSearchPath, *searchPath);
            strcpy(myLocalFileToFind, files[i]);
            int localIgnoreCapitalization = ignoreCapitalization;
            pthread_mutex_unlock(&mutex);

            //errorhandling directory open
            if ((dirp = opendir(*searchPath)) == NULL) {
                perror("Failed to open directory");
                return 1;
            }

            //dir opened, while until EOF dir
                    //readdir -> reads next/current file
            while ((direntp = readdir(dirp)) != NULL) {
                //if -i is true (strCASEcmp)
                if (localIgnoreCapitalization != 0) {

                    //comparing local file to find with the opened directories files
                    if (strcasecmp(direntp->d_name, myLocalFileToFind) == 0) {

                        //mutex lock for printf (outputstream)
                        pthread_mutex_lock(&mutex);
                        printf("++Process: %d -> Found File directory: %s%s\n", getpid(), myLocalSearchPath,
                               direntp->d_name);
                        printf("%d:%s:%s%s\n\n",getpid(), direntp->d_name, myLocalSearchPath, direntp->d_name);
                        pthread_mutex_unlock(&mutex);
                    }

                    //same without -i (not true)
                } else if (strcmp(myLocalFileToFind, direntp->d_name) == 0) {
                    pthread_mutex_lock(&mutex);
                    printf("++Process: %d -> Found File directory: %s%s\n", getpid(), myLocalSearchPath,
                           direntp->d_name);
                    printf("%d:%s:%s%s\n\n",getpid(), direntp->d_name, myLocalSearchPath, direntp->d_name);
                    pthread_mutex_unlock(&mutex);
                }
            }
            //wait for file close
            while ((closedir(dirp) == -1) && (errno == EINTR));
            //flush to delete remaining buffer of outputstream
            fflush(stdout);
            exit(0);
        } else if (pid > 0) {
            fflush(stdout);
        }
    }
    //wait for all children - asynchron
    while (wait(NULL) > 0) {}
    printf("Parentid: %d\n",getpid());
    printf("WAITED");

    return 0;
}
