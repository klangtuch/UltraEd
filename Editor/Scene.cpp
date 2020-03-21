#include "Scene.h"
#include <functional>

namespace UltraEd
{
    Scene::Scene() :
        m_d3d8(),
        m_device(),
        m_d3dpp()
    {
    }

    Scene::~Scene()
    {
    }

    bool Scene::Create(HWND hWnd)
    {
        if (hWnd == NULL)
            return false;

        m_d3d8 = std::unique_ptr<IDirect3D8, std::function<void(IDirect3D8*)>>(Direct3DCreate8(D3D_SDK_VERSION),
            [](IDirect3D8* d3d) {
                d3d->Release();
            }
        );

        if (m_d3d8 == NULL)
            return false;

        D3DDISPLAYMODE d3ddm;
        if (FAILED(m_d3d8->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
            return false;

        m_d3dpp.Windowed = TRUE;
        m_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
        m_d3dpp.BackBufferFormat = d3ddm.Format;
        m_d3dpp.EnableAutoDepthStencil = TRUE;
        m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

        IDirect3DDevice8 *tempDevice;
        if (FAILED(m_d3d8->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_d3dpp, &tempDevice)))
            return false;

        m_device = std::unique_ptr<IDirect3DDevice8, std::function<void(IDirect3DDevice8*)>>(tempDevice,
            [](IDirect3DDevice8* device) {
                device->Release();
            }
        );

        return true;
    }
}
