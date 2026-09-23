
struct VertexIn {
    float4 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct VertexOut {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct Uniforms {
    float alphaRef0;
    float alphaRef1;
    int alphaComp0;
    int alphaComp1;
    int alphaLogic;

    int activeTexture;
};

[[vk::binding(0, 1)]]
ConstantBuffer<Uniforms> uniforms;

[[vk::binding(2, 1)]]
Texture2D<float4> tex0;

[[vk::binding(3, 1)]]
Texture2D<float4> tex1;

[[vk::binding(4, 1)]]
Texture2D<float4> tex2;

[[vk::binding(5, 1)]]
Texture2D<float4> tex3;

[[vk::binding(6, 1)]]
Texture2D<float4> tex4;

[[vk::binding(7, 1)]]
Texture2D<float4> tex5;

[[vk::binding(8, 1)]]
Texture2D<float4> tex6;

[[vk::binding(9, 1)]]
Texture2D<float4> tex7;

[[vk::binding(10, 1)]]
SamplerState samp0;

[[vk::binding(11, 1)]]
SamplerState samp1;

[[vk::binding(12, 1)]]
SamplerState samp2;

[[vk::binding(13, 1)]]
SamplerState samp3;

[[vk::binding(14, 1)]]
SamplerState samp4;

[[vk::binding(15, 1)]]
SamplerState samp5;

[[vk::binding(16, 1)]]
SamplerState samp6;

[[vk::binding(17, 1)]]
SamplerState samp7;

[shader("vertex")]
VertexOut vertexMain(VertexIn input) {
    VertexOut output;
    output.position = input.position;
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

bool alphaCompare(float alpha, float ref, int op) {
    switch (op) {
    case 0:
        return false;
    case 1:
        return alpha < ref;
    case 2:
        return alpha == ref;
    case 3:
        return alpha <= ref;
    case 4:
        return alpha > ref;
    case 5:
        return alpha != ref;
    case 6:
        return alpha >= ref;
    case 7:
        return true;
    }

    return true;
}

bool alphaTest(float alpha) {
    bool test0 = alphaCompare(alpha, uniforms.alphaRef0, uniforms.alphaComp0);
    bool test1 = alphaCompare(alpha, uniforms.alphaRef1, uniforms.alphaComp1);

    switch (uniforms.alphaLogic) {
    case 0:
        return test0 && test1;
    case 1:
        return test0 || test1;
    case 2:
        return test0 != test1;
    case 3:
        return test0 == test1;
    }

    return true;
}

float4 sampleGXTexture(int index, float2 uv) {
    switch (index) {
    case 0:
        return tex0.Sample(samp0, uv);
    case 1:
        return tex1.Sample(samp1, uv);
    case 2:
        return tex2.Sample(samp2, uv);
    case 3:
        return tex3.Sample(samp3, uv);
    case 4:
        return tex4.Sample(samp4, uv);
    case 5:
        return tex5.Sample(samp5, uv);
    case 6:
        return tex6.Sample(samp6, uv);
    case 7:
        return tex7.Sample(samp7, uv);
    }

    return float4(1.0, 1.0, 1.0, 1.0);
}

[shader("fragment")]
float4 fragmentMain(VertexOut input) : SV_TARGET {
    float4 color = input.color;
    float4 texColor = sampleGXTexture(uniforms.activeTexture, input.uv);
    color = texColor;

    if (!alphaTest(color.a)) {
        discard;
    }

    return color;
}
