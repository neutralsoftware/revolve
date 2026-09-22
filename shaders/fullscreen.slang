struct VertexIn {
    float2 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VertexOut {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D<float4> xfbTexture;
SamplerState xfbSampler;

[shader("vertex")]
VertexOut vertexMain(VertexIn input) {
    VertexOut output;
    output.position = float4(input.position, 0.0, 1.0);
    output.uv = input.uv;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOut input) : SV_TARGET {
    return xfbTexture.Sample(xfbSampler, input.uv);
}
