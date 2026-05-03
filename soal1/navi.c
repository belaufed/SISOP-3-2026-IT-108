#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "protocol.h"

int sock;

void *receive_message(void *arg) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(sock, buffer, BUFFER_SIZE);

        if (valread <= 0) {
            break;
        }

        printf("%s", buffer);
        fflush(stdout);
    }

    return NULL;
}

int main() {
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    char name[50];
    pthread_t recv_thread;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    printf("Enter your name: ");
    fgets(name, 50, stdin);
    send(sock, name, strlen(name), 0);

    pthread_create(&recv_thread, NULL, receive_message, NULL);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);
        send(sock, buffer, strlen(buffer), 0);

        if (strncmp(buffer, "/exit", 5) == 0) {
            break;
        }
    }

    close(sock);
    return 0;
}
