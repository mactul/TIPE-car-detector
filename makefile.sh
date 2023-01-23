gcc -o server server.c easy_tcp_tls.c -I/usr/local/mysql/include -L/usr/local/mysql/lib -I/openssl/*, -lcrypto, -lssl, -lmysqlclient, -Wall

./server
