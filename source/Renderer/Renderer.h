// Renderer.h

#pragma once

#define DRAW_TERRAIN_SOLID		1
#define DRAW_TERRAIN_WIREFRAME	2
#define DRAW_SKYDOME			4
#define DRAW_CLOUD				8

/*
================================================================================
Renderer
================================================================================
*/
class Renderer {
public:
	Renderer();
	~Renderer();

	bool						Init(d3d11_ctx_s *ctx, const char_t *res_dir, int test_case);
	void						Shutdown();

	bool						Reshape(int view_width, int view_height);

	void						UpdateRenderTerrain(const mesh_s &mesh);
	void						Draw(float delta_seconds, unsigned draw_flags, const camera_s & cam, const demo_info_s &info);

private:

	d3d11_ctx_s *				mCtx;
	IDXGISwapChain *			mDXGISwapChain;
	ID3D11RenderTargetView *	mDefaultRenderTargetView;
	ID3D11Texture2D *			mDefaultDepthStencilTexture2D;
	ID3D11DepthStencilView *	mDefaultDepthStencilView;

	int							mViewCX;
	int							mViewCY;

	ConstantBuffers				mCBuffers;
	Shaders						mShaders;
	States						mStates;


	RenderText					mText;
	RenderSkydome				mSkydome;
	RenderCloud					mCloud;
	RenderTerrain				mTerrain;

	vec2_t						mCloudMove;

	void						SetupConstantBuffers(float delta_seconds, const camera_s & cam);
};
