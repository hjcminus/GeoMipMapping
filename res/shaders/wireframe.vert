cbuffer cbWorldViewProjMatrix : register (b0)
{
	row_major float4x4 g_WorldViewProjectionMatrix;
};

struct VS_INPUT_POS {
	float3 pos_local : POSITION;
};

struct VS_OUTPUT_POS {
	float4 pos_homo : SV_POSITION; // position in homogeneous space
};

VS_OUTPUT_POS main(VS_INPUT_POS vin)
{
	VS_OUTPUT_POS vout;
	vout.pos_homo = mul(float4(vin.pos_local, 1.0f), g_WorldViewProjectionMatrix);
	return vout;
}