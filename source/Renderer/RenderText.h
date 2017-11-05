// RenderText.h

#pragma once

/*
================================================================================
RenderText
================================================================================
*/
class RenderText {
public:
	RenderText();
	~RenderText();

	bool						Init(d3d11_ctx_s *ctx, const char_t *res_dir);
	void						Shutdown();

	void						Print(ID3D11DeviceContext *ctx, const char *text);

	void						Draw(ID3D11DeviceContext *ctx);

private:

	// texture coordinate
	struct char_info_s {
		float					mS0;
		float					mT0;
		float					mS1;
		float					mT1;
	};

	char_info_s					mCharInfos[95];

	ID3D11Buffer *				mVertexBuffer;
	UINT						mVerticesCount;

	ID3D11Texture2D *			mFontTexture;
	ID3D11ShaderResourceView *	mFontTextureSRV;
	ID3D11SamplerState *		mFontTextureSS;
};
