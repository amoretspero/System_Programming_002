COMP = gcc
EXEC = dup2_2 dns_query
SRC = dup2_2.c cns_query.c
FILE1 = dup2_2 dup2_2.c
FILE2 = dns_query dns_query.c
OPT = -o
CLEANOPT = rm -rf

all :
	$(COMP) $(OPT) $(FILE1)
	$(COMP) $(OPT) $(FILE2)

clean :
	$(CLEANOPT) $(EXEC)
