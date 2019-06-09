#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "chat.h"

int serverQueue;
int lastClientIndex = 0;
int clientsQueues[MAX_CLIENTS];
int lastFriendIndex = -1;
int friends[MAX_CLIENTS];

Message waitForMessage(int queueID);
void sendMessage(int queueID, long type, char* text, long clientID, int textLen);
void handleMessage(Message message);
void handleINIT(Message message);
void handleECHO(Message message);
void handleTOONE(Message message);
void handleLIST(Message message);
void handleADD(Message message);
void handleTOALL(Message message);
void handleFRIENDS(Message message);
void handleTOFRIENDS(Message message);
void handleDEL(Message message);
void handleSTOP(Message message);
char* connectTime(Message* message);
char* connectClientTime(Message* message);
void handleExit();
void handleSIGINT();


int main() {
    atexit(handleExit);
    signal(SIGINT, handleSIGINT);

    key_t serverKey = ftok(getenv("HOME"), 'h');

    for(int i = 0; i<MAX_CLIENTS; i++){
        friends[i] = -1;
    }

    if ((serverQueue = msgget(serverKey, IPC_CREAT | 0666)) == -1) {
        fprintf(stderr, "Wystąpił problem z tworzeniem kolejki dla serwera");
        exit(1);
    } else {
        printf("SERVER ID %d\n", serverQueue);

        while(1) {
            Message Message = waitForMessage(serverQueue);
            printf("Got message: \n\tType: %s \n\tClient: %ld\n\tText: %s\n", getTypeName(Message.mType), Message.clientID, Message.text);
            handleMessage(Message);
        }
    }
}

Message waitForMessage(int queueID) {
    Message message;
    if(msgrcv(queueID, &message, MAX_MESSAGE_SIZE, 0, 0) == -1) {
        fprintf(stderr, "Failed to get message from server");
        exit(1);
    }

    return message;
        
}

void sendMessage(int queueID, long type, char* text, long clientID, int textLen) {
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

    if (msgsnd(queueID, &message, MAX_MESSAGE_SIZE, -1) == -1){
        fprintf(stderr, "Failed to send a message");
        exit(1);
    }
}

void handleMessage(Message message) {
    switch (message.mType) {
        case ECHO:
            handleECHO(message);
            break;
        case LIST:
            handleLIST(message);
            break;
        case FRIENDS:
            handleFRIENDS(message);
            break;
        case TOALL:
            handleTOALL(message);
            break;
        case TOFRIENDS:
            handleTOFRIENDS(message);
            break;
        case TOONE:
            handleTOONE(message);
            break;
        case STOP:
            handleSTOP(message);
            break;
        case INIT:
            handleINIT(message);
            break;
        case ADD:
            handleADD(message);
            break;
        case DEL:
            handleDEL(message);
            break;
    }
}

void handleECHO(Message message) {
    char* text = connectTime(&message);
    sendMessage(clientsQueues[message.clientID], ECHO, text, 0, message.textLength);
    free(text);
}

void handleLIST(Message message) {
    char* result = calloc((lastClientIndex) * 15, sizeof(char));
    for(int i=0; i<lastClientIndex; ++i) {
        char part[10];
        sprintf(part, "Clnt:%d | ", i);
        strcat(result, part);
    }
    sendMessage(clientsQueues[message.clientID], ECHO, result, 0, (lastClientIndex) * 10);
}

void handleFRIENDS(Message message) {
    lastFriendIndex = -1;
    for(int i = 0; i<MAX_CLIENTS; i++) {
        friends[i] = -1;
    }
    
    handleADD(message);
}

void handleADD(Message message) {
    char* friendsList = calloc(message.textLength, sizeof(char));
    memcpy(friendsList, message.text, message.textLength);
    char *friend = strtok(friendsList, " ");

    while(friend != NULL) {
        int friendID = atoi(friend);
        lastFriendIndex++;

        for(int i = 0; i<lastFriendIndex; i++) {
            if(friends[i] == friendID) {
                fprintf(stderr, "You can't add two same friends");
                exit(1);
            }
        }
        friends[lastFriendIndex] = friendID;

        friend = strtok(NULL, " ");
    }
    free(friendsList);
}


void handleDEL(Message message) {
    char* friendsList = calloc(message.textLength, sizeof(char));
    memcpy(friendsList, message.text, message.textLength);
    char *friend = strtok(friendsList, " ");

    while(friend != NULL) {
        int friendID = atoi(friend);
        for(int i=0; i<=lastFriendIndex; ++i) {
            if(friends[i] == friendID) {
                for(int j=i; j<lastFriendIndex; ++j) {
                    friends[j] = friends[j+1];
                }
                lastFriendIndex--;
            }
        }
        friend = strtok(NULL, " ");
    }
    free(friendsList);
}

void handleTOONE(Message message) {
    char* index = malloc(1);
    memcpy(index, message.text, 1);
    char* text = connectClientTime(&message);
    sendMessage(clientsQueues[atoi(index)], ECHO, text+2, 0, message.textLength);
    free(text);
}


void handleTOALL(Message message) {
    char* text = connectClientTime(&message);
    for(int i=0; i<=lastClientIndex; ++i) {
        if(clientsQueues[i] != 0) {
            sendMessage(clientsQueues[i], ECHO, text, 0, message.textLength);
        }
    }
    free(text);
}


void handleTOFRIENDS(Message message) {
    char* text = connectClientTime(&message);
    for(int i=0; i<=lastFriendIndex; ++i) {
        if(clientsQueues[i] != 0) {
            sendMessage(clientsQueues[friends[i]], ECHO, text, 0, message.textLength);
        }
    }
    free(text);
}

void handleINIT(Message message) {
    if(lastClientIndex >= MAX_CLIENTS) {
        fprintf(stderr, "Too many clients");
        exit(1);
    }
    if ((clientsQueues[lastClientIndex] = msgget(message.clientID, 0)) == -1) {
        fprintf(stderr, "Couldn't find client queue");
        exit(1);
    } else {
        sendMessage(clientsQueues[lastClientIndex], INIT, "Server response", lastClientIndex, 0);
    }
    lastClientIndex++;
}

char* connectTime(Message* message) {
    time_t now;
    time(&now);
    char* result = calloc(MAX_MESSAGE_LENGTH + 100, sizeof(char));

    sprintf(result, "%s\n\tTime: %s", message->text, ctime(&now));
    message->textLength = strlen(result);
    return result;
}

char* connectClientTime(Message* message) {
    time_t now;
    time(&now);
    char* result = calloc(MAX_MESSAGE_LENGTH + 100, sizeof(char));

    sprintf(result, "%s\n\tClient: %ld\n\tTime: %s", message->text, message->clientID, ctime(&now));
    message->textLength = strlen(result);
    return result;
}


void handleSTOP(Message message) {
    clientsQueues[message.clientID] = 0;
    for(int i=0; i<=lastClientIndex; ++i) {
        if(clientsQueues[i] == 0) {
            for(int j=i; j<lastClientIndex; ++j) {
                clientsQueues[j] = clientsQueues[j+1];
            }
            lastClientIndex--;
        }
    }
}


void handleExit() {
    for (int i = 0; i<=lastClientIndex; ++i) {
        if (clientsQueues[i] != 0) {
            char* text = "STOP";
            sendMessage(clientsQueues[i], STOP, text, 0, strlen(text));
            waitForMessage(serverQueue);
        }
    }
    msgctl(serverQueue, IPC_RMID, NULL);
}

void handleSIGINT(int signum) {
    printf("\nServer end after SIGINT\n");
    exit(0);
}
