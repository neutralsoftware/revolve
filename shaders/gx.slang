
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

struct TevStage {
    int texCoord;
    int texMap;
    int colorChannel;
    int textureEnabled;

    int colorA;
    int colorB;
    int colorC;
    int colorD;

    int colorBias;
    int colorOp;
    int colorClamp;
    int colorScale;
    int colorOutput;

    int alphaA;
    int alphaB;
    int alphaC;
    int alphaD;

    int alphaBias;
    int alphaOp;
    int alphaClamp;
    int alphaScale;
    int alphaOutput;

    int rasterSwap;
    int textureSwap;

    int pad0;
    int pad1;
};

struct Uniforms {
    float alphaRef0;
    float alphaRef1;
    int alphaComp0;
    int alphaComp1;

    int alphaLogic;
    int tevStageCount;
    int pad0;
    int pad1;

    TevStage tevStages[16];
};

struct TevRegisters {
    float4 prev;
    float4 reg0;
    float4 reg1;
    float4 reg2;
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

float3 getTevColorArg(int arg, TevRegisters registers, float4 tex, float4 ras) {
    switch (arg) {
    case 0:
        return registers.prev.rgb;
    case 1:
        return registers.prev.aaa;
    case 2:
        return registers.reg0.rgb;
    case 3:
        return registers.reg0.aaa;
    case 4:
        return registers.reg1.rgb;
    case 5:
        return registers.reg1.aaa;
    case 6:
        return registers.reg2.rgb;
    case 7:
        return registers.reg2.aaa;
    case 8:
        return tex.rgb;
    case 9:
        return tex.aaa;
    case 10:
        return ras.rgb;
    case 11:
        return ras.aaa;
    case 12:
        return float3(1.0, 1.0, 1.0);
    case 13:
        return float3(0.5, 0.5, 0.5);
    case 14:
        // Konst color
        return float3(0.0, 0.0, 0.0);
    case 15:
        return float3(0.0, 0.0, 0.0);
    }

    return float3(0.0, 0.0, 0.0);
}

float getTevAlphaArg(int arg, TevRegisters registers, float4 tex, float4 ras) {
    switch (arg) {
    case 0:
        return registers.prev.a;
    case 1:
        return registers.reg0.a;
    case 2:
        return registers.reg1.a;
    case 3:
        return registers.reg2.a;
    case 4:
        return tex.a;
    case 5:
        return ras.a;
    case 6:
        // Konst color
        return 1.0;
    case 7:
        return 0.0;
    }

    return 0.0;
}

float3 applyTevColor(float3 A, float3 B, float3 C, float3 D, TevStage stage) {
    float3 blend = A * (1.0 - C) + B * C;

    float3 result;

    if (stage.colorOp == 0) {
        result = D + blend;
    } else {
        result = D - blend;
    }

    switch (stage.colorBias) {
    case 1:
        result += 0.5;
        break;
    case 2:
        result -= 0.5;
        break;
    }

    switch (stage.colorScale) {
    case 1:
        result *= 2.0;
        break;
    case 2:
        result *= 4.0;
        break;
    case 3:
        result *= 0.5;
        break;
    }

    if (stage.colorClamp != 0) {
        result = saturate(result);
    }

    return result;
}

float applyTevAlpha(float A, float B, float C, float D, TevStage stage) {
    float blend = A * (1.0 - C) + B * C;

    float result;

    if (stage.alphaOp == 0)
        result = D + blend;
    else
        result = D - blend;

    switch (stage.alphaBias) {
    case 1:
        result += 0.5;
        break;

    case 2:
        result -= 0.5;
        break;
    }

    switch (stage.alphaScale) {
    case 1:
        result *= 2.0;
        break;

    case 2:
        result *= 4.0;
        break;

    case 3:
        result *= 0.5;
        break;
    }

    if (stage.alphaClamp != 0)
        result = saturate(result);

    return result;
}

void writeTevColor(int output, float3 value, inout TevRegisters regs) {
    switch (output) {
    case 0:
        regs.prev.rgb = value;
        break;
    case 1:
        regs.reg0.rgb = value;
        break;
    case 2:
        regs.reg1.rgb = value;
        break;
    case 3:
        regs.reg2.rgb = value;
        break;
    }

    return;
}

void writeTevAlpha(int output, float value, inout TevRegisters regs) {
    switch (output) {
    case 0:
        regs.prev.a = value;
        break;
    case 1:
        regs.reg0.a = value;
        break;
    case 2:
        regs.reg1.a = value;
        break;
    case 3:
        regs.reg2.a = value;
        break;
    }

    return;
}

void executeTevStage(TevStage stage, VertexOut input, inout TevRegisters regs) {
    float4 ras = input.color;

    float4 tex = float4(1, 1, 1, 1);

    if (stage.textureEnabled != 0) {
        tex = sampleGXTexture(stage.texMap, input.uv);
    }

    float3 A = getTevColorArg(stage.colorA, regs, tex, ras);
    float3 B = getTevColorArg(stage.colorB, regs, tex, ras);
    float3 C = getTevColorArg(stage.colorC, regs, tex, ras);
    float3 D = getTevColorArg(stage.colorD, regs, tex, ras);

    float colorA = getTevAlphaArg(stage.alphaA, regs, tex, ras);
    float colorB = getTevAlphaArg(stage.alphaB, regs, tex, ras);
    float colorC = getTevAlphaArg(stage.alphaC, regs, tex, ras);
    float colorD = getTevAlphaArg(stage.alphaD, regs, tex, ras);

    float3 rgb = applyTevColor(A, B, C, D, stage);
    float alpha = applyTevAlpha(colorA, colorB, colorC, colorD, stage);

    writeTevColor(stage.colorOutput, rgb, regs);
    writeTevAlpha(stage.alphaOutput, alpha, regs);
}

[shader("fragment")]
float4 fragmentMain(VertexOut input) : SV_TARGET {
    TevRegisters registers;
    registers.prev = float4(1.0, 1.0, 1.0, 1.0);
    registers.reg0 = float4(0.0, 0.0, 0.0, 0.0);
    registers.reg1 = float4(0.0, 0.0, 0.0, 0.0);
    registers.reg2 = float4(0.0, 0.0, 0.0, 0.0);

    for (int i = 0; i < uniforms.tevStageCount; i++) {
        executeTevStage(uniforms.tevStages[i], input, registers);
    }

    float4 color = registers.prev;

    if (!alphaTest(color.a)) {
        discard;
    }

    return color;
}
