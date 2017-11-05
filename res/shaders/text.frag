Texture2D	g_tex		:	register(t0);
SamplerState	g_sampler	:	register(s0);

struct VS_OUTPUT_POS_TEXCOORD {
	float4		pos_homo	:	SV_POSITION;
	float2		tex_coord	:	TEXCOORD0;
};

float4 main(VS_OUTPUT_POS_TEXCOORD pin) : SV_Target
{
	float4 tex_color = g_tex.Sample(g_sampler, pin.tex_coord);
	float alpha = tex_color.r;
	float4 color = float4(0.0f, 0.5f, 0.0f, alpha);
	return color;
}