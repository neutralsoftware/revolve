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
    float2 entryPointParam_vertexMain_uv [[user(locn1)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 input_position [[attribute(0)]];
    float4 input_color [[attribute(1)]];
    float2 input_uv [[attribute(2)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = in.input_position;
    out.entryPointParam_vertexMain_color = in.input_color;
    out.entryPointParam_vertexMain_uv = in.input_uv;
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
    float2 input_uv [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant Uniforms_std140& uniforms [[buffer(0)]], texture2d<float> tex0 [[texture(0)]], texture2d<float> tex1 [[texture(1)]], texture2d<float> tex2 [[texture(2)]], texture2d<float> tex3 [[texture(3)]], texture2d<float> tex4 [[texture(4)]], texture2d<float> tex5 [[texture(5)]], texture2d<float> tex6 [[texture(6)]], texture2d<float> tex7 [[texture(7)]], sampler samp0 [[sampler(0)]], sampler samp1 [[sampler(1)]], sampler samp2 [[sampler(2)]], sampler samp3 [[sampler(3)]], sampler samp4 [[sampler(4)]], sampler samp5 [[sampler(5)]], sampler samp6 [[sampler(6)]], sampler samp7 [[sampler(7)]])
{
    bool _1401 = false;
    bool _1376 = false;
    bool _1338 = false;
    bool _1169 = false;
    bool _1140 = false;
    bool _1111 = false;
    bool _1082 = false;
    bool _1035 = false;
    bool _988 = false;
    bool _941 = false;
    bool _894 = false;
    bool _835 = false;
    main0_out out = {};
    float4 _1488 = float4(1.0);
    float4 _1489 = float4(0.0);
    float4 _1490 = float4(0.0);
    float4 _1491 = float4(0.0);
    int i = 0;
    float4 _786;
    float4 _836;
    float3 _895;
    float3 _942;
    float3 _989;
    float3 _1036;
    float _1083;
    float _1112;
    float _1141;
    float _1170;
    float3 _1198;
    float _1246;
    for (;;)
    {
        if (!(i < uniforms.tevStageCount))
        {
            break;
        }
        if (uniforms.tevStages.data[i].textureEnabled != 0)
        {
            _835 = false;
            do
            {
                switch (uniforms.tevStages.data[i].texMap)
                {
                    case 0:
                    {
                        _835 = true;
                        _836 = tex0.sample(samp0, in.input_uv);
                        break;
                    }
                    case 1:
                    {
                        _835 = true;
                        _836 = tex1.sample(samp1, in.input_uv);
                        break;
                    }
                    case 2:
                    {
                        _835 = true;
                        _836 = tex2.sample(samp2, in.input_uv);
                        break;
                    }
                    case 3:
                    {
                        _835 = true;
                        _836 = tex3.sample(samp3, in.input_uv);
                        break;
                    }
                    case 4:
                    {
                        _835 = true;
                        _836 = tex4.sample(samp4, in.input_uv);
                        break;
                    }
                    case 5:
                    {
                        _835 = true;
                        _836 = tex5.sample(samp5, in.input_uv);
                        break;
                    }
                    case 6:
                    {
                        _835 = true;
                        _836 = tex6.sample(samp6, in.input_uv);
                        break;
                    }
                    case 7:
                    {
                        _835 = true;
                        _836 = tex7.sample(samp7, in.input_uv);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_835)
                {
                    break;
                }
                _835 = true;
                _836 = float4(1.0);
                break;
            } while(false);
            _786 = _836;
        }
        else
        {
            _786 = float4(1.0);
        }
        _894 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorA)
            {
                case 0:
                {
                    _894 = true;
                    _895 = _1488.xyz;
                    break;
                }
                case 1:
                {
                    _894 = true;
                    _895 = _1488.www;
                    break;
                }
                case 2:
                {
                    _894 = true;
                    _895 = _1489.xyz;
                    break;
                }
                case 3:
                {
                    _894 = true;
                    _895 = _1489.www;
                    break;
                }
                case 4:
                {
                    _894 = true;
                    _895 = _1490.xyz;
                    break;
                }
                case 5:
                {
                    _894 = true;
                    _895 = _1490.www;
                    break;
                }
                case 6:
                {
                    _894 = true;
                    _895 = _1491.xyz;
                    break;
                }
                case 7:
                {
                    _894 = true;
                    _895 = _1491.www;
                    break;
                }
                case 8:
                {
                    _894 = true;
                    _895 = _786.xyz;
                    break;
                }
                case 9:
                {
                    _894 = true;
                    _895 = _786.www;
                    break;
                }
                case 10:
                {
                    _894 = true;
                    _895 = in.input_color.xyz;
                    break;
                }
                case 11:
                {
                    _894 = true;
                    _895 = in.input_color.www;
                    break;
                }
                case 12:
                {
                    _894 = true;
                    _895 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _894 = true;
                    _895 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _894 = true;
                    _895 = float3(0.0);
                    break;
                }
                case 15:
                {
                    _894 = true;
                    _895 = float3(0.0);
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
            _895 = float3(0.0);
            break;
        } while(false);
        _941 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorB)
            {
                case 0:
                {
                    _941 = true;
                    _942 = _1488.xyz;
                    break;
                }
                case 1:
                {
                    _941 = true;
                    _942 = _1488.www;
                    break;
                }
                case 2:
                {
                    _941 = true;
                    _942 = _1489.xyz;
                    break;
                }
                case 3:
                {
                    _941 = true;
                    _942 = _1489.www;
                    break;
                }
                case 4:
                {
                    _941 = true;
                    _942 = _1490.xyz;
                    break;
                }
                case 5:
                {
                    _941 = true;
                    _942 = _1490.www;
                    break;
                }
                case 6:
                {
                    _941 = true;
                    _942 = _1491.xyz;
                    break;
                }
                case 7:
                {
                    _941 = true;
                    _942 = _1491.www;
                    break;
                }
                case 8:
                {
                    _941 = true;
                    _942 = _786.xyz;
                    break;
                }
                case 9:
                {
                    _941 = true;
                    _942 = _786.www;
                    break;
                }
                case 10:
                {
                    _941 = true;
                    _942 = in.input_color.xyz;
                    break;
                }
                case 11:
                {
                    _941 = true;
                    _942 = in.input_color.www;
                    break;
                }
                case 12:
                {
                    _941 = true;
                    _942 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _941 = true;
                    _942 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _941 = true;
                    _942 = float3(0.0);
                    break;
                }
                case 15:
                {
                    _941 = true;
                    _942 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_941)
            {
                break;
            }
            _941 = true;
            _942 = float3(0.0);
            break;
        } while(false);
        _988 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorC)
            {
                case 0:
                {
                    _988 = true;
                    _989 = _1488.xyz;
                    break;
                }
                case 1:
                {
                    _988 = true;
                    _989 = _1488.www;
                    break;
                }
                case 2:
                {
                    _988 = true;
                    _989 = _1489.xyz;
                    break;
                }
                case 3:
                {
                    _988 = true;
                    _989 = _1489.www;
                    break;
                }
                case 4:
                {
                    _988 = true;
                    _989 = _1490.xyz;
                    break;
                }
                case 5:
                {
                    _988 = true;
                    _989 = _1490.www;
                    break;
                }
                case 6:
                {
                    _988 = true;
                    _989 = _1491.xyz;
                    break;
                }
                case 7:
                {
                    _988 = true;
                    _989 = _1491.www;
                    break;
                }
                case 8:
                {
                    _988 = true;
                    _989 = _786.xyz;
                    break;
                }
                case 9:
                {
                    _988 = true;
                    _989 = _786.www;
                    break;
                }
                case 10:
                {
                    _988 = true;
                    _989 = in.input_color.xyz;
                    break;
                }
                case 11:
                {
                    _988 = true;
                    _989 = in.input_color.www;
                    break;
                }
                case 12:
                {
                    _988 = true;
                    _989 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _988 = true;
                    _989 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _988 = true;
                    _989 = float3(0.0);
                    break;
                }
                case 15:
                {
                    _988 = true;
                    _989 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_988)
            {
                break;
            }
            _988 = true;
            _989 = float3(0.0);
            break;
        } while(false);
        _1035 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorD)
            {
                case 0:
                {
                    _1035 = true;
                    _1036 = _1488.xyz;
                    break;
                }
                case 1:
                {
                    _1035 = true;
                    _1036 = _1488.www;
                    break;
                }
                case 2:
                {
                    _1035 = true;
                    _1036 = _1489.xyz;
                    break;
                }
                case 3:
                {
                    _1035 = true;
                    _1036 = _1489.www;
                    break;
                }
                case 4:
                {
                    _1035 = true;
                    _1036 = _1490.xyz;
                    break;
                }
                case 5:
                {
                    _1035 = true;
                    _1036 = _1490.www;
                    break;
                }
                case 6:
                {
                    _1035 = true;
                    _1036 = _1491.xyz;
                    break;
                }
                case 7:
                {
                    _1035 = true;
                    _1036 = _1491.www;
                    break;
                }
                case 8:
                {
                    _1035 = true;
                    _1036 = _786.xyz;
                    break;
                }
                case 9:
                {
                    _1035 = true;
                    _1036 = _786.www;
                    break;
                }
                case 10:
                {
                    _1035 = true;
                    _1036 = in.input_color.xyz;
                    break;
                }
                case 11:
                {
                    _1035 = true;
                    _1036 = in.input_color.www;
                    break;
                }
                case 12:
                {
                    _1035 = true;
                    _1036 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _1035 = true;
                    _1036 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _1035 = true;
                    _1036 = float3(0.0);
                    break;
                }
                case 15:
                {
                    _1035 = true;
                    _1036 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1035)
            {
                break;
            }
            _1035 = true;
            _1036 = float3(0.0);
            break;
        } while(false);
        _1082 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaA)
            {
                case 0:
                {
                    _1082 = true;
                    _1083 = _1488.w;
                    break;
                }
                case 1:
                {
                    _1082 = true;
                    _1083 = _1489.w;
                    break;
                }
                case 2:
                {
                    _1082 = true;
                    _1083 = _1490.w;
                    break;
                }
                case 3:
                {
                    _1082 = true;
                    _1083 = _1491.w;
                    break;
                }
                case 4:
                {
                    _1082 = true;
                    _1083 = _786.w;
                    break;
                }
                case 5:
                {
                    _1082 = true;
                    _1083 = in.input_color.w;
                    break;
                }
                case 6:
                {
                    _1082 = true;
                    _1083 = 1.0;
                    break;
                }
                case 7:
                {
                    _1082 = true;
                    _1083 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1082)
            {
                break;
            }
            _1082 = true;
            _1083 = 0.0;
            break;
        } while(false);
        _1111 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaB)
            {
                case 0:
                {
                    _1111 = true;
                    _1112 = _1488.w;
                    break;
                }
                case 1:
                {
                    _1111 = true;
                    _1112 = _1489.w;
                    break;
                }
                case 2:
                {
                    _1111 = true;
                    _1112 = _1490.w;
                    break;
                }
                case 3:
                {
                    _1111 = true;
                    _1112 = _1491.w;
                    break;
                }
                case 4:
                {
                    _1111 = true;
                    _1112 = _786.w;
                    break;
                }
                case 5:
                {
                    _1111 = true;
                    _1112 = in.input_color.w;
                    break;
                }
                case 6:
                {
                    _1111 = true;
                    _1112 = 1.0;
                    break;
                }
                case 7:
                {
                    _1111 = true;
                    _1112 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1111)
            {
                break;
            }
            _1111 = true;
            _1112 = 0.0;
            break;
        } while(false);
        _1140 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaC)
            {
                case 0:
                {
                    _1140 = true;
                    _1141 = _1488.w;
                    break;
                }
                case 1:
                {
                    _1140 = true;
                    _1141 = _1489.w;
                    break;
                }
                case 2:
                {
                    _1140 = true;
                    _1141 = _1490.w;
                    break;
                }
                case 3:
                {
                    _1140 = true;
                    _1141 = _1491.w;
                    break;
                }
                case 4:
                {
                    _1140 = true;
                    _1141 = _786.w;
                    break;
                }
                case 5:
                {
                    _1140 = true;
                    _1141 = in.input_color.w;
                    break;
                }
                case 6:
                {
                    _1140 = true;
                    _1141 = 1.0;
                    break;
                }
                case 7:
                {
                    _1140 = true;
                    _1141 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1140)
            {
                break;
            }
            _1140 = true;
            _1141 = 0.0;
            break;
        } while(false);
        _1169 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaD)
            {
                case 0:
                {
                    _1169 = true;
                    _1170 = _1488.w;
                    break;
                }
                case 1:
                {
                    _1169 = true;
                    _1170 = _1489.w;
                    break;
                }
                case 2:
                {
                    _1169 = true;
                    _1170 = _1490.w;
                    break;
                }
                case 3:
                {
                    _1169 = true;
                    _1170 = _1491.w;
                    break;
                }
                case 4:
                {
                    _1169 = true;
                    _1170 = _786.w;
                    break;
                }
                case 5:
                {
                    _1169 = true;
                    _1170 = in.input_color.w;
                    break;
                }
                case 6:
                {
                    _1169 = true;
                    _1170 = 1.0;
                    break;
                }
                case 7:
                {
                    _1169 = true;
                    _1170 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1169)
            {
                break;
            }
            _1169 = true;
            _1170 = 0.0;
            break;
        } while(false);
        float3 _1205 = (_895 * (float3(1.0) - _989)) + (_942 * _989);
        if (uniforms.tevStages.data[i].colorOp == 0)
        {
            _1198 = _1036 + _1205;
        }
        else
        {
            _1198 = _1036 - _1205;
        }
        switch (uniforms.tevStages.data[i].colorBias)
        {
            case 1:
            {
                _1198 += float3(0.5);
                break;
            }
            case 2:
            {
                _1198 -= float3(0.5);
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
                _1198 *= 2.0;
                break;
            }
            case 2:
            {
                _1198 *= 4.0;
                break;
            }
            case 3:
            {
                _1198 *= 0.5;
                break;
            }
            default:
            {
                break;
            }
        }
        if (uniforms.tevStages.data[i].colorClamp != 0)
        {
            _1198 = fast::clamp(_1198, float3(0.0), float3(1.0));
        }
        float _1252 = (_1083 * (1.0 - _1141)) + (_1112 * _1141);
        if (uniforms.tevStages.data[i].alphaOp == 0)
        {
            _1246 = _1170 + _1252;
        }
        else
        {
            _1246 = _1170 - _1252;
        }
        switch (uniforms.tevStages.data[i].alphaBias)
        {
            case 1:
            {
                _1246 += 0.5;
                break;
            }
            case 2:
            {
                _1246 -= 0.5;
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
                _1246 *= 2.0;
                break;
            }
            case 2:
            {
                _1246 *= 4.0;
                break;
            }
            case 3:
            {
                _1246 *= 0.5;
                break;
            }
            default:
            {
                break;
            }
        }
        if (uniforms.tevStages.data[i].alphaClamp != 0)
        {
            _1246 = fast::clamp(_1246, 0.0, 1.0);
        }
        switch (uniforms.tevStages.data[i].colorOutput)
        {
            case 0:
            {
                float4 _1550 = _1488;
                _1550.x = _1198.x;
                _1550.y = _1198.y;
                _1550.z = _1198.z;
                _1488 = _1550;
                break;
            }
            case 1:
            {
                float4 _1544 = _1489;
                _1544.x = _1198.x;
                _1544.y = _1198.y;
                _1544.z = _1198.z;
                _1489 = _1544;
                break;
            }
            case 2:
            {
                float4 _1538 = _1490;
                _1538.x = _1198.x;
                _1538.y = _1198.y;
                _1538.z = _1198.z;
                _1490 = _1538;
                break;
            }
            case 3:
            {
                float4 _1532 = _1491;
                _1532.x = _1198.x;
                _1532.y = _1198.y;
                _1532.z = _1198.z;
                _1491 = _1532;
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
                float4 _1562 = _1488;
                _1562.w = _1246;
                _1488 = _1562;
                break;
            }
            case 1:
            {
                float4 _1560 = _1489;
                _1560.w = _1246;
                _1489 = _1560;
                break;
            }
            case 2:
            {
                float4 _1558 = _1490;
                _1558.w = _1246;
                _1490 = _1558;
                break;
            }
            case 3:
            {
                float4 _1556 = _1491;
                _1556.w = _1246;
                _1491 = _1556;
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
    _1338 = false;
    bool _1339;
    do
    {
        _1376 = false;
        bool _1377;
        do
        {
            switch (uniforms.alphaComp0)
            {
                case 0:
                {
                    _1376 = true;
                    _1377 = false;
                    break;
                }
                case 1:
                {
                    _1376 = true;
                    _1377 = _1488.w < uniforms.alphaRef0;
                    break;
                }
                case 2:
                {
                    _1376 = true;
                    _1377 = _1488.w == uniforms.alphaRef0;
                    break;
                }
                case 3:
                {
                    _1376 = true;
                    _1377 = _1488.w <= uniforms.alphaRef0;
                    break;
                }
                case 4:
                {
                    _1376 = true;
                    _1377 = _1488.w > uniforms.alphaRef0;
                    break;
                }
                case 5:
                {
                    _1376 = true;
                    _1377 = _1488.w != uniforms.alphaRef0;
                    break;
                }
                case 6:
                {
                    _1376 = true;
                    _1377 = _1488.w >= uniforms.alphaRef0;
                    break;
                }
                case 7:
                {
                    _1376 = true;
                    _1377 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1376)
            {
                break;
            }
            _1376 = true;
            _1377 = true;
            break;
        } while(false);
        _1401 = false;
        bool _1402;
        do
        {
            switch (uniforms.alphaComp1)
            {
                case 0:
                {
                    _1401 = true;
                    _1402 = false;
                    break;
                }
                case 1:
                {
                    _1401 = true;
                    _1402 = _1488.w < uniforms.alphaRef1;
                    break;
                }
                case 2:
                {
                    _1401 = true;
                    _1402 = _1488.w == uniforms.alphaRef1;
                    break;
                }
                case 3:
                {
                    _1401 = true;
                    _1402 = _1488.w <= uniforms.alphaRef1;
                    break;
                }
                case 4:
                {
                    _1401 = true;
                    _1402 = _1488.w > uniforms.alphaRef1;
                    break;
                }
                case 5:
                {
                    _1401 = true;
                    _1402 = _1488.w != uniforms.alphaRef1;
                    break;
                }
                case 6:
                {
                    _1401 = true;
                    _1402 = _1488.w >= uniforms.alphaRef1;
                    break;
                }
                case 7:
                {
                    _1401 = true;
                    _1402 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1401)
            {
                break;
            }
            _1401 = true;
            _1402 = true;
            break;
        } while(false);
        bool _1340;
        switch (uniforms.alphaLogic)
        {
            case 0:
            {
                if (_1377)
                {
                    _1340 = _1402;
                }
                else
                {
                    _1340 = false;
                }
                _1338 = true;
                _1339 = _1340;
                break;
            }
            case 1:
            {
                if (_1377)
                {
                    _1340 = true;
                }
                else
                {
                    _1340 = _1402;
                }
                _1338 = true;
                _1339 = _1340;
                break;
            }
            case 2:
            {
                _1338 = true;
                _1339 = _1377 != _1402;
                break;
            }
            case 3:
            {
                _1338 = true;
                _1339 = _1377 == _1402;
                break;
            }
            default:
            {
                break;
            }
        }
        if (_1338)
        {
            break;
        }
        _1338 = true;
        _1339 = true;
        break;
    } while(false);
    if (!_1339)
    {
        discard_fragment();
    }
    out.entryPointParam_fragmentMain = _1488;
    return out;
}

)SHDR3";

#endif
