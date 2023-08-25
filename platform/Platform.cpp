#ifndef PLATFORM
#define PLATFORM

#include <cstdint>
#include <SDL2/SDL.h>

class Platform {
    public:
    Platform(char const* title, int width, int height, int text_width, int text_height);
    ~Platform();

    void update(void const* buffer, int pitch);

    bool process_input(uint8_t* keys);

    private:
    SDL_Window* window{};
    SDL_Renderer* renderer{};
    SDL_Texture* texture{};
};

Platform::Platform(char const* title, int width, int height, int text_width, int text_height) {
    SDL_Init(SDL_INIT_VIDEO);
    this->window = SDL_CreateWindow(title, 0, 0, width, height, SDL_WINDOW_SHOWN);
    this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED);
    this->texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, text_width, text_height);
}

Platform::~Platform() {
    SDL_DestroyTexture(this->texture);
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
}

void Platform::update(void const* buffer, int pitch)
{
    SDL_UpdateTexture(this->texture, nullptr, buffer, pitch);
    SDL_RenderClear(this->renderer);
    SDL_RenderCopy(this->renderer, this->texture, nullptr, nullptr);
    SDL_RenderPresent(this->renderer);
}

bool Platform::process_input(uint8_t* keys) {
    bool quit = false;
    SDL_Event e;

    while(SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
            quit = true;
            break;

            case SDL_KEYDOWN:
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                quit = true;
                break;
                case SDLK_x:
                keys[0] = 1;
                break;
                case SDLK_1:
                keys[1] = 1;
                break;
                case SDLK_2:
                keys[2] = 1;
                break;
                case SDLK_3:
                keys[3] = 1;
                break;
                case SDLK_q:
                keys[4] = 1;
                break;
                case SDLK_w:
                keys[5] = 1;
                break;
                case SDLK_e:
                keys[6] = 1;
                break;
                case SDLK_a:
                keys[7] = 1;
                break;
                case SDLK_s:
                keys[8] = 1;
                break;
                case SDLK_d:
                keys[9] = 1;
                break;
                case SDLK_z:
                keys[0xA] = 1;
                break;
                case SDLK_c:
                keys[0xB] = 1;
                break;
                case SDLK_4:
                keys[0xC] = 1;
                break;
                case SDLK_r:
                keys[0xD] = 1;
                break;
                case SDLK_f:
                keys[0xE] = 1;
                break;
                case SDLK_v:
                keys[0xF] = 1;
                break;
            }
            break;
            case SDL_KEYUP:
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                quit = true;
                break;
                case SDLK_x:
                keys[0] = 0;
                break;
                case SDLK_1:
                keys[1] = 0;
                break;
                case SDLK_2:
                keys[2] = 0;
                break;
                case SDLK_3:
                keys[3] = 0;
                break;
                case SDLK_q:
                keys[4] = 0;
                break;
                case SDLK_w:
                keys[5] = 0;
                break;
                case SDLK_e:
                keys[6] = 0;
                break;
                case SDLK_a:
                keys[7] = 0;
                break;
                case SDLK_s:
                keys[8] = 0;
                break;
                case SDLK_d:
                keys[9] = 0;
                break;
                case SDLK_z:
                keys[0xA] = 0;
                break;
                case SDLK_c:
                keys[0xB] = 0;
                break;
                case SDLK_4:
                keys[0xC] = 0;
                break;
                case SDLK_r:
                keys[0xD] = 0;
                break;
                case SDLK_f:
                keys[0xE] = 0;
                break;
                case SDLK_v:
                keys[0xF] = 0;
                break;
            }
            break;
        }
    }

    return quit;
}

#endif