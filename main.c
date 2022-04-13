#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>
#define SERVER_PORT 8899
#define PATH "/home/kitush/Documents/Projects/library/library-client"
#define LIST_CMD "fshare list" // done
#define AUTH_CMD "fshare auth --key"
#define READ_CMD "fshare read" // done
#define WRITE_CMD "fshare write"
#define CREATE_CMD "fshare new"
#define DELETE_CMD "fshare delete"

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

int readFile(int client_sock, char *file_name)
{
    FILE *ptr;
    char ch;

    char str[50];

    char path[200] = PATH;
    strcat(path, file_name);

    puts(path);
    // Opening file in reading mode
    ptr = fopen(path, "r");

    if (NULL == ptr)
    {
        printf("file can't be opened \n");
    }

    char log[200] = "Executing command fshare read....";
    write(client_sock, log, strlen(log));
    write(client_sock, "\n", 1);

    // Printing what is written in file
    // character by character using loop.
    while (fgets(str, 50, ptr) != NULL)
    {
        printf("%s", str);
        write(client_sock, str, strlen(str));
    }

    write(client_sock, "fshare> ", 8);

    // Closing the file
    fclose(ptr);
    return 0;
}

int listFilesInFolder(char *folderName, int client_sock)
{
    DIR *d;

    struct dirent *dir;
    d = opendir(folderName);
    char log[200] = "Executing command fshare list....";
    write(client_sock, log, strlen(log));

    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {

            write(client_sock, dir->d_name, strlen(dir->d_name));
            write(client_sock, "\n", 1);
        }

        write(client_sock, "fshare> ", 8);
        closedir(d);
    }
    return (0);
}

int create_socket()
{
    // instantiate int values in one line
    int socket_desc, client_sock, c, read_size;

    struct sockaddr_in server, client;

    char client_message[2000];

    // create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }

    puts("Socket created");

    // Prepare the sockaddr data
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(SERVER_PORT);

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind failed. Error");
        return 1;
    }
    puts("Bind done");

    listen(socket_desc, 3);

    puts("Waiting for incoming connections");
    c = sizeof(struct sockaddr_in);

    // accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");

    write(client_sock, "fshare> ", 8);

    // Receive a message from client
    while ((read_size = read(client_sock, client_message, sizeof(client_message))) > 0)
    {

        if (startsWith(LIST_CMD, client_message))
        {
            listFilesInFolder(PATH, client_sock);
        }
        else if (startsWith(READ_CMD, client_message))
        {
            readFile(client_sock, "/package.json");
        }
        else if (startsWith("exit", client_message))
        {
            exit(0);
        }
        else
        {
            write(client_sock, "Invalid command", 15);
            write(client_sock, "\n", 1);
            write(client_sock, "fshare> ", 8);
        }

        memset(&client_message, 0, sizeof(client_message));
    }

    if (read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if (read_size == -1)
    {
        perror("recv failed");
    }

    return 0;
}

int main()
{
    create_socket();
    // readFile();
    return 0;
}