// States.h

#pragma once

/*
================================================================================
States
================================================================================
*/
class States {
public:

	States();
	~States();

	bool						Init(d3d11_ctx_s *ctx);
	void						Shutdown();

public:

	ID3D11RasterizerState *		mSolidRS;
	ID3D11RasterizerState *		mWireframeRS;

	ID3D11DepthStencilState *	mDisableDepthDSS;
	ID3D11DepthStencilState *	mEnableDepthDSS;

	ID3D11BlendState *			mDisableBlendBS;
	ID3D11BlendState *			mEnableBlendBS;
};
