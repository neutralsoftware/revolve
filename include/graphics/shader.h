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
    int konstColorSel;
    int konstAlphaSel;
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
    bool _2753 = false;
    bool _2728 = false;
    bool _2690 = false;
    bool _2250 = false;
    bool _2221 = false;
    bool _2192 = false;
    bool _2163 = false;
    bool _2116 = false;
    bool _2069 = false;
    bool _2022 = false;
    bool _1975 = false;
    bool _1952 = false;
    bool _1917 = false;
    bool _1894 = false;
    bool _1842 = false;
    bool _1783 = false;
    bool _1756 = false;
    bool _1737 = false;
    bool _1718 = false;
    bool _1699 = false;
    bool _1680 = false;
    bool _1653 = false;
    bool _1622 = false;
    bool _1603 = false;
    bool _1584 = false;
    bool _1565 = false;
    bool _1538 = false;
    bool _1511 = false;
    main0_out out = {};
    float4 _2857 = uniforms.tevRegister0;
    float4 _2858 = uniforms.tevRegister1;
    float4 _2859 = uniforms.tevRegister2;
    float4 _2860 = uniforms.tevRegister3;
    int i = 0;
    float4 _1452;
    float4 _1512;
    int4 _1539;
    float _1566;
    float _1585;
    float _1604;
    float _1623;
    int4 _1654;
    float _1681;
    float _1700;
    float _1719;
    float _1738;
    float2 _1757;
    float4 _1784;
    float3 _1843;
    float _1844;
    float4 _1895;
    float _1918;
    float4 _1953;
    float3 _1976;
    float3 _2023;
    float3 _2070;
    float3 _2117;
    float _2164;
    float _2193;
    float _2222;
    float _2251;
    float3 _2280;
    float3 _2281;
    float _2282;
    float _2283;
    float _2284;
    float3 _2285;
    bool _2454;
    bool _2455;
    float _2556;
    float _2557;
    bool _2558;
    for (;;)
    {
        if (!(i < uniforms.tevStageCount))
        {
            break;
        }
        _1511 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorChannel)
            {
                case 0:
                {
                    _1511 = true;
                    _1512 = in.input_color0;
                    break;
                }
                case 1:
                {
                    _1511 = true;
                    _1512 = in.input_color1;
                    break;
                }
                default:
                {
                    _1511 = true;
                    _1512 = float4(0.0);
                    break;
                }
            }
            if (_1511)
            {
                break;
            }
            break;
        } while(false);
        _1538 = false;
        do
        {
            switch (uniforms.tevStages.data[i].rasterSwap)
            {
                case 0:
                {
                    _1538 = true;
                    _1539 = int4(uniforms.swapTable0);
                    break;
                }
                case 1:
                {
                    _1538 = true;
                    _1539 = int4(uniforms.swapTable1);
                    break;
                }
                case 2:
                {
                    _1538 = true;
                    _1539 = int4(uniforms.swapTable2);
                    break;
                }
                case 3:
                {
                    _1538 = true;
                    _1539 = int4(uniforms.swapTable3);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1538)
            {
                break;
            }
            _1538 = true;
            _1539 = int4(0, 1, 2, 3);
            break;
        } while(false);
        _1565 = false;
        do
        {
            switch (_1539.x)
            {
                case 0:
                {
                    _1565 = true;
                    _1566 = _1512.x;
                    break;
                }
                case 1:
                {
                    _1565 = true;
                    _1566 = _1512.y;
                    break;
                }
                case 2:
                {
                    _1565 = true;
                    _1566 = _1512.z;
                    break;
                }
                case 3:
                {
                    _1565 = true;
                    _1566 = _1512.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1565)
            {
                break;
            }
            _1565 = true;
            _1566 = 0.0;
            break;
        } while(false);
        _1584 = false;
        do
        {
            switch (_1539.y)
            {
                case 0:
                {
                    _1584 = true;
                    _1585 = _1512.x;
                    break;
                }
                case 1:
                {
                    _1584 = true;
                    _1585 = _1512.y;
                    break;
                }
                case 2:
                {
                    _1584 = true;
                    _1585 = _1512.z;
                    break;
                }
                case 3:
                {
                    _1584 = true;
                    _1585 = _1512.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1584)
            {
                break;
            }
            _1584 = true;
            _1585 = 0.0;
            break;
        } while(false);
        _1603 = false;
        do
        {
            switch (_1539.z)
            {
                case 0:
                {
                    _1603 = true;
                    _1604 = _1512.x;
                    break;
                }
                case 1:
                {
                    _1603 = true;
                    _1604 = _1512.y;
                    break;
                }
                case 2:
                {
                    _1603 = true;
                    _1604 = _1512.z;
                    break;
                }
                case 3:
                {
                    _1603 = true;
                    _1604 = _1512.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1603)
            {
                break;
            }
            _1603 = true;
            _1604 = 0.0;
            break;
        } while(false);
        _1622 = false;
        do
        {
            switch (_1539.w)
            {
                case 0:
                {
                    _1622 = true;
                    _1623 = _1512.x;
                    break;
                }
                case 1:
                {
                    _1622 = true;
                    _1623 = _1512.y;
                    break;
                }
                case 2:
                {
                    _1622 = true;
                    _1623 = _1512.z;
                    break;
                }
                case 3:
                {
                    _1622 = true;
                    _1623 = _1512.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1622)
            {
                break;
            }
            _1622 = true;
            _1623 = 0.0;
            break;
        } while(false);
        float4 _1537 = float4(_1566, _1585, _1604, _1623);
        _1653 = false;
        do
        {
            switch (uniforms.tevStages.data[i].textureSwap)
            {
                case 0:
                {
                    _1653 = true;
                    _1654 = int4(uniforms.swapTable0);
                    break;
                }
                case 1:
                {
                    _1653 = true;
                    _1654 = int4(uniforms.swapTable1);
                    break;
                }
                case 2:
                {
                    _1653 = true;
                    _1654 = int4(uniforms.swapTable2);
                    break;
                }
                case 3:
                {
                    _1653 = true;
                    _1654 = int4(uniforms.swapTable3);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1653)
            {
                break;
            }
            _1653 = true;
            _1654 = int4(0, 1, 2, 3);
            break;
        } while(false);
        _1680 = false;
        do
        {
            switch (_1654.x)
            {
                case 0:
                {
                    _1680 = true;
                    _1681 = 1.0;
                    break;
                }
                case 1:
                {
                    _1680 = true;
                    _1681 = 1.0;
                    break;
                }
                case 2:
                {
                    _1680 = true;
                    _1681 = 1.0;
                    break;
                }
                case 3:
                {
                    _1680 = true;
                    _1681 = 1.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1680)
            {
                break;
            }
            _1680 = true;
            _1681 = 0.0;
            break;
        } while(false);
        _1699 = false;
        do
        {
            switch (_1654.y)
            {
                case 0:
                {
                    _1699 = true;
                    _1700 = 1.0;
                    break;
                }
                case 1:
                {
                    _1699 = true;
                    _1700 = 1.0;
                    break;
                }
                case 2:
                {
                    _1699 = true;
                    _1700 = 1.0;
                    break;
                }
                case 3:
                {
                    _1699 = true;
                    _1700 = 1.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1699)
            {
                break;
            }
            _1699 = true;
            _1700 = 0.0;
            break;
        } while(false);
        _1718 = false;
        do
        {
            switch (_1654.z)
            {
                case 0:
                {
                    _1718 = true;
                    _1719 = 1.0;
                    break;
                }
                case 1:
                {
                    _1718 = true;
                    _1719 = 1.0;
                    break;
                }
                case 2:
                {
                    _1718 = true;
                    _1719 = 1.0;
                    break;
                }
                case 3:
                {
                    _1718 = true;
                    _1719 = 1.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1718)
            {
                break;
            }
            _1718 = true;
            _1719 = 0.0;
            break;
        } while(false);
        _1737 = false;
        do
        {
            switch (_1654.w)
            {
                case 0:
                {
                    _1737 = true;
                    _1738 = 1.0;
                    break;
                }
                case 1:
                {
                    _1737 = true;
                    _1738 = 1.0;
                    break;
                }
                case 2:
                {
                    _1737 = true;
                    _1738 = 1.0;
                    break;
                }
                case 3:
                {
                    _1737 = true;
                    _1738 = 1.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1737)
            {
                break;
            }
            _1737 = true;
            _1738 = 0.0;
            break;
        } while(false);
        if (uniforms.tevStages.data[i].textureEnabled != 0)
        {
            _1756 = false;
            do
            {
                switch (uniforms.tevStages.data[i].texCoord)
                {
                    case 0:
                    {
                        _1756 = true;
                        _1757 = in.input_uv0;
                        break;
                    }
                    case 1:
                    {
                        _1756 = true;
                        _1757 = in.input_uv1;
                        break;
                    }
                    case 2:
                    {
                        _1756 = true;
                        _1757 = in.input_uv2;
                        break;
                    }
                    case 3:
                    {
                        _1756 = true;
                        _1757 = in.input_uv3;
                        break;
                    }
                    case 4:
                    {
                        _1756 = true;
                        _1757 = in.input_uv4;
                        break;
                    }
                    case 5:
                    {
                        _1756 = true;
                        _1757 = in.input_uv5;
                        break;
                    }
                    case 6:
                    {
                        _1756 = true;
                        _1757 = in.input_uv6;
                        break;
                    }
                    case 7:
                    {
                        _1756 = true;
                        _1757 = in.input_uv7;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_1756)
                {
                    break;
                }
                _1756 = true;
                _1757 = float2(0.0);
                break;
            } while(false);
            _1783 = false;
            do
            {
                switch (uniforms.tevStages.data[i].texMap)
                {
                    case 0:
                    {
                        _1783 = true;
                        _1784 = tex0.sample(samp0, _1757);
                        break;
                    }
                    case 1:
                    {
                        _1783 = true;
                        _1784 = tex1.sample(samp1, _1757);
                        break;
                    }
                    case 2:
                    {
                        _1783 = true;
                        _1784 = tex2.sample(samp2, _1757);
                        break;
                    }
                    case 3:
                    {
                        _1783 = true;
                        _1784 = tex3.sample(samp3, _1757);
                        break;
                    }
                    case 4:
                    {
                        _1783 = true;
                        _1784 = tex4.sample(samp4, _1757);
                        break;
                    }
                    case 5:
                    {
                        _1783 = true;
                        _1784 = tex5.sample(samp5, _1757);
                        break;
                    }
                    case 6:
                    {
                        _1783 = true;
                        _1784 = tex6.sample(samp6, _1757);
                        break;
                    }
                    case 7:
                    {
                        _1783 = true;
                        _1784 = tex7.sample(samp7, _1757);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_1783)
                {
                    break;
                }
                _1783 = true;
                _1784 = float4(1.0);
                break;
            } while(false);
            _1452 = _1784;
        }
        else
        {
            _1452 = float4(_1681, _1700, _1719, _1738);
        }
        _1842 = false;
        do
        {
            switch (uniforms.tevStages.data[i].konstColorSel)
            {
                case 0:
                {
                    _1842 = true;
                    _1843 = float3(1.0);
                    break;
                }
                case 1:
                {
                    _1842 = true;
                    _1843 = float3(0.875);
                    break;
                }
                case 2:
                {
                    _1842 = true;
                    _1843 = float3(0.75);
                    break;
                }
                case 3:
                {
                    _1842 = true;
                    _1843 = float3(0.625);
                    break;
                }
                case 4:
                {
                    _1842 = true;
                    _1843 = float3(0.5);
                    break;
                }
                case 5:
                {
                    _1842 = true;
                    _1843 = float3(0.375);
                    break;
                }
                case 6:
                {
                    _1842 = true;
                    _1843 = float3(0.25);
                    break;
                }
                case 7:
                {
                    _1842 = true;
                    _1843 = float3(0.125);
                    break;
                }
                case 8:
                case 9:
                case 10:
                case 11:
                {
                    _1842 = true;
                    _1843 = float3(0.0);
                    break;
                }
                case 12:
                {
                    _1842 = true;
                    _1843 = uniforms.konst0.xyz;
                    break;
                }
                case 13:
                {
                    _1842 = true;
                    _1843 = uniforms.konst1.xyz;
                    break;
                }
                case 14:
                {
                    _1842 = true;
                    _1843 = uniforms.konst2.xyz;
                    break;
                }
                case 15:
                {
                    _1842 = true;
                    _1843 = uniforms.konst3.xyz;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1842)
            {
                break;
            }
            int _1877 = uniforms.tevStages.data[i].konstColorSel - 16;
            _1894 = false;
            do
            {
                switch (_1877 - 4 * (_1877 / 4))
                {
                    case 0:
                    {
                        _1894 = true;
                        _1895 = uniforms.konst0;
                        break;
                    }
                    case 1:
                    {
                        _1894 = true;
                        _1895 = uniforms.konst1;
                        break;
                    }
                    case 2:
                    {
                        _1894 = true;
                        _1895 = uniforms.konst2;
                        break;
                    }
                    case 3:
                    {
                        _1894 = true;
                        _1895 = uniforms.konst3;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_1894)
                {
                    break;
                }
                _1894 = true;
                _1895 = float4(0.0);
                break;
            } while(false);
            switch (_1877 / 4)
            {
                case 0:
                {
                    _1844 = _1895.x;
                    break;
                }
                case 1:
                {
                    _1844 = _1895.y;
                    break;
                }
                case 2:
                {
                    _1844 = _1895.z;
                    break;
                }
                default:
                {
                    _1844 = _1895.w;
                    break;
                }
            }
            _1842 = true;
            _1843 = float3(_1844);
            break;
        } while(false);
        _1917 = false;
        do
        {
            switch (uniforms.tevStages.data[i].konstAlphaSel)
            {
                case 0:
                {
                    _1917 = true;
                    _1918 = 1.0;
                    break;
                }
                case 1:
                {
                    _1917 = true;
                    _1918 = 0.875;
                    break;
                }
                case 2:
                {
                    _1917 = true;
                    _1918 = 0.75;
                    break;
                }
                case 3:
                {
                    _1917 = true;
                    _1918 = 0.625;
                    break;
                }
                case 4:
                {
                    _1917 = true;
                    _1918 = 0.5;
                    break;
                }
                case 5:
                {
                    _1917 = true;
                    _1918 = 0.375;
                    break;
                }
                case 6:
                {
                    _1917 = true;
                    _1918 = 0.25;
                    break;
                }
                case 7:
                {
                    _1917 = true;
                    _1918 = 0.125;
                    break;
                }
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                {
                    _1917 = true;
                    _1918 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1917)
            {
                break;
            }
            int _1935 = uniforms.tevStages.data[i].konstAlphaSel - 16;
            _1952 = false;
            do
            {
                switch (_1935 - 4 * (_1935 / 4))
                {
                    case 0:
                    {
                        _1952 = true;
                        _1953 = uniforms.konst0;
                        break;
                    }
                    case 1:
                    {
                        _1952 = true;
                        _1953 = uniforms.konst1;
                        break;
                    }
                    case 2:
                    {
                        _1952 = true;
                        _1953 = uniforms.konst2;
                        break;
                    }
                    case 3:
                    {
                        _1952 = true;
                        _1953 = uniforms.konst3;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_1952)
                {
                    break;
                }
                _1952 = true;
                _1953 = float4(0.0);
                break;
            } while(false);
            switch (_1935 / 4)
            {
                case 0:
                {
                    _1917 = true;
                    _1918 = _1953.x;
                    break;
                }
                case 1:
                {
                    _1917 = true;
                    _1918 = _1953.y;
                    break;
                }
                case 2:
                {
                    _1917 = true;
                    _1918 = _1953.z;
                    break;
                }
                default:
                {
                    _1917 = true;
                    _1918 = _1953.w;
                    break;
                }
            }
            if (_1917)
            {
                break;
            }
            break;
        } while(false);
        _1975 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorA)
            {
                case 0:
                {
                    _1975 = true;
                    _1976 = _2857.xyz;
                    break;
                }
                case 1:
                {
                    _1975 = true;
                    _1976 = _2857.www;
                    break;
                }
                case 2:
                {
                    _1975 = true;
                    _1976 = _2858.xyz;
                    break;
                }
                case 3:
                {
                    _1975 = true;
                    _1976 = _2858.www;
                    break;
                }
                case 4:
                {
                    _1975 = true;
                    _1976 = _2859.xyz;
                    break;
                }
                case 5:
                {
                    _1975 = true;
                    _1976 = _2859.www;
                    break;
                }
                case 6:
                {
                    _1975 = true;
                    _1976 = _2860.xyz;
                    break;
                }
                case 7:
                {
                    _1975 = true;
                    _1976 = _2860.www;
                    break;
                }
                case 8:
                {
                    _1975 = true;
                    _1976 = _1452.xyz;
                    break;
                }
                case 9:
                {
                    _1975 = true;
                    _1976 = _1452.www;
                    break;
                }
                case 10:
                {
                    _1975 = true;
                    _1976 = _1537.xyz;
                    break;
                }
                case 11:
                {
                    _1975 = true;
                    _1976 = _1537.www;
                    break;
                }
                case 12:
                {
                    _1975 = true;
                    _1976 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _1975 = true;
                    _1976 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _1975 = true;
                    _1976 = _1843;
                    break;
                }
                case 15:
                {
                    _1975 = true;
                    _1976 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_1975)
            {
                break;
            }
            _1975 = true;
            _1976 = float3(0.0);
            break;
        } while(false);
        _2022 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorB)
            {
                case 0:
                {
                    _2022 = true;
                    _2023 = _2857.xyz;
                    break;
                }
                case 1:
                {
                    _2022 = true;
                    _2023 = _2857.www;
                    break;
                }
                case 2:
                {
                    _2022 = true;
                    _2023 = _2858.xyz;
                    break;
                }
                case 3:
                {
                    _2022 = true;
                    _2023 = _2858.www;
                    break;
                }
                case 4:
                {
                    _2022 = true;
                    _2023 = _2859.xyz;
                    break;
                }
                case 5:
                {
                    _2022 = true;
                    _2023 = _2859.www;
                    break;
                }
                case 6:
                {
                    _2022 = true;
                    _2023 = _2860.xyz;
                    break;
                }
                case 7:
                {
                    _2022 = true;
                    _2023 = _2860.www;
                    break;
                }
                case 8:
                {
                    _2022 = true;
                    _2023 = _1452.xyz;
                    break;
                }
                case 9:
                {
                    _2022 = true;
                    _2023 = _1452.www;
                    break;
                }
                case 10:
                {
                    _2022 = true;
                    _2023 = _1537.xyz;
                    break;
                }
                case 11:
                {
                    _2022 = true;
                    _2023 = _1537.www;
                    break;
                }
                case 12:
                {
                    _2022 = true;
                    _2023 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _2022 = true;
                    _2023 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _2022 = true;
                    _2023 = _1843;
                    break;
                }
                case 15:
                {
                    _2022 = true;
                    _2023 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2022)
            {
                break;
            }
            _2022 = true;
            _2023 = float3(0.0);
            break;
        } while(false);
        _2069 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorC)
            {
                case 0:
                {
                    _2069 = true;
                    _2070 = _2857.xyz;
                    break;
                }
                case 1:
                {
                    _2069 = true;
                    _2070 = _2857.www;
                    break;
                }
                case 2:
                {
                    _2069 = true;
                    _2070 = _2858.xyz;
                    break;
                }
                case 3:
                {
                    _2069 = true;
                    _2070 = _2858.www;
                    break;
                }
                case 4:
                {
                    _2069 = true;
                    _2070 = _2859.xyz;
                    break;
                }
                case 5:
                {
                    _2069 = true;
                    _2070 = _2859.www;
                    break;
                }
                case 6:
                {
                    _2069 = true;
                    _2070 = _2860.xyz;
                    break;
                }
                case 7:
                {
                    _2069 = true;
                    _2070 = _2860.www;
                    break;
                }
                case 8:
                {
                    _2069 = true;
                    _2070 = _1452.xyz;
                    break;
                }
                case 9:
                {
                    _2069 = true;
                    _2070 = _1452.www;
                    break;
                }
                case 10:
                {
                    _2069 = true;
                    _2070 = _1537.xyz;
                    break;
                }
                case 11:
                {
                    _2069 = true;
                    _2070 = _1537.www;
                    break;
                }
                case 12:
                {
                    _2069 = true;
                    _2070 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _2069 = true;
                    _2070 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _2069 = true;
                    _2070 = _1843;
                    break;
                }
                case 15:
                {
                    _2069 = true;
                    _2070 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2069)
            {
                break;
            }
            _2069 = true;
            _2070 = float3(0.0);
            break;
        } while(false);
        _2116 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorD)
            {
                case 0:
                {
                    _2116 = true;
                    _2117 = _2857.xyz;
                    break;
                }
                case 1:
                {
                    _2116 = true;
                    _2117 = _2857.www;
                    break;
                }
                case 2:
                {
                    _2116 = true;
                    _2117 = _2858.xyz;
                    break;
                }
                case 3:
                {
                    _2116 = true;
                    _2117 = _2858.www;
                    break;
                }
                case 4:
                {
                    _2116 = true;
                    _2117 = _2859.xyz;
                    break;
                }
                case 5:
                {
                    _2116 = true;
                    _2117 = _2859.www;
                    break;
                }
                case 6:
                {
                    _2116 = true;
                    _2117 = _2860.xyz;
                    break;
                }
                case 7:
                {
                    _2116 = true;
                    _2117 = _2860.www;
                    break;
                }
                case 8:
                {
                    _2116 = true;
                    _2117 = _1452.xyz;
                    break;
                }
                case 9:
                {
                    _2116 = true;
                    _2117 = _1452.www;
                    break;
                }
                case 10:
                {
                    _2116 = true;
                    _2117 = _1537.xyz;
                    break;
                }
                case 11:
                {
                    _2116 = true;
                    _2117 = _1537.www;
                    break;
                }
                case 12:
                {
                    _2116 = true;
                    _2117 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _2116 = true;
                    _2117 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _2116 = true;
                    _2117 = _1843;
                    break;
                }
                case 15:
                {
                    _2116 = true;
                    _2117 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2116)
            {
                break;
            }
            _2116 = true;
            _2117 = float3(0.0);
            break;
        } while(false);
        _2163 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaA)
            {
                case 0:
                {
                    _2163 = true;
                    _2164 = _2857.w;
                    break;
                }
                case 1:
                {
                    _2163 = true;
                    _2164 = _2858.w;
                    break;
                }
                case 2:
                {
                    _2163 = true;
                    _2164 = _2859.w;
                    break;
                }
                case 3:
                {
                    _2163 = true;
                    _2164 = _2860.w;
                    break;
                }
                case 4:
                {
                    _2163 = true;
                    _2164 = _1452.w;
                    break;
                }
                case 5:
                {
                    _2163 = true;
                    _2164 = _1537.w;
                    break;
                }
                case 6:
                {
                    _2163 = true;
                    _2164 = _1918;
                    break;
                }
                case 7:
                {
                    _2163 = true;
                    _2164 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2163)
            {
                break;
            }
            _2163 = true;
            _2164 = 0.0;
            break;
        } while(false);
        _2192 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaB)
            {
                case 0:
                {
                    _2192 = true;
                    _2193 = _2857.w;
                    break;
                }
                case 1:
                {
                    _2192 = true;
                    _2193 = _2858.w;
                    break;
                }
                case 2:
                {
                    _2192 = true;
                    _2193 = _2859.w;
                    break;
                }
                case 3:
                {
                    _2192 = true;
                    _2193 = _2860.w;
                    break;
                }
                case 4:
                {
                    _2192 = true;
                    _2193 = _1452.w;
                    break;
                }
                case 5:
                {
                    _2192 = true;
                    _2193 = _1537.w;
                    break;
                }
                case 6:
                {
                    _2192 = true;
                    _2193 = _1918;
                    break;
                }
                case 7:
                {
                    _2192 = true;
                    _2193 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2192)
            {
                break;
            }
            _2192 = true;
            _2193 = 0.0;
            break;
        } while(false);
        _2221 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaC)
            {
                case 0:
                {
                    _2221 = true;
                    _2222 = _2857.w;
                    break;
                }
                case 1:
                {
                    _2221 = true;
                    _2222 = _2858.w;
                    break;
                }
                case 2:
                {
                    _2221 = true;
                    _2222 = _2859.w;
                    break;
                }
                case 3:
                {
                    _2221 = true;
                    _2222 = _2860.w;
                    break;
                }
                case 4:
                {
                    _2221 = true;
                    _2222 = _1452.w;
                    break;
                }
                case 5:
                {
                    _2221 = true;
                    _2222 = _1537.w;
                    break;
                }
                case 6:
                {
                    _2221 = true;
                    _2222 = _1918;
                    break;
                }
                case 7:
                {
                    _2221 = true;
                    _2222 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2221)
            {
                break;
            }
            _2221 = true;
            _2222 = 0.0;
            break;
        } while(false);
        _2250 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaD)
            {
                case 0:
                {
                    _2250 = true;
                    _2251 = _2857.w;
                    break;
                }
                case 1:
                {
                    _2250 = true;
                    _2251 = _2858.w;
                    break;
                }
                case 2:
                {
                    _2250 = true;
                    _2251 = _2859.w;
                    break;
                }
                case 3:
                {
                    _2250 = true;
                    _2251 = _2860.w;
                    break;
                }
                case 4:
                {
                    _2250 = true;
                    _2251 = _1452.w;
                    break;
                }
                case 5:
                {
                    _2250 = true;
                    _2251 = _1537.w;
                    break;
                }
                case 6:
                {
                    _2250 = true;
                    _2251 = _1918;
                    break;
                }
                case 7:
                {
                    _2250 = true;
                    _2251 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2250)
            {
                break;
            }
            _2250 = true;
            _2251 = 0.0;
            break;
        } while(false);
        do
        {
            if (uniforms.tevStages.data[i].colorBias == 3)
            {
                if (uniforms.tevStages.data[i].colorScale == 3)
                {
                    uint _2419 = uint(round(fast::clamp(_1976.x, 0.0, 1.0) * 255.0));
                    uint _2425 = uint(round(fast::clamp(_1976.y, 0.0, 1.0) * 255.0));
                    uint _2431 = uint(round(fast::clamp(_1976.z, 0.0, 1.0) * 255.0));
                    uint _2437 = uint(round(fast::clamp(_2023.x, 0.0, 1.0) * 255.0));
                    uint _2443 = uint(round(fast::clamp(_2023.y, 0.0, 1.0) * 255.0));
                    uint _2449 = uint(round(fast::clamp(_2023.z, 0.0, 1.0) * 255.0));
                    if (uniforms.tevStages.data[i].colorOp != 0)
                    {
                        if (_2419 == _2437)
                        {
                            _2282 = 1.0;
                        }
                        else
                        {
                            _2282 = 0.0;
                        }
                        if (_2425 == _2443)
                        {
                            _2283 = 1.0;
                        }
                        else
                        {
                            _2283 = 0.0;
                        }
                        if (_2431 == _2449)
                        {
                            _2284 = 1.0;
                        }
                        else
                        {
                            _2284 = 0.0;
                        }
                        _2281 = float3(_2282, _2283, _2284);
                    }
                    else
                    {
                        if (_2419 > _2437)
                        {
                            _2282 = 1.0;
                        }
                        else
                        {
                            _2282 = 0.0;
                        }
                        if (_2425 > _2443)
                        {
                            _2283 = 1.0;
                        }
                        else
                        {
                            _2283 = 0.0;
                        }
                        if (_2431 > _2449)
                        {
                            _2284 = 1.0;
                        }
                        else
                        {
                            _2284 = 0.0;
                        }
                        _2281 = float3(_2282, _2283, _2284);
                    }
                    float3 _2346 = _2117 + (_2070 * _2281);
                    if (uniforms.tevStages.data[i].colorClamp != 0)
                    {
                        _2285 = fast::clamp(_2346, float3(0.0), float3(1.0));
                    }
                    else
                    {
                        _2285 = _2346;
                    }
                    _2280 = _2285;
                    break;
                }
                do
                {
                    uint _2518 = uint(round(fast::clamp(_1976.x, 0.0, 1.0) * 255.0));
                    uint _2524 = uint(round(fast::clamp(_1976.y, 0.0, 1.0) * 255.0));
                    uint _2536 = uint(round(fast::clamp(_2023.x, 0.0, 1.0) * 255.0));
                    uint _2542 = uint(round(fast::clamp(_2023.y, 0.0, 1.0) * 255.0));
                    bool _2471 = uniforms.tevStages.data[i].colorOp != 0;
                    if (uniforms.tevStages.data[i].colorScale == 0)
                    {
                        if (_2471)
                        {
                            _2455 = _2518 == _2536;
                        }
                        else
                        {
                            _2455 = _2518 > _2536;
                        }
                        _2454 = _2455;
                        break;
                    }
                    if (uniforms.tevStages.data[i].colorScale == 1)
                    {
                        uint _2484 = (_2524 << uint(8)) | _2518;
                        uint _2486 = (_2542 << uint(8)) | _2536;
                        if (_2471)
                        {
                            _2455 = _2484 == _2486;
                        }
                        else
                        {
                            _2455 = _2484 > _2486;
                        }
                        _2454 = _2455;
                        break;
                    }
                    if (uniforms.tevStages.data[i].colorScale == 2)
                    {
                        uint _2499 = ((uint(round(fast::clamp(_1976.z, 0.0, 1.0) * 255.0)) << uint(16)) | (_2524 << uint(8))) | _2518;
                        uint _2503 = ((uint(round(fast::clamp(_2023.z, 0.0, 1.0) * 255.0)) << uint(16)) | (_2542 << uint(8))) | _2536;
                        if (_2471)
                        {
                            _2455 = _2499 == _2503;
                        }
                        else
                        {
                            _2455 = _2499 > _2503;
                        }
                        _2454 = _2455;
                        break;
                    }
                    _2454 = false;
                    break;
                } while(false);
                if (_2454)
                {
                    _2281 = _2070;
                }
                else
                {
                    _2281 = float3(0.0);
                }
                float3 _2360 = _2281;
                float3 _2361 = _2117 + _2360;
                if (uniforms.tevStages.data[i].colorClamp != 0)
                {
                    _2281 = fast::clamp(_2361, float3(0.0), float3(1.0));
                }
                else
                {
                    _2281 = _2361;
                }
                _2280 = _2281;
                break;
            }
            float3 _2374 = (_1976 * (float3(1.0) - _2070)) + (_2023 * _2070);
            if (uniforms.tevStages.data[i].colorOp == 0)
            {
                _2281 = _2117 + _2374;
            }
            else
            {
                _2281 = _2117 - _2374;
            }
            switch (uniforms.tevStages.data[i].colorBias)
            {
                case 1:
                {
                    _2281 += float3(0.5);
                    break;
                }
                case 2:
                {
                    _2281 -= float3(0.5);
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
                    _2281 *= 2.0;
                    break;
                }
                case 2:
                {
                    _2281 *= 4.0;
                    break;
                }
                case 3:
                {
                    _2281 *= 0.5;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (uniforms.tevStages.data[i].colorClamp != 0)
            {
                _2281 = fast::clamp(_2281, float3(0.0), float3(1.0));
            }
            _2280 = _2281;
            break;
        } while(false);
        do
        {
            if (uniforms.tevStages.data[i].alphaBias == 3)
            {
                uint _2633 = uint(round(fast::clamp(_2164, 0.0, 1.0) * 255.0));
                uint _2639 = uint(round(fast::clamp(_2193, 0.0, 1.0) * 255.0));
                if (uniforms.tevStages.data[i].alphaOp != 0)
                {
                    _2558 = _2633 == _2639;
                }
                else
                {
                    _2558 = _2633 > _2639;
                }
                if (_2558)
                {
                    _2557 = _2222;
                }
                else
                {
                    _2557 = 0.0;
                }
                float _2578 = _2557;
                float _2579 = _2251 + _2578;
                if (uniforms.tevStages.data[i].alphaClamp != 0)
                {
                    _2557 = fast::clamp(_2579, 0.0, 1.0);
                }
                else
                {
                    _2557 = _2579;
                }
                _2556 = _2557;
                break;
            }
            float _2591 = (_2164 * (1.0 - _2222)) + (_2193 * _2222);
            if (uniforms.tevStages.data[i].alphaOp == 0)
            {
                _2557 = _2251 + _2591;
            }
            else
            {
                _2557 = _2251 - _2591;
            }
            switch (uniforms.tevStages.data[i].alphaBias)
            {
                case 1:
                {
                    _2557 += 0.5;
                    break;
                }
                case 2:
                {
                    _2557 -= 0.5;
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
                    _2557 *= 2.0;
                    break;
                }
                case 2:
                {
                    _2557 *= 4.0;
                    break;
                }
                case 3:
                {
                    _2557 *= 0.5;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (uniforms.tevStages.data[i].alphaClamp != 0)
            {
                _2557 = fast::clamp(_2557, 0.0, 1.0);
            }
            _2556 = _2557;
            break;
        } while(false);
        switch (uniforms.tevStages.data[i].colorOutput)
        {
            case 0:
            {
                float4 _2919 = _2857;
                _2919.x = _2280.x;
                _2919.y = _2280.y;
                _2919.z = _2280.z;
                _2857 = _2919;
                break;
            }
            case 1:
            {
                float4 _2913 = _2858;
                _2913.x = _2280.x;
                _2913.y = _2280.y;
                _2913.z = _2280.z;
                _2858 = _2913;
                break;
            }
            case 2:
            {
                float4 _2907 = _2859;
                _2907.x = _2280.x;
                _2907.y = _2280.y;
                _2907.z = _2280.z;
                _2859 = _2907;
                break;
            }
            case 3:
            {
                float4 _2901 = _2860;
                _2901.x = _2280.x;
                _2901.y = _2280.y;
                _2901.z = _2280.z;
                _2860 = _2901;
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
                float4 _2931 = _2857;
                _2931.w = _2556;
                _2857 = _2931;
                break;
            }
            case 1:
            {
                float4 _2929 = _2858;
                _2929.w = _2556;
                _2858 = _2929;
                break;
            }
            case 2:
            {
                float4 _2927 = _2859;
                _2927.w = _2556;
                _2859 = _2927;
                break;
            }
            case 3:
            {
                float4 _2925 = _2860;
                _2925.w = _2556;
                _2860 = _2925;
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
    _2690 = false;
    bool _2691;
    do
    {
        _2728 = false;
        bool _2729;
        do
        {
            switch (uniforms.alphaComp0)
            {
                case 0:
                {
                    _2728 = true;
                    _2729 = false;
                    break;
                }
                case 1:
                {
                    _2728 = true;
                    _2729 = _2857.w < uniforms.alphaRef0;
                    break;
                }
                case 2:
                {
                    _2728 = true;
                    _2729 = _2857.w == uniforms.alphaRef0;
                    break;
                }
                case 3:
                {
                    _2728 = true;
                    _2729 = _2857.w <= uniforms.alphaRef0;
                    break;
                }
                case 4:
                {
                    _2728 = true;
                    _2729 = _2857.w > uniforms.alphaRef0;
                    break;
                }
                case 5:
                {
                    _2728 = true;
                    _2729 = _2857.w != uniforms.alphaRef0;
                    break;
                }
                case 6:
                {
                    _2728 = true;
                    _2729 = _2857.w >= uniforms.alphaRef0;
                    break;
                }
                case 7:
                {
                    _2728 = true;
                    _2729 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2728)
            {
                break;
            }
            _2728 = true;
            _2729 = true;
            break;
        } while(false);
        _2753 = false;
        bool _2754;
        do
        {
            switch (uniforms.alphaComp1)
            {
                case 0:
                {
                    _2753 = true;
                    _2754 = false;
                    break;
                }
                case 1:
                {
                    _2753 = true;
                    _2754 = _2857.w < uniforms.alphaRef1;
                    break;
                }
                case 2:
                {
                    _2753 = true;
                    _2754 = _2857.w == uniforms.alphaRef1;
                    break;
                }
                case 3:
                {
                    _2753 = true;
                    _2754 = _2857.w <= uniforms.alphaRef1;
                    break;
                }
                case 4:
                {
                    _2753 = true;
                    _2754 = _2857.w > uniforms.alphaRef1;
                    break;
                }
                case 5:
                {
                    _2753 = true;
                    _2754 = _2857.w != uniforms.alphaRef1;
                    break;
                }
                case 6:
                {
                    _2753 = true;
                    _2754 = _2857.w >= uniforms.alphaRef1;
                    break;
                }
                case 7:
                {
                    _2753 = true;
                    _2754 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2753)
            {
                break;
            }
            _2753 = true;
            _2754 = true;
            break;
        } while(false);
        bool _2692;
        switch (uniforms.alphaLogic)
        {
            case 0:
            {
                if (_2729)
                {
                    _2692 = _2754;
                }
                else
                {
                    _2692 = false;
                }
                _2690 = true;
                _2691 = _2692;
                break;
            }
            case 1:
            {
                if (_2729)
                {
                    _2692 = true;
                }
                else
                {
                    _2692 = _2754;
                }
                _2690 = true;
                _2691 = _2692;
                break;
            }
            case 2:
            {
                _2690 = true;
                _2691 = _2729 != _2754;
                break;
            }
            case 3:
            {
                _2690 = true;
                _2691 = _2729 == _2754;
                break;
            }
            default:
            {
                break;
            }
        }
        if (_2690)
        {
            break;
        }
        _2690 = true;
        _2691 = true;
        break;
    } while(false);
    if (!_2691)
    {
        discard_fragment();
    }
    out.entryPointParam_fragmentMain = _2857;
    return out;
}

)SHDR3";

#endif
