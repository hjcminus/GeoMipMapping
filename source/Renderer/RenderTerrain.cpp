// RenderTerrain.cpp

#include "Precompiled.h"

/*
================================================================================
RenderTerrain
================================================================================
*/
RenderTerrain::RenderTerrain() :
	mVertexBuffer(nullptr),
	mVerticesCount(0),
	mIndexBuffer(nullptr),
	mIndicesCount(0),
	mDiffuseTexture(nullptr),
	mDiffuseTextureSRV(nullptr),
	mDiffuseTextureSS(nullptr),
	mDetailTexture(nullptr),
	mDetailTextureSRV(nullptr),
	mDetailTextureSS(nullptr),
	mTriangleCount(0),
	mTexWidth(1),
	mTexHeight(1)
{
}

RenderTerrain::~RenderTerrain() {
	// do nothing
}

bool RenderTerrain::Init(d3d11_ctx_s *ctx, const char_t *res_dir, int test_case) {
	if (!CreateVertexAndIndexBuffers(ctx, sizeof(vertex_pos_s) * MAX_DRAW_VERT_COUNT, nullptr, 
		sizeof(unsigned int) * MAX_DRAW_IDX_COUNT, nullptr, true, mVertexBuffer, mIndexBuffer)) {
		return false;
	}
	
	// load base texture
	char_t full_tex_filename[MAX_PATH_];
	sprintf_s_(full_tex_filename, TXT_("%s\\terrain\\%s"), res_dir, TEST_DATA[test_case].mColorMap);

	if (!LoadTexture(ctx, full_tex_filename, false, true, mDiffuseTexture, mDiffuseTextureSRV, mDiffuseTextureSS, mTexWidth, mTexHeight)) {
		return false;
	}

	// load detail texture
	sprintf_s_(full_tex_filename, TXT_("%s\\terrain\\detail.png"), res_dir);

	int detail_width, detail_height;
	if (!LoadTexture(ctx, full_tex_filename, false, true, mDetailTexture, mDetailTextureSRV, mDetailTextureSS, detail_width, detail_height)) {
		return false;
	}

	return true;
}

void RenderTerrain::Shutdown() {
	DX_SAFE_RELEASE(mDetailTextureSS);
	DX_SAFE_RELEASE(mDetailTextureSRV);
	DX_SAFE_RELEASE(mDetailTexture);
	DX_SAFE_RELEASE(mDiffuseTextureSS);
	DX_SAFE_RELEASE(mDiffuseTextureSRV);
	DX_SAFE_RELEASE(mDiffuseTexture);
	DX_SAFE_RELEASE(mIndexBuffer);
	DX_SAFE_RELEASE(mVertexBuffer);
}

int RenderTerrain::GetTexWidth() const {
	return mTexWidth;
}

int	RenderTerrain::GetTexHeight() const {
	return mTexHeight;
}

void RenderTerrain::Update(ID3D11DeviceContext *ctx, const mesh_s &mesh) {
	mTriangleCount = mesh.mTriangleCount;

	D3D11_MAPPED_SUBRESOURCE ms;

	if (!DX_FAILED(ctx->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms))) {
		memcpy(ms.pData, mesh.mVertices, sizeof(vertex_pos_s) * mesh.mVerticesCount);
		mVerticesCount = mesh.mVerticesCount;
		ctx->Unmap(mVertexBuffer, 0);
	}

	if (!DX_FAILED(ctx->Map(mIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms))) {
		memcpy(ms.pData, mesh.mIndices, sizeof(unsigned int) * mesh.mIndicesCount);
		mIndicesCount = mesh.mIndicesCount;
		ctx->Unmap(mIndexBuffer, 0);
	}
}

void RenderTerrain::Draw(ID3D11DeviceContext *ctx, bool solid) {
	if (mIndicesCount > 0) {
		if (solid) {
			ctx->PSSetShaderResources(0, 1, &mDiffuseTextureSRV);
			ctx->PSSetSamplers(0, 1, &mDiffuseTextureSS);
			ctx->PSSetShaderResources(1, 1, &mDetailTextureSRV);
			ctx->PSSetSamplers(1, 1, &mDetailTextureSS);
		}

		UINT stride = sizeof(vertex_pos_s);
		UINT offset = 0;
		ctx->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		ctx->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		ctx->DrawIndexed(mIndicesCount, 0, 0);

		ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);

		if (solid) {
			ID3D11ShaderResourceView * null_srv_list[1] = { nullptr };
			ID3D11SamplerState * null_ss_list[1] = { nullptr };
			ctx->PSSetShaderResources(0, 1, null_srv_list);
			ctx->PSSetSamplers(0, 1, null_ss_list);
			ctx->PSSetShaderResources(1, 1, null_srv_list);
			ctx->PSSetSamplers(1, 1, null_ss_list);
		}
	}
}

int	RenderTerrain::GetTriangleCount() const {
	return mTriangleCount;
}
