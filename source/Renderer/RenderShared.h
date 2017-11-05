// RenderShared.h

#pragma once

// sky dome
#define ZENITH_COLOR		{ 0.1f, 0.5f, 0.9f, 1.0f }
#define HORIZONTAL_COLOR	{ 0.8f, 0.9f, 0.9f, 1.0f }

#define SKYDOME_SLICES		20
#define SKYDOME_STACKS		15
#define SKYDOME_RADIUS		64.0f

// fog
#define FOG_DENSITY			0.002f


// texture
bool LoadTexture(d3d11_ctx_s *ctx, const char_t *file, bool point_filter, bool repeat,
	ID3D11Texture2D *&tex, ID3D11ShaderResourceView *&srv, ID3D11SamplerState *&ss, int &width, int &height);

// buffers
bool CreateConstantBuffer(d3d11_ctx_s *ctx, size_t size, ID3D11Buffer *&buffer);
void SetConstantBuffer(d3d11_ctx_s *ctx, ID3D11Buffer *buffer, const void *data, size_t size);

bool CreateBuffer(d3d11_ctx_s *ctx, unsigned int type, unsigned int size, const void * buf_data, bool CPU_access_write, ID3D11Buffer *&b);
bool CreateVertexAndIndexBuffers(d3d11_ctx_s *ctx, unsigned int vb_size, const void * vb_buf, unsigned int ib_size, const void * ib_buf, bool CPU_access_write,
	ID3D11Buffer *&vb, ID3D11Buffer *&ib);

// shaders
bool CreateVertexShaderAndInputLayout(ID3D11Device * device,
	const char_t *filename, const D3D11_INPUT_ELEMENT_DESC *desc, UINT desc_count, ID3D11VertexShader *&shader, ID3D11InputLayout *&layout);
bool CreatePixelShader(ID3D11Device * device, const char_t *filename, ID3D11PixelShader *&shader);
