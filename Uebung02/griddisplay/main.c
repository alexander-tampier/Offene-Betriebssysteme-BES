#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX 256

void myhandler(int sig)
{
    printf("CTRL+C (Sig Nr:%d)ignored...\n",sig);
    fflush(stdout);
    exit(0);
}

int main() {
    FILE *fp;
    char *myfifo = "/tmp/myfifo";
    char puffer[MAX];

   (void) signal(SIGINT,myhandler);
while(1){
   if ((fp=fopen(myfifo,"r"))!=NULL) {
      while (fgets(puffer, MAX, fp) != NULL) 
      {
         printf("%s",puffer);
      }
      printf("\n");
   }
    fclose (fp); 
}
}