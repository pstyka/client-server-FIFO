#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_MESSAGE_LENGTH 128
#define FIFO_SERVER_NAME "server_fifo"

struct data_to_transfer {
  pid_t client_pid;
  char message[MAX_MESSAGE_LENGTH];
};

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Niepoprawna liczba argumentow!\n");
        exit(EXIT_FAILURE);
    }

    char *message[3];
    for (int i = 0; i <  argc - 1; i++) {
        message[i] = argv[i + 1];
    }
    
    for (int i = 0; i < argc - 1; i++){
        if (strlen(message[i]) > MAX_MESSAGE_LENGTH - 1) {
            printf("Wiadomosc jest za dluga!\n");
            exit(EXIT_FAILURE);
        }
    }

    int server_fifo_fd = open(FIFO_SERVER_NAME, O_WRONLY);
    if (server_fifo_fd < 0) {
        perror("Blad otwarcia potoku serwera");
        exit(EXIT_FAILURE);
    }
    
    char client_fifo_name[20];
    sprintf(client_fifo_name, "client_%d", getpid());
    if (mkfifo(client_fifo_name, 0600) < 0) {
        perror("Blad tworzenia potoku klienta");
        exit(EXIT_FAILURE);
    }
    struct data_to_transfer data;
    data.client_pid = getpid();

    for (int i = 0; i < argc - 1; i++){
        strcpy(data.message, message[i]);
        if (write(server_fifo_fd, &data, sizeof(struct data_to_transfer)) < 0) {
            perror("Blad zapisu do potoku serwera");
            exit(EXIT_FAILURE);
        }
        printf("[KLIENT] Wysylam wiadomosc do Serwera o tresci %s\n", message[i]);
    }

    close(server_fifo_fd);

    int client_fifo_fd = open(client_fifo_name, O_RDONLY);
    if (client_fifo_fd < 0) {
        perror("Blad otwarcia potoku klienta");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < argc - 1; i++){

        int readBytes;

        while (readBytes = read(client_fifo_fd, &data, sizeof(struct data_to_transfer)) == 0)
            sleep(1);

        if (readBytes == -1){
            perror("[KLIENT] Blad odczytu wiadomosci z kolejki klienta");
            exit(EXIT_FAILURE);
        }
        else
            printf("[KLIENT] Odebralem wiadomosc dla %d o tresci %s\n", data.client_pid, data.message);
    }

    close(client_fifo_fd);

    if(unlink(client_fifo_name) < 0){
        perror("[SERWER] Blad usuniecia kolejki fifo klienta");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

