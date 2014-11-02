OS := $(shell uname)
OPTIONS:= 

ifeq ($(OS),Darwin)
	OPTIONS += -framework OpenCL
else
	OPTIONS += -l OpenCL
endif

main: raytracer.cpp
	gcc -Wall -g raytracer.cpp -o main $(OPTIONS)

clean:
	rm -rf main
