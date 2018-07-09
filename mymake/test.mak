OBGFLAG=-g -Wall 
LINKOPT=-L/usr/local/pgsql/lib/ -lpq

test:create_make.o main.o utility.o 
	g++ -o test create_make.o main.o utility.o  $(LINKOPT) 
create_make.o:.//create_make.cpp .//create_make.h .//utility.h
	gcc -c .//create_make.cpp $(OBGFLAG)
main.o:.//main.cpp .//create_make.h .//utility.h
	gcc -c .//main.cpp $(OBGFLAG)
utility.o:.//utility.cpp .//utility.h
	gcc -c .//utility.cpp $(OBGFLAG)
clean:
	rm -rf create_make.o main.o utility.o 