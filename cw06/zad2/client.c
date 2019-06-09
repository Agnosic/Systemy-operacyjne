#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>


#include "chat.h"

pid_t childPID;
int ClientNumber;
mqd_t serverQueue;
mqd_t ClientQueue;
char clientPath[20];

Message waitForMessage(mqd_t queueID);
void sendMessage(mqd_t queueID, long type, char* text, long clientID, int textLen);
void handleCommands(char* command);
void handleExit();
void handleSIGINT(int signum);


int main(int argc, char* argv[]) {
    signal(SIGINT, handleSIGINT);
    atexit(handleExit);

    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MESSAGE_SIZE;

    sprintf(clientPath, "/%d", getpid());

    serverQueue = mq_open("/server", O_WRONLY);
    ClientQueue = mq_open(clientPath, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);

    char* text = "Client connected with server";
    sendMessage(serverQueue, INIT, text, getpid(), strlen(text));
    Message message = waitForMessage(ClientQueue);

    ClientNumber = message.clientID;

    printf("Client ID: %d\n", ClientNumber);

    if ((childPID = fork()) == 0) {
        while (1) {
            message = waitForMessage(ClientQueue);
            if (message.mType == STOP) {
                printf("Got message: \n\tType: %s \n\tClient ID: %ld\n\tText: %s\n", getTypeName(message.mType), message.clientID, message.text);
                kill(getppid(), SIGUSR1);
                exit(0);
            }
            else if (message.mType == ECHO){
                printf("Got message: \n\tType: %s \n\tClient ID: %ld\n\tText: %s\n", getTypeName(message.mType), message.clientID, message.text);
            }
        }
    } else {
        signal(SIGUSR1, handleSIGINT);
        char* command = calloc(MAX_MESSAGE_LENGTH, sizeof(char));
        if(argc == 2){
            FILE *fd;
            fd = fopen(argv[1], "r");
            char* input = malloc(MAX_FILE_SIZE);
            fread(input, sizeof(char), MAX_FILE_SIZE, fd);
            char* line = strtok(input, "\n");//
            while(line != NULL){
                handleCommands(line);
                line = strtok(NULL, "\n");
                sleep(1);
            }
            free(input);
        }
        while (1) {
            fgets(command, MAX_MESSAGE_LENGTH, stdin);
            handleCommands(command);
        }
        free(command);
    }
}

Message waitForMessage(mqd_t queueID) {
    Message message;
    if(mq_receive(queueID, (char*)&message, MAX_MESSAGE_SIZE, NULL) == -1) {
        fprintf(stderr, "Failed to get message from server %s", strerror(errno));
        exit(1);
    }

    return message;
        
}

void sendMessage(mqd_t queueID, long type, char* text, long clientID, int textLen) {
    Message message;
    message.mType = type;
    message.clientID = clientID;
    message.textLength = textLen;

    if(textLen > MAX_MESSAGE_LENGTH) {
        fprintf(stderr, "Too long message");
        exit(1);
    }
    memset(message.text, 0, 99);
    memcpy(message.text, text, textLen);

    printf("Send Message: \n\tType: %s \n\tClient: %ld\n\tText: %s\n", getTypeName(message.mType), message.clientID, message.text);

    if (mq_send(queueID, (char*)&message, MAX_MESSAGE_SIZE, (unsigned int)type) == -1){
        fprintf(stderr, "Failed to send a message\n");
        exit(1);
    }
}

void handleCommands(char* command) {
    if (strncmp(command, "ECHO", 4) == 0) {
        sendMessage(serverQueue, ECHO, command + 5, ClientNumber, strlen(command) - 5);
    }
    else if (strncmp(command, "LIST", 4) == 0) {
        char* text = "Clients list";
        sendMessage(serverQueue, LIST, text, ClientNumber, strlen(text));
    }
    else if (strncmp(command, "FRIENDS", 7) == 0) {
        sendMessage(serverQueue, FRIENDS, command + 8, ClientNumber, strlen(command)  - 8);
    }   
    else if(strncmp(command, "2ALL", 4) == 0) {
        sendMessage(serverQueue, TOALL, command + 5, ClientNumber, strlen(command)  - 5);
    }
    else if (strncmp(command, "2FRIENDS", 8) == 0) {
        sendMessage(serverQueue, TOFRIENDS, command + 9, ClientNumber, strlen(command)  - 9);
    }
    else if (strncmp(command, "2ONE", 4) == 0) {
        sendMessage(serverQueue, TOONE, command + 5, ClientNumber, strlen(command)  - 5);
    }
    else if (strncmp(command, "STOP", 4) == 0) {
        char* text = "STOP WORK";
        sendMessage(serverQueue, STOP, text, ClientNumber, strlen(text));
        exit(0);
    } 
    else if (strncmp(command, "ADD", 3) == 0) {
        sendMessage(serverQueue, ADD, command + 4, ClientNumber, strlen(command)  - 4);
    } 
    else if (strncmp(command, "DEL", 3) == 0) {
        sendMessage(serverQueue, DEL, command + 4, ClientNumber, strlen(command)  - 4);
    }
    else {
        printf("WRONG COMMAND!!: %s\n", command);
    }
}

void handleExit() {
    if (childPID == 0) {
        mq_close(ClientQueue);
        mq_unlink(clientPath);
        char* text = "STOP WORK";//
        sendMessage(serverQueue, STOP, text, ClientNumber, strlen(text));
        mq_close(serverQueue);
    }
    exit(0);
}

void handleSIGINT(int signum) {
    exit(0);
}

//