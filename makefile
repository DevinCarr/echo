# makefile

TARGET = echo

CXXFlags = -g -std=c++11

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
	CXX = clang++
else
	CXX = g++
endif

all: $(TARGET) 

logger.o: logger.cpp logger.h
	$(CXX) -c $(CXXFlags) $<

semaphore.o: semaphore.cpp semaphore.h
	$(CXX) -c $(CXXFlags) $<

irc_socket.o: irc_socket.cpp irc_socket.h
	$(CXX) -c $(CXXFlags) $<

irc_client.o: irc_client.cpp irc_client.h message.h
	$(CXX) -c $(CXXFlags) $<

watcher.o: watcher.cpp watcher.h
	$(CXX) -c $(CXXFlags) $<

main.o: main.cpp
	$(CXX) -c $(CXXFlags) $<

$(TARGET): main.o irc_client.o irc_socket.o logger.o semaphore.o watcher.o
	$(CXX) $(CXXFlags) -o $(TARGET) $^ -lpthread

clean:
	rm *.o
	rm echo

.PHONY: clean
