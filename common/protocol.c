#include "protocol.h"


return_code_protocol validateWrite(int response){
    switch (response)
    {
    case -1:
        printf("Error writing a message\n");  
        return ERROR;
    default:
        return ALL_OK;
    }
}

return_code_protocol validateRead(int response){
    switch (response)
    {
    case 0:
        printf("A client has been disconected\n");
        return ERROR;
    case -1:
        printf("Error with conexion in a read\n");
        return ERROR;
    default:
        return ALL_OK;
    }
}

int send_file(int socket, char *pathFile, int sizeFile) {
    // 1. Open the file
    int file = open(pathFile, O_RDONLY);
    if (file < 0) {
        perror("Error opening file");
        return -1;
    }

    // 2. Get the file size
    off_t fileSize = sizeFile;

    // 4. Read the file and send its contents
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(file, buffer, sizeof(buffer))) > 0) {
        if (write(socket, buffer, bytesRead) < 0) {
            perror("Error sending file");
            close(file);
            return -1;
        }
    }

    if (bytesRead < 0) {
        perror("Error reading file");
        close(file);
        return -1;
    }

    // 5. Close the file
    close(file);
    return 0;
}

int receive_file(int socket, char *pathFile, int sizeFile) {
    // 1. Open the file
    int file = open(pathFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file < 0) {
        perror("Error opening file");
        return -1;
    }

    // 2. Receive the file size
    off_t fileSize = sizeFile;

    // 3. Receive the file and write its contents
    char buffer[BUFFER_SIZE];
    ssize_t bytesReceived;
    off_t totalBytesReceived = 0;
    while (totalBytesReceived < fileSize && (bytesReceived = read(socket, buffer, sizeof(buffer))) > 0) {
        if (write(file, buffer, bytesReceived) < 0) {
            perror("Error writing to file");
            close(file);
            return -1;
        }
        totalBytesReceived += bytesReceived;
    }

    if (bytesReceived < 0) {
        perror("Error reading from socket");
        close(file);
        return -1;
    }

    // 4. Close the file
    close(file);
    return 0;
}