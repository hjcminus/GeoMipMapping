// RenderSkydome.h

#pragma once

/*
================================================================================
RenderSkydome
================================================================================
*/
class RenderSkydome {
public:
	RenderSkydome();
	~RenderSkydome();

	bool						Init(d3d11_ctx_s *ctx);
	void						Shutdown();

	void						Draw(ID3D11DeviceContext *ctx);

private:

	ID3D11Buffer *				mVertexBuffer;
	ID3D11Buffer *				mIndexBuffer;
	int							mTrianglesCount;
};
