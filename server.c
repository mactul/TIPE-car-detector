// https://www.geeksforgeeks.org/explicitly-assigning-port-number-client-socket/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <mysql/mysql.h>
#include "bdd_credentials.h"
#include "easy_tcp_tls.h"

MYSQL* _bdd;

typedef struct sensor_data {  // this structure will contain unpacked client data
    int id;
    uint32_t state;
} Sensor_data;

void decode_datas(Sensor_data* poutput, uint32_t data)
{
    // data is structured as follows: [state: 1 bit][ID: 31 bits]
    // this function will decode the data and put it in the structure
    poutput->id = (data & (~(0x1 << (8*sizeof(uint32_t) - 1))));
    poutput->state = (data >> (8*sizeof(uint32_t) - 1));
}


void finish_with_error()
{
    fprintf(stderr, "%s\n", mysql_error(_bdd));
    mysql_close(_bdd);
    exit(1);
}


void update_sensor(int sensor_ID, int state)
{
    // ID exists
    char sql[1024];
    sprintf(sql, "UPDATE sensors SET state=%d WHERE sensor_ID=%d", state, sensor_ID);
    if (mysql_query(_bdd, sql))
        finish_with_error(_bdd);
}

int main()
{
    SocketHandler* server_handler;

    socket_start();

    server_handler = socket_ssl_server_init("127.0.0.1", 12000, 1, "cert.pem", "key.pem");

    if(server_handler == NULL)
    {
        socket_print_last_error();
        return 1;
    }

    _bdd = mysql_init(NULL);
    if (_bdd == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(_bdd));
        exit(1);
    }

    if (mysql_real_connect(_bdd, BDD_HOST, BDD_LOGIN, BDD_PASSWORD, BDD_MAIN_DB, 0, NULL, 0) == NULL)
    {
        finish_with_error(_bdd);
    }


    // while loop is iterated infinitely to
    // accept infinite connection one by one
    while (1)
    {
        SocketHandler* client_handler;
        ClientData infos;

        client_handler = socket_accept(server_handler, &infos);

        if(client_handler != NULL)
        {
            uint32_t data;
            Sensor_data sdata;

            printf("connection established with IP : %s and PORT : %d\n", infos.ip, infos.port);

            socket_recv(client_handler, (char*)&data, sizeof(data), 0);

            decode_datas(&sdata, data);
            printf("id: %d, state: %d\n", sdata.id, sdata.state);
            update_sensor(sdata.id, sdata.state);

            socket_close(&client_handler);
        }
        else
        {
            socket_print_last_error();
        }
    }

    socket_cleanup();

    return 0;
}