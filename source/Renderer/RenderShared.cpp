// RenderShared.cpp

#include "Precompiled.h"

bool LoadTexture(d3d11_ctx_s *ctx, const char_t *file, bool point_filter, bool repeat,
	ID3D11Texture2D *&tex, ID3D11ShaderResourceView *&srv, ID3D11SamplerState *&ss, int &width, int &height) {
	image_s tex_data;
	if (ReadPNG(file, &tex_data)) {
		width = tex_data.mCX;
		height = tex_data.mCY;

		void * pixels = nullptr;
		bool free_pixels = false;
		if (tex_data.mFormat == IMG_FORMAT_1BYTE_RGB) {
			unsigned char * rgba = (unsigned char *)malloc(tex_data.mCX * tex_data.mCY * 4);
			pixels = rgba;
			free_pixels = true;

			const unsigned char * rgb = (const unsigned char *)tex_data.mData;
			for (int y = 0; y < tex_data.mCY; ++y) {
				const unsigned char * src_line = rgb + y * tex_data.mCX * 3;
				unsigned char * dst_line = rgba + y * tex_data.mCX * 4;
				for (int x = 0; x < tex_data.mCX; ++x) {
					const unsigned char * src_pixel = src_line + x * 3;
					unsigned char * dst_pixel = dst_line + x * 4;
					dst_pixel[0] = src_pixel[0];
					dst_pixel[1] = src_pixel[1];
					dst_pixel[2] = src_pixel[2];
					dst_pixel[3] = 255;
				}
			}
		}
		else if (tex_data.mFormat == IMG_FORMAT_1BYTE_RGBA) {
			pixels = tex_data.mData;
		}
		else {
			FreeImg(&tex_data);
			return false;
		}

		D3D11_TEXTURE2D_DESC td;
		memset(&td, 0, sizeof(td));

		td.Width = tex_data.mCX;
		td.Height = tex_data.mCY;
		td.MipLevels = 1; // no existing mip-map
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0; // no CPU access
		td.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA data;

		data.pSysMem = pixels;
		data.SysMemPitch = tex_data.mCX * 4;
		data.SysMemSlicePitch = tex_data.mCX * tex_data.mCY * 4;

		if (DX_FAILED(ctx->mDevice->CreateTexture2D(&td, &data, &tex))) {

			if (free_pixels) {
				free(pixels);
			}

			FreeImg(&tex_data);
			return false;
		}

		if (free_pixels) {
			free(pixels);
		}
		FreeImg(&tex_data);

		// create shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC vd;

		memset(&vd, 0, sizeof(vd));

		vd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		vd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		vd.Texture2D.MipLevels = 1;

		if (DX_FAILED(ctx->mDevice->CreateShaderResourceView(tex, &vd, &srv))) {
			return false;
		}

		// create sample state
		D3D11_SAMPLER_DESC sd;

		sd.Filter = point_filter ? D3D11_FILTER_MIN_MAG_MIP_POINT : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sd.AddressU = repeat ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.AddressV = repeat ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.AddressW = repeat ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
		sd.MipLODBias = 0.0f;
		sd.MaxAnisotropy = 1;
		sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sd.BorderColor[0] = sd.BorderColor[1] = sd.BorderColor[2] = sd.BorderColor[3] = 1.0f;
		sd.MinLOD = 0;
		sd.MaxLOD = D3D11_FLOAT32_MAX;

		if (DX_FAILED(ctx->mDevice->CreateSamplerState(&sd, &ss))) {
			return false;
		}

		return true;
	}
	else {
		return false;
	}
}

bool CreateConstantBuffer(d3d11_ctx_s *ctx, size_t size, ID3D11Buffer *&buffer) {
	D3D11_BUFFER_DESC desc;

	desc.ByteWidth = (UINT)size;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	if (DX_FAILED(ctx->mDevice->CreateBuffer(&desc, nullptr, &buffer))) {
		return false;
	}

	return true;
}

void SetConstantBuffer(d3d11_ctx_s *ctx, ID3D11Buffer *buffer, const void *data, size_t size) {
	D3D11_MAPPED_SUBRESOURCE mapped_resource;
	if (!DX_FAILED(ctx->mDeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource))) {
		memcpy(mapped_resource.pData, data, size);
		ctx->mDeviceContext->Unmap(buffer, 0);
	}
}

bool CreateBuffer(d3d11_ctx_s *ctx, unsigned int type, unsigned int size, const void * buf_data, bool CPU_access_write, ID3D11Buffer *&b) {
	D3D11_BUFFER_DESC desc;
	D3D11_SUBRESOURCE_DATA rd;

	desc.ByteWidth = size;
	desc.Usage = CPU_access_write ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	desc.BindFlags = type;
	desc.CPUAccessFlags = CPU_access_write ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	if (buf_data) {
		rd.pSysMem = buf_data;
		rd.SysMemPitch = 0;
		rd.SysMemSlicePitch = 0;

		if (DX_FAILED(ctx->mDevice->CreateBuffer(&desc, &rd, &b))) {
			return false;
		}
	}
	else {
		if (DX_FAILED(ctx->mDevice->CreateBuffer(&desc, nullptr, &b))) {
			return false;
		}
	}

	return true;
}

bool CreateVertexAndIndexBuffers(d3d11_ctx_s *ctx, unsigned int vb_size, const void * vb_buf, unsigned int ib_size, const void * ib_buf, bool CPU_access_write,
	ID3D11Buffer *&vb, ID3D11Buffer *&ib) {

	return CreateBuffer(ctx, D3D11_BIND_VERTEX_BUFFER, vb_size, vb_buf, CPU_access_write, vb)
		&& CreateBuffer(ctx, D3D11_BIND_INDEX_BUFFER, ib_size, ib_buf, CPU_access_write, ib);
}

// shaders
static const int STACK_BUFFER_SIZE = 65536;

bool CreateVertexShaderAndInputLayout(ID3D11Device * device, const char_t *filename,
	const D3D11_INPUT_ELEMENT_DESC *desc, UINT desc_count, ID3D11VertexShader *&shader, ID3D11InputLayout *&layout) {
	char buffer[STACK_BUFFER_SIZE];
	int size = File_LoadBinary(filename, buffer, STACK_BUFFER_SIZE);
	if (size <= 0) {
		return false;
	}

	if (DX_FAILED(device->CreateVertexShader(buffer, size, nullptr, &shader))) {
		return false;
	}

	if (DX_FAILED(device->CreateInputLayout(desc, desc_count, buffer, size, &layout))) {
		return false;
	}

	return true;
}

bool CreatePixelShader(ID3D11Device * device, const char_t *filename, ID3D11PixelShader *&shader) {
	char buffer[STACK_BUFFER_SIZE];
	int size = File_LoadBinary(filename, buffer, STACK_BUFFER_SIZE);
	if (size <= 0) {
		return false;
	}

	if (DX_FAILED(device->CreatePixelShader(buffer, size, nullptr, &shader))) {
		return false;
	}

	return true;
}
