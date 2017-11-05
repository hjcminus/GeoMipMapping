struct VS_OUTPUT_POS_COLOR {
	float4		pos_homo	:	SV_POSITION;	// position in homogeneous space
	float4		color		:	COLOR;
};

float4 main(VS_OUTPUT_POS_COLOR pin) : SV_Target
{
	float4 color = pin.color;
	return color;
}

