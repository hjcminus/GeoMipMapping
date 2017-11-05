// RenderSkydome.cpp

#include "Precompiled.h"

/*
================================================================================
RenderSkydome
================================================================================
*/
RenderSkydome::RenderSkydome() :
	mVertexBuffer(nullptr),
	mIndexBuffer(nullptr),
	mTrianglesCount(0)
{
}

RenderSkydome::~RenderSkydome() {
	// do nothing
}

bool RenderSkydome::Init(d3d11_ctx_s *ctx) {
	int vertices_count = 1 + (SKYDOME_STACKS + 1) * SKYDOME_SLICES;
	mTrianglesCount = SKYDOME_SLICES + SKYDOME_STACKS * SKYDOME_SLICES * 2;

	vertex_pos_clr_s * vert_buf = NEW__ vertex_pos_clr_s[vertices_count];
	unsigned int * idx_buf = NEW__ unsigned int[mTrianglesCount * 3];

	vertex_pos_clr_s * v = vert_buf;
	unsigned int * i = idx_buf;

	float radius = SKYDOME_RADIUS;
	vec4_t zenith_color = ZENITH_COLOR;
	vec4_t horizontal_color = HORIZONTAL_COLOR;
	vec4_t color_delta;
	Vec4Sub(horizontal_color, zenith_color, color_delta);

	Vec3Set(v->mPosition, 0.0f, 0.0f, radius);
	Vec4Copy(zenith_color, v->mColor);

	float half_rad = Deg2Rad(90.0f);
	float stack_step = half_rad / (float)SKYDOME_STACKS;
	float slice_step = PI * 2.0f / (float)SKYDOME_SLICES;

	for (int stack = 0; stack <= SKYDOME_STACKS; ++stack) {
		float stack_angle = (stack + 1) * stack_step;

		float z = radius * cosf(stack_angle);
		float stack_radius = radius * sinf(stack_angle);

		if (stack == SKYDOME_STACKS) { // the last stack
			z = -radius * 0.5f;
			stack_radius = radius;
		}

		for (int slice = 0; slice < SKYDOME_SLICES; ++slice) {
			float slice_angle = slice * slice_step;

			float x = stack_radius * cosf(slice_angle);
			float y = stack_radius * sinf(slice_angle);

			v++;

			float factor = 1.0f;
			if (stack < SKYDOME_STACKS) {
				float persent = ((float)stack + 1.0f) / (float)SKYDOME_STACKS;
				factor = powf(persent, 2.0f); // as exponential increase
			}

			Vec3Set(v->mPosition, x, y, z);
			Vec4MulAdd(color_delta, factor, zenith_color, v->mColor);

			if (0 == stack) {// triangle fan
				i[0] = 0;
				i[1] = 1 + (slice + 1) % SKYDOME_SLICES;
				i[2] = 1 + slice;

				i += 3;
			}
			else { // triangle strip
				   /* indices

				   zenith
				   / \
				   0---3
				   | \ |
				   1---2
				   */

				int idx0 = 1 /* first zenith vertex */ + (stack - 1) * SKYDOME_SLICES /* stack offset */ + slice /* current slice */;
				int idx1 = 1 + stack * SKYDOME_SLICES + slice;
				int idx2 = 1 + stack * SKYDOME_SLICES + (slice + 1) % SKYDOME_SLICES;
				int idx3 = 1 + (stack - 1) * SKYDOME_SLICES + (slice + 1) % SKYDOME_SLICES;

				i[0] = idx0;
				i[1] = idx2;
				i[2] = idx1;
				i[3] = idx0;
				i[4] = idx3;
				i[5] = idx2;

				i += 6;
			}
		}
	}

	// create buffer objects
	if (!CreateVertexAndIndexBuffers(ctx, sizeof(vertex_pos_clr_s) * vertices_count, vert_buf,
		sizeof(unsigned int) * mTrianglesCount * 3, idx_buf, false, mVertexBuffer, mIndexBuffer)) {
		
		delete[]idx_buf;
		delete[]vert_buf;

		return false;
	}
	else {
		delete[]idx_buf;
		delete[]vert_buf;

		return true;
	}
}

void RenderSkydome::Shutdown() {
	DX_SAFE_RELEASE(mIndexBuffer);
	DX_SAFE_RELEASE(mVertexBuffer);
}

void RenderSkydome::Draw(ID3D11DeviceContext *ctx) {
	UINT stride = sizeof(vertex_pos_clr_s);
	UINT offset = 0;

	ctx->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	ctx->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->DrawIndexed(mTrianglesCount * 3, 0, 0);
	
	ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
}
