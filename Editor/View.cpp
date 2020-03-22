#include "View.h"

namespace UltraEd
{
    View::View()
    {
    }

    ViewType View::GetType()
    {
        return m_type;
    }

    float View::GetZoom()
    {
        switch (m_type)
        {
            case ViewType::Top:
                return m_pos.y;
            case ViewType::Left:
                return -m_pos.x;
            case ViewType::Front:
                return m_pos.z;
            default:
                return 0;
        }
    }
}
