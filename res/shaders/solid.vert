cbuffer cbWorldViewMatrix : register (b0)
{
	row_major float4x4 g_WorldViewMatrix;
};

cbuffer cbWorldViewProjMatrix : register (b1)
{
	row_major float4x4 g_WorldViewProjectionMatrix;
};

cbuffer cbTexSize : register (b2)
{
	float		g_TexWidth;
	float		g_TexHeight;
	float2		g_TexSizePad;
	
};

struct VS_INPUT_POS {
	float3 pos_local : POSITION;
};

struct VS_OUTPUT_POS {
	float4 pos_homo :  SV_POSITION; // position in homogeneous space
	float2 tex_coord1: TEXCOORD0;
	float2 tex_coord2: TEXCOORD1;
	float3 pos_world:  TEXCOORD2;
};

VS_OUTPUT_POS main(VS_INPUT_POS vin)
{
	VS_OUTPUT_POS vout;
	vout.pos_homo = mul(float4(vin.pos_local, 1.0f), g_WorldViewProjectionMatrix);
	vout.tex_coord1.x = vin.pos_local.x / g_TexWidth;
	vout.tex_coord1.y = 1.0f - vin.pos_local.y / g_TexHeight;
	vout.tex_coord2.x = vin.pos_local.x / 16.0f;
	vout.tex_coord2.y = (g_TexHeight - vin.pos_local.y) / 16.0f;
	float4 temp = mul(float4(vin.pos_local, 1.0f), g_WorldViewMatrix);
	vout.pos_world = float3(temp.x, temp.y, temp.z);
	return vout;
}