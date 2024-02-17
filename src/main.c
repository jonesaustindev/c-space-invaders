#include "SDL_rect.h"
#include "SDL_render.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "defs.h"

const float SHIP_SPEED = 0.025f;

struct State {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  struct {
    int x;
    int y;
  } ship;
  struct {
    int64_t delta;
    double delta_ns;
    int64_t last_frame;
    int64_t last_second;
    int32_t frames;
    int32_t fps;
  } time;
};

int main(int argc, char *argv[]) {
  struct State state;

  SDL_Init(SDL_INIT_VIDEO);

  // create window
  state.window = SDL_CreateWindow("Space Invaders", 200, 200, 1280, 720, 0);

  if (state.window == NULL) {
    printf("Could not create window: %s\n", SDL_GetError());
    return STATUS_ERROR;
  }

  // create renderer
  state.renderer = SDL_CreateRenderer(
      state.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (state.renderer == NULL) {
    printf("Could not create renderer: %s\n", SDL_GetError());
    return STATUS_ERROR;
  }

  // create backbuffer
  state.texture = SDL_CreateTexture(state.renderer, SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_TARGET, 256, 144);
  if (state.texture == NULL) {
    printf("Could not create texture: %s\n", SDL_GetError());
    return STATUS_ERROR;
  }

  // setup ship
  state.ship.x = 128;
  state.ship.y = 72;

  //// Main loop
  SDL_Event e;
  bool quit = false;

  SDL_RendererInfo info;
  SDL_GetRendererInfo(state.renderer, &info);
  printf("Renderer name: %s\n", info.name);

  if (SDL_GL_SetSwapInterval(1) < 0) {
    printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
  }

  // Setup timing
  state.time.last_frame = SDL_GetPerformanceCounter();
  state.time.last_second = state.time.last_frame;
  state.time.frames = 0;
  state.time.fps = 0;

  while (!quit) {
    uint64_t now = SDL_GetPerformanceCounter();
    uint64_t frequency = SDL_GetPerformanceFrequency();

    state.time.delta = now - state.time.last_frame;
    state.time.delta_ns =
        (double)state.time.delta * 1000000000.0 / (double)frequency;
    state.time.last_frame = now;
    state.time.frames++;

    if (now - state.time.last_second >= frequency) {
      state.time.fps = state.time.frames;
      state.time.frames = 0;
      state.time.last_second = now;

      printf("FPS: %d\n", state.time.fps);
    }

    // setup background
    SDL_SetRenderTarget(state.renderer, state.texture);

    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 0xFF);
    SDL_RenderClear(state.renderer);

    // draw square
    SDL_SetRenderDrawColor(state.renderer, 0xFF, 0, 0xFF, 0xFF);
    SDL_Rect rect = {state.ship.x, state.ship.y, 16, 16};
    SDL_RenderFillRect(state.renderer, &rect);

    // draw texture to screen
    SDL_SetRenderTarget(state.renderer, NULL);
    SDL_RenderCopy(state.renderer, state.texture, NULL, NULL);

    SDL_RenderPresent(state.renderer);

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
        case SDLK_LEFT:
        case SDLK_a:
          state.ship.x -= 1;
          break;
        case SDLK_RIGHT:
        case SDLK_d:
          state.ship.x += 1;
          break;
        case SDLK_UP:
        case SDLK_w:
          state.ship.y -= 1;
          break;
        case SDLK_DOWN:
        case SDLK_s:
          state.ship.y += 1;
          break;
        }
      }
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }
  }

  // Clean up and exit
  SDL_DestroyTexture(state.texture);
  SDL_DestroyRenderer(state.renderer);
  SDL_DestroyWindow(state.window);

  SDL_Quit();
  return STATUS_OK;
}
