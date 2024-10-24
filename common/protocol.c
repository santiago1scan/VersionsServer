#include "protocol.h"

/**
 * @brief Validate the bytes of a message by socket
 * @param bytes_int bytes of responde of a write or rea
 * @param bytes_excepcted bytes expected of the message
 * @return an apropiate code
 */
status_operation_socket validate_message(int bytes_int, int bytes_expected);


status_operation_socket send_file(int socket, char *pathFile, int sizeFile) {
    // 1. Open the file
    int file = open(pathFile, O_RDONLY);
    if (file < 0) {
        perror("Error opening file");
        return ERROR;
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
            return ERROR;
        }
    }

    if (bytesRead < 0) {
        perror("Error reading file");
        close(file);
        return ERROR;
    }

    // 5. Close the file
    close(file);
    return OK;
}

status_operation_socket receive_file(int socket, char *pathFile, int sizeFile) {
    // 1. Open the file
    int file = open(pathFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file < 0) {
        perror("Error opening file");
        return ERROR;
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
            return ERROR;
        }
        totalBytesReceived += bytesReceived;
    }

    if (bytesReceived < 0) {
        perror("Error reading from socket");
        close(file);
        return ERROR;
    }

    // 4. Close the file
    close(file);
    return OK;
}

status_operation_socket receive_first_request(int socket, struct first_request *first_request_param){
	size_t bite_expected  = sizeof(struct first_request);
    size_t bite_read = read(socket, (void*)&first_request_param, bite_expected);    
    return  validate_messagee(bite_read, bite_expected);
}

status_operation_socket receive_file_request(int socket, struct file_request *file_request_param){
    size_t bite_expected  = sizeof(struct first_request);
    size_t bite_read = read(socket, (void*)&file_request_param, bite_expected);    
    return  validate_messagee(bite_read, bite_expected);
}

status_operation_socket receive_file_transfer(int socket, struct file_transfer *file_transfer_param ){
    size_t bite_expected  = sizeof(struct file_transfer);
    size_t bite_read = read(socket, (void*)&file_transfer_param, bite_expected);    
    return  validate_messagee(bite_read, bite_expected);
}

status_operation_socket receive_status_code(int socket,return_code *status_operation){
    size_t bite_expected  = sizeof(return_code);
    size_t bite_read = read(socket, (void*)&status_operation, bite_expected);    
    return  validate_messagee(bite_read, bite_expected);
}

status_operation_socket receive_element_list(int socket, char elementList[SIZE_ELEMENT_LIST]){
    size_t bite_expected  = SIZE_ELEMENT_LIST;
    size_t bite_read = read(socket, (void*)&elementList, bite_expected);    
    return  validate_messagee(bite_read, bite_expected);
}

status_operation_socket send_first_request(int socket, struct first_request *first_request_param){
    size_t size_struct = sizeof(struct first_request);
    int bytes_writen = write(socket, (void *) first_request_param, size_struct);
    return validate_message(bytes_writen, size_struct);
}

status_operation_socket send_file_request(int socket, struct file_request *file_request_param){
    size_t size_struct = sizeof(struct file_request);
    int bytes_writen = write(socket, (void *) file_request_param, size_struct);
    return validate_message(bytes_writen, size_struct);
}

status_operation_socket send_file_transfer(int socket, struct file_transfer *file_transfer_param ){
    size_t size_struct = sizeof(struct file_transfer);
    int bytes_writen = write(socket, (void *) file_transfer_param, size_struct);
    return validate_message(bytes_writen, size_struct);
}

status_operation_socket send_status_code(int socket, return_code code){
    size_t size_struct = sizeof(return_code);
    int bytes_writen = write(socket, (void *) &code, size_struct);
    return validate_message(bytes_writen, size_struct);
}

status_operation_socket send_element_list(int socket, char elementList[SIZE_ELEMENT_LIST]){
    size_t size_struct = SIZE_ELEMENT_LIST;
    int bytes_writen = write(socket, (void *) elementList, size_struct);
    return validate_message(bytes_writen, size_struct);
}

status_operation_socket validate_message(int bytes_int, int bytes_expected){
    if(bytes_int == -1)
        return ERROR_SOCKET;
    else if(bytes_int == 0)
        return CLIENT_DISCONECT;
    else if(bytes_int != bytes_expected)
        return INVALID_RESPONSE;
    return OK;    
}
