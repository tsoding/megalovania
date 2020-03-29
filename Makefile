PKGS=sdl2
CXXFLAGS=-Wall -Wextra -Wconversion -Werror -pedantic -std=c++17 -fno-exceptions -ggdb $(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -lm

megalovania: main.cpp
	$(CXX) $(CXXFLAGS) -o megalovania main.cpp $(LIBS)
