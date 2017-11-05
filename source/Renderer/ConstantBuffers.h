// ConstantBuffers.h

#pragma once

/*
================================================================================
ConstantBuffers
================================================================================
*/
class ConstantBuffers {
public:

	ConstantBuffers();
	~ConstantBuffers();

	bool						Init(d3d11_ctx_s *ctx);
	void						Shutdown();

public:

	ID3D11Buffer *				mTextMVPBuffer;
	ID3D11Buffer *				mSkyMVPBuffer;
	ID3D11Buffer *				mSceneMVPBuffer;
	ID3D11Buffer *				mSceneMVBuffer;
	ID3D11Buffer *				mTexSizeBuffer;
	ID3D11Buffer *				mFogBuffer;
	ID3D11Buffer *				mCloudMoveBuffer;
	
};
