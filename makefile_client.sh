gcc -o client client.c easy_tcp_tls.c -I/openssl/* -lcrypto -lssl -lwiringPi -Wall

./client
