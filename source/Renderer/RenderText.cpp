// RenderText.cpp

#include "Precompiled.h"

/*
================================================================================
RenderText
================================================================================
*/
RenderText::RenderText() :
	mVertexBuffer(nullptr),
	mVerticesCount(0),
	mFontTexture(nullptr),
	mFontTextureSRV(nullptr),
	mFontTextureSS(nullptr)
{
}

RenderText::~RenderText() {
	// do nothing
}

bool RenderText::Init(d3d11_ctx_s *ctx, const char_t *res_dir) {
	// init texture coordinates
	int count = 0;
	for (float y = 128.0f; y > 16.0f; y -= 16.0f) {
		for (float x = 0.0f; x < 112.0f; x += 8.0f) {
			mCharInfos[count].mS0 = x / 128.0f;
			mCharInfos[count].mT0 = 1.0f - ((y - 16.0f) / 128.0f);
			mCharInfos[count].mS1 = (x + 8.0f) / 128.0f;
			mCharInfos[count].mT1 = 1.0f - (y / 128.0f);
			count++;
			if (count >= 95) {
				break;
			}
		}

		if (count >= 95) {
			break;
		}
	}

	// vertex buffer
	if (!CreateBuffer(ctx, D3D11_BIND_VERTEX_BUFFER, sizeof(vertex_pos_texcoord_s) * MAX_PRINT_TEXT_LEN * 6, nullptr, true, mVertexBuffer)) {
		return false;
	}
	
	// font texture
	char_t full_tex_filename[MAX_PATH_];
	sprintf_s_(full_tex_filename, TXT_("%s\\font\\font.png"), res_dir);

	int tex_width, tex_height; // dummy
	return LoadTexture(ctx, full_tex_filename, true, false, mFontTexture, mFontTextureSRV, mFontTextureSS, tex_width, tex_height);
}

void RenderText::Shutdown() {
	DX_SAFE_RELEASE(mFontTextureSS);
	DX_SAFE_RELEASE(mFontTextureSRV);
	DX_SAFE_RELEASE(mFontTexture);
	DX_SAFE_RELEASE(mVertexBuffer);
}

void RenderText::Print(ID3D11DeviceContext *ctx, const char *text) {
#define TEXT_CX	8.0f
#define TEXT_CY	16.0f

	D3D11_MAPPED_SUBRESOURCE ms;

	if (!DX_FAILED(ctx->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms))) {
		vertex_pos_texcoord_s * v = (vertex_pos_texcoord_s*)ms.pData;
		mVerticesCount = 0;

		float x = 0.0f;
		float y = 0.0f;
		const char * pc = text;
		while (*pc) {
			if (mVerticesCount >= MAX_PRINT_TEXT_LEN * 6) {
				break; // truncate
			}

			mVerticesCount += 6;
			char c = *pc;

			// clamp char
			if (c < 32) {
				c = 32;
			}

			int index = c - 32;

			if (index >= 95) {
				index = 0;
			}

			char_info_s * tex_coord = mCharInfos + index;

			v[0].mPosition[0] = x;
			v[0].mPosition[1] = y;
			v[0].mPosition[2] = 0.0f;
			v[0].mTexCoord[0] = tex_coord->mS0;
			v[0].mTexCoord[1] = tex_coord->mT0;

			v[1].mPosition[0] = x + TEXT_CX;
			v[1].mPosition[1] = y;
			v[1].mPosition[2] = 0.0f;
			v[1].mTexCoord[0] = tex_coord->mS1;
			v[1].mTexCoord[1] = tex_coord->mT0;

			v[2].mPosition[0] = x;
			v[2].mPosition[1] = y + TEXT_CY;
			v[2].mPosition[2] = 0.0f;
			v[2].mTexCoord[0] = tex_coord->mS0;
			v[2].mTexCoord[1] = tex_coord->mT1;

			v[3].mPosition[0] = x + TEXT_CX;
			v[3].mPosition[1] = y + TEXT_CY;
			v[3].mPosition[2] = 0.0f;
			v[3].mTexCoord[0] = tex_coord->mS1;
			v[3].mTexCoord[1] = tex_coord->mT1;

			v[4].mPosition[0] = x;
			v[4].mPosition[1] = y + TEXT_CY;
			v[4].mPosition[2] = 0.0f;
			v[4].mTexCoord[0] = tex_coord->mS0;
			v[4].mTexCoord[1] = tex_coord->mT1;

			v[5].mPosition[0] = x + TEXT_CX;
			v[5].mPosition[1] = y;
			v[5].mPosition[2] = 0.0f;
			v[5].mTexCoord[0] = tex_coord->mS1;
			v[5].mTexCoord[1] = tex_coord->mT0;
			
			x += TEXT_CX;
			v += 6;

			pc++;
		}

		ctx->Unmap(mVertexBuffer, 0);
	}
}

void RenderText::Draw(ID3D11DeviceContext *ctx) {
	if (mVerticesCount) {
		ctx->PSSetShaderResources(0, 1, &mFontTextureSRV);
		ctx->PSSetSamplers(0, 1, &mFontTextureSS);

		UINT stride = sizeof(vertex_pos_texcoord_s);
		UINT offset = 0;
		ctx->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);

		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->Draw(mVerticesCount, 0);

		ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);

		ID3D11ShaderResourceView * null_srv_list[1] = { nullptr };
		ID3D11SamplerState * null_ss_list[1] = { nullptr };
		ctx->PSSetShaderResources(0, 1, null_srv_list);
		ctx->PSSetSamplers(0, 1, null_ss_list);
	}
}
