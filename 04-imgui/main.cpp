#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

// Include the Emscripten library only if targetting WebAssembly
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define GLFW_INCLUDE_ES3
#else
#include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// NOTE: Uncomment the following line for GL error handling
//#define GL_DEBUG

#ifdef GL_DEBUG
#define GLCALL(function) \
   { \
      GLenum error = GL_INVALID_ENUM; \
      while (error != GL_NO_ERROR) \
      { \
         error = glGetError(); \
      } \
      function; \
      error = glGetError(); \
      if (error != GL_NO_ERROR) \
      { \
         fprintf(stderr, "OpenGL Error: GL_ENUM(%d) at %s:%d\n", error, __FILE__, __LINE__); \
      } \
   }
#else
#define GLCALL(function) function;
#endif

GLFWwindow* window;

#ifdef __EMSCRIPTEN__
std::string vertex_shader_source = R"(
    precision mediump float;

    attribute vec3 aPos;
    attribute vec4 aColor;
    attribute vec2 aTexCoord;

    varying vec2 tex_coord;
    varying vec4 color;

    void main()
    {
        gl_Position = vec4(aPos, 1.0);
        tex_coord = aTexCoord;
        color = aColor;
    }
)";

std::string fragment_shader_source = R"(
    precision mediump float;

    varying vec2 tex_coord;
    varying vec4 color;

    void main()
    {
        gl_FragColor = color;
    }
)";
#else
std::string vertex_shader_source = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec4 aColor;
    layout (location = 2) in vec2 aTexCoord;

    out vec2 tex_coord;
    out vec4 color;
    //uniform mat4 mvp;

    void main()
    {
        gl_Position = vec4(aPos, 1.0);
        tex_coord = aTexCoord;
        color = aColor;
    }
)";

std::string fragment_shader_source = R"(
    #version 330 core

    in vec2 tex_coord;
    in vec4 color;

    void main()
    {
        gl_FragColor = color;
    }
)";
#endif

struct vec4
{
   float r;
   float g;
   float b;
   float a;
};

GLuint program;
GLuint vao;
GLuint vbo;
GLuint ebo;
bool initialize_buffers = true;
bool draw_rect = true;
float radius = 0.0;
float border_thickness = 0.0;
float edge_softness = 0.0;
vec4 color_ul = { 1.0f, 0.0f, 0.0f, 1.0f };
vec4 color_ur = { 0.0f, 1.0f, 0.0f, 1.0f };
vec4 color_lr = { 0.0f, 0.0f, 1.0f, 1.0f };
vec4 color_ll = { 1.0f, 1.0f, 0.0f, 1.0f };
vec4 border_color = { 1.0f, 1.0f, 0.0f, 1.0f };
float rect[4] = { 0.5f, 0.5f, 1.0f, 1.0f };
int width = 640;
int height = 480;

#ifdef __EMSCRIPTEN__
// Function used by c++ to get the size of the html canvas
EM_JS(int, canvas_get_width, (), {
  return Module.canvas.width;
});

// Function used by c++ to get the size of the html canvas
EM_JS(int, canvas_get_height, (), {
  return Module.canvas.height;
});

// Function called by javascript
EM_JS(void, resizeCanvas, (), {
  js_resizeCanvas();
});

void on_size_changed()
{
  glfwSetWindowSize(window, width, height);

  ImGui::SetCurrentContext(ImGui::GetCurrentContext());
}
#endif

// Handle GLFW Errors
static void error_callback(int error, const char* description)
{
   fprintf(stderr, "Error: %s\n", description);
}

// Handle key presses
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
   if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
   {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
   }
   else if (key == GLFW_KEY_W)
   {
      rect[3] += 0.01f;
      if (rect[3] > 1.0f)
         rect[3] = 1.0f;

      rect[1] = rect[3] - 0.5f;
   }
   else if (key == GLFW_KEY_S)
   {
      rect[1] -= 0.01f;
      if (rect[1] < -1.0f)
         rect[1] = -1.0f;

      rect[3] = rect[1] + 0.5f;
   }
   else if (key == GLFW_KEY_D)
   {
      rect[2] += 0.01f;
      if (rect[2] > 1.0f)
         rect[2] = 1.0f;

      rect[0] = rect[2] - 0.5f;
   }
   else if (key == GLFW_KEY_A)
   {
      rect[0] -= 0.01f;
      if (rect[0] < -1.0f)
         rect[0] = -1.0f;

      rect[2] = rect[0] + 0.5f;
   }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
   //if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
   //   draw_rect = !draw_rect;
}

// Handle window resize
void resize(GLFWwindow* window, int width, int height)
{
   GLCALL(glViewport(0, 0, width, height));
}

static void force_exit()
{
#ifdef __EMSCRIPTEN__
   emscripten_force_exit(EXIT_FAILURE);
#else
   exit(EXIT_FAILURE);
#endif
}

static void render()
{
#ifdef __EMSCRIPTEN__
   int curr_width = canvas_get_width();
   int curr_height = canvas_get_height();

   if (curr_width != width || curr_height != height)
   {
      width = curr_width;
      height = curr_height;
      on_size_changed();
   }
#endif

   float vertices[4][9] =
   {
        // aPos                 // aColor                                       // aTexCoord
      { rect[0], rect[1], 0.0f, color_ul.r, color_ul.g, color_ul.b, color_ul.a, 0.0f, 1.0f },
      { rect[2], rect[1], 0.0f, color_ur.r, color_ur.g, color_ur.b, color_ur.a, 1.0f, 1.0f },
      { rect[2], rect[3], 0.0f, color_lr.r, color_lr.g, color_lr.b, color_lr.a, 1.0f, 0.0f },
      { rect[0], rect[3], 0.0f, color_ll.r, color_ll.g, color_ll.b, color_ll.a, 0.0f, 0.0f },
   };

   if (initialize_buffers)
   {
      GLuint indices[6] =
      {
         0, 1, 2,
         0, 2, 3,
      };

      printf("Initialize buffers, program %d\n", program);
      GLCALL(glGenVertexArrays(1, &vao));
      GLCALL(glGenBuffers(1, &vbo));
      GLCALL(glGenBuffers(1, &ebo));

      GLCALL(glBindVertexArray(vao));
      GLCALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
      GLCALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW));
      GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
      GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));
      GLCALL(glEnableVertexAttribArray(0));
      GLCALL(glVertexAttribPointer(0, 3, GL_FLOAT, false, 9 * sizeof(float), (void*)0));
      GLCALL(glEnableVertexAttribArray(1));
      GLCALL(glVertexAttribPointer(1, 4, GL_FLOAT, false, 9 * sizeof(float), (void*)(3 * sizeof(float))));
      GLCALL(glEnableVertexAttribArray(2));
      GLCALL(glVertexAttribPointer(2, 2, GL_FLOAT, false, 9 * sizeof(float), (void*)(7 * sizeof(float))));

      initialize_buffers = false;
   }

   // Clear the window with the background color
   glClear(GL_COLOR_BUFFER_BIT);

   if (draw_rect)
   {
      GLCALL(glUseProgram(program));
      GLCALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
      GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
   }

   // Start the Dear ImGui frame
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   ImGui::Begin("Debug");
   ImGui::Checkbox("Draw Rect", &draw_rect);
   ImGui::End();

   // Render ImGui
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

   glfwSwapBuffers(window);
   glfwPollEvents();
}

int main()
{
#ifdef __EMSCRIPTEN__
   width = canvas_get_width();
   height = canvas_get_height();
#endif

   // Setup the Error handler
   glfwSetErrorCallback(error_callback);

   // Start GLFW
   if (!glfwInit())
   {
      fprintf(stderr, "Error: GLFW Initialization failed.");
      force_exit();
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

   // Create the display window
   window = glfwCreateWindow(width, height, "Demo", NULL, NULL);
   if (!window)
   {
      fprintf(stderr, "Error: GLFW Window Creation Failed");
      glfwTerminate();
      force_exit();
   }

   // Setup the Key Press handler
   glfwSetKeyCallback(window, key_callback);
   glfwSetMouseButtonCallback(window, mouse_button_callback);

   // Setup the resize handler
   glfwSetWindowSizeCallback(window, resize);

   // Select the window as the drawing destination
   glfwMakeContextCurrent(window);

#ifndef __EMSCRIPTEN__
   // use glad to load OpenGL function pointers
   if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
   {
      fprintf(stderr, "Failed to initialize GLAD.\n");
      glfwTerminate();
      force_exit();
   }
#endif

   // Setup Dear ImGui
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGui::StyleColorsDark();

   // Setup Platform/Render backends
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   ImGui_ImplOpenGL3_Init();

   ImGuiIO& io = ImGui::GetIO();

   // Load fonts
   printf("Load ImGui fonts\n");
   io.Fonts->AddFontFromFileTTF("data/ProggyClean.ttf", 13.0f);
   io.Fonts->AddFontDefault();

   // Compile vertex shader
   GLuint vertex_shader;
   GLCALL(vertex_shader = glCreateShader(GL_VERTEX_SHADER));

   const char* shader_code = vertex_shader_source.c_str();
   GLCALL(glShaderSource(vertex_shader, 1, &shader_code, nullptr));
   GLCALL(glCompileShader(vertex_shader));

   // check vertex shader
   GLint result = GL_FALSE;
   int info_log_length;
   GLCALL(glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &result));
   GLCALL(glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &info_log_length));
   if (info_log_length > 0)
   {
      std::vector<char> error_msg(info_log_length+1);
      GLCALL(glGetShaderInfoLog(vertex_shader, info_log_length, NULL, &error_msg[0]));
      printf("Error in vertex shader\n");
      printf("%s\n", &error_msg[0]);
   }

   // Compile fragment shader
   GLuint fragment_shader;
   GLCALL(fragment_shader = glCreateShader(GL_FRAGMENT_SHADER));

   shader_code = fragment_shader_source.c_str();
   GLCALL(glShaderSource(fragment_shader, 1, &shader_code, nullptr));
   GLCALL(glCompileShader(fragment_shader));

   // check fragment shader
   GLCALL(glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &result));
   GLCALL(glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &info_log_length));
   if (info_log_length > 0)
   {
      std::vector<char> error_msg(info_log_length+1);
      GLCALL(glGetShaderInfoLog(fragment_shader, info_log_length, NULL, &error_msg[0]));
      printf("Error in fragment shader\n");
      printf("%s\n", &error_msg[0]);
   }

   // Link program
   GLCALL(program = glCreateProgram());
   GLCALL(glAttachShader(program, vertex_shader));
   GLCALL(glAttachShader(program, fragment_shader));
   GLCALL(glLinkProgram(program));

   // check the program
   GLCALL(glGetProgramiv(program, GL_LINK_STATUS, &result));
   GLCALL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length));
   if (info_log_length > 0)
   {
      std::vector<char> error_msg(info_log_length+1);
      GLCALL(glGetProgramInfoLog(program, info_log_length, NULL, &error_msg[0]));
      printf("Error linking program %d\n", info_log_length);
      printf("%s\n", &error_msg[0]);
   }

   GLCALL(glDetachShader(program, vertex_shader));
   GLCALL(glDetachShader(program, fragment_shader));

   GLCALL(glDeleteShader(vertex_shader));
   GLCALL(glDeleteShader(fragment_shader));

   glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

#ifdef __EMSCRIPTEN__
   emscripten_set_main_loop(render, 0, true);
#else
   while (!glfwWindowShouldClose(window))
   {
      render();
   }

   glfwDestroyWindow(window);
   glfwTerminate();
   exit(EXIT_SUCCESS);
#endif
}
