#ifndef GRAPH_VIS_WINDOW
#define GRAPH_VIS_WINDOW

#include <stdio.h>
#include <SDL3/SDL.h>
#include <cimgui.h>
#include <cimgui_impl.h>

#define igGetIO igGetIO_Nil

// ponytail: SDLRenderer3 backend not in cimgui_impl.h; extern "C" via IMGUI_IMPL_API
extern bool ImGui_ImplSDLRenderer3_Init(SDL_Renderer* renderer);
extern void ImGui_ImplSDLRenderer3_Shutdown(void);
extern void ImGui_ImplSDLRenderer3_NewFrame(void);
extern void ImGui_ImplSDLRenderer3_RenderDrawData(ImDrawData* draw_data, SDL_Renderer* renderer);

extern SDL_Window* win;
extern SDL_Renderer* ren;
extern SDL_Texture* tex;
extern void* pixels;
extern int pitch;
extern uint16_t width, height;

int  W_Init(const char* title);
void W_Close(void);
void W_StartFrame(void);
void W_EndFrame(void);
void W_LockTex(void);
void W_UnlockTex(void);
bool W_PollEvents(void);

#endif // GRAPH_VIS_WINDOW
