#include <iostream>
#include <SDL2/SDL.h>
#include <memory>
#include <algorithm>
#include <vector>
#include <random>


using Window = std::shared_ptr<SDL_Window>;
using Renderer = std::shared_ptr<SDL_Renderer>;
using Texture = std::shared_ptr<SDL_Texture>;
using Pixels = std::vector<std::uint32_t>;
using Cells = std::vector<std::uint8_t>;


SDL_Window *create_window(int width, int height);
SDL_Renderer *create_renderer(const Window &w);
SDL_Texture *create_texture(const Renderer &r, int width, int height);
void destroy_window(SDL_Window *w);
void destroy_renderer(SDL_Renderer *r);
void destroy_texture(SDL_Texture *r);
Pixels createPixelData(int width, int height);
Cells createCells(int width, int height);
void updateCells(Cells &cells, std::uint32_t w, std::uint32_t h);
void updatePixelData(Pixels &pixels, const Cells &cells);

int main()
try{
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
        throw std::runtime_error("SDL initialize failed!");

    SDL_DisplayMode current;

    if(SDL_GetCurrentDisplayMode(0, &current) != 0)
        throw std::runtime_error("SDL can't get resolution!");

    int width = current.w;
    int height = current.h;
    int field_w = width / 2;
    int field_h= height / 2;


    Window window = Window(create_window(width, height), destroy_window);
    if(!window)
        throw std::runtime_error("Create window failed");

    Renderer renderer = Renderer(create_renderer(window), destroy_renderer);
    if(!renderer)
        throw std::runtime_error("Create renderer failed");

    Texture texture = Texture(create_texture(renderer, field_w, field_h), destroy_texture);
    if(!texture)
        throw std::runtime_error("Create texture failed");

    Pixels pixels = createPixelData(field_w, field_h);
    Cells cells = createCells(field_w, field_h);


    SDL_Event event;
    bool run = true;
    while(run){
        updatePixelData(pixels, cells);
        updateCells(cells, field_w, field_h);
        SDL_UpdateTexture(texture.get(), nullptr, pixels.data(), field_w * 4);
        SDL_RenderClear(renderer.get());
        SDL_RenderCopy(renderer.get(), texture.get(), nullptr, nullptr);
        SDL_RenderPresent(renderer.get());
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    if(event.key.keysym.sym == SDLK_ESCAPE)
                        run = false;
                    break;
                case SDL_QUIT:
                    run = false;
                    break;
            }
        }
    }

    SDL_Quit();
    return 0;
}catch(std::exception e){
    std::cerr << e.what() << std::endl;
    SDL_Quit();
    return 1;
}

SDL_Window *create_window(int width, int height){
    std::cout << "Create window" << std::endl;
    return SDL_CreateWindow("Life the game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
}
void destroy_window(SDL_Window *w){
    std::cout << "Destroy window" << std::endl;
    SDL_DestroyWindow(w);
}

SDL_Renderer *create_renderer(const Window &w){
    std::cout << "Create renderer" << std::endl;
    return SDL_CreateRenderer(w.get(), -1, SDL_RENDERER_ACCELERATED);
}
void destroy_renderer(SDL_Renderer *r){
    std::cout << "Destroy renderer" << std::endl;
    SDL_DestroyRenderer(r);
}

SDL_Texture *create_texture(const Renderer &r, int width, int height){
    std::cout << "Create texture" << std::endl;
    return SDL_CreateTexture(r.get(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
}

void destroy_texture(SDL_Texture *t){
    std::cout << "Destroy texture" << std::endl;
    SDL_DestroyTexture(t);
}

Pixels createPixelData(int width, int height){
    return Pixels(width * height, 0);
}

Cells createCells(int width, int height){
    Cells cells (width * height, 0 );
    std::generate(cells.begin(), cells.end(), [](){
        static std::random_device rd;
        static std::mt19937 mt(rd());
        static std::uniform_int_distribution<uint8_t> dist(0, 100);
        return dist(mt) < 15;
    });

    return cells;
}

inline bool isLive(const std::uint8_t *data, int x, int y, int w, int h){
    if(x > 0 && y > 0 && x < w && y < h)
        return data[y * w + x];
    return false;
}

inline std::uint8_t neighbors(const std::uint8_t *data, int x, int y, int w, int h){
    std::uint8_t count = 0;
    count += isLive(data, x - 1, y - 1, w, h);
    count += isLive(data, x    , y - 1, w, h);
    count += isLive(data, x + 1, y - 1, w, h);

    count += isLive(data, x - 1, y    , w, h);
    count += isLive(data, x + 1, y    , w, h);

    count += isLive(data, x - 1, y + 1, w, h);
    count += isLive(data, x    , y + 1, w, h);
    count += isLive(data, x + 1, y + 1, w, h);
    return count;
}

void updateCells(Cells &cells, std::uint32_t w, std::uint32_t h){
    const Cells old(cells);
    const std::uint8_t *data = old.data();

    for(auto y = 0u; y < h; y++)
        for(auto x = 0u; x < w; x++){
            auto index = y * w + x;
            std::uint8_t count = neighbors(data, x, y, w, h);
            if(data[index]){
                if(count < 2 || count > 3)
                    cells[index] = 0;
                else
                    if(cells[index] < 0xFF)
                        ++cells[index];
            }else{
                if(count == 3)
                    cells[index] = 1;
            }
        }

}

void updatePixelData(Pixels &pixels, const Cells &cells){
    auto c = cells.begin();
    auto p = pixels.begin();
    for(; c != cells.end(); ++c, ++p){
        *p = *c ? (0x0000ff00 | *c << 24): 0x00000000;
    };
}
