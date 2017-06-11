#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "Mesh.h"

class CDebug  
{
public:
	CDebug();
	~CDebug();
  void DrawLine(D3DXVECTOR3 from, D3DXVECTOR3 to);
  void Release();
  void Render(IDirect3DDevice8*);
  static CDebug& Instance();
  
private:
  static CDebug* m_instance;
  D3DMATERIAL8 m_material;
  IDirect3DVertexBuffer8* m_vertexBuffer;
  IDirect3DVertexBuffer8* GetBuffer(IDirect3DDevice8*);
  std::vector<MeshVertex> m_vertices;
};

#endif