////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "modelclass.h"
#include <cmath>

#ifndef XM_2PI
#define XM_2PI 6.283185307179586476925286766559f
#endif

ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_vertexCount = 0;
	m_indexCount = 0;
	m_shape = ShapeRectangle;
}

ModelClass::ModelClass(const ModelClass& other) {}
ModelClass::~ModelClass() {}

bool ModelClass::Initialize(ID3D11Device* device, ShapeType shape)
{
	return InitializeBuffers(device, shape);
}

void ModelClass::Shutdown()
{
	ShutdownBuffers();
}

void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	RenderBuffers(deviceContext);
}

int ModelClass::GetIndexCount()
{
	return m_indexCount;
}

bool ModelClass::SetColor(ID3D11DeviceContext* deviceContext, const XMFLOAT4& color)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result;

	result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	VertexType* verticesPtr = (VertexType*)mappedResource.pData;

	// Con WRITE_DISCARD DirectX può scartare il contenuto precedente del buffer.
	// Per questo riscriviamo sempre sia le posizioni sia i colori.
	if (m_shape == ShapeRectangle)
	{
		verticesPtr[0].position = XMFLOAT3(-0.5f, -0.5f, 0.0f);
		verticesPtr[1].position = XMFLOAT3(-0.5f,  0.5f, 0.0f);
		verticesPtr[2].position = XMFLOAT3( 0.5f,  0.5f, 0.0f);
		verticesPtr[3].position = XMFLOAT3( 0.5f, -0.5f, 0.0f);
	}
	else if (m_shape == ShapeCircle)
	{
		const int segments = 32;

		verticesPtr[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f);

		for (int i = 0; i < segments; i++)
		{
			float angle = XM_2PI * static_cast<float>(i) / static_cast<float>(segments);
			float x = cosf(angle) * 0.5f;
			float y = sinf(angle) * 0.5f;

			verticesPtr[i + 1].position = XMFLOAT3(x, y, 0.0f);
		}
	}

	for (int i = 0; i < m_vertexCount; i++)
	{
		verticesPtr[i].color = color;
	}

	deviceContext->Unmap(m_vertexBuffer, 0);
	return true;
}

bool ModelClass::InitializeBuffers(ID3D11Device* device, ShapeType shape)
{
	VertexType* vertices = 0;
	unsigned long* indices = 0;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	m_shape = shape;

	XMFLOAT4 defaultColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	if (shape == ShapeRectangle)
	{
		m_vertexCount = 4;
		m_indexCount = 6;

		vertices = new VertexType[m_vertexCount];
		if (!vertices)
		{
			return false;
		}

		indices = new unsigned long[m_indexCount];
		if (!indices)
		{
			delete[] vertices;
			return false;
		}

		vertices[0].position = XMFLOAT3(-0.5f, -0.5f, 0.0f); // basso sinistra
		vertices[1].position = XMFLOAT3(-0.5f,  0.5f, 0.0f); // alto sinistra
		vertices[2].position = XMFLOAT3( 0.5f,  0.5f, 0.0f); // alto destra
		vertices[3].position = XMFLOAT3( 0.5f, -0.5f, 0.0f); // basso destra

		// Winding orario, coerente con il rasterizer DirectX del template.
		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
		indices[3] = 0;
		indices[4] = 2;
		indices[5] = 3;
	}
	else
	{
		// Cerchio triangolato con triangle fan convertito in triangle list.
		// 32 segmenti bastano per pallina e modificatori piccoli.
		const int segments = 32;
		m_vertexCount = segments + 1;      // vertice 0 = centro, poi punti sul bordo
		m_indexCount = segments * 3;       // un triangolo per segmento

		vertices = new VertexType[m_vertexCount];
		if (!vertices)
		{
			return false;
		}

		indices = new unsigned long[m_indexCount];
		if (!indices)
		{
			delete[] vertices;
			return false;
		}

		vertices[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f);

		for (int i = 0; i < segments; i++)
		{
			float angle = XM_2PI * static_cast<float>(i) / static_cast<float>(segments);
			float x = cosf(angle) * 0.5f;
			float y = sinf(angle) * 0.5f;
			vertices[i + 1].position = XMFLOAT3(x, y, 0.0f);
		}

		for (int i = 0; i < segments; i++)
		{
			int next = (i + 1) % segments;

			// Winding orario: centro, prossimo, corrente.
			indices[i * 3 + 0] = 0;
			indices[i * 3 + 1] = next + 1;
			indices[i * 3 + 2] = i + 1;
		}
	}

	for (int i = 0; i < m_vertexCount; i++)
	{
		vertices[i].color = defaultColor;
	}

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		delete[] vertices;
		delete[] indices;
		return false;
	}

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		delete[] vertices;
		delete[] indices;
		return false;
	}

	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

void ModelClass::ShutdownBuffers()
{
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}
}

void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
