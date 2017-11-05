// RenderCloud.h

#pragma once

/*
================================================================================
RenderCloud
================================================================================
*/
class RenderCloud {
public:
	RenderCloud();
	~RenderCloud();

	bool						Init(d3d11_ctx_s *ctx, const char_t *res_dir);
	void						Shutdown();

	void						Draw(ID3D11DeviceContext *ctx);

private:

	ID3D11Buffer *				mVertexBuffer;
	ID3D11Buffer *				mIndexBuffer;
	int							mTrianglesCount;

	ID3D11Texture2D *			mTexture;
	ID3D11ShaderResourceView *	mTextureSRV;
	ID3D11SamplerState *		mTextureSS;
};
