#ifndef DRAW_HPP
#define DRAW_HPP

//#include "linmath.h"

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

int colorcoding = 0;

GLenum GLerr;

// 2do: move to camera class -->
class camera
{
public:
  vec3 rot;
  vec3 trans;
};
camera cam;
float zoom = 0;// -30.0f;

// small explanation of GLSL entities
// ----------------------------------
// attribute is an openGL variable space. contrary to uniform it cannot be ... (used in a shader?) 
// uniform
// varying   # deprecated > version 140
const char* vertex_shader_text =
#include "./shader/start.vs"
;
const char* fragment_shader_text =
#include "./shader/start.fs"
;

GLuint program;
GLint mvp_location;
GLint T_location;

// --------------
// vertex + color
// --------------
// (a) gpu data storage
// vertex buffer object, e.g. for ...
// ... point cloud
GLuint vertex_buffer;
GLuint colorbuffer[3]; // 3 color buffer (Gl-)objects to store precalculated color codings (height, dist, ...)
// ... grid
GLuint grid_vbo;
GLuint grid_vbo_col;

///unsigned int VBO;
///unsigned int n_vertices; // <-- dem draw call übergeben

// (b) gpu shader attributes
GLint vpos_location, vcol_location, tex_location;

#include "gpu_primitives.hpp" // include after vpos_location, 2do: fix this

gpu_prim two_tris; // to be after incl. gpu_primitives
gpu_prim_indexed two_tris_i;



void gpu_create_shaders() {  
  GLuint vertex_shader, fragment_shader;
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShader(vertex_shader);
  // https://www.khronos.org/opengl/wiki/Example/GLSL_Shader_Compile_Error_Testing
  GLint isCompiled = 0;
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE)
  {
    GLint maxLength = 0;
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(vertex_shader, maxLength, &maxLength, &errorLog[0]);

    // Provide the infolog in whatever manor you deem best.
    // Exit with failure.
    glDeleteShader(vertex_shader); // Don't leak the shader.
    return;
  }
  // Shader compilation is successful.

  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
  glCompileShader(fragment_shader);
  isCompiled = 0;
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE)
  {
    GLint maxLength = 0;
    glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(fragment_shader, maxLength, &maxLength, &errorLog[0]);

    // Provide the infolog in whatever manor you deem best.
    // Exit with failure.
    glDeleteShader(fragment_shader); // Don't leak the shader.
    return;
  }
  // Shader compilation is successful.

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  GLerr = glGetError();
}

void gpu_create_variables() {
  mvp_location = glGetUniformLocation(program, "MVP");
  vpos_location = glGetAttribLocation(program, "vPos");
  vcol_location = glGetAttribLocation(program, "vCol");
  tex_location = glGetAttribLocation(program, "texCoord");
  T_location = glGetUniformLocation(program, "T"); // translation
  GLenum err = glGetError();
  if (err != 0)
  {
    int i = 1;
  }
}

// put into a camera class?
mat4x4 mvp;
void cam_update_center(int width, int height) { // rotate around center
  float aspect;
  mat4x4 view, projection;
//  mat4x4 m, p;

  // Calculate aspect of window
  if (height > 0)
    aspect = (float)width / (float)height;
  else
    aspect = 1.f;

  // Setup perspective projection matrix
  glMatrixMode(GL_PROJECTION);
  mat4x4_perspective(projection,
    65.f * (float)M_PI / 180.f,
    aspect,
    0.1f, 300.f);

  glMatrixMode(GL_MODELVIEW);
  {
    vec3 eye = { 3.f, 1.5f, 3.f };
    vec3 center = { 0.f, 0.f, 0.f };
    vec3 up = { 0.f, 1.f, 0.f };
    mat4x4_look_at(view, eye, center, up);
  }
// "move camera", translate + rotate around center
  mat4x4_translate(view, cam.trans[0], cam.trans[1], zoom + cam.trans[2]);
  //    mat4x4_rotate_Z(m, m, (float)glfwGetTime());
  mat4x4_rotate_X(view, view, cam.rot[0]);
  mat4x4_rotate_Y(view, view, 180.0f + cam.rot[1]);
  mat4x4_rotate_Z(view, view, 90.0f + cam.rot[2]);

  mat4x4_mul(mvp, projection, view);
}

void cam_update(int width, int height) {
  float aspect;
  mat4x4 view, projection;
  //  mat4x4 m, p;

    // Calculate aspect of window
  if (height > 0)
    aspect = (float)width / (float)height;
  else
    aspect = 1.f;

  // Setup perspective projection matrix
  glMatrixMode(GL_PROJECTION);
  mat4x4_perspective(projection,
    65.f * (float)M_PI / 180.f,
    aspect,
    0.1f, 300.f);

  glMatrixMode(GL_MODELVIEW);
  {
    vec3 eye = { 3.f, 1.5f, 3.f };
    vec3 center = { 0.f, 0.f, 0.f };
    vec3 up = { 0.f, 1.f, 0.f };
    mat4x4_look_at(view, eye, center, up);
  }
  // "move camera", translate + rotate around center
/////  mat4x4_translate(view, cam.trans[0], cam.trans[1], zoom + cam.trans[2]);
  //    mat4x4_rotate_Z(m, m, (float)glfwGetTime());
///  mat4x4_translate(view, -5, 0, 0); // rotate around 10,0,0
  mat4x4_rotate_X(view, view, cam.rot[0]);
  mat4x4_rotate_Y(view, view, 180.0f + cam.rot[1]);
  mat4x4_rotate_Z(view, view, 90.0f + cam.rot[2]);
///  mat4x4_translate(view, 0.0, 0, 0);
  mat4x4_translate_in_place(view, cam.trans[0], cam.trans[1], zoom + cam.trans[2]);

  mat4x4_mul(mvp, projection, view);
}


void render(SDL_Window* window) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    cam_update(width, height);

    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // https://stackoverflow.com/questions/46020871/opengl-depth-testing-not-working-glew-sdl2

    glUseProgram(program);
    GLerr = glGetError();
    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp); // I got an GLerr 1282, when the shader was not loaded+compiled
    GLerr = glGetError();

    // draw/render - this also sets the correct VAO
    grid_render();
    GLerr = glGetError();

    two_tris.render();
    two_tris_i.render();

    pcl_render();

    GLerr = glGetError();
    if (GLerr != 0)
    {
        int i = 1;
    }
}

void resize(int w, int h) {
  if (h == 0)                     // Prevent A Divide By Zero By
  {
    h = 1;                      // Making Height Equal One
  }
  glViewport(0, 0, w, h);   // Reset The Current Viewport
 }

#endif // DRAW_HPP
