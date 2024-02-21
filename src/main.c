#include "SDL_keyboard.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include "SDL_surface.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <wchar.h>

#include "defs.h"

typedef struct {
  float x;
  float y;
} Vector2f;

typedef struct {
  int32_t x;
  int32_t y;
} Vector2i;

typedef enum { PEACH, PURPLE, BLUE, PINK } AlienTypeEnum;

typedef struct {
  Vector2i size;
  Vector2i sprite_index;
} AlienType;

typedef struct {
  Vector2f pos;
  AlienType type;
  int num;
} Alien;

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Texture *sprites;
  TTF_Font *font;
  struct {
    Vector2f pos;
  } ship;
  struct {
    int64_t delta;
    double delta_ns;
    int64_t last_frame;
    int64_t last_second;
    int32_t frames;
    int32_t fps;
    int64_t start;
    int32_t now_seconds;
  } time;
  struct {
    struct {
      bool down, pressed;
    } left, right, shoot;
  } input;
  Alien *aliens;
  size_t alien_count;
  AlienType alien_types[4];
} State;

const float SHIP_SPEED = 160.0f;
const int64_t NANOS_PER_SECOND = 1000000000;
const int SPRITE_SIZE = 16;
const Vector2i RENDER_SIZE = {224, 256};

// sprite indices
const Vector2i SHIP_SPRITE_INDEX = {0, 0};

Vector2f add_vectors(Vector2f v1, Vector2f v2) {
  Vector2f result;
  result.x = v1.x + v2.x;
  result.y = v1.y + v2.y;
  return result;
}

void draw_fps(SDL_Renderer *renderer, TTF_Font *font, int fps) {
  SDL_Color text_color = {255, 255, 255, 255};
  char fps_text[32];
  snprintf(fps_text, sizeof(fps_text), "FPS: %d", fps);

  SDL_Surface *text_surface = TTF_RenderText_Solid(font, fps_text, text_color);
  if (text_surface == NULL) {
    printf("Unable to render text surface! SDL_ttf Error: %s\n",
           TTF_GetError());
    return;
  }

  SDL_Texture *text_texture =
      SDL_CreateTextureFromSurface(renderer, text_surface);
  if (text_texture == NULL) {
    printf("Unable to create texture from rendered text! SDL Error: %s\n",
           SDL_GetError());
    SDL_FreeSurface(text_surface);
    return;
  }

  int text_width = text_surface->w;
  int text_height = text_surface->h;
  SDL_Rect render_quad = {RENDER_SIZE.x - text_width - 10, 10, text_width,
                          text_height};

  SDL_FreeSurface(text_surface);

  SDL_RenderCopy(renderer, text_texture, NULL, &render_quad);
  SDL_DestroyTexture(text_texture);
}

void draw_background(SDL_Renderer *renderer, SDL_Texture *texture) {
  SDL_SetRenderTarget(renderer, texture);
  SDL_SetRenderDrawColor(renderer, 20, 20, 20, 0xFF);
  SDL_RenderClear(renderer);

  // Reset to default render target
  SDL_SetRenderTarget(renderer, NULL);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
}

void draw_sprite(State *state, Vector2i index, Vector2f pos) {
  SDL_Rect src_rect = {index.x * SPRITE_SIZE, index.y * SPRITE_SIZE,
                       SPRITE_SIZE, SPRITE_SIZE};
  SDL_Rect dest_rect = {(int)pos.x, (int)pos.y, SPRITE_SIZE, SPRITE_SIZE};
  SDL_RenderCopy(state->renderer, state->sprites, &src_rect, &dest_rect);
}

void initialize_alien_types(State *state) {
  state->alien_types[PEACH] =
      (AlienType){.size = {16, 16}, .sprite_index = {1, 0}};
  state->alien_types[PURPLE] =
      (AlienType){.size = {16, 16}, .sprite_index = {2, 0}};
  state->alien_types[BLUE] =
      (AlienType){.size = {16, 16}, .sprite_index = {3, 0}};
  state->alien_types[PINK] =
      (AlienType){.size = {16, 16}, .sprite_index = {4, 0}};
}

void initialize_stage(State *state) {
  int initial_y_count = 4;
  int initial_x_count = 10;
  size_t initial_size = initial_y_count * initial_x_count;
  state->alien_count = 0;
  state->aliens = malloc(initial_size * sizeof(Alien));
  if (!state->aliens) {
    printf("Failed to allocate memory for aliens\n");
    exit(STATUS_ERROR);
  }

  int num_count = 0;
  for (int y = 0; y < initial_y_count; y++) {
    for (int x = 0; x < initial_x_count; x++) {
      if (state->alien_count == initial_size) {
        initial_size *= 2;
        Alien *new_ptr = realloc(state->aliens, initial_size * sizeof(Alien));
        if (!new_ptr) {
          printf("Failed to reallocate memory for aliens\n");
          free(state->aliens);
          exit(STATUS_ERROR);
        }
        state->aliens = new_ptr;
      }

      Alien new_alien = {
          .num = num_count,
          .pos = add_vectors((Vector2f){10, 32}, (Vector2f){x * 18, y * 18}),
          .type = state->alien_types[y % 4]};
      state->aliens[state->alien_count++] = new_alien;

      num_count += 1;
    }
  }
}

void update(State *state) {
  double delta_seconds = state->time.delta_ns / NANOS_PER_SECOND;

  if (state->input.left.down) {
    state->ship.pos.x -= SHIP_SPEED * delta_seconds;
  }

  if (state->input.right.down) {
    state->ship.pos.x += SHIP_SPEED * delta_seconds;
  }

  for (int i = 0; i < state->alien_count; i++) {
    Alien *alien = &state->aliens[i];

    alien->pos.x += sin(state->time.now_seconds + (alien->num * 0.15));
  }
}

void render(State *state) {
  // clear the renderer
  SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 0xFF);
  SDL_RenderClear(state->renderer);

  // draw background, ship, and enemies
  draw_background(state->renderer, state->texture);
  draw_sprite(state, SHIP_SPRITE_INDEX, state->ship.pos);

  for (int i = 0; i < state->alien_count; i++) {
    draw_sprite(state, state->aliens[i].type.sprite_index,
                state->aliens[i].pos);
  }

  // overlays
  draw_fps(state->renderer, state->font, state->time.fps);

  // update the screen
  SDL_RenderPresent(state->renderer);
}

int main(int argc, char *argv[]) {
  State state;

  // initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
    return STATUS_ERROR;
  }

  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    printf("SDL_image could not initialize! SDL_image Error: %s\n",
           IMG_GetError());
    SDL_Quit();
    return STATUS_ERROR;
  }

  if (TTF_Init() == -1) {
    printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
    SDL_Quit();
    return STATUS_ERROR;
  }

  // create window
  state.window = SDL_CreateWindow("Space Invaders", 200, 200, SCREEN_WIDTH,
                                  SCREEN_HEIGHT, 0);
  if (!state.window) {
    printf("Could not create window: %s\n", SDL_GetError());
    return STATUS_ERROR;
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

  // create renderer
  state.renderer = SDL_CreateRenderer(
      state.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (state.renderer == NULL) {
    printf("Could not create renderer: %s\n", SDL_GetError());
    return STATUS_ERROR;
  }

  SDL_RenderSetLogicalSize(state.renderer, RENDER_SIZE.x, RENDER_SIZE.y);

  // create backbuffer
  state.texture =
      SDL_CreateTexture(state.renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_TARGET, RENDER_SIZE.x, RENDER_SIZE.y);
  if (!state.texture) {
    printf("Could not create texture: %s\n", SDL_GetError());
    return STATUS_ERROR;
  }

  // setup font
  state.font = TTF_OpenFont("assets/fonts/Micro5-Regular.ttf", 12);
  if (!state.font) {
    printf("Failed to load font: %s\n", TTF_GetError());
    return STATUS_ERROR;
  }

  // load sprites from file
  SDL_Surface *sprite_image_surface = IMG_Load("assets/images/sprite.png");
  if (!sprite_image_surface) {
    printf("Could not load sprite image: %s\n", SDL_GetError());
    SDL_FreeSurface(sprite_image_surface);
    return STATUS_ERROR;
  }

  state.sprites =
      SDL_CreateTextureFromSurface(state.renderer, sprite_image_surface);
  if (!state.sprites) {
    SDL_FreeSurface(sprite_image_surface);
    printf("Could not create texture: %s\n", SDL_GetError());
    return STATUS_ERROR;
  }

  SDL_FreeSurface(sprite_image_surface);

  //// Main loop
  SDL_Event e;
  bool quit = false;

  if (SDL_GL_SetSwapInterval(1) < 0) {
    printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
  }

  // Setup initial state //
  // timing
  state.time.last_frame = SDL_GetPerformanceCounter();
  state.time.last_second = state.time.last_frame;
  state.time.frames = 0;
  state.time.fps = 0;

  // ship
  state.ship.pos.x = (float)RENDER_SIZE.x / 2;
  state.ship.pos.y = RENDER_SIZE.y - SPRITE_SIZE - 10;

  initialize_alien_types(&state);
  initialize_stage(&state);

  while (!quit) {
    uint64_t now = SDL_GetPerformanceCounter();
    uint64_t frequency = SDL_GetPerformanceFrequency();

    if (state.time.start == 0) {
      state.time.start = now;
    }

    state.time.now_seconds =
        (int32_t)((now - state.time.start) / NANOS_PER_SECOND);

    state.time.delta = now - state.time.last_frame;
    state.time.delta_ns =
        (double)state.time.delta * NANOS_PER_SECOND / (double)frequency;
    state.time.last_frame = now;
    state.time.frames++;

    if (now - state.time.last_second >= frequency) {
      state.time.fps = state.time.frames;
      state.time.frames = 0;
      state.time.last_second = now;
    }

    // update input state based off of keyboard state
    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);

    state.input.left.down = keyboard_state[SDL_SCANCODE_LEFT] != 0 ||
                            keyboard_state[SDL_SCANCODE_A] != 0;

    state.input.right.down = keyboard_state[SDL_SCANCODE_RIGHT] != 0 ||
                             keyboard_state[SDL_SCANCODE_D] != 0;

    state.input.shoot.down = keyboard_state[SDL_SCANCODE_SPACE] != 0;

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
        case SDLK_LEFT:
        case SDLK_a:
          state.input.left.pressed = true;
          break;
        case SDLK_RIGHT:
        case SDLK_d:
          state.input.right.pressed = true;
          break;
        case SDLK_SPACE:
          state.input.shoot.pressed = true;
          break;
        case SDLK_ESCAPE:
          quit = true;
          break;
        }
      }
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }

    update(&state);
    render(&state);
  }

  // clean up and exit
  free(state.aliens);
  TTF_CloseFont(state.font);
  SDL_DestroyTexture(state.sprites);
  SDL_DestroyTexture(state.texture);
  SDL_DestroyRenderer(state.renderer);
  SDL_DestroyWindow(state.window);

  TTF_Quit();
  IMG_Quit();
  SDL_Quit();
  return STATUS_OK;
}
