#ifndef SCENE_H
#define SCENE_H

#include <QMainWindow>
#include "Deps/DXSDK/include/d3d8.h"
#include "Deps/DXSDK/include/d3dx8.h"

namespace UltraEd
{
    class Scene
    {
    public:
        Scene();
        ~Scene();
        bool Create(HWND windowHandle);

    private:
        std::unique_ptr<IDirect3D8, std::function<void(IDirect3D8*)>> m_d3d8;
        std::unique_ptr<IDirect3DDevice8, std::function<void(IDirect3DDevice8*)>> m_device;
        D3DPRESENT_PARAMETERS m_d3dpp;
    };
}

#endif // SCENE_H
