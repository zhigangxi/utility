NAME = mymake 
LINK = g++
CC  = gcc


#CCFLAGS	+= -O3
CCFLAGS	+= -g
CCFLAGS	+= -Wall
#CCFLAGS += -I/usr/local/include/

#LINKOPTS += -L/usr/local/lib/

CPPFILES = main.cpp \
            utility.cpp \
            create_make.cpp

$(NAME):$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) 
	$(LINK) -o $(NAME) $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(LINKOPTS)

.cpp.o:
	$(CC)  -c $< $(CCFLAGS)
			
clean:
	rm -f $(CPPFILES:.cpp=.o) $(CFILES:.c=.o)


#$(CPPFILES:.cpp=.o):$(CPPFILES) $(HEADFILES)
#	$(CC) $(CCFLAGS) -c $(CPPFILES)
#	#c.o:
#	#	$(CC) -c -o $*.o $(CFILES) $(CCFLAGS) $*.c
#
