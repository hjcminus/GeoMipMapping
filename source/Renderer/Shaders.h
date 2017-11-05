// Shaders.h

#pragma once

/*
================================================================================
Shaders
================================================================================
*/
class Shaders {
public:

	Shaders();
	~Shaders();

	bool						Init(d3d11_ctx_s *ctx, const char_t *shader_dir);
	void						Shutdown();

	void						SetTextShaders(ID3D11DeviceContext * device_context, bool set);
	void						SetSkydomeShaders(ID3D11DeviceContext * device_context, bool set);
	void						SetCloudShaders(ID3D11DeviceContext * device_context, bool set);
	void						SetWireframeShaders(ID3D11DeviceContext * device_context, bool set);
	void						SetSolidShaders(ID3D11DeviceContext * device_context, bool set);

public:

	// shaders
	ID3D11VertexShader *		mTextVertexShader;
	ID3D11PixelShader *			mTextPixelShader;
	ID3D11InputLayout *			mTextInputLayout;

	ID3D11VertexShader *		mSkydomeVertexShader;
	ID3D11PixelShader *			mSkydomePixelShader;
	ID3D11InputLayout *			mSkydomeInputLayout;

	ID3D11VertexShader *		mCloudVertexShader;
	ID3D11PixelShader *			mCloudPixelShader;
	ID3D11InputLayout *			mCloudInputLayout;

	ID3D11VertexShader *		mWireframeVertexShader;
	ID3D11PixelShader *			mWireframePixelShader;
	ID3D11InputLayout *			mWireframeInputLayout;

	ID3D11VertexShader *		mSolidVertexShader;
	ID3D11PixelShader *			mSolidPixelShader;
	ID3D11InputLayout *			mSolidInputLayout;
};

