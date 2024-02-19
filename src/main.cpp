// 
// pointcloud viewer / processor for ARM
//

#include <iostream>
#define GLEW_STATIC                     // damit GLEW statisch gelinkt werden kann
#include <GL/glew.h>
//#define SDL_MAIN_HANDLED                // eigene Hauptfunktion für SDL                                                                                                                                                                         #ifdef _WIN32                           // nur für Windows                                                              #include 

#include "imgui.h"
#ifdef _WIN32               // Win
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#else
#include "src/imgui_impl_sdl2.h"
#include "src/imgui_impl_opengl3.h"
#endif

#ifdef _WIN32               // Win
#include <SDL.h>
#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")
#else
#include <SDL2/SDL.h>       // Linux / MacOS  
#endif

#include "pointcloud.hpp" // <-- need to be fixed, uncomment this and gpu_primitives will not work anymore ...1/5/2024

// simulation
#include <chrono>
#include <SDL_thread.h>
#include "sim.hpp"

#include "draw.hpp"
#include "draw_imgui.hpp"

#include "init.hpp"
#include "user.hpp"


#ifdef eCAL_ON
void Pointcloud_Callback(const pcl::PointCloud2 pointcloud_msg)
{
  pio.ecal_callback(pointcloud_msg); // 2do: directly give this to AddReceiveCallback
//  gpu_push_buffers(p.numpoints);
}
#endif // eCAL_ON


int main(int argc, char** argv)
{
  SDL_Window* window = init_SDL();

  SDL_GLContext glContext = SDL_GL_CreateContext(window);

  // GLEW initialisieren (Context muss bereits existieren)
  init_GlEW();

  init_GL();
///  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe
// debugging, why is second tri not drawn? --> vertex attribs were missing
  glDisable(GL_CULL_FACE);

  gpu_create_shaders();
  gpu_create_variables(); // <-- das hatte gefehlt, um das grid (+color) darzustellen
                          // ... muss VOR dem pushen der buffers sein! (create buffers vorher möglich)


  // (b) point cloud
#ifdef __linux__
  std::string s[2] = { "./data/veloout_0_10.pcd","./data/hrlframe_0_back.pcd" };
#endif
#ifdef WIN32
  std::string s[2] = { "../data/veloout_0_10.pcd","../data/hrlframe_0_back.pcd" };
#endif

//  int i_toggle = 1;
  std::cout << "main.cpp: loading " << s[1] << std::endl;
  int numpoints = lloft::pointcloud_load(s[0]);
  pcl_gpu_create_buffers();
  pcl_gpu_push_buffers(numpoints);
  
  
///  gpu_prim two_tris;
  two_tris.create_buffers();
  two_tris_i.create_buffers_from_faces();

  // (c) grid
// debug, why is the grid not drawn? ---> attribute location was missing (for color) --> gpu_create_variables()
  grid_create(-300, 300, 2.5);
  grid_gpu_push_buffers();

  // 5 draw --> s. draw.hpp --> render()
  // Setup Dear ImGui context
  ImGuiIO& io = init_Imgui(window, glContext);

  SDL_Thread* tsim;
  tsim = SDL_CreateThread(thread1, "sim", 0);

  // Game Loop
  bool close = false;
  do
  {
    // 2do: split up into
        // (a) sim step             --> done 1/4/2024 --> thread
        // (b) collision detection
        // (c) draw
    render(window);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    draw_imgui(io);

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Double Buffering (angezeigter Buffer tauschen)
    SDL_GL_SwapWindow(window);

    // user interaction (mouse, keys, ...)
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);
      
      process_event(event); // mouse wheel, drag & drop, ..., quit

      if (event.type == SDL_QUIT)
      {
        close = true;
      }
    }
  } while (!close);
  
  SDL_Quit();
  
  grid_free();
  pcl_gpu_free_buffers();
  // two_tris cleanup ...

  return 0;
}
