cbuffer cbFog : register (b0)
{
	float4	g_fog;
};


Texture2D	g_tex_base	:	register(t0);
Texture2D	g_tex_detail	:	register(t1);
SamplerState	g_sampler_base	:	register(s0);
SamplerState	g_sampler_detail:	register(s1);

struct VS_OUTPUT_POS {
	float4 pos_homo : SV_POSITION;
	float2 tex_coord1: TEXCOORD0;
	float2 tex_coord2: TEXCOORD1;
	float3 pos_world:  TEXCOORD2;
};

float4 main(VS_OUTPUT_POS pin) : SV_Target
{
	float4	fog_color = float4(g_fog.x, g_fog.y, g_fog.z, 1.0f);
	float	fog_density = g_fog.w;
	float4	base_color = g_tex_base.Sample(g_sampler_base, pin.tex_coord1);
	float4	detail_color = g_tex_detail.Sample(g_sampler_detail, pin.tex_coord2);
	float4	combined_color = lerp(base_color, detail_color, 0.5f);
	
	float distance = length(pin.pos_world);
	float visibility = exp(-pow((distance * fog_density), 0.5f));
	visibility = saturate(visibility);

	float a = 1.0 - visibility;

	float4 color = lerp(combined_color, fog_color, a);

	return color;
}
