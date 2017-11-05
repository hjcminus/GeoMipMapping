// RenderCloud.cpp

#include "Precompiled.h"

/*
================================================================================
RenderCloud
================================================================================
*/
RenderCloud::RenderCloud() :
	mVertexBuffer(nullptr),
	mIndexBuffer(nullptr),
	mTrianglesCount(0),
	mTexture(nullptr),
	mTextureSRV(nullptr),
	mTextureSS(nullptr)
{
}

RenderCloud::~RenderCloud() {
	// do nothing
}

bool RenderCloud::Init(d3d11_ctx_s *ctx, const char_t *res_dir) {
	// load texture
	char_t full_tex_filename[MAX_PATH_];
	sprintf_s_(full_tex_filename, TXT_("%s\\skydome\\cloud.png"), res_dir);
	int tex_width, tex_height;
	if (!LoadTexture(ctx, full_tex_filename, false, true, mTexture, mTextureSRV, mTextureSS, tex_width, tex_height)) {
		return false;
	}

	// setup geometry
	int vertices_count = 1 + SKYDOME_SLICES;
	mTrianglesCount = SKYDOME_SLICES;

	vertex_pos_clr_s * vert_buf = NEW__ vertex_pos_clr_s[vertices_count];
	unsigned int * idx_buf = NEW__ unsigned int[mTrianglesCount * 3];

	vertex_pos_clr_s * v = vert_buf;

	float fz = 2.0f;
	float radius = sqrtf(SKYDOME_RADIUS * SKYDOME_RADIUS - fz * fz);

	Vec3Set(v->mPosition, 0.0f, 0.0f, fz);
	Vec4Set(v->mColor, 0.5f, 0.5f, 0.0f, 0.2f); // r, g: texture coordinates, b: not used, a: alpha
	v++;

	float rad_step = (PI * 2.0f / (float)SKYDOME_SLICES);
	for (int i = 0; i < SKYDOME_SLICES; ++i) {
		float rad = i * rad_step;

		float fx = radius * sinf(rad);
		float fy = radius * cosf(rad);

		float t0 = (sinf(rad) + 1.0f) * 0.5f;
		float t1 = 1.0f - (cosf(rad) + 1.0f) * 0.5f;

		Vec3Set(v->mPosition, fx, fy, fz);
		Vec4Set(v->mColor, t0, t1, 0.0f, 0.0f); // alpha 0: cloud edge fade out
		v++;
	}

	// repeat cloud texture
	for (int i = 0; i < vertices_count; ++i) {
		vert_buf[i].mColor[0] *= 16.0f;
		vert_buf[i].mColor[1] *= 16.0f;
	}

	unsigned int * t = idx_buf;
	for (int i = 0; i < SKYDOME_SLICES; ++i) {
		t[0] = 0;
		t[1] = i + 1;
		t[2] = i + 2;

		if ((int)t[2] >= vertices_count) {
			t[2] = 1;
		}

		t += 3;
	}

	// create buffer objects
	if (!CreateVertexAndIndexBuffers(ctx, sizeof(vertex_pos_clr_s) * vertices_count, vert_buf,
		sizeof(unsigned int) * mTrianglesCount * 3, idx_buf, false, mVertexBuffer, mIndexBuffer)) {

		delete[]idx_buf;
		delete[]vert_buf;

		return false;
	}
	else {
		delete[]idx_buf;
		delete[]vert_buf;

		return true;
	}
}

void RenderCloud::Shutdown() {
	DX_SAFE_RELEASE(mIndexBuffer);
	DX_SAFE_RELEASE(mVertexBuffer);
	DX_SAFE_RELEASE(mTextureSS);
	DX_SAFE_RELEASE(mTextureSRV);
	DX_SAFE_RELEASE(mTexture);
}

void RenderCloud::Draw(ID3D11DeviceContext *ctx) {
	ctx->PSSetShaderResources(0, 1, &mTextureSRV);
	ctx->PSSetSamplers(0, 1, &mTextureSS);

	UINT stride = sizeof(vertex_pos_clr_s);
	UINT offset = 0;

	ctx->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	ctx->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->DrawIndexed(mTrianglesCount * 3, 0, 0);

	ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);

	ID3D11ShaderResourceView * null_srv_list[1] = { nullptr };
	ID3D11SamplerState * null_ss_list[1] = { nullptr };
	ctx->PSSetShaderResources(0, 1, null_srv_list);
	ctx->PSSetSamplers(0, 1, null_ss_list);
}
