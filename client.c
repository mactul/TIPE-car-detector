#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include <sys/time.h>

#define trigPin 4                 // this is the GPIO pin for the trigger of the ultrasonic sensor
#define echoPin 5                 // this is the GPIO pin for the echo of the ultrasonic sensor
#define MAX_DISTANCE 220          // define the maximum measured distance
#define timeOut MAX_DISTANCE*60   // define the timeout in microseconds


#define ID 24  // ID of the client, must be unique

#define CAR_OFF 0
#define CAR_ON 1


int pulseIn(int pin, int level, int timeout);  // function to get the pulse time
float getSonar(void);                          // function to get the distance from the ultrasonic sensor
uint32_t create_datas(uint32_t state);         // function to encode the data to send


float getSonar()  // get the measurement result of ultrasonic module with unit: cm
{
    long pingTime;
    float distance;
    digitalWrite(trigPin,HIGH); // send 10us high level to trigPin 
    delayMicroseconds(10);
    digitalWrite(trigPin,LOW);
    pingTime = pulseIn(echoPin,HIGH,timeOut);   // read plus time of echoPin
    distance = (float)pingTime * 340.0 / 2.0 / 10000.0; // calculate distance with sound speed 340m/s
    return distance;
}

int pulseIn(int pin, int level, int timeout)
{
    // timeout is in microseconds
    struct timeval tn, t0, t1;
    long micros;
    gettimeofday(&t0, NULL);  // get current time in microseconds
    micros = 0;
    while (digitalRead(pin) != level)
    {
        gettimeofday(&tn, NULL);
        if(tn.tv_sec > t0.tv_sec)
            micros = 1000000L;
        else
            micros = 0;
        micros += (tn.tv_usec - t0.tv_usec);
        if(micros > timeout)
            return 0;
    }
    gettimeofday(&t1, NULL);
    while (digitalRead(pin) == level)
    {
        gettimeofday(&tn, NULL);
        if(tn.tv_sec > t0.tv_sec)
            micros = 1000000L;
        else
            micros = 0;
        micros = micros + (tn.tv_usec - t0.tv_usec);
        if(micros > timeout)
            return 0;
    }
    if(tn.tv_sec > t1.tv_sec)
        micros = 1000000L;
    else
        micros = 0;
    micros = micros + (tn.tv_usec - t1.tv_usec);
    return micros;
}


uint32_t create_datas(uint32_t state)
{
    return ID | (state << (8*sizeof(uint32_t) - 1));  // data format: [state: 1 bit][ID: 31 bits]
}

int main()
{
    // Two buffer are for message communication
    char buffer1[256], buffer2[256];
    struct sockaddr_in my_addr, my_addr1;
    int client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0)
        printf("Error in client creating\n");
    else
        printf("Client Created\n");
         
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(12000);
    
    // This ip address is the server ip address
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
     
    socklen_t addr_size = sizeof my_addr;
    int con = connect(client, (struct sockaddr*) &my_addr, sizeof my_addr);
    if (con == 0)
        printf("Client Connected\n");
    else
        printf("Error in Connection\n");
    
    
    wiringPiSetup();  // start listening to the ultrasonic sensor

    pinMode(trigPin,OUTPUT);
    pinMode(echoPin,INPUT);
    
    while(1)
    {
        uint32_t data;
        if(getSonar() < 40)  // if there is an object within 40 cm of the sensor, then a car is on top
            data = create_datas(CAR_ON);
        else
            data = create_datas(CAR_OFF);
        
        printf("%u %f\n", data, getSonar());  // this is for debugging
        send(client, &data, sizeof(uint32_t), 0);  // send the data to the server
        sleep(5);  // every 5 seconds
    }
    
    return 0;
}