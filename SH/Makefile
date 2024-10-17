Target=main
Object=main.o epoll.o mqtt.o link.o mysqlite3.o
RM=rm -rf

CFLAGS=-lsqlite3 -ljson-c

$(Target):$(Object)
	gcc $(Object) -o $(Target) -Wall $(CFLAGS) $(CFLAGS) -g


clean:
	$(RM) *.o 
