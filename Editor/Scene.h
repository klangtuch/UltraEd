#ifndef SCENE_H
#define SCENE_H

#include <QMainWindow>
#include "Deps/DXSDK/Include/d3d9.h"
#include "Deps/DXSDK/Include/d3dx9.h"
#include "View.h"

namespace UltraEd
{
    class Scene
    {
    public:
        Scene();
        ~Scene();
        bool Create(HWND windowHandle);
        void Render();

    private:
        void Resize();
        void UpdateViewMatrix();
        HWND GetWndHandle();
        View *GetActiveView();

    private:
        std::unique_ptr<IDirect3D9, std::function<void(IDirect3D9*)>> m_d3d9;
        std::unique_ptr<IDirect3DDevice9, std::function<void(IDirect3DDevice9*)>> m_device;
        D3DPRESENT_PARAMETERS m_d3dpp;
        ViewType m_activeViewType;
        View m_views[4];
    };
}

#endif // SCENE_H
