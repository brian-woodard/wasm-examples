CXXFLAGS = -I. -I../imgui -I../imgui/backends

all:
	gcc -c $(CXXFLAGS) glad.c
	g++ $(CXXFLAGS) -c ../imgui/imgui.cpp -o imgui.o
	g++ $(CXXFLAGS) -c ../imgui/imgui_draw.cpp -o imgui_draw.o
	g++ $(CXXFLAGS) -c ../imgui/imgui_tables.cpp -o imgui_tables.o
	g++ $(CXXFLAGS) -c ../imgui/imgui_widgets.cpp -o imgui_widgets.o
	g++ $(CXXFLAGS) -c ../imgui/backends/imgui_impl_glfw.cpp -o imgui_impl_glfw.o
	g++ $(CXXFLAGS) -c ../imgui/backends/imgui_impl_opengl3.cpp -o imgui_impl_opengl3.o
	g++ $(CXXFLAGS) main.cpp -o main -lglfw -lGL glad.o imgui.o imgui_draw.o imgui_tables.o imgui_widgets.o imgui_impl_glfw.o imgui_impl_opengl3.o

wasm:
	emcc $(CXXFLAGS) -c ../imgui/imgui.cpp -o imgui.o
	emcc $(CXXFLAGS) -c ../imgui/imgui_draw.cpp -o imgui_draw.o
	emcc $(CXXFLAGS) -c ../imgui/imgui_tables.cpp -o imgui_tables.o
	emcc $(CXXFLAGS) -c ../imgui/imgui_widgets.cpp -o imgui_widgets.o
	emcc $(CXXFLAGS) -c ../imgui/backends/imgui_impl_glfw.cpp -o imgui_impl_glfw.o
	emcc $(CXXFLAGS) -c ../imgui/backends/imgui_impl_opengl3.cpp -o imgui_impl_opengl3.o
	emcc $(CXXFLAGS) main.cpp -o main.js -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 imgui.o imgui_draw.o imgui_tables.o imgui_widgets.o imgui_impl_glfw.o imgui_impl_opengl3.o --preload-file data

clean:
	rm -f *.o
	rm -f main
	rm -f main.js
	rm -f main.wasm
	rm -f main.data
