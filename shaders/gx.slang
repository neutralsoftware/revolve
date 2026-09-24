
struct VertexIn {
    float4 position : POSITION;

    float4 color0 : COLOR0;
    float4 color1 : COLOR1;

    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float2 uv2 : TEXCOORD2;
    float2 uv3 : TEXCOORD3;
    float2 uv4 : TEXCOORD4;
    float2 uv5 : TEXCOORD5;
    float2 uv6 : TEXCOORD6;
    float2 uv7 : TEXCOORD7;
};

struct VertexOut {
    float4 position : SV_POSITION;

    float4 color0 : COLOR0;
    float4 color1 : COLOR1;

    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float2 uv2 : TEXCOORD2;
    float2 uv3 : TEXCOORD3;
    float2 uv4 : TEXCOORD4;
    float2 uv5 : TEXCOORD5;
    float2 uv6 : TEXCOORD6;
    float2 uv7 : TEXCOORD7;
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

    int konstColorSel;
    int konstAlphaSel;
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

    float4 tevRegister0;
    float4 tevRegister1;
    float4 tevRegister2;
    float4 tevRegister3;

    float4 konst0;
    float4 konst1;
    float4 konst2;
    float4 konst3;

    float4 swapTable0;
    float4 swapTable1;
    float4 swapTable2;
    float4 swapTable3;

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

    output.color0 = input.color0;
    output.color1 = input.color1;

    output.uv0 = input.uv0;
    output.uv1 = input.uv1;
    output.uv2 = input.uv2;
    output.uv3 = input.uv3;
    output.uv4 = input.uv4;
    output.uv5 = input.uv5;
    output.uv6 = input.uv6;
    output.uv7 = input.uv7;

    return output;
}

uint tev8(float value) { return (uint)round(saturate(value) * 255.0); }

bool compareTevColor(float3 A, float3 B, int scale, int op) {
    uint3 a = uint3(tev8(A.r), tev8(A.g), tev8(A.b));
    uint3 b = uint3(tev8(B.r), tev8(B.g), tev8(B.b));

    bool equality = op != 0;

    if (scale == 0) {
        return equality ? a.r == b.r : a.r > b.r;
    }

    if (scale == 1) {
        uint av = (a.g << 8) | a.r;
        uint bv = (b.g << 8) | b.r;

        return equality ? av == bv : av > bv;
    }

    if (scale == 2) {
        uint av = (a.b << 16) | (a.g << 8) | a.r;
        uint bv = (b.b << 16) | (b.g << 8) | b.r;

        return equality ? av == bv : av > bv;
    }

    return false;
}

int4 getSwapTable(int index) {
    switch (index) {
    case 0:
        return int4(uniforms.swapTable0);
    case 1:
        return int4(uniforms.swapTable1);
    case 2:
        return int4(uniforms.swapTable2);
    case 3:
        return int4(uniforms.swapTable3);
    }

    return int4(0, 1, 2, 3);
}

float getChannel(float4 value, int channel) {
    switch (channel) {
    case 0:
        return value.r;
    case 1:
        return value.g;
    case 2:
        return value.b;
    case 3:
        return value.a;
    }

    return 0.0;
}

float4 getRasterColor(int channel, VertexOut input) {
    switch (channel) {
    case 0:
        return input.color0;
    case 1:
        return input.color1;
    default:
        return float4(0.0, 0.0, 0.0, 0.0);
    }
}

float2 getGXTexCoord(int index, VertexOut input) {
    switch (index) {
    case 0:
        return input.uv0;
    case 1:
        return input.uv1;
    case 2:
        return input.uv2;
    case 3:
        return input.uv3;
    case 4:
        return input.uv4;
    case 5:
        return input.uv5;
    case 6:
        return input.uv6;
    case 7:
        return input.uv7;
    }

    return float2(0.0, 0.0);
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

float4 applySwap(float4 value, int tableIndex) {
    int4 table = getSwapTable(tableIndex);

    return float4(getChannel(value, table.x), getChannel(value, table.y),
                  getChannel(value, table.z), getChannel(value, table.w));
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

float4 getKonstRegister(int index) {
    switch (index) {
    case 0:
        return uniforms.konst0;
    case 1:
        return uniforms.konst1;
    case 2:
        return uniforms.konst2;
    case 3:
        return uniforms.konst3;
    }

    return float4(0, 0, 0, 0);
}

float3 getKonstColor(int sel) {
    switch (sel) {
    case 0:
        return float3(1.0);
    case 1:
        return float3(7.0 / 8.0);
    case 2:
        return float3(3.0 / 4.0);
    case 3:
        return float3(5.0 / 8.0);
    case 4:
        return float3(1.0 / 2.0);
    case 5:
        return float3(3.0 / 8.0);
    case 6:
        return float3(1.0 / 4.0);
    case 7:
        return float3(1.0 / 8.0);

    case 8:
    case 9:
    case 10:
    case 11:
        return float3(0.0);

    case 12:
        return uniforms.konst0.rgb;
    case 13:
        return uniforms.konst1.rgb;
    case 14:
        return uniforms.konst2.rgb;
    case 15:
        return uniforms.konst3.rgb;
    }

    int group = (sel - 16) / 4;

    int reg = (sel - 16) % 4;

    float4 k = getKonstRegister(reg);

    float component;

    switch (group) {
    case 0:
        component = k.r;
        break;
    case 1:
        component = k.g;
        break;
    case 2:
        component = k.b;
        break;
    default:
        component = k.a;
        break;
    }

    return float3(component);
}

float getKonstAlpha(int sel) {
    switch (sel) {
    case 0:
        return 1.0;
    case 1:
        return 7.0 / 8.0;
    case 2:
        return 3.0 / 4.0;
    case 3:
        return 5.0 / 8.0;
    case 4:
        return 1.0 / 2.0;
    case 5:
        return 3.0 / 8.0;
    case 6:
        return 1.0 / 4.0;
    case 7:
        return 1.0 / 8.0;

    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        return 0.0;
    }

    int group = (sel - 16) / 4;

    int reg = (sel - 16) % 4;

    float4 k = getKonstRegister(reg);

    switch (group) {
    case 0:
        return k.r;
    case 1:
        return k.g;
    case 2:
        return k.b;
    default:
        return k.a;
    }
}

float3 getTevColorArg(int arg, TevRegisters registers, float4 tex, float4 ras,
                      float3 konst) {
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
        return konst;
    case 15:
        return float3(0.0, 0.0, 0.0);
    }

    return float3(0.0, 0.0, 0.0);
}

float getTevAlphaArg(int arg, TevRegisters registers, float4 tex, float4 ras,
                     float konst) {
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
        return konst;
    case 7:
        return 0.0;
    }

    return 0.0;
}

float3 applyTevColor(float3 A, float3 B, float3 C, float3 D, TevStage stage) {
    if (stage.colorBias == 3) {
        if (stage.colorScale == 3) {
            uint3 a = uint3(tev8(A.r), tev8(A.g), tev8(A.b));

            uint3 b = uint3(tev8(B.r), tev8(B.g), tev8(B.b));

            bool equality = stage.colorOp != 0;

            float3 mask;

            if (equality) {
                mask = float3(a.r == b.r ? 1.0 : 0.0, a.g == b.g ? 1.0 : 0.0,
                              a.b == b.b ? 1.0 : 0.0);
            } else {
                mask = float3(a.r > b.r ? 1.0 : 0.0, a.g > b.g ? 1.0 : 0.0,
                              a.b > b.b ? 1.0 : 0.0);
            }

            float3 result = D + C * mask;

            if (stage.colorClamp != 0)
                result = saturate(result);

            return result;
        }

        bool pass = compareTevColor(A, B, stage.colorScale, stage.colorOp);

        float3 result = D + (pass ? C : float3(0));

        if (stage.colorClamp != 0)
            result = saturate(result);

        return result;
    }

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
    if (stage.alphaBias == 3) {
        uint a = tev8(A);

        uint b = tev8(B);

        bool equality = stage.alphaOp != 0;

        bool pass = equality ? a == b : a > b;

        float result = D + (pass ? C : 0.0);

        if (stage.alphaClamp != 0)
            result = saturate(result);

        return result;
    }

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
    float4 ras = getRasterColor(stage.colorChannel, input);
    ras = applySwap(ras, stage.rasterSwap);

    float4 tex = float4(1, 1, 1, 1);
    tex = applySwap(tex, stage.textureSwap);

    if (stage.textureEnabled != 0) {
        float2 uv = getGXTexCoord(stage.texCoord, input);

        tex = sampleGXTexture(stage.texMap, uv);
    }

    float3 konstColor = getKonstColor(stage.konstColorSel);
    float konstAlpha = getKonstAlpha(stage.konstAlphaSel);

    float3 A = getTevColorArg(stage.colorA, regs, tex, ras, konstColor);
    float3 B = getTevColorArg(stage.colorB, regs, tex, ras, konstColor);
    float3 C = getTevColorArg(stage.colorC, regs, tex, ras, konstColor);
    float3 D = getTevColorArg(stage.colorD, regs, tex, ras, konstColor);

    float colorA = getTevAlphaArg(stage.alphaA, regs, tex, ras, konstAlpha);
    float colorB = getTevAlphaArg(stage.alphaB, regs, tex, ras, konstAlpha);
    float colorC = getTevAlphaArg(stage.alphaC, regs, tex, ras, konstAlpha);
    float colorD = getTevAlphaArg(stage.alphaD, regs, tex, ras, konstAlpha);

    float3 rgb = applyTevColor(A, B, C, D, stage);
    float alpha = applyTevAlpha(colorA, colorB, colorC, colorD, stage);

    writeTevColor(stage.colorOutput, rgb, regs);
    writeTevAlpha(stage.alphaOutput, alpha, regs);
}

[shader("fragment")]
float4 fragmentMain(VertexOut input) : SV_TARGET {
    TevRegisters registers;
    registers.prev = uniforms.tevRegister0;
    registers.reg0 = uniforms.tevRegister1;
    registers.reg1 = uniforms.tevRegister2;
    registers.reg2 = uniforms.tevRegister3;

    for (int i = 0; i < uniforms.tevStageCount; i++) {
        executeTevStage(uniforms.tevStages[i], input, registers);
    }

    float4 color = registers.prev;

    if (!alphaTest(color.a)) {
        discard;
    }

    return color;
}
