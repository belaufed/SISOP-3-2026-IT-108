#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "protocol.h"

int clients[MAX_CLIENTS];
char names[MAX_CLIENTS][50];
int client_count = 0;
time_t start_time;

void write_log(const char *type, const char *message) {
    FILE *fp = fopen("history.log", "a");
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] %s %s\n",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec,
        type, message);

    fclose(fp);
}

int name_exists(char *name) {
    for (int i = 0; i < client_count; i++) {
        if (strcmp(names[i], name) == 0) return 1;
    }
    return 0;
}

void broadcast(int sender, char *msg) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != sender) {
            send(clients[i], msg, strlen(msg), 0);
        }
    }
}

void remove_client(int index) {
    char logmsg[100];
    sprintf(logmsg, "[User '%s' disconnected]", names[index]);
    write_log("[System]", logmsg);

    close(clients[index]);

    for (int i = index; i < client_count - 1; i++) {
        clients[i] = clients[i + 1];
        strcpy(names[i], names[i + 1]);
    }

    client_count--;
}

void handle_admin(int sock) {
    char menu[] =
        "\n=== THE KNIGHTS CONSOLE ===\n"
        "1. Check Active Entities (Users)\n"
        "2. Check Server Uptime\n"
        "3. Execute Emergency Shutdown\n"
        "4. Disconnect\n"
        "Command >> ";

    char buffer[BUFFER_SIZE];

    while (1) {
        send(sock, menu, strlen(menu), 0);
        memset(buffer, 0, BUFFER_SIZE);
        read(sock, buffer, BUFFER_SIZE);

        if (strncmp(buffer, "1", 1) == 0) {
            char out[BUFFER_SIZE] = "Active Users:\n";
            for (int i = 0; i < client_count; i++) {
                strcat(out, "- ");
                strcat(out, names[i]);
                strcat(out, "\n");
            }
            send(sock, out, strlen(out), 0);
            write_log("[Admin]", "[RPC_GET_USERS]");
        } else if (strncmp(buffer, "2", 1) == 0) {
            time_t now = time(NULL);
            char out[100];
            sprintf(out, "Server uptime: %ld seconds\n", now - start_time);
            send(sock, out, strlen(out), 0);
            write_log("[Admin]", "[RPC_GET_UPTIME]");
        } else if (strncmp(buffer, "3", 1) == 0) {
            printf("[System] EMERGENCY SHUTDOWN INITIATED\n");
            fflush(stdout);
            write_log("[Admin]", "[RPC_SHUTDOWN]");
            write_log("[System]", "[EMERGENCY SHUTDOWN INITIATED]");
            exit(0);
        } else {
            send(sock, "[System] Disconnecting from The Wired...\n", 41, 0);
            break;
        }
    }
}
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    fd_set readfds;
    char buffer[BUFFER_SIZE];

    start_time = time(NULL);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    write_log("[System]", "[SERVER ONLINE]");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        for (int i = 0; i < client_count; i++) {
            FD_SET(clients[i], &readfds);
            if (clients[i] > max_sd) max_sd = clients[i];
        }

        select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &readfds)) {
            new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

            char name[50], pass[50];
            memset(name, 0, 50);
            read(new_socket, name, 50);
            name[strcspn(name, "\n")] = 0;

            if (strcmp(name, ADMIN_NAME) == 0) {
                send(new_socket, "Enter Password: ", 16, 0);
                read(new_socket, pass, 50);
                pass[strcspn(pass, "\n")] = 0;

                if (strcmp(pass, ADMIN_PASS) == 0) {
                    send(new_socket, "[System] Authentication Successful. Granted Admin privileges.\n", 62, 0);
                    handle_admin(new_socket);
                } else {
                    send(new_socket, "[System] Wrong Password.\n", 25, 0);
                }

                close(new_socket);
                continue;
            }

            if (name_exists(name)) {
                send(new_socket, "[System] The identity is already synchronized in The Wired.\n", 60, 0);
                close(new_socket);
                continue;
            }

            clients[client_count] = new_socket;
            strcpy(names[client_count], name);
            client_count++;

            char welcome[100], logmsg[100];
            sprintf(welcome, "--- Welcome to The Wired, %s ---\n", name);
            send(new_socket, welcome, strlen(welcome), 0);

            sprintf(logmsg, "[User '%s' connected]", name);
            write_log("[System]", logmsg);
        }

        for (int i = 0; i < client_count; i++) {
            int sd = clients[i];

            if (FD_ISSET(sd, &readfds)) {
                memset(buffer, 0, BUFFER_SIZE);
                int valread = read(sd, buffer, BUFFER_SIZE);

                if (valread <= 0 || strncmp(buffer, "/exit", 5) == 0) {
                    send(sd, "[System] Disconnecting from The Wired...\n", 41, 0);
                    remove_client(i);
                    i--;
                } else {
                    char msg[BUFFER_SIZE + 100];
                    sprintf(msg, "[%s]: %s", names[i], buffer);
                    broadcast(sd, msg);

                    char logmsg[BUFFER_SIZE + 100];
                    sprintf(logmsg, "[[%s]: %s]", names[i], buffer);
                    write_log("[User]", logmsg);
                }
            }
        }
    }

    return 0;
}
