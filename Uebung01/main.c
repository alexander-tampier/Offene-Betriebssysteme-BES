#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>


int main(int argc, char **argv) {
    int c;
    int Recursive = 0;
    int ignoreCapitalization = 0;
    char *path[100];

    while ((c = getopt(argc, argv, "iR")) != -1)
        switch (c) {
            case 'R':
                Recursive = 1;
                printf("-R\n");
                break;
            case 'i':
                ignoreCapitalization = 1;
                printf("-i\n");
                break;
            default:
                fprintf(stderr, "Usage: %s [-R] [-i] searchpath filename1 [filename2] ...[filenamen]\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }

    printf("name argument = %s\n", argv[optind]);
    *path=argv[optind];

    char* files[argc-optind+1];

    for(int i=optind+1;i<argc;i++){
        files[i] = argv[i];
        printf("%s\n", files[i]);
    }

    return 0;
}
