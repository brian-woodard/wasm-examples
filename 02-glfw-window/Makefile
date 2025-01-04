all:
	g++ main.cpp -o main -lglfw -lGL

wasm:
	emcc main.cpp -o main.html -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3

clean:
	rm -f main
	rm -f main.html
	rm -f main.js
	rm -f main.wasm
