struct VS_OUTPUT_POS {
	float4 pos_homo : SV_POSITION;
};

float4 main(VS_OUTPUT_POS pin) : SV_Target
{
	float4 color = float4(0.3f, 0.3f, 0.3f, 1.0f);
	return color;
}