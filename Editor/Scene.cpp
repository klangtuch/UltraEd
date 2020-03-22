#include "Scene.h"
#include <functional>

namespace UltraEd
{
    Scene::Scene() :
        m_d3d9(),
        m_device(),
        m_d3dpp(),
        m_activeViewType(ViewType::Perspective),
        m_views()
    {
    }

    Scene::~Scene()
    {
    }

    bool Scene::Create(HWND hWnd)
    {
        if (hWnd == NULL)
            return false;

        m_d3d9 = std::unique_ptr<IDirect3D9, std::function<void(IDirect3D9*)>>(Direct3DCreate9(D3D_SDK_VERSION),
            [](IDirect3D9* d3d) {
                d3d->Release();
            }
        );

        if (m_d3d9 == NULL)
            return false;

        D3DDISPLAYMODE d3ddm;
        if (FAILED(m_d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
            return false;

        m_d3dpp.Windowed = TRUE;
        m_d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
        m_d3dpp.BackBufferFormat = d3ddm.Format;
        m_d3dpp.EnableAutoDepthStencil = TRUE;
        m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

        IDirect3DDevice9 *tempDevice;
        if (FAILED(m_d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_d3dpp, &tempDevice)))
            return false;

        m_device = std::unique_ptr<IDirect3DDevice9, std::function<void(IDirect3DDevice9*)>>(tempDevice,
            [](IDirect3DDevice9* device) {
                device->Release();
            }
        );

        return true;
    }

    void Scene::Resize()
    {
        if (m_device)
        {
            m_device->Reset(&m_d3dpp);
            UpdateViewMatrix();
        }
    }

    void Scene::UpdateViewMatrix()
    {
        D3DXMATRIX viewMat;
        RECT rect;
        GetClientRect(GetWndHandle(), &rect);
        const float aspect = static_cast<float>(rect.right) / static_cast<float>(rect.bottom);

        if (GetActiveView()->GetType() == ViewType::Perspective)
        {
            const float fov = D3DX_PI / 2.0f;
            D3DXMatrixPerspectiveFovLH(&viewMat, fov, aspect, 0.1f, 1000.0f);
        }
        else
        {
            const float zoom = GetActiveView()->GetZoom();
            D3DXMatrixOrthoLH(&viewMat, zoom * aspect, zoom, -1000.0f, 1000.0f);
        }

        m_device->SetTransform(D3DTS_PROJECTION, &viewMat);
    }

    HWND Scene::GetWndHandle()
    {
        D3DDEVICE_CREATION_PARAMETERS params;
        if (m_device && SUCCEEDED(m_device->GetCreationParameters(&params)))
        {
            return params.hFocusWindow;
        }
        return NULL;
   }

    View *Scene::GetActiveView()
    {
        return &m_views[static_cast<int>(m_activeViewType)];
    }
}
