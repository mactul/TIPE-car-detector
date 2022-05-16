// https://www.geeksforgeeks.org/explicitly-assigning-port-number-client-socket/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>


typedef struct sensor_data {
    int id;
    uint32_t state;
} Sensor_data;


void decode_datas(Sensor_data* poutput, uint32_t data)
{
    poutput->id = (data & (~(0x1 << (8*sizeof(uint32_t) - 1))));
    poutput->state = (data >> (8*sizeof(uint32_t) - 1));
}

void* manage_sensor(void* pdata)
{
    uint32_t data;
    int acc = (int) pdata;
    int n = 1;
    Sensor_data sdata;

    while(n > 0)
    {
        n = recv(acc, &data, sizeof(data), 0);
        printf("cc");
        if(n > 0)
        {
            decode_datas(&sdata, data);
            printf("ID: %d\nstate: %d\n", sdata.id, sdata.state);
        }
    }
    close(acc);
    return NULL;
}

int main()
{
    int server = socket(AF_INET, SOCK_STREAM, 0);
    struct timeval tv;
    
    if (server < 0)
        printf("Error in server creating\n");
    else
        printf("Server Created\n");
         
    struct sockaddr_in my_addr, peer_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    // This ip address will change according to the machine
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
     
    my_addr.sin_port = htons(12000);

    tv.tv_sec = 60;
    tv.tv_usec = 0;
    setsockopt(server, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
 
    if (bind(server, (struct sockaddr*) &my_addr, sizeof(my_addr)) == 0)
        printf("Binded Correctly\n");
    else
        printf("Unable to bind\n");
         
    if (listen(server, 3) == 0)
        printf("Listening ...\n");
    else
        printf("Unable to listen\n");
     
    socklen_t addr_size;
    addr_size = sizeof(struct sockaddr_in);
    
    
    
    // while loop is iterated infinitely to
    // accept infinite connection one by one
    while (1)
    {
        pthread_t thread;
        int acc = accept(server, (struct sockaddr*) &peer_addr, &addr_size);
        
        
        printf("Connection Established\n");
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(peer_addr.sin_addr), ip, INET_ADDRSTRLEN);
     
        // "ntohs(peer_addr.sin_port)" function is
        // for finding port number of client
        printf("connection established with IP : %s and PORT : %d\n",
                                            ip, ntohs(peer_addr.sin_port));
 
        pthread_create (&thread, NULL, *manage_sensor, (void*) acc);
    }
    return 0;
}