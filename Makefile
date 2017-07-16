CXX     =g++
CXXFLAGS=-g -Wall -Wextra -Werror -pedantic -std=gnu++0x
%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $<

all: client server

client: client.o conn.o
	$(CXX) $^ -o $@

server: server.o conn.o
	$(CXX) $^ -o $@

server.o: server.cpp conn.hpp
client.o: client.cpp conn.hpp
conn.o: conn.cpp conn.hpp

clean:
	rm -f *.o client server
