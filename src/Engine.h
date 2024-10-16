#pragma once
#include "Window/DisplayManager.h"
#include "Renderer/RenderManager.h"
#include "Window/InputManager.h"

namespace RetroRenderer
{

class Engine
{
public:
    Engine() = default;
    ~Engine() = default;

    bool Init();
    void Run();
    void Destroy();
private:
    DisplayManager m_DisplayManager;
    RenderManager m_RenderManager;
    InputManager m_InputManager;
};

}
