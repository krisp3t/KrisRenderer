#pragma once

namespace RetroRenderer
{

class ConfigPanel
{
public:
    ConfigPanel(SDL_Window *window, SDL_Renderer *renderer);
    ~ConfigPanel();
    bool Init(SDL_Window *window, SDL_Renderer *renderer);
    void Destroy();
};

}





