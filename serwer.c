#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX 128 // max dlugosc wiadomosci
#define FIFO_SERVER_NAME    "server_fifo"

struct dane_do_przekazania {
    pid_t client_pid;
    char wiadomosc[MAX];
};

int main() 
{

    if (mkfifo(FIFO_SERVER_NAME, 0600) < 0) 
    {
        perror("Blad tworzenia potoku serwera");
        exit(EXIT_FAILURE);
    }
    printf("[SERWER] Blokuje się dopoki moje FIFO nie zostanie otwarte do zapisu.\n");
    
    int server_fifo_fd = open(FIFO_SERVER_NAME, O_RDONLY);
    if (server_fifo_fd < 0) 
    {
        perror("Blad otwarcia potoku serwera");
        exit(EXIT_FAILURE);
    }

    printf("[SERWER] zasypiam na 4 sekundy\n");
    sleep(4);

    int readBytes;
    struct data_to_transfer data;
    char client_fifo_name[20];
    int client_fifo_fd;

    while (readBytes = read(server_fifo_fd, &data, sizeof(struct dane_do_przekazania)) > 0) 
    {

        printf("[SERWER] Otrzymalem wiadomosc od %d o tresci %s\n",data.client_pid, data.wiadomosc);

        if (readBytes == -1) 
        {
            perror("Blad odczytu z potoku serwera");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; data.wiadomosc[i] != '\0' ; i++)
            data.wiadomosc[i] = toupper(data.wiadomosc[i]);

        sprintf(client_fifo_name, "client_%d", data.client_pid);

        client_fifo_fd = open(client_fifo_name, O_WRONLY);
        if (client_fifo_fd < 0)
        {
            perror("Blad otwarcia potoku klienta");
            exit(EXIT_FAILURE);
        }

        printf("[SERWER] Wysylam wiadomosc dla %d o tresci %s\n",data.client_pid, data.wiadomosc);

        if (write(client_fifo_fd, &data, sizeof(struct data_to_transfer)) < 0)
        {
            perror("Blad zapisu do potoku klienta");
            exit(EXIT_FAILURE);
        }

        close(client_fifo_fd);
        sleep(2);
    }

    close(server_fifo_fd);

    if(unlink(FIFO_SERVER_NAME) < 0)
    {
        perror("[SERWER] Blad usuniecia kolejki fifo serwera");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_FAILURE);
}