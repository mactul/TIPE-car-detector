gcc -o server server.c `mysql_config --cflags` -lpthread -I/usr/local/mysql/include -L/usr/local/mysql/lib -lmysqlclient

./server
