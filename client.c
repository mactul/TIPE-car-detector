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

#define trigPin 4       
#define echoPin 5
#define MAX_DISTANCE 220        // define the maximum measured distance
#define timeOut MAX_DISTANCE*60


#define ID 24

#define CAR_OFF 0
#define CAR_ON 1

typedef int State;

int pulseIn(int pin, int level, int timeout);
float getSonar(void);
uint32_t create_datas(uint32_t state);


float getSonar(){   //get the measurement result of ultrasonic module with unit: cm
    long pingTime;
    float distance;
    digitalWrite(trigPin,HIGH); //send 10us high level to trigPin 
    delayMicroseconds(10);
    digitalWrite(trigPin,LOW);
    pingTime = pulseIn(echoPin,HIGH,timeOut);   //read plus time of echoPin
    distance = (float)pingTime * 340.0 / 2.0 / 10000.0; //calculate distance with sound speed 340m/s
    return distance;
}

int pulseIn(int pin, int level, int timeout)
{
   struct timeval tn, t0, t1;
   long micros;
   gettimeofday(&t0, NULL);
   micros = 0;
   while (digitalRead(pin) != level)
   {
      gettimeofday(&tn, NULL);
      if (tn.tv_sec > t0.tv_sec) micros = 1000000L; else micros = 0;
      micros += (tn.tv_usec - t0.tv_usec);
      if (micros > timeout) return 0;
   }
   gettimeofday(&t1, NULL);
   while (digitalRead(pin) == level)
   {
      gettimeofday(&tn, NULL);
      if (tn.tv_sec > t0.tv_sec) micros = 1000000L; else micros = 0;
      micros = micros + (tn.tv_usec - t0.tv_usec);
      if (micros > timeout) return 0;
   }
   if (tn.tv_sec > t1.tv_sec) micros = 1000000L; else micros = 0;
   micros = micros + (tn.tv_usec - t1.tv_usec);
   return micros;
}


uint32_t create_datas(uint32_t state)
{
    return ID | (state << (8*sizeof(uint32_t) - 1));
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
     
    // This ip address will change according to the machine
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    /*
    // Explicitly assigning port number 12010 by
    // binding client with that port
    my_addr1.sin_family = AF_INET;
    my_addr1.sin_addr.s_addr = INADDR_ANY;
    my_addr1.sin_port = htons(12010);
     
    // This ip address will change according to the machine
    my_addr1.sin_addr.s_addr = inet_addr("10.32.40.213");
    if (bind(client, (struct sockaddr*) &my_addr1, sizeof(struct sockaddr_in)) == 0)
        printf("Binded Correctly\n");
    else
        printf("Unable to bind\n");*/
     
    socklen_t addr_size = sizeof my_addr;
    int con = connect(client, (struct sockaddr*) &my_addr, sizeof my_addr);
    if (con == 0)
        printf("Client Connected\n");
    else
        printf("Error in Connection\n");
    
    
    wiringPiSetup();
    
    while(1)
    {
        uint32_t data;
        if(getSonar() < 40)
            data = create_datas(CAR_ON);
        else
            data = create_datas(CAR_OFF);
        printf("%u %f\n", data, getSonar());
        send(client, &data, sizeof(uint32_t), 0);
        sleep(5);
    }
    
    return 0;
}