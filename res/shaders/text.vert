cbuffer cbWorldViewProjMatrix : register (b0)
{
	row_major float4x4	g_WorldViewProjectionMatrix;
};

// vertex
struct VS_INPUT_POS_TEXCOORD {
	float3		pos_local	:	POSITION;
	float2		tex_coord	:	TEXCOORD0;
};

struct VS_OUTPUT_POS_TEXCOORD {
	float4		pos_homo	:	SV_POSITION;
	float2		tex_coord	:	TEXCOORD0;
};

VS_OUTPUT_POS_TEXCOORD main(VS_INPUT_POS_TEXCOORD vin)
{
	VS_OUTPUT_POS_TEXCOORD vout;

	vout.pos_homo = mul(float4(vin.pos_local, 1.0f), g_WorldViewProjectionMatrix);
	vout.tex_coord = vin.tex_coord;
	return vout;
}
