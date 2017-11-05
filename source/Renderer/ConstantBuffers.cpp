// ConstantBuffers.cpp

#include "Precompiled.h"

/*
================================================================================
ConstantBuffers
================================================================================
*/
ConstantBuffers::ConstantBuffers() :
	mTextMVPBuffer(nullptr),
	mSkyMVPBuffer(nullptr),
	mSceneMVPBuffer(nullptr),
	mSceneMVBuffer(nullptr),
	mTexSizeBuffer(nullptr),
	mFogBuffer(nullptr),
	mCloudMoveBuffer(nullptr)
{
}

ConstantBuffers::~ConstantBuffers() {
	// do nothing
}

bool ConstantBuffers::Init(d3d11_ctx_s *ctx) {
	if (!CreateConstantBuffer(ctx, sizeof(XMMATRIX), mTextMVPBuffer)) {
		return false;
	}

	if (!CreateConstantBuffer(ctx, sizeof(XMMATRIX), mSkyMVPBuffer)) {
		return false;
	}

	if (!CreateConstantBuffer(ctx, sizeof(XMMATRIX), mSceneMVPBuffer)) {
		return false;
	}

	if (!CreateConstantBuffer(ctx, sizeof(XMMATRIX), mSceneMVBuffer)) {
		return false;
	}

	if (!CreateConstantBuffer(ctx, sizeof(XMVECTOR), mTexSizeBuffer)) {
		return false;
	}

	if (!CreateConstantBuffer(ctx, sizeof(XMVECTOR), mFogBuffer)) {
		return false;
	}

	if (!CreateConstantBuffer(ctx, sizeof(XMVECTOR), mCloudMoveBuffer)) {
		return false;
	}

	return true;
}

void ConstantBuffers::Shutdown() {
	DX_SAFE_RELEASE(mCloudMoveBuffer);
	DX_SAFE_RELEASE(mFogBuffer);
	DX_SAFE_RELEASE(mTexSizeBuffer);
	DX_SAFE_RELEASE(mSceneMVBuffer);
	DX_SAFE_RELEASE(mSceneMVPBuffer);
	DX_SAFE_RELEASE(mSkyMVPBuffer);
	DX_SAFE_RELEASE(mTextMVPBuffer);

}
