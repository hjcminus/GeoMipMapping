// Shaders.cpp

#include "Precompiled.h"

/*
================================================================================
Shaders
================================================================================
*/
Shaders::Shaders():
	mTextVertexShader(nullptr),
	mTextPixelShader(nullptr),
	mTextInputLayout(nullptr),
	mSkydomeVertexShader(nullptr),
	mSkydomePixelShader(nullptr),
	mSkydomeInputLayout(nullptr),
	mCloudVertexShader(nullptr),
	mCloudPixelShader(nullptr),
	mCloudInputLayout(nullptr),
	mWireframeVertexShader(nullptr),
	mWireframePixelShader(nullptr),
	mWireframeInputLayout(nullptr),
	mSolidVertexShader(nullptr),
	mSolidPixelShader(nullptr),
	mSolidInputLayout(nullptr)
{
}

Shaders::~Shaders() {
}

bool Shaders::Init(d3d11_ctx_s *ctx, const char_t *shader_dir) {
	char_t full_filename[MAX_PATH_];
	D3D11_INPUT_ELEMENT_DESC desc[16];

	// text
	sprintf_s_(full_filename, TXT_("%s\\binary\\text.vert.bin"), shader_dir);

	desc[0].SemanticName = "POSITION";
	desc[0].SemanticIndex = 0;
	desc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc[0].InputSlot = 0;
	desc[0].AlignedByteOffset = 0;
	desc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[0].InstanceDataStepRate = 0;

	desc[1].SemanticName = "TEXCOORD";
	desc[1].SemanticIndex = 0;
	desc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	desc[1].InputSlot = 0;
	desc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	desc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[1].InstanceDataStepRate = 0;

	if (!CreateVertexShaderAndInputLayout(ctx->mDevice, full_filename, desc, 2, mTextVertexShader, mTextInputLayout)) {
		return false;
	}

	sprintf_s_(full_filename, TXT_("%s\\binary\\text.frag.bin"), shader_dir);
	if (!CreatePixelShader(ctx->mDevice, full_filename, mTextPixelShader)) {
		return false;
	}

	// skydome
	sprintf_s_(full_filename, TXT_("%s\\binary\\skydome.vert.bin"), shader_dir);

	desc[0].SemanticName = "POSITION";
	desc[0].SemanticIndex = 0;
	desc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc[0].InputSlot = 0;
	desc[0].AlignedByteOffset = 0;
	desc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[0].InstanceDataStepRate = 0;

	desc[1].SemanticName = "COLOR";
	desc[1].SemanticIndex = 0;
	desc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc[1].InputSlot = 0;
	desc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	desc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[1].InstanceDataStepRate = 0;

	if (!CreateVertexShaderAndInputLayout(ctx->mDevice, full_filename, desc, 2, mSkydomeVertexShader, mSkydomeInputLayout)) {
		return false;
	}

	sprintf_s_(full_filename, TXT_("%s\\binary\\skydome.frag.bin"), shader_dir);
	if (!CreatePixelShader(ctx->mDevice, full_filename, mSkydomePixelShader)) {
		return false;
	}

	// cloud
	sprintf_s_(full_filename, TXT_("%s\\binary\\cloud.vert.bin"), shader_dir);

	desc[0].SemanticName = "POSITION";
	desc[0].SemanticIndex = 0;
	desc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc[0].InputSlot = 0;
	desc[0].AlignedByteOffset = 0;
	desc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[0].InstanceDataStepRate = 0;

	desc[1].SemanticName = "COLOR";
	desc[1].SemanticIndex = 0;
	desc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc[1].InputSlot = 0;
	desc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	desc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[1].InstanceDataStepRate = 0;

	if (!CreateVertexShaderAndInputLayout(ctx->mDevice, full_filename, desc, 2, mCloudVertexShader, mCloudInputLayout)) {
		return false;
	}

	sprintf_s_(full_filename, TXT_("%s\\binary\\cloud.frag.bin"), shader_dir);
	if (!CreatePixelShader(ctx->mDevice, full_filename, mCloudPixelShader)) {
		return false;
	}

	// wireframe mode
	sprintf_s_(full_filename, TXT_("%s\\binary\\wireframe.vert.bin"), shader_dir);

	desc[0].SemanticName = "POSITION";
	desc[0].SemanticIndex = 0;
	desc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc[0].InputSlot = 0;
	desc[0].AlignedByteOffset = 0;
	desc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[0].InstanceDataStepRate = 0;

	if (!CreateVertexShaderAndInputLayout(ctx->mDevice, full_filename, desc, 1, mWireframeVertexShader, mWireframeInputLayout)) {
		return false;
	}

	sprintf_s_(full_filename, TXT_("%s\\binary\\wireframe.frag.bin"), shader_dir);
	if (!CreatePixelShader(ctx->mDevice, full_filename, mWireframePixelShader)) {
		return false;
	}

	// solid mode
	sprintf_s_(full_filename, TXT_("%s\\binary\\solid.vert.bin"), shader_dir);

	desc[0].SemanticName = "POSITION";
	desc[0].SemanticIndex = 0;
	desc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	desc[0].InputSlot = 0;
	desc[0].AlignedByteOffset = 0;
	desc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc[0].InstanceDataStepRate = 0;

	if (!CreateVertexShaderAndInputLayout(ctx->mDevice, full_filename, desc, 1, mSolidVertexShader, mSolidInputLayout)) {
		return false;
	}

	sprintf_s_(full_filename, TXT_("%s\\binary\\solid.frag.bin"), shader_dir);
	if (!CreatePixelShader(ctx->mDevice, full_filename, mSolidPixelShader)) {
		return false;
	}

	return true;
}

void Shaders::Shutdown() {
	DX_SAFE_RELEASE(mSolidInputLayout);
	DX_SAFE_RELEASE(mSolidPixelShader);
	DX_SAFE_RELEASE(mSolidVertexShader);

	DX_SAFE_RELEASE(mWireframeInputLayout);
	DX_SAFE_RELEASE(mWireframePixelShader);
	DX_SAFE_RELEASE(mWireframeVertexShader);

	DX_SAFE_RELEASE(mCloudInputLayout);
	DX_SAFE_RELEASE(mCloudVertexShader);
	DX_SAFE_RELEASE(mCloudPixelShader);

	DX_SAFE_RELEASE(mSkydomeInputLayout);
	DX_SAFE_RELEASE(mSkydomeVertexShader);
	DX_SAFE_RELEASE(mSkydomePixelShader);

	DX_SAFE_RELEASE(mTextInputLayout);
	DX_SAFE_RELEASE(mTextVertexShader);
	DX_SAFE_RELEASE(mTextPixelShader);
}

void Shaders::SetTextShaders(ID3D11DeviceContext * device_context, bool set) {
	device_context->IASetInputLayout(set ? mTextInputLayout : nullptr);
	device_context->VSSetShader(set ? mTextVertexShader : nullptr, nullptr, 0);
	device_context->PSSetShader(set ? mTextPixelShader : nullptr, nullptr, 0);
}

void Shaders::SetSkydomeShaders(ID3D11DeviceContext * device_context, bool set) {
	device_context->IASetInputLayout(set ? mSkydomeInputLayout : nullptr);
	device_context->VSSetShader(set ? mSkydomeVertexShader : nullptr, nullptr, 0);
	device_context->PSSetShader(set ? mSkydomePixelShader : nullptr, nullptr, 0);
}

void Shaders::SetCloudShaders(ID3D11DeviceContext * device_context, bool set) {
	device_context->IASetInputLayout(set ? mCloudInputLayout : nullptr);
	device_context->VSSetShader(set ? mCloudVertexShader : nullptr, nullptr, 0);
	device_context->PSSetShader(set ? mCloudPixelShader : nullptr, nullptr, 0);
}

void Shaders::SetWireframeShaders(ID3D11DeviceContext * device_context, bool set) {
	device_context->IASetInputLayout(set ? mWireframeInputLayout : nullptr);
	device_context->VSSetShader(set ? mWireframeVertexShader : nullptr, nullptr, 0);
	device_context->PSSetShader(set ? mWireframePixelShader : nullptr, nullptr, 0);
}

void Shaders::SetSolidShaders(ID3D11DeviceContext * device_context, bool set) {
	device_context->IASetInputLayout(set ? mSolidInputLayout : nullptr);
	device_context->VSSetShader(set ? mSolidVertexShader : nullptr, nullptr, 0);
	device_context->PSSetShader(set ? mSolidPixelShader : nullptr, nullptr, 0);
}
