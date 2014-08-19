cbuffer simpleConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

struct VertexShaderInput
{
    float4 pos : POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD0;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
	float4 col : COLOR;
    float2 tex : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput vertexShaderOutput;
    float4 pos = input.pos;

    pos = mul(pos, model);
    pos = mul(pos, view);
    pos = mul(pos, projection);
    vertexShaderOutput.pos = pos;

    vertexShaderOutput.tex = input.tex;
	vertexShaderOutput.col = input.col;

    return vertexShaderOutput;
}