#ifndef REVOLVE_GRAPHICS_SHADER
#define REVOLVE_GRAPHICS_SHADER

inline constexpr const char *FULLSCREEN_VERTEX_SHADER = R"SHDR0(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float2 entryPointParam_vertexMain_uv [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float2 input_position [[attribute(0)]];
    float2 input_uv [[attribute(1)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = float4(in.input_position, 0.0, 1.0);
    out.entryPointParam_vertexMain_uv = in.input_uv;
    return out;
}

)SHDR0";

inline constexpr const char *FULLSCREEN_FRAGMENT_SHADER = R"SHDR1(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 entryPointParam_fragmentMain [[color(0)]];
};

struct main0_in
{
    float2 input_uv [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> xfbTexture [[texture(0)]], sampler xfbSampler [[sampler(0)]])
{
    main0_out out = {};
    out.entryPointParam_fragmentMain = xfbTexture.sample(xfbSampler, in.input_uv);
    return out;
}

)SHDR1";

inline constexpr const char *GX_VERTEX_SHADER = R"SHDR2(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 entryPointParam_vertexMain_color0 [[user(locn0)]];
    float4 entryPointParam_vertexMain_color1 [[user(locn1)]];
    float2 entryPointParam_vertexMain_uv0 [[user(locn2)]];
    float2 entryPointParam_vertexMain_uv1 [[user(locn3)]];
    float2 entryPointParam_vertexMain_uv2 [[user(locn4)]];
    float2 entryPointParam_vertexMain_uv3 [[user(locn5)]];
    float2 entryPointParam_vertexMain_uv4 [[user(locn6)]];
    float2 entryPointParam_vertexMain_uv5 [[user(locn7)]];
    float2 entryPointParam_vertexMain_uv6 [[user(locn8)]];
    float2 entryPointParam_vertexMain_uv7 [[user(locn9)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 input_position [[attribute(0)]];
    float4 input_color0 [[attribute(1)]];
    float4 input_color1 [[attribute(2)]];
    float2 input_uv0 [[attribute(3)]];
    float2 input_uv1 [[attribute(4)]];
    float2 input_uv2 [[attribute(5)]];
    float2 input_uv3 [[attribute(6)]];
    float2 input_uv4 [[attribute(7)]];
    float2 input_uv5 [[attribute(8)]];
    float2 input_uv6 [[attribute(9)]];
    float2 input_uv7 [[attribute(10)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = in.input_position;
    out.entryPointParam_vertexMain_color0 = in.input_color0;
    out.entryPointParam_vertexMain_color1 = in.input_color1;
    out.entryPointParam_vertexMain_uv0 = in.input_uv0;
    out.entryPointParam_vertexMain_uv1 = in.input_uv1;
    out.entryPointParam_vertexMain_uv2 = in.input_uv2;
    out.entryPointParam_vertexMain_uv3 = in.input_uv3;
    out.entryPointParam_vertexMain_uv4 = in.input_uv4;
    out.entryPointParam_vertexMain_uv5 = in.input_uv5;
    out.entryPointParam_vertexMain_uv6 = in.input_uv6;
    out.entryPointParam_vertexMain_uv7 = in.input_uv7;
    return out;
}

)SHDR2";

inline constexpr const char *GX_FRAGMENT_SHADER = R"SHDR3(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct TevStage_std140
{
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
    char _m0_final_padding[8];
};

struct _Array_std140_TevStage16
{
    TevStage_std140 data[16];
};

struct Uniforms_std140
{
    float alphaRef0;
    float alphaRef1;
    int alphaComp0;
    int alphaComp1;
    int alphaLogic;
    int tevStageCount;
    int pad0;
    int pad1;
    _Array_std140_TevStage16 tevStages;
};

struct main0_out
{
    float4 entryPointParam_fragmentMain [[color(0)]];
};

struct main0_in
{
    float4 input_color0 [[user(locn0)]];
    float4 input_color1 [[user(locn1)]];
    float2 input_uv0 [[user(locn2)]];
    float2 input_uv1 [[user(locn3)]];
    float2 input_uv2 [[user(locn4)]];
    float2 input_uv3 [[user(locn5)]];
    float2 input_uv4 [[user(locn6)]];
    float2 input_uv5 [[user(locn7)]];
    float2 input_uv6 [[user(locn8)]];
    float2 input_uv7 [[user(locn9)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant Uniforms_std140& uniforms [[buffer(0)]], texture2d<float> tex0 [[texture(0)]], texture2d<float> tex1 [[texture(1)]], texture2d<float> tex2 [[texture(2)]], texture2d<float> tex3 [[texture(3)]], texture2d<float> tex4 [[texture(4)]], texture2d<float> tex5 [[texture(5)]], texture2d<float> tex6 [[texture(6)]], texture2d<float> tex7 [[texture(7)]], sampler samp0 [[sampler(0)]], sampler samp1 [[sampler(1)]], sampler samp2 [[sampler(2)]], sampler samp3 [[sampler(3)]], sampler samp4 [[sampler(4)]], sampler samp5 [[sampler(5)]], sampler samp6 [[sampler(6)]], sampler samp7 [[sampler(7)]])
{
    bool _1530 = false;
    bool _1505 = false;
    bool _1467 = false;
    bool _1298 = false;
    bool _1269 = false;
    bool _1240 = false;
    bool _1211 = false;
    bool _1164 = false;
    bool _1117 = false;
    bool _1070 = false;
    bool _1023 = false;
    bool _964 = false;
    bool _937 = false;
    bool _922 = false;
    main0_out out = {};
    float4 _1623 = float4(1.0);
    float4 _1624 = float4(0.0);
    float4 _1625 = float4(0.0);
    float4 _1626 = float4(0.0);
    int i = 0;
    float4 _871;
    float4 _923;
    float2 _938;
    float4 _965;
    float3 _1024;
    float3 _1071;
    float3 _1118;
    float3 _1165;
    float _1212;
    float _1241;
    float _1270;
    float _1299;
    float3 _1327;
    float _1375;
    for (;;)
    {
        if (!(i < uniforms.tevStageCount))
        {
            break;
        }
        _922 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorChannel)
            {
                case 0:
                {
                    _922 = true;
                    _923 = in.input_color0;
                    break;
                }
                case 1:
                {
                    _922 = true;
                    _923 = in.input_color1;
                    break;
                }
                default:
                {
                    _922 = true;
                    _923 = float4(0.0);
                    break;
                }
            }
            if (_922)
            {
                break;
            }
            break;
        } while(false);
        if (uniforms.tevStages.data[i].textureEnabled != 0)
        {
            _937 = false;
            do
            {
                switch (uniforms.tevStages.data[i].texCoord)
                {
                    case 0:
                    {
                        _937 = true;
                        _938 = in.input_uv0;
                        break;
                    }
                    case 1:
                    {
                        _937 = true;
                        _938 = in.input_uv1;
                        break;
                    }
                    case 2:
                    {
                        _937 = true;
                        _938 = in.input_uv2;
                        break;
                    }
                    case 3:
                    {
                        _937 = true;
                        _938 = in.input_uv3;
                        break;
                    }
                    case 4:
                    {
                        _937 = true;
                        _938 = in.input_uv4;
                        break;
                    }
                    case 5:
                    {
                        _937 = true;
                        _938 = in.input_uv5;
                        break;
                    }
                    case 6:
                    {
                        _937 = true;
                        _938 = in.input_uv6;
                        break;
                    }
                    case 7:
                    {
                        _937 = true;
                        _938 = in.input_uv7;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_937)
                {
                    break;
                }
                _937 = true;
                _938 = float2(0.0);
                break;
            } while(false);
            _964 = false;
            do
            {
                switch (uniforms.tevStages.data[i].texMap)
                {
                    case 0:
                    {
                        _964 = true;
                        _965 = tex0.sample(samp0, _938);
                        break;
                    }
                    case 1:
                    {
                        _964 = true;
                        _965 = tex1.sample(samp1, _938);
                        break;
                    }
                    case 2:
                    {
                        _964 = true;
                        _965 = tex2.sample(samp2, _938);
                        break;
                    }
                    case 3:
                    {
                        _964 = true;
                        _965 = tex3.sample(samp3, _938);
                        break;
                    }
                    case 4:
                    {
                        _964 = true;
                        _965 = tex4.sample(samp4, _938);
                        break;
                    }
                    case 5:
                    {
                        _964 = true;
                        _965 = tex5.sample(samp5, _938);
                        break;
                    }
                    case 6:
                    {
                        _964 = true;
                        _965 = tex6.sample(samp6, _938);
                        break;
                    }
                    case 7:
                    {
                        _964 = true;
                        _965 = tex7.sample(samp7, _938);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_964)
                {
                    break;
                }
                _964 = true;
                _965 = float4(1.0);
                break;
            } while(false);
            _871 = _965;
        }
        else
        {
            _871 = float4(1.0);
        }
        _1023 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorA)
            {
                case 0:
                {
                    _1023 = true;
                    _1024 = _1623.xyz;
                    break;
                }
                case 1:
                {
                    _1023 = true;
                    _1024 = _1623.www;
                    break;
                }
                case 2:
                {
                    _1023 = true;
                    _1024 = _1624.xyz;
                    break;
                }
                case 3:
                {
                    _1023 = true;
                    _1024 = _1624.www;
                    break;
                }
                case 4:
                {
                    _1023 = true;
                    _1024 = _1625.xyz;
                    break;
                }
                case 5:
                {
                    _1023 = true;
                    _1024 = _1625.www;
                    break;
                }
                case 6:
                {
                    _1023 = true;
                    _1024 = _1626.xyz;
                    break;
                }
                case 7:
                {
                    _1023 = true;
                    _1024 = _1626.www;
                    break;
                }
                case 8:
                {
                    _1023 = true;
                    _1024 = _871.xyz;
                    break;
                }
                case 9:
                {
                    _1023 = true;
                    _1024 = _871.www;
                    break;
                }
                case 10:
                {
                    _1023 = true;
                    _1024 = _923.xyz;
                    break;
                }
                case 11:
                {
                    _1023 = true;
                    _1024 = _923.www;
                    break;
                }
                case 12:
                {
                    _1023 = true;
                    _1024 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _1023 = true;
                    _1024 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _1023 = true;
                    _1024 = float3(0.0);
                    break;
                }
                case 15:
                {
                    _1023 = true;
                    _1024 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1023)
            {
                break;
            }
            _1023 = true;
            _1024 = float3(0.0);
            break;
        } while(false);
        _1070 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorB)
            {
                case 0:
                {
                    _1070 = true;
                    _1071 = _1623.xyz;
                    break;
                }
                case 1:
                {
                    _1070 = true;
                    _1071 = _1623.www;
                    break;
                }
                case 2:
                {
                    _1070 = true;
                    _1071 = _1624.xyz;
                    break;
                }
                case 3:
                {
                    _1070 = true;
                    _1071 = _1624.www;
                    break;
                }
                case 4:
                {
                    _1070 = true;
                    _1071 = _1625.xyz;
                    break;
                }
                case 5:
                {
                    _1070 = true;
                    _1071 = _1625.www;
                    break;
                }
                case 6:
                {
                    _1070 = true;
                    _1071 = _1626.xyz;
                    break;
                }
                case 7:
                {
                    _1070 = true;
                    _1071 = _1626.www;
                    break;
                }
                case 8:
                {
                    _1070 = true;
                    _1071 = _871.xyz;
                    break;
                }
                case 9:
                {
                    _1070 = true;
                    _1071 = _871.www;
                    break;
                }
                case 10:
                {
                    _1070 = true;
                    _1071 = _923.xyz;
                    break;
                }
                case 11:
                {
                    _1070 = true;
                    _1071 = _923.www;
                    break;
                }
                case 12:
                {
                    _1070 = true;
                    _1071 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _1070 = true;
                    _1071 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _1070 = true;
                    _1071 = float3(0.0);
                    break;
                }
                case 15:
                {
                    _1070 = true;
                    _1071 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1070)
            {
                break;
            }
            _1070 = true;
            _1071 = float3(0.0);
            break;
        } while(false);
        _1117 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorC)
            {
                case 0:
                {
                    _1117 = true;
                    _1118 = _1623.xyz;
                    break;
                }
                case 1:
                {
                    _1117 = true;
                    _1118 = _1623.www;
                    break;
                }
                case 2:
                {
                    _1117 = true;
                    _1118 = _1624.xyz;
                    break;
                }
                case 3:
                {
                    _1117 = true;
                    _1118 = _1624.www;
                    break;
                }
                case 4:
                {
                    _1117 = true;
                    _1118 = _1625.xyz;
                    break;
                }
                case 5:
                {
                    _1117 = true;
                    _1118 = _1625.www;
                    break;
                }
                case 6:
                {
                    _1117 = true;
                    _1118 = _1626.xyz;
                    break;
                }
                case 7:
                {
                    _1117 = true;
                    _1118 = _1626.www;
                    break;
                }
                case 8:
                {
                    _1117 = true;
                    _1118 = _871.xyz;
                    break;
                }
                case 9:
                {
                    _1117 = true;
                    _1118 = _871.www;
                    break;
                }
                case 10:
                {
                    _1117 = true;
                    _1118 = _923.xyz;
                    break;
                }
                case 11:
                {
                    _1117 = true;
                    _1118 = _923.www;
                    break;
                }
                case 12:
                {
                    _1117 = true;
                    _1118 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _1117 = true;
                    _1118 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _1117 = true;
                    _1118 = float3(0.0);
                    break;
                }
                case 15:
                {
                    _1117 = true;
                    _1118 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1117)
            {
                break;
            }
            _1117 = true;
            _1118 = float3(0.0);
            break;
        } while(false);
        _1164 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorD)
            {
                case 0:
                {
                    _1164 = true;
                    _1165 = _1623.xyz;
                    break;
                }
                case 1:
                {
                    _1164 = true;
                    _1165 = _1623.www;
                    break;
                }
                case 2:
                {
                    _1164 = true;
                    _1165 = _1624.xyz;
                    break;
                }
                case 3:
                {
                    _1164 = true;
                    _1165 = _1624.www;
                    break;
                }
                case 4:
                {
                    _1164 = true;
                    _1165 = _1625.xyz;
                    break;
                }
                case 5:
                {
                    _1164 = true;
                    _1165 = _1625.www;
                    break;
                }
                case 6:
                {
                    _1164 = true;
                    _1165 = _1626.xyz;
                    break;
                }
                case 7:
                {
                    _1164 = true;
                    _1165 = _1626.www;
                    break;
                }
                case 8:
                {
                    _1164 = true;
                    _1165 = _871.xyz;
                    break;
                }
                case 9:
                {
                    _1164 = true;
                    _1165 = _871.www;
                    break;
                }
                case 10:
                {
                    _1164 = true;
                    _1165 = _923.xyz;
                    break;
                }
                case 11:
                {
                    _1164 = true;
                    _1165 = _923.www;
                    break;
                }
                case 12:
                {
                    _1164 = true;
                    _1165 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _1164 = true;
                    _1165 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _1164 = true;
                    _1165 = float3(0.0);
                    break;
                }
                case 15:
                {
                    _1164 = true;
                    _1165 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1164)
            {
                break;
            }
            _1164 = true;
            _1165 = float3(0.0);
            break;
        } while(false);
        _1211 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaA)
            {
                case 0:
                {
                    _1211 = true;
                    _1212 = _1623.w;
                    break;
                }
                case 1:
                {
                    _1211 = true;
                    _1212 = _1624.w;
                    break;
                }
                case 2:
                {
                    _1211 = true;
                    _1212 = _1625.w;
                    break;
                }
                case 3:
                {
                    _1211 = true;
                    _1212 = _1626.w;
                    break;
                }
                case 4:
                {
                    _1211 = true;
                    _1212 = _871.w;
                    break;
                }
                case 5:
                {
                    _1211 = true;
                    _1212 = _923.w;
                    break;
                }
                case 6:
                {
                    _1211 = true;
                    _1212 = 1.0;
                    break;
                }
                case 7:
                {
                    _1211 = true;
                    _1212 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1211)
            {
                break;
            }
            _1211 = true;
            _1212 = 0.0;
            break;
        } while(false);
        _1240 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaB)
            {
                case 0:
                {
                    _1240 = true;
                    _1241 = _1623.w;
                    break;
                }
                case 1:
                {
                    _1240 = true;
                    _1241 = _1624.w;
                    break;
                }
                case 2:
                {
                    _1240 = true;
                    _1241 = _1625.w;
                    break;
                }
                case 3:
                {
                    _1240 = true;
                    _1241 = _1626.w;
                    break;
                }
                case 4:
                {
                    _1240 = true;
                    _1241 = _871.w;
                    break;
                }
                case 5:
                {
                    _1240 = true;
                    _1241 = _923.w;
                    break;
                }
                case 6:
                {
                    _1240 = true;
                    _1241 = 1.0;
                    break;
                }
                case 7:
                {
                    _1240 = true;
                    _1241 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1240)
            {
                break;
            }
            _1240 = true;
            _1241 = 0.0;
            break;
        } while(false);
        _1269 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaC)
            {
                case 0:
                {
                    _1269 = true;
                    _1270 = _1623.w;
                    break;
                }
                case 1:
                {
                    _1269 = true;
                    _1270 = _1624.w;
                    break;
                }
                case 2:
                {
                    _1269 = true;
                    _1270 = _1625.w;
                    break;
                }
                case 3:
                {
                    _1269 = true;
                    _1270 = _1626.w;
                    break;
                }
                case 4:
                {
                    _1269 = true;
                    _1270 = _871.w;
                    break;
                }
                case 5:
                {
                    _1269 = true;
                    _1270 = _923.w;
                    break;
                }
                case 6:
                {
                    _1269 = true;
                    _1270 = 1.0;
                    break;
                }
                case 7:
                {
                    _1269 = true;
                    _1270 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1269)
            {
                break;
            }
            _1269 = true;
            _1270 = 0.0;
            break;
        } while(false);
        _1298 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaD)
            {
                case 0:
                {
                    _1298 = true;
                    _1299 = _1623.w;
                    break;
                }
                case 1:
                {
                    _1298 = true;
                    _1299 = _1624.w;
                    break;
                }
                case 2:
                {
                    _1298 = true;
                    _1299 = _1625.w;
                    break;
                }
                case 3:
                {
                    _1298 = true;
                    _1299 = _1626.w;
                    break;
                }
                case 4:
                {
                    _1298 = true;
                    _1299 = _871.w;
                    break;
                }
                case 5:
                {
                    _1298 = true;
                    _1299 = _923.w;
                    break;
                }
                case 6:
                {
                    _1298 = true;
                    _1299 = 1.0;
                    break;
                }
                case 7:
                {
                    _1298 = true;
                    _1299 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1298)
            {
                break;
            }
            _1298 = true;
            _1299 = 0.0;
            break;
        } while(false);
        float3 _1334 = (_1024 * (float3(1.0) - _1118)) + (_1071 * _1118);
        if (uniforms.tevStages.data[i].colorOp == 0)
        {
            _1327 = _1165 + _1334;
        }
        else
        {
            _1327 = _1165 - _1334;
        }
        switch (uniforms.tevStages.data[i].colorBias)
        {
            case 1:
            {
                _1327 += float3(0.5);
                break;
            }
            case 2:
            {
                _1327 -= float3(0.5);
                break;
            }
            default:
            {
                break;
            }
        }
        switch (uniforms.tevStages.data[i].colorScale)
        {
            case 1:
            {
                _1327 *= 2.0;
                break;
            }
            case 2:
            {
                _1327 *= 4.0;
                break;
            }
            case 3:
            {
                _1327 *= 0.5;
                break;
            }
            default:
            {
                break;
            }
        }
        if (uniforms.tevStages.data[i].colorClamp != 0)
        {
            _1327 = fast::clamp(_1327, float3(0.0), float3(1.0));
        }
        float _1381 = (_1212 * (1.0 - _1270)) + (_1241 * _1270);
        if (uniforms.tevStages.data[i].alphaOp == 0)
        {
            _1375 = _1299 + _1381;
        }
        else
        {
            _1375 = _1299 - _1381;
        }
        switch (uniforms.tevStages.data[i].alphaBias)
        {
            case 1:
            {
                _1375 += 0.5;
                break;
            }
            case 2:
            {
                _1375 -= 0.5;
                break;
            }
            default:
            {
                break;
            }
        }
        switch (uniforms.tevStages.data[i].alphaScale)
        {
            case 1:
            {
                _1375 *= 2.0;
                break;
            }
            case 2:
            {
                _1375 *= 4.0;
                break;
            }
            case 3:
            {
                _1375 *= 0.5;
                break;
            }
            default:
            {
                break;
            }
        }
        if (uniforms.tevStages.data[i].alphaClamp != 0)
        {
            _1375 = fast::clamp(_1375, 0.0, 1.0);
        }
        switch (uniforms.tevStages.data[i].colorOutput)
        {
            case 0:
            {
                float4 _1685 = _1623;
                _1685.x = _1327.x;
                _1685.y = _1327.y;
                _1685.z = _1327.z;
                _1623 = _1685;
                break;
            }
            case 1:
            {
                float4 _1679 = _1624;
                _1679.x = _1327.x;
                _1679.y = _1327.y;
                _1679.z = _1327.z;
                _1624 = _1679;
                break;
            }
            case 2:
            {
                float4 _1673 = _1625;
                _1673.x = _1327.x;
                _1673.y = _1327.y;
                _1673.z = _1327.z;
                _1625 = _1673;
                break;
            }
            case 3:
            {
                float4 _1667 = _1626;
                _1667.x = _1327.x;
                _1667.y = _1327.y;
                _1667.z = _1327.z;
                _1626 = _1667;
                break;
            }
            default:
            {
                break;
            }
        }
        switch (uniforms.tevStages.data[i].alphaOutput)
        {
            case 0:
            {
                float4 _1697 = _1623;
                _1697.w = _1375;
                _1623 = _1697;
                break;
            }
            case 1:
            {
                float4 _1695 = _1624;
                _1695.w = _1375;
                _1624 = _1695;
                break;
            }
            case 2:
            {
                float4 _1693 = _1625;
                _1693.w = _1375;
                _1625 = _1693;
                break;
            }
            case 3:
            {
                float4 _1691 = _1626;
                _1691.w = _1375;
                _1626 = _1691;
                break;
            }
            default:
            {
                break;
            }
        }
        i++;
        continue;
    }
    _1467 = false;
    bool _1468;
    do
    {
        _1505 = false;
        bool _1506;
        do
        {
            switch (uniforms.alphaComp0)
            {
                case 0:
                {
                    _1505 = true;
                    _1506 = false;
                    break;
                }
                case 1:
                {
                    _1505 = true;
                    _1506 = _1623.w < uniforms.alphaRef0;
                    break;
                }
                case 2:
                {
                    _1505 = true;
                    _1506 = _1623.w == uniforms.alphaRef0;
                    break;
                }
                case 3:
                {
                    _1505 = true;
                    _1506 = _1623.w <= uniforms.alphaRef0;
                    break;
                }
                case 4:
                {
                    _1505 = true;
                    _1506 = _1623.w > uniforms.alphaRef0;
                    break;
                }
                case 5:
                {
                    _1505 = true;
                    _1506 = _1623.w != uniforms.alphaRef0;
                    break;
                }
                case 6:
                {
                    _1505 = true;
                    _1506 = _1623.w >= uniforms.alphaRef0;
                    break;
                }
                case 7:
                {
                    _1505 = true;
                    _1506 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1505)
            {
                break;
            }
            _1505 = true;
            _1506 = true;
            break;
        } while(false);
        _1530 = false;
        bool _1531;
        do
        {
            switch (uniforms.alphaComp1)
            {
                case 0:
                {
                    _1530 = true;
                    _1531 = false;
                    break;
                }
                case 1:
                {
                    _1530 = true;
                    _1531 = _1623.w < uniforms.alphaRef1;
                    break;
                }
                case 2:
                {
                    _1530 = true;
                    _1531 = _1623.w == uniforms.alphaRef1;
                    break;
                }
                case 3:
                {
                    _1530 = true;
                    _1531 = _1623.w <= uniforms.alphaRef1;
                    break;
                }
                case 4:
                {
                    _1530 = true;
                    _1531 = _1623.w > uniforms.alphaRef1;
                    break;
                }
                case 5:
                {
                    _1530 = true;
                    _1531 = _1623.w != uniforms.alphaRef1;
                    break;
                }
                case 6:
                {
                    _1530 = true;
                    _1531 = _1623.w >= uniforms.alphaRef1;
                    break;
                }
                case 7:
                {
                    _1530 = true;
                    _1531 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1530)
            {
                break;
            }
            _1530 = true;
            _1531 = true;
            break;
        } while(false);
        bool _1469;
        switch (uniforms.alphaLogic)
        {
            case 0:
            {
                if (_1506)
                {
                    _1469 = _1531;
                }
                else
                {
                    _1469 = false;
                }
                _1467 = true;
                _1468 = _1469;
                break;
            }
            case 1:
            {
                if (_1506)
                {
                    _1469 = true;
                }
                else
                {
                    _1469 = _1531;
                }
                _1467 = true;
                _1468 = _1469;
                break;
            }
            case 2:
            {
                _1467 = true;
                _1468 = _1506 != _1531;
                break;
            }
            case 3:
            {
                _1467 = true;
                _1468 = _1506 == _1531;
                break;
            }
            default:
            {
                break;
            }
        }
        if (_1467)
        {
            break;
        }
        _1467 = true;
        _1468 = true;
        break;
    } while(false);
    if (!_1468)
    {
        discard_fragment();
    }
    out.entryPointParam_fragmentMain = _1623;
    return out;
}

)SHDR3";

#endif
