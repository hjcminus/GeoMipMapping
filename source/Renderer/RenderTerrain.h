// RenderTerrain.h

#pragma once

/*
================================================================================
RenderTerrain
================================================================================
*/
class RenderTerrain {
public:
	RenderTerrain();
	~RenderTerrain();

	bool						Init(d3d11_ctx_s *ctx, const char_t *res_dir, int test_case);
	void						Shutdown();

	int							GetTexWidth() const;
	int							GetTexHeight() const;

	void						Update(ID3D11DeviceContext *ctx, const mesh_s &mesh);
	void						Draw(ID3D11DeviceContext *ctx, bool solid);

	int							GetTriangleCount() const;

private:

	ID3D11Buffer *				mVertexBuffer;
	UINT						mVerticesCount;

	ID3D11Buffer *				mIndexBuffer;
	UINT						mIndicesCount;

	ID3D11Texture2D *			mDiffuseTexture;
	ID3D11ShaderResourceView *	mDiffuseTextureSRV;
	ID3D11SamplerState *		mDiffuseTextureSS;

	ID3D11Texture2D *			mDetailTexture;
	ID3D11ShaderResourceView *	mDetailTextureSRV;
	ID3D11SamplerState *		mDetailTextureSS;

	int							mTriangleCount;

	int							mTexWidth;
	int							mTexHeight;

	void						Destroy();
};
