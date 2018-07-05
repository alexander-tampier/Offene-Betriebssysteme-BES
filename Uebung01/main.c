#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int
searchFiles(const char *searchPath, int localRecursive, int localIgnoreCapitalization, char *myLocalFileToFind) {
    //struct for directory operations
    struct dirent *direntp;
    DIR *dirp;

    //errorhandling directory open
    if (!(dirp = opendir(searchPath))) {
        perror("Failed to open directory");
        return 0;
    }


    //dir opened, while until EOF dir
    //readdir -> reads next/current file
    while ((direntp = readdir(dirp)) != NULL) {
        //if -i is true (strCASEcmp)
        if (localIgnoreCapitalization != 0) {

            //comparing local file to find with the opened directories files
            if (strcasecmp(direntp->d_name, myLocalFileToFind) == 0) {

                //mutex lock for printf (outputstream)
                printf("++Process: %d -> Found File directory: %s/%s\n", getpid(), searchPath,
                       direntp->d_name);
                //flush to delete remaining buffer of outputstream
                fflush(stdout);
                //printf("%d:%s:%s%s\n\n",getpid(), direntp->d_name, myLocalSearchPath, direntp->d_name);
                //wait for file close
                while ((closedir(dirp) == -1) && (errno == EINTR));

                return 0;
            }

            //same without -i (not true)
        } else if (strcmp(myLocalFileToFind, direntp->d_name) == 0) {
            printf("++Process: %d -> Found File directory: %s/%s\n", getpid(), searchPath,
                   direntp->d_name);
            //flush to delete remaining buffer of outputstream
            fflush(stdout);
            //printf("%d:%s:%s%s\n\n",getpid(), direntp->d_name, myLocalSearchPath, direntp->d_name);
            //wait for file close
            while ((closedir(dirp) == -1) && (errno == EINTR));

            return 0;
        }

        if (direntp->d_type == DT_DIR && localRecursive && direntp->d_name[0] != '.' && direntp->d_name[1] != '.') {
            char newPath[1024];
            //strcpy(newPath, direntp->d_name);
            //printf("Hello World: %s\n", newPath);
            snprintf(newPath, sizeof(newPath), "%s/%s", searchPath, direntp->d_name);
            return searchFiles(newPath, localRecursive, localIgnoreCapitalization, myLocalFileToFind);

        }

    }
    //wait for file close
    while ((closedir(dirp) == -1) && (errno == EINTR));

    return 0;
}

int main(int argc, char **argv) {
    int c;
    int recursive = 0;
    int ignoreCapitalization = 0;

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
    for (int i = 0; i < n; i++) {
        pid = fork();

        if (pid == 0) {
            /* Kindprozess
             * wenn fork() eine 0 zurückgibt, befinden wir uns im Kindprozess
             */
            char myLocalFileToFind[1024];



            //mutexes locked and unlocked while children could try to access same
            //resources at the SAME time, so they save them locally
            strcpy(myLocalFileToFind, files[i]);
            int localIgnoreCapitalization = ignoreCapitalization;
            int localRecursive = recursive;


            //remove slash
            char *p = argv[optind];
            p[strlen(argv[optind]) - 1] = 0;
            //printf("p: %s\n", p);

            return searchFiles(p, localRecursive, localIgnoreCapitalization, myLocalFileToFind);


        } else if (pid > 0) {
        }
    }
    //wait for all children - asynchron
    sleep(1);
    while (wait(NULL) > 0) {}
    printf("Parentid: %d\n", getpid());
    printf("WAITED");

    return 0;
}
