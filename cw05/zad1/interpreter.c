#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_ARGS 20
#define MAX_COMMANDS 10
#define MAX_LINES 200

struct command{
	char* name;
	char** args;
};

struct commands{
	struct command* list;
	int size;
};

struct stringv{
	char** list;
	int size;
};

struct stringv read_file(char* path);
struct commands get_commands_from_string(char* cmds_str);
struct command parse_command(char* command_str);
void execute_cmds(struct commands commands);

int main(int argc, char* argv[]){
	if(argc != 2){
		fprintf(stderr, "Wrong command\n");
		exit(1);
	}

	struct stringv commands_str = read_file(argv[1]);
	for(int i = 0; i < commands_str.size; i++){
		//printf("%d", cmds.size);
		struct commands cmds = get_commands_from_string(commands_str.list[i]);
		execute_cmds(cmds);
	}
	// for(int i = 0; i < cmds.size; i++){
	// 	wait(NULL);
	// }

	return 0;
}

struct commands get_commands_from_string(char* cmds_str){
	struct command* list = calloc(MAX_COMMANDS, sizeof(struct command));
	char* rest;

	char* cmd_str = strtok_r(cmds_str, "|\n", &rest);

	int count = 0;
	while(cmd_str != NULL){
		if(count < MAX_COMMANDS){
			list[count] = parse_command(cmd_str);
		}
		else{
			fprintf(stderr, "Toom many commands in one line\n");
		}
		cmd_str = strtok_r(rest, "|\n", &rest);
		count++;
	}
	struct commands cmds;
	cmds.list = list;
	cmds.size = count;
	return cmds;

}

struct command parse_command(char* cmd_str){
	char** args = calloc(MAX_ARGS + 1, sizeof(char*));
	char* rest;
	struct command cmd;
	char* arg = strtok_r(cmd_str, " ", &rest);

	for(int i = 0; arg != NULL; i++){
		if(i == 0){
			cmd.name = arg;
		}
		if(i < MAX_ARGS){
			args[i] = arg;
		}
		else{
			fprintf(stderr, "Toom many arguments in one command\n");
		}
		arg = strtok_r(rest, " ", &rest);
	}
	cmd.args = args;
	return cmd;
}

void execute_cmds(struct commands cmds){
	int fd[2][2];

  	int i;
  	for(i = 0; i < cmds.size; i++){
		struct command cmd = cmds.list[i];
		if(i > 0){
			close(fd[i % 2][0]);
			close(fd[i % 2][1]);
		} 
		pipe(fd[i % 2]);
		pid_t pid = fork();

    	if(pid == 0){
			if(i != cmds.size -1){                //sprawdza czy to nie jest ostatnia komenda
				close(fd[i % 2][0]);
				dup2(fd[i % 2][1], STDOUT_FILENO); //zapisuje wynik komendy
			}
			if(i != 0){
				close(fd[(i-1)%2][1]);           // czyta wynik poprzedniej komendy
				dup2(fd[(i-1)%2][0], STDIN_FILENO);
			}
			
			execvp(cmd.name, cmd.args);
			exit(0);
		}

	}
	close(fd[(i-1)%2][0]);
	close(fd[(i-1)%2][1]);
	wait(NULL);
}

struct stringv read_file(char* path){
  	FILE* file = fopen(path, "r");
	if(!file){
		fprintf(stderr, "%s", strerror(errno));
		exit(1);
	}

	char** list = calloc(MAX_LINES, sizeof(char*));
	char* buffer = calloc(2001, sizeof(char)); //max line char
	int count = 0;
	while(fgets(buffer, 2000, file)){
		if(count == MAX_LINES){
			fprintf(stderr, "Too many lines in file\n");
		}
		list[count] = calloc(strlen(buffer), sizeof(char));
		strcpy(list[count], buffer);
		count++;
	}

	struct stringv commands_str;
	commands_str.list = list;
	commands_str.size = count;
	free(buffer);
	fclose(file);
	return commands_str;
}
