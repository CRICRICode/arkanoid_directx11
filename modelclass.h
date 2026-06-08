////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Class name: ModelClass
// Modello geometrico riutilizzabile.
// Può creare:
// - un rettangolo unitario 1x1 centrato nell'origine;
// - un cerchio unitario triangolato, utile per la pallina e per i modificatori.
////////////////////////////////////////////////////////////////////////////////
class ModelClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

public:
	enum ShapeType
	{
		ShapeRectangle,
		ShapeCircle
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device*, ShapeType shape = ShapeRectangle);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	bool SetColor(ID3D11DeviceContext*, const XMFLOAT4&);

	int GetIndexCount();

private:
	bool InitializeBuffers(ID3D11Device*, ShapeType);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	ShapeType m_shape;
};

#endif
