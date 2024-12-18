#include "protocol.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define BUFFER_SIZE 1024

/**
 * @brief Validate the bytes of a message by socket
 * @param bytes_int bytes of response of a write or read
 * @param bytes_expected bytes expected of the message
 * @return an appropriate code
 */
status_operation_socket validate_message(int bytes_int, int bytes_expected);

status_operation_socket send_file(int socket, const char *pathFile) {
    // 1. Abrir el archivo en modo solo lectura
    int file = open(pathFile, O_RDONLY);
    if (file < 0) {
        perror("Error opening file");
        return ERROR;
    }

    // 2. Obtener el tamaño del archivo
    off_t fileSize = lseek(file, 0, SEEK_END);
    lseek(file, 0, SEEK_SET);  // Regresar al inicio del archivo
    if (fileSize < 0) {
        perror("Error obtaining file size");
        close(file);
        return ERROR;
    }

    // 3. Enviar el tamaño del archivo
    if (write(socket, &fileSize, sizeof(fileSize)) < 0) {
        perror("Error sending file size");
        close(file);
        return ERROR;
    }

    // 4. Leer el archivo y enviar su contenido
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    off_t totalBytesSent = 0;
    while ((bytesRead = read(file, buffer, sizeof(buffer))) > 0) {
        ssize_t totalBytesWritten = 0;
        while (totalBytesWritten < bytesRead) {
            ssize_t bytesWritten = write(socket, buffer + totalBytesWritten, bytesRead - totalBytesWritten);
            if (bytesWritten < 0) {
                perror("Error sending file");
                close(file);
                return ERROR;
            }
            totalBytesWritten += bytesWritten;
        }
        totalBytesSent += bytesRead;
    }

    if (bytesRead < 0) {
        perror("Error reading file");
        close(file);
        return ERROR;
    }

    // 5. Cerrar el archivo
    close(file);
    return OK;
}

status_operation_socket receive_file(int socket, const char *pathFile) {
    // 1. Abrir el archivo en modo escritura, creando uno nuevo o truncando el existente
    int file = open(pathFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file < 0) {
        perror("Error opening file");
        return ERROR;
    }

    // 2. Recibir el tamaño del archivo
    off_t fileSize;
    if (read(socket, &fileSize, sizeof(fileSize)) <= 0) {
        perror("Error receiving file size");
        close(file);
        return ERROR;
    }

    // 3. Recibir y escribir los datos del archivo
    char buffer[BUFFER_SIZE];
    ssize_t bytesReceived;
    off_t totalBytesReceived = 0;
    while (totalBytesReceived < fileSize && (bytesReceived = read(socket, buffer, sizeof(buffer))) > 0) {
        ssize_t totalBytesWritten = 0;
        while (totalBytesWritten < bytesReceived) {
            ssize_t bytesWritten = write(file, buffer + totalBytesWritten, bytesReceived - totalBytesWritten);
            if (bytesWritten < 0) {
                perror("Error writing to file");
                close(file);
                return ERROR;
            }
            totalBytesWritten += bytesWritten;
        }
        totalBytesReceived += bytesReceived;
    }

    if (bytesReceived < 0) {
        perror("Error reading from socket");
        close(file);
        return ERROR;
    }

    // 4. Cerrar el archivo
    close(file);
    return OK;
}

status_operation_socket receive_first_request(int socket, struct first_request *first_request_param) {
    size_t bytes_expected = sizeof(struct first_request);
    ssize_t totalBytesRead = 0;

    while (totalBytesRead < bytes_expected) {
        ssize_t bytes_read = read(socket, (void*)first_request_param + totalBytesRead, bytes_expected - totalBytesRead);
        if (bytes_read < 0) {
            perror("Error reading from socket");
            return ERROR_SOCKET;
        } else if (bytes_read == 0) {
            return CLIENT_DISCONECT;
        }
        totalBytesRead += bytes_read;
    }

    // printf("----------- RECEIVE FIRST REQUEST------------- \n");
    // printf("file_idUser:  %d\n", first_request_param->idUser);
    // printf("file_request:  %d \n", first_request_param->request);
    // printf("_________________________________________________ \n");

    return validate_message(totalBytesRead, bytes_expected);
}

status_operation_socket receive_file_request(int socket, struct file_request *file_request_param) {
    size_t bytes_expected = sizeof(struct file_request);
    ssize_t totalBytesRead = 0;

    while (totalBytesRead < bytes_expected) {
        ssize_t bytes_read = read(socket, (void*)file_request_param + totalBytesRead, bytes_expected - totalBytesRead);
        if (bytes_read < 0) {
            perror("Error reading from socket");
            return ERROR_SOCKET;
        } else if (bytes_read == 0) {
            return CLIENT_DISCONECT;
        }
        totalBytesRead += bytes_read;
    }

    // printf("______________RECEIVE FILE REQUEST______________ \n");
    // printf("file_hashfile:  %s \n", file_request_param->hashFile);
    // printf("file_nameFile:  %s \n", file_request_param->nameFile);
    // printf("file_sizeHashFile:  %d \n", file_request_param->sizeHashFile);
    // printf("file_sizeNamefile:  %d \n", file_request_param->sizeNameFile);
    // printf("_________________________________________________ \n");

    return validate_message(totalBytesRead, bytes_expected);
}

status_operation_socket receive_file_transfer(int socket, struct file_transfer *file_transfer_param) {
    size_t bytes_expected = sizeof(struct file_transfer);
    ssize_t totalBytesRead = 0;

    while (totalBytesRead < bytes_expected) {
        ssize_t bytes_read = read(socket, (void*)file_transfer_param + totalBytesRead, bytes_expected - totalBytesRead);
        if (bytes_read < 0) {
            perror("Error reading from socket");
            return ERROR_SOCKET;
        } else if (bytes_read == 0) {
            return CLIENT_DISCONECT;
        }
        totalBytesRead += bytes_read;
    }

    // printf("______________RECEIVE FILE TRANSFER_______________ \n");
    // printf("file_comment:  %s \n", file_transfer_param->comment);
    // printf("file_fileSize:  %d \n", file_transfer_param->filseSize);
    // printf("_________________________________________________ \n");

    return validate_message(totalBytesRead, bytes_expected);
}

status_operation_socket receive_status_code(int socket, return_code *status_operation) {
    size_t bytes_expected = sizeof(return_code);
    ssize_t totalBytesRead = 0;

    while (totalBytesRead < bytes_expected) {
        ssize_t bytes_read = read(socket, (void*)status_operation + totalBytesRead, bytes_expected - totalBytesRead);
        if (bytes_read < 0) {
            perror("Error reading from socket");
            return ERROR_SOCKET;
        } else if (bytes_read == 0) {
            return CLIENT_DISCONECT;
        }
        totalBytesRead += bytes_read;
    }

    // printf("-----------RECEIVE STATUS OPERATION------------- \n");
    // printf("status Operation:  %d \n", *status_operation);
    // printf("_________________________________________________ \n");

    return validate_message(totalBytesRead, bytes_expected);
}

status_operation_socket receive_element_list(int socket, char elementList[SIZE_ELEMENT_LIST]) {
    size_t size_struct = SIZE_ELEMENT_LIST;
    ssize_t totalBytesRead = 0;

    while (totalBytesRead < size_struct) {
        ssize_t bytes_read = read(socket, elementList + totalBytesRead, size_struct - totalBytesRead);
        if (bytes_read < 0) {
            perror("Error reading from socket");
            return ERROR_SOCKET;
        } else if (bytes_read == 0) {
            // El cliente cerró la conexión
            return CLIENT_DISCONECT;
        }
        totalBytesRead += bytes_read;
    }

    // printf("Received element list: %s\n", elementList);
    // printf("tamaño string recibido %zu\n", strlen(elementList));
    // printf("____________RECEIVE ELEMENT LIST______________ \n");
    // printf("Element List: %s\n", elementList);
    // printf("TAMAÑO ELEMENLIST : ------------------------------  %zu  \n", strlen(elementList));
    // printf("_________________________________________________ \n");

    return validate_message(totalBytesRead, size_struct);
}

status_operation_socket send_first_request(int socket, struct first_request *first_request_param) {
    size_t size_struct = sizeof(struct first_request);
    ssize_t totalBytesWritten = 0;

    while (totalBytesWritten < size_struct) {
        ssize_t bytes_written = write(socket, (char *)first_request_param + totalBytesWritten, size_struct - totalBytesWritten);
        if (bytes_written < 0) {
            perror("Error writing to socket");
            return ERROR_SOCKET;
        }
        totalBytesWritten += bytes_written;
    }

    // printf("_____________SEND FIRST REQUEST_______________\n");
    // printf("Element idUser: %d\n", first_request_param->idUser);
    // printf("Element request: %d\n", first_request_param->request);
    // printf("_________________________________________________ \n");

    return validate_message(totalBytesWritten, size_struct);
}

status_operation_socket send_file_request(int socket, struct file_request *file_request_param) {
    size_t size_struct = sizeof(struct file_request);
    file_request_param->sizeHashFile = strlen(file_request_param->hashFile);
    file_request_param->sizeNameFile = strlen(file_request_param->nameFile);
    ssize_t totalBytesWritten = 0;

    while (totalBytesWritten < size_struct) {
        ssize_t bytes_written = write(socket, (char *)file_request_param + totalBytesWritten, size_struct - totalBytesWritten);
        if (bytes_written < 0) {
            perror("Error writing to socket");
            return ERROR_SOCKET;
        }
        totalBytesWritten += bytes_written;
    }

    // printf("_____________SEND FILE REQUEST _______________\n");
    // printf("Element hashFile: %s\n", file_request_param->hashFile);
    // printf("Element nameFile: %s\n", file_request_param->nameFile);
    // printf("Element hashFile: %d\n", file_request_param->sizeHashFile);
    // printf("Element sizeNemaFIle: %d\n", file_request_param->sizeNameFile);
    // printf("Element version: %d\n", file_request_param->version);
    // printf("_________________________________________________ \n");

    return validate_message(totalBytesWritten, size_struct);
}

status_operation_socket send_file_transfer(int socket, struct file_transfer *file_transfer_param) {
    size_t size_struct = sizeof(struct file_transfer);
    ssize_t totalBytesWritten = 0;

    while (totalBytesWritten < size_struct) {
        ssize_t bytes_written = write(socket, (char *)file_transfer_param + totalBytesWritten, size_struct - totalBytesWritten);
        if (bytes_written < 0) {
            perror("Error writing to socket");
            return ERROR_SOCKET;
        }
        totalBytesWritten += bytes_written;
    }

    // printf("_____________SEND FILE TRANSFER _______________\n");
    // printf("Element comment: %s\n", file_transfer_param->comment);
    // printf("Element fileSize: %d\n", file_transfer_param->filseSize);
    // printf("_________________________________________________ \n");

    return validate_message(totalBytesWritten, size_struct);
}

status_operation_socket send_status_code(int socket, return_code code) {
    // printf("send_status_code(%d)\n", code);
    size_t size_struct = sizeof(return_code);
    ssize_t totalBytesWritten = 0;

    while (totalBytesWritten < size_struct) {
        ssize_t bytes_written = write(socket, (void *)&code + totalBytesWritten, size_struct - totalBytesWritten);
        if (bytes_written < 0) {
            perror("Error writing to socket");
            return ERROR_SOCKET;
        }
        totalBytesWritten += bytes_written;
    }

    return validate_message(totalBytesWritten, size_struct);
}

status_operation_socket send_element_list(int socket, char elementList[SIZE_ELEMENT_LIST]) {
    // printf("send_element_list(%s)\n", elementList);
    // printf("tamaño string enviado %zu\n", strlen(elementList));

    size_t size_struct = SIZE_ELEMENT_LIST;
    ssize_t totalBytesWritten = 0;

    while (totalBytesWritten < size_struct) {
        ssize_t bytes_written = write(socket, elementList + totalBytesWritten, size_struct - totalBytesWritten);
        if (bytes_written < 0) {
            perror("Error writing to socket");
            return ERROR_SOCKET;
        }
        totalBytesWritten += bytes_written;
    }

    return validate_message(totalBytesWritten, size_struct);
}

status_operation_socket validate_message(int bytes_int, int bytes_expected) {
    if (bytes_int == -1)
        return ERROR_SOCKET;
    else if (bytes_int == 0)
        return CLIENT_DISCONECT;
    else if (bytes_int != bytes_expected)
        return INVALID_RESPONSE;
    return OK;    
}