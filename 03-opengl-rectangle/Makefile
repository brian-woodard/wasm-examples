CXXFLAGS = -I.

all:
	gcc -c $(CXXFLAGS) glad.c
	g++ $(CXXFLAGS) main.cpp -o main -lglfw -lGL glad.o

wasm:
	emcc $(CXXFLAGS) main.cpp -o main.html -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3

clean:
	rm -f *.o
	rm -f main
	rm -f main.html
	rm -f main.js
	rm -f main.wasm
