#ifndef VIEW_H
#define VIEW_H

#include "Deps/DXSDK/Include/d3dx9.h"

namespace UltraEd
{
    enum class ViewType { Perspective, Top, Left, Front };

    class View
    {
    public:
        View();
        ViewType GetType();
        float GetZoom();

    private:
        ViewType m_type;
        D3DXVECTOR3 m_pos;
    };
}

#endif // VIEW_H
