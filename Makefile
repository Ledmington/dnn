CXX=g++
CXXFLAGS=-std=c++17 -Wall -Wextra -Werror -Wshadow -fno-exceptions -fno-rtti -s -static
OPTFLAGS=-O3 -DNDEBUG -march=native -mtune=native
DEBUGFLAGS=-O0 -g -fno-omit-frame-pointer -fsanitize=thread,leak,address,undefined,float-divide-by-zero

all: release

release:
	${CXX} ${CXXFLAGS} ${OPTFLAGS} main.cpp -o main.x

debug:
	${CXX} ${CXXFLAGS} ${DEBUGFLAGS} main.cpp -o main.x

clean:
	rm -f main.x
