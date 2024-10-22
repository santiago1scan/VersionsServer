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

int send_file(int socket){

}

int receive_file(int socket, char **filedata, struct file_request *information){
    
}
