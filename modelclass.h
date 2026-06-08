#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;
// Modello geometrico riutilizzabile per rettangoli e cerchi unitari.
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
	ModelClass(const ModelClass&) = delete;
	ModelClass& operator=(const ModelClass&) = delete;
	~ModelClass();

	bool Initialize(ID3D11Device*, ShapeType shape = ShapeRectangle);
	void Shutdown();
	void Render(ID3D11DeviceContext*);
	bool SetColor(ID3D11DeviceContext*, const XMFLOAT4&);

	int GetIndexCount();

private:
	void WriteVertexPositions(VertexType*) const;
	bool InitializeBuffers(ID3D11Device*, ShapeType);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	ShapeType m_shape;
};

#endif
