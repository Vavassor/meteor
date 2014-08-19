cbuffer simpleConstantBuffer : register(b[0])
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

struct VertexShaderInput
{
    float4 pos : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput vertexShaderOutput;
    float4 pos = input.pos;

    // Transform the vertex position into projection space.
    pos = mul(pos, model);
    pos = mul(pos, view);
    pos = mul(pos, projection);
    vertexShaderOutput.pos = pos;

    // Pass through the texture coordinate without modification.
    vertexShaderOutput.tex = input.tex;

    return vertexShaderOutput;
}