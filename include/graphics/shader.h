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
    float4 entryPointParam_vertexMain_color [[user(locn0)]];
    float2 entryPointParam_vertexMain_uv0 [[user(locn1)]];
    float2 entryPointParam_vertexMain_uv1 [[user(locn2)]];
    float2 entryPointParam_vertexMain_uv2 [[user(locn3)]];
    float2 entryPointParam_vertexMain_uv3 [[user(locn4)]];
    float2 entryPointParam_vertexMain_uv4 [[user(locn5)]];
    float2 entryPointParam_vertexMain_uv5 [[user(locn6)]];
    float2 entryPointParam_vertexMain_uv6 [[user(locn7)]];
    float2 entryPointParam_vertexMain_uv7 [[user(locn8)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 input_position [[attribute(0)]];
    float4 input_color [[attribute(1)]];
    float2 input_uv0 [[attribute(2)]];
    float2 input_uv1 [[attribute(3)]];
    float2 input_uv2 [[attribute(4)]];
    float2 input_uv3 [[attribute(5)]];
    float2 input_uv4 [[attribute(6)]];
    float2 input_uv5 [[attribute(7)]];
    float2 input_uv6 [[attribute(8)]];
    float2 input_uv7 [[attribute(9)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = in.input_position;
    out.entryPointParam_vertexMain_color = in.input_color;
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
    float4 input_color [[user(locn0)]];
    float2 input_uv0 [[user(locn1)]];
    float2 input_uv1 [[user(locn2)]];
    float2 input_uv2 [[user(locn3)]];
    float2 input_uv3 [[user(locn4)]];
    float2 input_uv4 [[user(locn5)]];
    float2 input_uv5 [[user(locn6)]];
    float2 input_uv6 [[user(locn7)]];
    float2 input_uv7 [[user(locn8)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant Uniforms_std140& uniforms [[buffer(0)]], texture2d<float> tex0 [[texture(0)]], texture2d<float> tex1 [[texture(1)]], texture2d<float> tex2 [[texture(2)]], texture2d<float> tex3 [[texture(3)]], texture2d<float> tex4 [[texture(4)]], texture2d<float> tex5 [[texture(5)]], texture2d<float> tex6 [[texture(6)]], texture2d<float> tex7 [[texture(7)]], sampler samp0 [[sampler(0)]], sampler samp1 [[sampler(1)]], sampler samp2 [[sampler(2)]], sampler samp3 [[sampler(3)]], sampler samp4 [[sampler(4)]], sampler samp5 [[sampler(5)]], sampler samp6 [[sampler(6)]], sampler samp7 [[sampler(7)]])
{
    bool _1487 = false;
    bool _1462 = false;
    bool _1424 = false;
    bool _1255 = false;
    bool _1226 = false;
    bool _1197 = false;
    bool _1168 = false;
    bool _1121 = false;
    bool _1074 = false;
    bool _1027 = false;
    bool _980 = false;
    bool _921 = false;
    bool _894 = false;
    main0_out out = {};
    float4 _1577 = float4(1.0);
    float4 _1578 = float4(0.0);
    float4 _1579 = float4(0.0);
    float4 _1580 = float4(0.0);
    int i = 0;
    float4 _844;
    float2 _895;
    float4 _922;
    float3 _981;
    float3 _1028;
    float3 _1075;
    float3 _1122;
    float _1169;
    float _1198;
    float _1227;
    float _1256;
    float3 _1284;
    float _1332;
    for (;;)
    {
        if (!(i < uniforms.tevStageCount))
        {
            break;
        }
        if (uniforms.tevStages.data[i].textureEnabled != 0)
        {
            _894 = false;
            do
            {
                switch (uniforms.tevStages.data[i].texCoord)
                {
                    case 0:
                    {
                        _894 = true;
                        _895 = in.input_uv0;
                        break;
                    }
                    case 1:
                    {
                        _894 = true;
                        _895 = in.input_uv1;
                        break;
                    }
                    case 2:
                    {
                        _894 = true;
                        _895 = in.input_uv2;
                        break;
                    }
                    case 3:
                    {
                        _894 = true;
                        _895 = in.input_uv3;
                        break;
                    }
                    case 4:
                    {
                        _894 = true;
                        _895 = in.input_uv4;
                        break;
                    }
                    case 5:
                    {
                        _894 = true;
                        _895 = in.input_uv5;
                        break;
                    }
                    case 6:
                    {
                        _894 = true;
                        _895 = in.input_uv6;
                        break;
                    }
                    case 7:
                    {
                        _894 = true;
                        _895 = in.input_uv7;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_894)
                {
                    break;
                }
                _894 = true;
                _895 = float2(0.0);
                break;
            } while(false);
            _921 = false;
            do
            {
                switch (uniforms.tevStages.data[i].texMap)
                {
                    case 0:
                    {
                        _921 = true;
                        _922 = tex0.sample(samp0, _895);
                        break;
                    }
                    case 1:
                    {
                        _921 = true;
                        _922 = tex1.sample(samp1, _895);
                        break;
                    }
                    case 2:
                    {
                        _921 = true;
                        _922 = tex2.sample(samp2, _895);
                        break;
                    }
                    case 3:
                    {
                        _921 = true;
                        _922 = tex3.sample(samp3, _895);
                        break;
                    }
                    case 4:
                    {
                        _921 = true;
                        _922 = tex4.sample(samp4, _895);
                        break;
                    }
                    case 5:
                    {
                        _921 = true;
                        _922 = tex5.sample(samp5, _895);
                        break;
                    }
                    case 6:
                    {
                        _921 = true;
                        _922 = tex6.sample(samp6, _895);
                        break;
                    }
                    case 7:
                    {
                        _921 = true;
                        _922 = tex7.sample(samp7, _895);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_921)
                {
                    break;
                }
                _921 = true;
                _922 = float4(1.0);
                break;
            } while(false);
            _844 = _922;
        }
        else
        {
            _844 = float4(1.0);
        }
        _980 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorA)
            {
                case 0:
                {
                    _980 = true;
                    _981 = _1577.xyz;
                    break;
                }
                case 1:
                {
                    _980 = true;
                    _981 = _1577.www;
                    break;
                }
                case 2:
                {
                    _980 = true;
                    _981 = _1578.xyz;
                    break;
                }
                case 3:
                {
                    _980 = true;
                    _981 = _1578.www;
                    break;
                }
                case 4:
                {
                    _980 = true;
                    _981 = _1579.xyz;
                    break;
                }
                case 5:
                {
                    _980 = true;
                    _981 = _1579.www;
                    break;
                }
                case 6:
                {
                    _980 = true;
                    _981 = _1580.xyz;
                    break;
                }
                case 7:
                {
                    _980 = true;
                    _981 = _1580.www;
                    break;
                }
                case 8:
                {
                    _980 = true;
                    _981 = _844.xyz;
                    break;
                }
                case 9:
                {
                    _980 = true;
                    _981 = _844.www;
                    break;
                }
                case 10:
                {
                    _980 = true;
                    _981 = in.input_color.xyz;
                    break;
                }
                case 11:
                {
                    _980 = true;
                    _981 = in.input_color.www;
                    break;
                }
                case 12:
                {
                    _980 = true;
                    _981 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _980 = true;
                    _981 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _980 = true;
                    _981 = float3(0.0);
                    break;
                }
                case 15:
                {
                    _980 = true;
                    _981 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_980)
            {
                break;
            }
            _980 = true;
            _981 = float3(0.0);
            break;
        } while(false);
        _1027 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorB)
            {
                case 0:
                {
                    _1027 = true;
                    _1028 = _1577.xyz;
                    break;
                }
                case 1:
                {
                    _1027 = true;
                    _1028 = _1577.www;
                    break;
                }
                case 2:
                {
                    _1027 = true;
                    _1028 = _1578.xyz;
                    break;
                }
                case 3:
                {
                    _1027 = true;
                    _1028 = _1578.www;
                    break;
                }
                case 4:
                {
                    _1027 = true;
                    _1028 = _1579.xyz;
                    break;
                }
                case 5:
                {
                    _1027 = true;
                    _1028 = _1579.www;
                    break;
                }
                case 6:
                {
                    _1027 = true;
                    _1028 = _1580.xyz;
                    break;
                }
                case 7:
                {
                    _1027 = true;
                    _1028 = _1580.www;
                    break;
                }
                case 8:
                {
                    _1027 = true;
                    _1028 = _844.xyz;
                    break;
                }
                case 9:
                {
                    _1027 = true;
                    _1028 = _844.www;
                    break;
                }
                case 10:
                {
                    _1027 = true;
                    _1028 = in.input_color.xyz;
                    break;
                }
                case 11:
                {
                    _1027 = true;
                    _1028 = in.input_color.www;
                    break;
                }
                case 12:
                {
                    _1027 = true;
                    _1028 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _1027 = true;
                    _1028 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _1027 = true;
                    _1028 = float3(0.0);
                    break;
                }
                case 15:
                {
                    _1027 = true;
                    _1028 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1027)
            {
                break;
            }
            _1027 = true;
            _1028 = float3(0.0);
            break;
        } while(false);
        _1074 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorC)
            {
                case 0:
                {
                    _1074 = true;
                    _1075 = _1577.xyz;
                    break;
                }
                case 1:
                {
                    _1074 = true;
                    _1075 = _1577.www;
                    break;
                }
                case 2:
                {
                    _1074 = true;
                    _1075 = _1578.xyz;
                    break;
                }
                case 3:
                {
                    _1074 = true;
                    _1075 = _1578.www;
                    break;
                }
                case 4:
                {
                    _1074 = true;
                    _1075 = _1579.xyz;
                    break;
                }
                case 5:
                {
                    _1074 = true;
                    _1075 = _1579.www;
                    break;
                }
                case 6:
                {
                    _1074 = true;
                    _1075 = _1580.xyz;
                    break;
                }
                case 7:
                {
                    _1074 = true;
                    _1075 = _1580.www;
                    break;
                }
                case 8:
                {
                    _1074 = true;
                    _1075 = _844.xyz;
                    break;
                }
                case 9:
                {
                    _1074 = true;
                    _1075 = _844.www;
                    break;
                }
                case 10:
                {
                    _1074 = true;
                    _1075 = in.input_color.xyz;
                    break;
                }
                case 11:
                {
                    _1074 = true;
                    _1075 = in.input_color.www;
                    break;
                }
                case 12:
                {
                    _1074 = true;
                    _1075 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _1074 = true;
                    _1075 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _1074 = true;
                    _1075 = float3(0.0);
                    break;
                }
                case 15:
                {
                    _1074 = true;
                    _1075 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1074)
            {
                break;
            }
            _1074 = true;
            _1075 = float3(0.0);
            break;
        } while(false);
        _1121 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorD)
            {
                case 0:
                {
                    _1121 = true;
                    _1122 = _1577.xyz;
                    break;
                }
                case 1:
                {
                    _1121 = true;
                    _1122 = _1577.www;
                    break;
                }
                case 2:
                {
                    _1121 = true;
                    _1122 = _1578.xyz;
                    break;
                }
                case 3:
                {
                    _1121 = true;
                    _1122 = _1578.www;
                    break;
                }
                case 4:
                {
                    _1121 = true;
                    _1122 = _1579.xyz;
                    break;
                }
                case 5:
                {
                    _1121 = true;
                    _1122 = _1579.www;
                    break;
                }
                case 6:
                {
                    _1121 = true;
                    _1122 = _1580.xyz;
                    break;
                }
                case 7:
                {
                    _1121 = true;
                    _1122 = _1580.www;
                    break;
                }
                case 8:
                {
                    _1121 = true;
                    _1122 = _844.xyz;
                    break;
                }
                case 9:
                {
                    _1121 = true;
                    _1122 = _844.www;
                    break;
                }
                case 10:
                {
                    _1121 = true;
                    _1122 = in.input_color.xyz;
                    break;
                }
                case 11:
                {
                    _1121 = true;
                    _1122 = in.input_color.www;
                    break;
                }
                case 12:
                {
                    _1121 = true;
                    _1122 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _1121 = true;
                    _1122 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _1121 = true;
                    _1122 = float3(0.0);
                    break;
                }
                case 15:
                {
                    _1121 = true;
                    _1122 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1121)
            {
                break;
            }
            _1121 = true;
            _1122 = float3(0.0);
            break;
        } while(false);
        _1168 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaA)
            {
                case 0:
                {
                    _1168 = true;
                    _1169 = _1577.w;
                    break;
                }
                case 1:
                {
                    _1168 = true;
                    _1169 = _1578.w;
                    break;
                }
                case 2:
                {
                    _1168 = true;
                    _1169 = _1579.w;
                    break;
                }
                case 3:
                {
                    _1168 = true;
                    _1169 = _1580.w;
                    break;
                }
                case 4:
                {
                    _1168 = true;
                    _1169 = _844.w;
                    break;
                }
                case 5:
                {
                    _1168 = true;
                    _1169 = in.input_color.w;
                    break;
                }
                case 6:
                {
                    _1168 = true;
                    _1169 = 1.0;
                    break;
                }
                case 7:
                {
                    _1168 = true;
                    _1169 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1168)
            {
                break;
            }
            _1168 = true;
            _1169 = 0.0;
            break;
        } while(false);
        _1197 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaB)
            {
                case 0:
                {
                    _1197 = true;
                    _1198 = _1577.w;
                    break;
                }
                case 1:
                {
                    _1197 = true;
                    _1198 = _1578.w;
                    break;
                }
                case 2:
                {
                    _1197 = true;
                    _1198 = _1579.w;
                    break;
                }
                case 3:
                {
                    _1197 = true;
                    _1198 = _1580.w;
                    break;
                }
                case 4:
                {
                    _1197 = true;
                    _1198 = _844.w;
                    break;
                }
                case 5:
                {
                    _1197 = true;
                    _1198 = in.input_color.w;
                    break;
                }
                case 6:
                {
                    _1197 = true;
                    _1198 = 1.0;
                    break;
                }
                case 7:
                {
                    _1197 = true;
                    _1198 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1197)
            {
                break;
            }
            _1197 = true;
            _1198 = 0.0;
            break;
        } while(false);
        _1226 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaC)
            {
                case 0:
                {
                    _1226 = true;
                    _1227 = _1577.w;
                    break;
                }
                case 1:
                {
                    _1226 = true;
                    _1227 = _1578.w;
                    break;
                }
                case 2:
                {
                    _1226 = true;
                    _1227 = _1579.w;
                    break;
                }
                case 3:
                {
                    _1226 = true;
                    _1227 = _1580.w;
                    break;
                }
                case 4:
                {
                    _1226 = true;
                    _1227 = _844.w;
                    break;
                }
                case 5:
                {
                    _1226 = true;
                    _1227 = in.input_color.w;
                    break;
                }
                case 6:
                {
                    _1226 = true;
                    _1227 = 1.0;
                    break;
                }
                case 7:
                {
                    _1226 = true;
                    _1227 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1226)
            {
                break;
            }
            _1226 = true;
            _1227 = 0.0;
            break;
        } while(false);
        _1255 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaD)
            {
                case 0:
                {
                    _1255 = true;
                    _1256 = _1577.w;
                    break;
                }
                case 1:
                {
                    _1255 = true;
                    _1256 = _1578.w;
                    break;
                }
                case 2:
                {
                    _1255 = true;
                    _1256 = _1579.w;
                    break;
                }
                case 3:
                {
                    _1255 = true;
                    _1256 = _1580.w;
                    break;
                }
                case 4:
                {
                    _1255 = true;
                    _1256 = _844.w;
                    break;
                }
                case 5:
                {
                    _1255 = true;
                    _1256 = in.input_color.w;
                    break;
                }
                case 6:
                {
                    _1255 = true;
                    _1256 = 1.0;
                    break;
                }
                case 7:
                {
                    _1255 = true;
                    _1256 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1255)
            {
                break;
            }
            _1255 = true;
            _1256 = 0.0;
            break;
        } while(false);
        float3 _1291 = (_981 * (float3(1.0) - _1075)) + (_1028 * _1075);
        if (uniforms.tevStages.data[i].colorOp == 0)
        {
            _1284 = _1122 + _1291;
        }
        else
        {
            _1284 = _1122 - _1291;
        }
        switch (uniforms.tevStages.data[i].colorBias)
        {
            case 1:
            {
                _1284 += float3(0.5);
                break;
            }
            case 2:
            {
                _1284 -= float3(0.5);
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
                _1284 *= 2.0;
                break;
            }
            case 2:
            {
                _1284 *= 4.0;
                break;
            }
            case 3:
            {
                _1284 *= 0.5;
                break;
            }
            default:
            {
                break;
            }
        }
        if (uniforms.tevStages.data[i].colorClamp != 0)
        {
            _1284 = fast::clamp(_1284, float3(0.0), float3(1.0));
        }
        float _1338 = (_1169 * (1.0 - _1227)) + (_1198 * _1227);
        if (uniforms.tevStages.data[i].alphaOp == 0)
        {
            _1332 = _1256 + _1338;
        }
        else
        {
            _1332 = _1256 - _1338;
        }
        switch (uniforms.tevStages.data[i].alphaBias)
        {
            case 1:
            {
                _1332 += 0.5;
                break;
            }
            case 2:
            {
                _1332 -= 0.5;
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
                _1332 *= 2.0;
                break;
            }
            case 2:
            {
                _1332 *= 4.0;
                break;
            }
            case 3:
            {
                _1332 *= 0.5;
                break;
            }
            default:
            {
                break;
            }
        }
        if (uniforms.tevStages.data[i].alphaClamp != 0)
        {
            _1332 = fast::clamp(_1332, 0.0, 1.0);
        }
        switch (uniforms.tevStages.data[i].colorOutput)
        {
            case 0:
            {
                float4 _1639 = _1577;
                _1639.x = _1284.x;
                _1639.y = _1284.y;
                _1639.z = _1284.z;
                _1577 = _1639;
                break;
            }
            case 1:
            {
                float4 _1633 = _1578;
                _1633.x = _1284.x;
                _1633.y = _1284.y;
                _1633.z = _1284.z;
                _1578 = _1633;
                break;
            }
            case 2:
            {
                float4 _1627 = _1579;
                _1627.x = _1284.x;
                _1627.y = _1284.y;
                _1627.z = _1284.z;
                _1579 = _1627;
                break;
            }
            case 3:
            {
                float4 _1621 = _1580;
                _1621.x = _1284.x;
                _1621.y = _1284.y;
                _1621.z = _1284.z;
                _1580 = _1621;
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
                float4 _1651 = _1577;
                _1651.w = _1332;
                _1577 = _1651;
                break;
            }
            case 1:
            {
                float4 _1649 = _1578;
                _1649.w = _1332;
                _1578 = _1649;
                break;
            }
            case 2:
            {
                float4 _1647 = _1579;
                _1647.w = _1332;
                _1579 = _1647;
                break;
            }
            case 3:
            {
                float4 _1645 = _1580;
                _1645.w = _1332;
                _1580 = _1645;
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
    _1424 = false;
    bool _1425;
    do
    {
        _1462 = false;
        bool _1463;
        do
        {
            switch (uniforms.alphaComp0)
            {
                case 0:
                {
                    _1462 = true;
                    _1463 = false;
                    break;
                }
                case 1:
                {
                    _1462 = true;
                    _1463 = _1577.w < uniforms.alphaRef0;
                    break;
                }
                case 2:
                {
                    _1462 = true;
                    _1463 = _1577.w == uniforms.alphaRef0;
                    break;
                }
                case 3:
                {
                    _1462 = true;
                    _1463 = _1577.w <= uniforms.alphaRef0;
                    break;
                }
                case 4:
                {
                    _1462 = true;
                    _1463 = _1577.w > uniforms.alphaRef0;
                    break;
                }
                case 5:
                {
                    _1462 = true;
                    _1463 = _1577.w != uniforms.alphaRef0;
                    break;
                }
                case 6:
                {
                    _1462 = true;
                    _1463 = _1577.w >= uniforms.alphaRef0;
                    break;
                }
                case 7:
                {
                    _1462 = true;
                    _1463 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1462)
            {
                break;
            }
            _1462 = true;
            _1463 = true;
            break;
        } while(false);
        _1487 = false;
        bool _1488;
        do
        {
            switch (uniforms.alphaComp1)
            {
                case 0:
                {
                    _1487 = true;
                    _1488 = false;
                    break;
                }
                case 1:
                {
                    _1487 = true;
                    _1488 = _1577.w < uniforms.alphaRef1;
                    break;
                }
                case 2:
                {
                    _1487 = true;
                    _1488 = _1577.w == uniforms.alphaRef1;
                    break;
                }
                case 3:
                {
                    _1487 = true;
                    _1488 = _1577.w <= uniforms.alphaRef1;
                    break;
                }
                case 4:
                {
                    _1487 = true;
                    _1488 = _1577.w > uniforms.alphaRef1;
                    break;
                }
                case 5:
                {
                    _1487 = true;
                    _1488 = _1577.w != uniforms.alphaRef1;
                    break;
                }
                case 6:
                {
                    _1487 = true;
                    _1488 = _1577.w >= uniforms.alphaRef1;
                    break;
                }
                case 7:
                {
                    _1487 = true;
                    _1488 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1487)
            {
                break;
            }
            _1487 = true;
            _1488 = true;
            break;
        } while(false);
        bool _1426;
        switch (uniforms.alphaLogic)
        {
            case 0:
            {
                if (_1463)
                {
                    _1426 = _1488;
                }
                else
                {
                    _1426 = false;
                }
                _1424 = true;
                _1425 = _1426;
                break;
            }
            case 1:
            {
                if (_1463)
                {
                    _1426 = true;
                }
                else
                {
                    _1426 = _1488;
                }
                _1424 = true;
                _1425 = _1426;
                break;
            }
            case 2:
            {
                _1424 = true;
                _1425 = _1463 != _1488;
                break;
            }
            case 3:
            {
                _1424 = true;
                _1425 = _1463 == _1488;
                break;
            }
            default:
            {
                break;
            }
        }
        if (_1424)
        {
            break;
        }
        _1424 = true;
        _1425 = true;
        break;
    } while(false);
    if (!_1425)
    {
        discard_fragment();
    }
    out.entryPointParam_fragmentMain = _1577;
    return out;
}

)SHDR3";

#endif
