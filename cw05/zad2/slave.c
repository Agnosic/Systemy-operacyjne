#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>


int main(int argc, char* argv[]){
	if(argc != 3){
		fprintf(stderr, "Wrong command\n");
		exit(1);
	}
	srand(time(NULL));

	int count = atoi(argv[2]);
	const char* cmd = "date";
	char data_str[50];
	char pipe_str[80];
	int pid = getpid();
	printf("%d\n", pid);

	int pipe = open(argv[1], O_WRONLY);
	if(pipe < 0){
		fprintf(stderr, "%s", strerror(errno));
		exit(1);
	}
	for(int i = 0; i < count; i++){
		FILE *datefile = popen(cmd, "r");
		fgets(data_str, 50, datefile);

		sprintf(pipe_str, "PID: %d _ %s", pid, data_str);
		pclose(datefile);
		if(write(pipe, pipe_str, strlen(pipe_str)) == (-1)){
			fprintf(stderr, "%s", strerror(errno));
			exit(1);
		}
		sleep(rand()%3 + 2);

	}

	close(pipe);
	return 0;
}
