#pragma once
#include "Window.h"
#include "Renderer.h"
#include "Camera.h"

namespace pbr {

class Application
{
public:
    void Run();
    void Mainloop();
    Renderer* GetRenderer() const { return m_renderer.get(); }
    static Application& GetSingleton();
private:
    Application();
    void initialize();
    void finalize();
private:
    unique_ptr<Window>      m_window;
    unique_ptr<Renderer>    m_renderer;
    Camera                  m_camera;
    CameraController        m_cameraController;
};

} // namespace pbr
