#include "SDL_rect.h"
#include "SDL_render.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>

#include "defs.h"

struct State {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
};

int main(int argc, char *argv[]) {
  struct State state;

  // create window
  state.window = SDL_CreateWindow("Space Invaders", 200, 200, 1280, 720, 0);

  if (state.window == NULL) {
    printf("Could not create window: %s\n", SDL_GetError());
    return STATUS_ERROR;
  }

  // create renderer
  state.renderer = SDL_CreateRenderer(state.window, -1, 1);

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

  // setup background
  SDL_SetRenderTarget(state.renderer, state.texture);

  SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 0xFF);
  SDL_RenderClear(state.renderer);

  // draw square
  SDL_SetRenderDrawColor(state.renderer, 0xFF, 0, 0xFF, 0xFF);
  SDL_Rect rect = {0, 0, 128, 128};
  SDL_RenderFillRect(state.renderer, &rect);

  // draw texture to screen
  SDL_SetRenderTarget(state.renderer, NULL);
  SDL_RenderCopy(state.renderer, state.texture, NULL, NULL);

  SDL_RenderPresent(state.renderer);

  // Main loop
  SDL_Event e;
  bool quit = false;
  while (!quit) {
    while (SDL_PollEvent(&e)) {
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
