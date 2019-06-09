#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>



int main(int argc, char* argv[]){
	if(argc != 2){
		fprintf(stderr, "Wrong command\n");
		exit(1);
	}

	char line[80];

  //mknod(argv[1], S_IFIFO | S_IRUSR | S_IWUSR, 0);
  mkfifo(argv[1], S_IRUSR | S_IWUSR);
  FILE* pipe = fopen(argv[1], "r");
  if(!pipe){
    fprintf(stderr, "Failed to read pipe\n");
  }
  while(fgets(line, 80, pipe)){
    printf("%s", line);
  }

  fclose(pipe);
	return 0;
}