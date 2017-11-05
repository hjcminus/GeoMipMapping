// States.cpp

#include "Precompiled.h"

/*
================================================================================
States
================================================================================
*/
States::States() :
	mSolidRS(nullptr),
	mWireframeRS(nullptr),
	mDisableDepthDSS(nullptr),
	mEnableDepthDSS(nullptr),
	mDisableBlendBS(nullptr),
	mEnableBlendBS(nullptr)
{
}

States::~States() {
	// do nothing
}

bool States::Init(d3d11_ctx_s *ctx) {
	D3D11_RASTERIZER_DESC	ras_desc;

	ras_desc.FillMode = D3D11_FILL_SOLID;
	ras_desc.CullMode = D3D11_CULL_BACK;
	ras_desc.FrontCounterClockwise = TRUE;
	ras_desc.DepthBias = 0;
	ras_desc.DepthBiasClamp = 0.0f;
	ras_desc.SlopeScaledDepthBias = 0.0f;
	ras_desc.DepthClipEnable = TRUE;
	ras_desc.ScissorEnable = FALSE;
	ras_desc.MultisampleEnable = FALSE;
	ras_desc.AntialiasedLineEnable = FALSE;

	if (DX_FAILED(ctx->mDevice->CreateRasterizerState(&ras_desc, &mSolidRS))) {
		return false;
	}

	ras_desc.FillMode = D3D11_FILL_WIREFRAME;
	ras_desc.CullMode = D3D11_CULL_BACK;
	ras_desc.FrontCounterClockwise = TRUE;
	ras_desc.DepthBias = 0;
	ras_desc.DepthBiasClamp = 0.0f;
	ras_desc.SlopeScaledDepthBias = -1.0f;
	ras_desc.DepthClipEnable = TRUE;
	ras_desc.ScissorEnable = FALSE;
	ras_desc.MultisampleEnable = FALSE;
	ras_desc.AntialiasedLineEnable = FALSE;

	if (DX_FAILED(ctx->mDevice->CreateRasterizerState(&ras_desc, &mWireframeRS))) {
		return false;
	}


	D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
	memset(&depth_stencil_desc, 0, sizeof(depth_stencil_desc));

	depth_stencil_desc.DepthEnable = FALSE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	depth_stencil_desc.StencilEnable = FALSE;

	if (DX_FAILED(ctx->mDevice->CreateDepthStencilState(&depth_stencil_desc, &mDisableDepthDSS))) {
		return false;
	}

	depth_stencil_desc.DepthEnable = TRUE;
	depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depth_stencil_desc.StencilEnable = FALSE;

	if (DX_FAILED(ctx->mDevice->CreateDepthStencilState(&depth_stencil_desc, &mEnableDepthDSS))) {
		return false;
	}


	D3D11_BLEND_DESC blend_desc = { 0 };

	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = FALSE;
	blend_desc.RenderTarget[0].BlendEnable = FALSE;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	if (DX_FAILED(ctx->mDevice->CreateBlendState(&blend_desc, &mDisableBlendBS))) {
		return false;
	}

	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = FALSE;
	blend_desc.RenderTarget[0].BlendEnable = TRUE;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	if (DX_FAILED(ctx->mDevice->CreateBlendState(&blend_desc, &mEnableBlendBS))) {
		return false;
	}

	return true;
}

void States::Shutdown() {
	DX_SAFE_RELEASE(mEnableBlendBS);
	DX_SAFE_RELEASE(mDisableBlendBS);
	DX_SAFE_RELEASE(mEnableDepthDSS);
	DX_SAFE_RELEASE(mDisableDepthDSS);
	DX_SAFE_RELEASE(mWireframeRS);
	DX_SAFE_RELEASE(mSolidRS);
}
