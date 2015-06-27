CC=gcc
CXX=g++
INC+= -I../ -I./ -I/usr/include/mysql/ -I/usr/local/mysql/include/ 
LIB+=-lstdc++ -lrt -L/usr/lib -L/usr/local/lib -L/usr/lib64/mysql -lpthread -lz -L/usr/local/mysql/lib -lmysqlclient
CFLAGS+= -pipe -Wall -O3
TARGET=itp
ALL:$(TARGET)

C_SRCFILES := $(wildcard *.c) 
CPP_SRCFILES := $(wildcard *.cpp)

C_OBJS := $(patsubst %.c,%.o, $(C_SRCFILES))
CPP_OBJS := $(patsubst %.cpp,%.o, $(CPP_SRCFILES))

OBJS := $(C_OBJS) $(CPP_OBJS)

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) $(INC) -o $@ $^ $(LIB)

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INC) -c -o $@ $< $(LIB)
	
%.o: %.c
	$(CC) $(CFLAGS)  $(INC) -c -o $@ $< $(LIB)

clean:
	rm -f *.o
	rm -f $(TARGET)
