
struct VertexIn {
    float4 position : POSITION;
    float4 color : COLOR0;
};

struct VertexOut {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

struct Uniforms {
    float alphaRef0;
    float alphaRef1;
    int alphaComp0;
    int alphaComp1;
    int alphaLogic;
};

[[vk::binding(0, 1)]]
ConstantBuffer<Uniforms> uniforms;

[shader("vertex")]
VertexOut vertexMain(VertexIn input) {
    VertexOut output;
    output.position = input.position;
    output.color = input.color;
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

[shader("fragment")]
float4 fragmentMain(VertexOut input) : SV_TARGET {
    float4 color = input.color;
    if (!alphaTest(color.a)) {
        discard;
    }

    return color;
}
