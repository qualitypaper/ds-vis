#include "window.h"

SDL_Window* win;
SDL_Renderer* ren;
SDL_Texture* tex;
void* pixels;
int pitch;
uint16_t width = 1280, height = 720;

extern bool done;

int W_Init(const char* title) {
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return 1;
	}

	float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	win = SDL_CreateWindow(title,
		(int)(width * scale), (int)(height * scale),
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
	if (!win) { fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError()); return 1; }
	SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	ren = SDL_CreateRenderer(win, NULL);
	if (!ren) { fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError()); return 1; }
	SDL_SetRenderVSync(ren, 1);

	tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		width, height);
	SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);

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
	return 0;
}

void W_Close() {
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	igDestroyContext(NULL);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
}

void W_StartFrame() {
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	igNewFrame();
	SDL_RenderClear(ren);
}

void W_EndFrame() {
	igRender();
	ImGui_ImplSDLRenderer3_RenderDrawData(igGetDrawData(), ren);
	SDL_RenderPresent(ren);
}

void W_LockTex() {
	SDL_LockTexture(tex, NULL, &pixels, &pitch);
}

void W_UnlockTex() {
	SDL_UnlockTexture(tex);
}

bool W_PollEvents() {
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		ImGui_ImplSDL3_ProcessEvent(&ev);

		switch (ev.type) {
		case SDL_EVENT_QUIT: done = true;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			if (ev.window.windowID == SDL_GetWindowID(win))
				done = true;
		}

		if (done) return false;
	}
	if (SDL_GetWindowFlags(win) & SDL_WINDOW_MINIMIZED) {
		SDL_Delay(10);
		return false;
	}

	return true;
}