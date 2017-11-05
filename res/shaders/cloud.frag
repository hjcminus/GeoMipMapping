cbuffer cbMove : register (b0)
{
	float2	g_move;
	float2	g_move_pad;
};

Texture2D	g_tex		:	register(t0);
SamplerState	g_sampler	:	register(s0);

struct VS_OUTPUT_POS_COLOR {
	float4		pos_homo	:	SV_POSITION;	// position in homogeneous space
	float4		color		:	COLOR;
};

float4 main(VS_OUTPUT_POS_COLOR pin) : SV_Target
{
	float2 tex_coord = float2(pin.color.x, pin.color.y) + g_move;
	float4 color = g_tex.Sample(g_sampler, tex_coord);
	color.w = pin.color.w;
	return color;
}

