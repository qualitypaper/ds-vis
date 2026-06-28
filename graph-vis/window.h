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

int W_Run(const char* title) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_Window* win = SDL_CreateWindow(title,
        (int)(1280 * scale), (int)(720 * scale),
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!win) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); return 1; }
    SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    SDL_Renderer* ren = SDL_CreateRenderer(win, NULL);
    if (!ren) { fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError()); return 1; }
    SDL_SetRenderVSync(ren, 1);

    igCreateContext(NULL);
    ImGuiIO* io = igGetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    igStyleColorsDark(NULL);
    ImGuiStyle* style = igGetStyle();
    ImGuiStyle_ScaleAllSizes(style, scale);
    style->FontScaleDpi = scale;
    io->ConfigDpiScaleFonts = true;

    ImGui_ImplSDL3_InitForSDLRenderer(win, ren);
    ImGui_ImplSDLRenderer3_Init(ren);

    bool show_demo = true;

    for (bool done = false; !done; ) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL3_ProcessEvent(&ev);
            if (ev.type == SDL_EVENT_QUIT) done = true;
            if (ev.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                ev.window.windowID == SDL_GetWindowID(win)) done = true;
        }
        if (SDL_GetWindowFlags(win) & SDL_WINDOW_MINIMIZED) { SDL_Delay(10); continue; }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        igNewFrame();
        igShowDemoWindow(&show_demo);
        igRender();

        SDL_SetRenderDrawColorFloat(ren, 0.45f, 0.55f, 0.60f, 1.0f);
        SDL_RenderClear(ren);
        ImGui_ImplSDLRenderer3_RenderDrawData(igGetDrawData(), ren);
        SDL_RenderPresent(ren);
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    igDestroyContext(NULL);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

#endif // GRAPH_VIS_WINDOW
