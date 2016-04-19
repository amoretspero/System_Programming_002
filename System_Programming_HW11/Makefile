COMP = gcc
OPT = -o
CLI = echo_client
#SERV = echo_server

#all : echo_client echo_server
all : echo_client

echo_client.o : echo_client.c

echo_client : echo_client.c
	$(COMP) $(OPT) $(CLI) $(CLI).c

#echo_server.o : echo_server

#echo_server : echo_server.c
#	$(COMP) $(OPT) $(SERV) $(SERV).c

#clean :
#	rm -rf $(CLI) $(CLI).o $(SERV) $(SERV).o

clean :
	rm -rf $(CLI) $(CLI).o

#run :
#	./$(SERV) &
#	./$(CLI)
