// Renderer.cpp

#include "Precompiled.h"

/*
================================================================================
Renderer
================================================================================
*/
static const float BLEND_FACTORS[4] = { 0.0f };

Renderer::Renderer() :
	mCtx(nullptr),
	mDXGISwapChain(nullptr),
	mDefaultRenderTargetView(nullptr),
	mDefaultDepthStencilTexture2D(nullptr),
	mDefaultDepthStencilView(nullptr),
	mViewCX(1),
	mViewCY(1)
{
	mCloudMove[0] = mCloudMove[1] = 0.0f;
}

Renderer::~Renderer() {
	// do nothing
}

bool Renderer::Init(d3d11_ctx_s *ctx, const char_t *res_dir, int test_case) {
	mCtx = ctx;

	if (!Reshape(VIEWPORT_CX, VIEWPORT_CY)) {
		return false;
	}

	
	if (!mStates.Init(ctx)) {
		return false;
	}

	if (!mCBuffers.Init(ctx)) {
		return false;
	}

	char_t shader_dir[MAX_PATH_];
	sprintf_s_(shader_dir, TXT_("%s\\shaders"), res_dir);
	if (!mShaders.Init(ctx, shader_dir)) {
		return false;
	}

	if (!mText.Init(ctx, res_dir)) {
		return false;
	}

	if (!mSkydome.Init(ctx)) {
		return false;
	}

	if (!mCloud.Init(ctx, res_dir)) {
		return false;
	}

	if (!mTerrain.Init(ctx, res_dir, test_case)) {
		return false;
	}

	return true;
}

void Renderer::Shutdown() {
	mTerrain.Shutdown();
	mCloud.Shutdown();
	mSkydome.Shutdown();
	mText.Shutdown();
	mShaders.Shutdown();
	mCBuffers.Shutdown();
	mStates.Shutdown();

	DX_SAFE_RELEASE(mDefaultDepthStencilView);
	DX_SAFE_RELEASE(mDefaultDepthStencilTexture2D);
	DX_SAFE_RELEASE(mDefaultRenderTargetView);
	DX_SAFE_RELEASE(mDXGISwapChain);
}

bool Renderer::Reshape(int view_width, int view_height) {
	if (view_width < 1) {
		view_width = 1;
	}

	if (view_height < 1) {
		view_height = 1;
	}

	mViewCX = view_width;
	mViewCY = view_height;

	DX_SAFE_RELEASE(mDefaultDepthStencilView);
	DX_SAFE_RELEASE(mDefaultDepthStencilTexture2D);
	DX_SAFE_RELEASE(mDefaultRenderTargetView);
	DX_SAFE_RELEASE(mDXGISwapChain);

	// describe the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	scd.BufferDesc.Width = view_width;
	scd.BufferDesc.Height = view_height;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1; // one back buffer
	scd.OutputWindow = mCtx->mWnd;
	scd.Windowed = true;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	if (!DX_CreateSwapChain(&scd, &mDXGISwapChain)) {
		return false;
	}

	// create the render target view
	if (!DX_CreateRenderTargetView(mDXGISwapChain, &mDefaultRenderTargetView)) {
		return false;
	}

	// create the depth / stencil buffer and view
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = view_width;
	depthStencilDesc.Height = view_height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// must match swap chain MSAAA values
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	if (DX_FAILED(mCtx->mDevice->CreateTexture2D(&depthStencilDesc, nullptr, &mDefaultDepthStencilTexture2D))) {
		return false;
	}

	if (DX_FAILED(mCtx->mDevice->CreateDepthStencilView(mDefaultDepthStencilTexture2D, nullptr, &mDefaultDepthStencilView))) {
		return false;
	}

	// bind the views to the output merger stage
	mCtx->mDeviceContext->OMSetRenderTargets(1, &mDefaultRenderTargetView, mDefaultDepthStencilView);

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)view_width;
	vp.Height = (float)view_height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	mCtx->mDeviceContext->RSSetViewports(1, &vp);

	return true;
}

void Renderer::UpdateRenderTerrain(const mesh_s &mesh) {
	mTerrain.Update(mCtx->mDeviceContext, mesh);
}

void Renderer::Draw(float delta_seconds, unsigned draw_flags, const camera_s & cam, const demo_info_s &info) {
	SetupConstantBuffers(delta_seconds, cam);
	
	float color[] = { 0.75f, 0.75f, 0.75f, 1.0f };
	mCtx->mDeviceContext->ClearRenderTargetView(mDefaultRenderTargetView, color);
	mCtx->mDeviceContext->ClearDepthStencilView(mDefaultDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// draw sky dome
	if (draw_flags & DRAW_SKYDOME) {
		mCtx->mDeviceContext->RSSetState(mStates.mSolidRS);
		mCtx->mDeviceContext->OMSetDepthStencilState(mStates.mDisableDepthDSS, 1);
		mCtx->mDeviceContext->OMSetBlendState(mStates.mDisableBlendBS, BLEND_FACTORS, 0xffffffff);

		mShaders.SetSkydomeShaders(mCtx->mDeviceContext, true); {
			mCtx->mDeviceContext->VSSetConstantBuffers(0, 1, &mCBuffers.mSkyMVPBuffer);

			mSkydome.Draw(mCtx->mDeviceContext);
		}
		mShaders.SetSkydomeShaders(mCtx->mDeviceContext, false);
	}

	// draw cloud
	if (draw_flags & DRAW_CLOUD) {
		mCtx->mDeviceContext->RSSetState(mStates.mSolidRS);
		mCtx->mDeviceContext->OMSetDepthStencilState(mStates.mDisableDepthDSS, 1);
		mCtx->mDeviceContext->OMSetBlendState(mStates.mEnableBlendBS, BLEND_FACTORS, 0xffffffff);

		mShaders.SetCloudShaders(mCtx->mDeviceContext, true); {
			mCtx->mDeviceContext->VSSetConstantBuffers(0, 1, &mCBuffers.mSkyMVPBuffer);
			mCtx->mDeviceContext->PSSetConstantBuffers(0, 1, &mCBuffers.mCloudMoveBuffer);

			mCloud.Draw(mCtx->mDeviceContext);
		}
		mShaders.SetCloudShaders(mCtx->mDeviceContext, false);
	}

	// draw terrain
	if (draw_flags & DRAW_TERRAIN_SOLID) {
		mCtx->mDeviceContext->RSSetState(mStates.mSolidRS);
		mCtx->mDeviceContext->OMSetDepthStencilState(mStates.mEnableDepthDSS, 1);
		mCtx->mDeviceContext->OMSetBlendState(mStates.mDisableBlendBS, BLEND_FACTORS, 0xffffffff);

		mShaders.SetSolidShaders(mCtx->mDeviceContext, true); {
			mCtx->mDeviceContext->VSSetConstantBuffers(0, 1, &mCBuffers.mSceneMVBuffer);
			mCtx->mDeviceContext->VSSetConstantBuffers(1, 1, &mCBuffers.mSceneMVPBuffer);
			mCtx->mDeviceContext->VSSetConstantBuffers(2, 1, &mCBuffers.mTexSizeBuffer);
			mCtx->mDeviceContext->PSSetConstantBuffers(0, 1, &mCBuffers.mFogBuffer);

			mTerrain.Draw(mCtx->mDeviceContext, true);
		}
		mShaders.SetSolidShaders(mCtx->mDeviceContext, false);
	}

	if (draw_flags & DRAW_TERRAIN_WIREFRAME) {
		mCtx->mDeviceContext->RSSetState(mStates.mWireframeRS);
		mCtx->mDeviceContext->OMSetDepthStencilState(mStates.mEnableDepthDSS, 1);
		mCtx->mDeviceContext->OMSetBlendState(mStates.mDisableBlendBS, BLEND_FACTORS, 0xffffffff);

		mShaders.SetWireframeShaders(mCtx->mDeviceContext, true); {
			mCtx->mDeviceContext->VSSetConstantBuffers(0, 1, &mCBuffers.mSceneMVPBuffer);

			mTerrain.Draw(mCtx->mDeviceContext, false);
		}
		mShaders.SetWireframeShaders(mCtx->mDeviceContext, false);
	}

	// draw text
	char text[1024];
	int ms = (int)(info.mUpdateTimeElapsed * 1000.0);
	sprintf_s(text, "pos: %.1f, %.1f, %.1f, update: %d ms, triangles: %d, pixel error: %d",
		cam.mPos[0], cam.mPos[1], cam.mPos[2], ms,
		mTerrain.GetTriangleCount(),
		(int)info.mMaxPixelError);
	mText.Print(mCtx->mDeviceContext, text);

	mCtx->mDeviceContext->RSSetState(mStates.mSolidRS);
	mCtx->mDeviceContext->OMSetDepthStencilState(mStates.mDisableDepthDSS, 1);
	mCtx->mDeviceContext->OMSetBlendState(mStates.mEnableBlendBS, BLEND_FACTORS, 0xffffffff);

	mShaders.SetTextShaders(mCtx->mDeviceContext, true); {
		mCtx->mDeviceContext->VSSetConstantBuffers(0, 1, &mCBuffers.mTextMVPBuffer);

		mText.Draw(mCtx->mDeviceContext);
	}
	mShaders.SetTextShaders(mCtx->mDeviceContext, false);


	mDXGISwapChain->Present(0, 0);
}

void Renderer::SetupConstantBuffers(float delta_seconds, const camera_s & cam) {
	static const vec4_t FOG_COLOR = HORIZONTAL_COLOR;

	// using right-hand coordinate

	float aspect = (float)mViewCX / mViewCY;

	vec3_t skydome_view_to_target;
	Vec3Sub(cam.mTarget, cam.mPos, skydome_view_to_target);

	// setup matrices
	XMMATRIX persp_matrix = XMMatrixPerspectiveFovRH(Deg2Rad(cam.mFovy), aspect, cam.mZNear, cam.mZFar);
	XMMATRIX ortho_matrix = XMMatrixOrthographicRH((float)mViewCX, (float)mViewCY, -1.0f, 1.0f);
	XMMATRIX text_trans = XMMatrixTranslation((float)mViewCX * -0.5f + 8.0f, (float)mViewCY * 0.5f - 24.0f, 0.0f);
	XMMATRIX skydome_view_matrix = XMMatrixLookAtRH(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), Vec3ToXMVECTOR(skydome_view_to_target, 1.0f), XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
	XMMATRIX skydome_mvp_matrix = XMMatrixMultiply(skydome_view_matrix, persp_matrix);
	XMMATRIX scene_view_matrix = XMMatrixLookAtRH(Vec3ToXMVECTOR(cam.mPos, 1.0f), Vec3ToXMVECTOR(cam.mTarget, 1.0f), Vec3ToXMVECTOR(cam.mUp, 0.0f));
	XMMATRIX scene_mvp_matrix = XMMatrixMultiply(scene_view_matrix, persp_matrix);
	XMMATRIX text_mvp_matrix = XMMatrixMultiply(text_trans, ortho_matrix);

	SetConstantBuffer(mCtx, mCBuffers.mTextMVPBuffer, &text_mvp_matrix, sizeof(text_mvp_matrix));
	SetConstantBuffer(mCtx, mCBuffers.mSkyMVPBuffer, &skydome_mvp_matrix, sizeof(skydome_mvp_matrix));
	SetConstantBuffer(mCtx, mCBuffers.mSceneMVPBuffer, &scene_mvp_matrix, sizeof(scene_mvp_matrix));
	SetConstantBuffer(mCtx, mCBuffers.mSceneMVBuffer, &scene_view_matrix, sizeof(scene_view_matrix));

	XMVECTOR tex_size = XMVectorSet((float)mTerrain.GetTexWidth(), (float)mTerrain.GetTexHeight(), 0.0f, 0.0f);
	SetConstantBuffer(mCtx, mCBuffers.mTexSizeBuffer, &tex_size, sizeof(tex_size));

	XMVECTOR fog = XMVectorSet(FOG_COLOR[0], FOG_COLOR[1], FOG_COLOR[2], FOG_DENSITY);
	SetConstantBuffer(mCtx, mCBuffers.mFogBuffer, &fog, sizeof(fog));

	mCloudMove[0] += 0.01f * delta_seconds;
	mCloudMove[1] += 0.01f * delta_seconds;

	XMVECTOR cloud_move = XMVectorSet(mCloudMove[0], mCloudMove[1], 0.0f, 0.0f);
	SetConstantBuffer(mCtx, mCBuffers.mCloudMoveBuffer, &cloud_move, sizeof(cloud_move));
}
