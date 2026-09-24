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
    float3 entryPointParam_vertexMain_uv0 [[user(locn2)]];
    float3 entryPointParam_vertexMain_uv1 [[user(locn3)]];
    float3 entryPointParam_vertexMain_uv2 [[user(locn4)]];
    float3 entryPointParam_vertexMain_uv3 [[user(locn5)]];
    float3 entryPointParam_vertexMain_uv4 [[user(locn6)]];
    float3 entryPointParam_vertexMain_uv5 [[user(locn7)]];
    float3 entryPointParam_vertexMain_uv6 [[user(locn8)]];
    float3 entryPointParam_vertexMain_uv7 [[user(locn9)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 input_position [[attribute(0)]];
    float4 input_color0 [[attribute(1)]];
    float4 input_color1 [[attribute(2)]];
    float3 input_uv0 [[attribute(3)]];
    float3 input_uv1 [[attribute(4)]];
    float3 input_uv2 [[attribute(5)]];
    float3 input_uv3 [[attribute(6)]];
    float3 input_uv4 [[attribute(7)]];
    float3 input_uv5 [[attribute(8)]];
    float3 input_uv6 [[attribute(9)]];
    float3 input_uv7 [[attribute(10)]];
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
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, size_t Num>
struct spvUnsafeArray
{
    T elements[Num ? Num : 1];
    
    thread T& operator [] (size_t pos) thread
    {
        return elements[pos];
    }
    constexpr const thread T& operator [] (size_t pos) const thread
    {
        return elements[pos];
    }
    
    device T& operator [] (size_t pos) device
    {
        return elements[pos];
    }
    constexpr const device T& operator [] (size_t pos) const device
    {
        return elements[pos];
    }
    
    constexpr const constant T& operator [] (size_t pos) const constant
    {
        return elements[pos];
    }
    
    threadgroup T& operator [] (size_t pos) threadgroup
    {
        return elements[pos];
    }
    constexpr const threadgroup T& operator [] (size_t pos) const threadgroup
    {
        return elements[pos];
    }
};

struct _Array_std140_vector_float_4_8
{
    float4 data[8];
};

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
    int indirectStage;
    int indirectFormat;
    int indirectBias;
    int indirectAlpha;
    int indirectMatrix;
    int indirectWrapS;
    int indirectWrapT;
    int indirectUseOriginalLod;
    int indirectAddPrevious;
    char _m0_final_padding[4];
};

struct _Array_std140_TevStage16
{
    TevStage_std140 data[16];
};

struct IndirectStage_std140
{
    int texCoord;
    int texMap;
    int scaleS;
    int scaleT;
};

struct _Array_std140_IndirectStage4
{
    IndirectStage_std140 data[4];
};

struct _Array_std140_float10
{
    float4 data[10];
};

struct Uniforms_std140
{
    float alphaRef0;
    float alphaRef1;
    int alphaComp0;
    int alphaComp1;
    int alphaLogic;
    int tevStageCount;
    int indirectStageCount;
    int pad0;
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
    _Array_std140_vector_float_4_8 textureSizes;
    _Array_std140_TevStage16 tevStages;
    _Array_std140_IndirectStage4 indirectStages;
    float4 indirectMatrix0A;
    float4 indirectMatrix0B;
    float4 indirectMatrix1A;
    float4 indirectMatrix1B;
    float4 indirectMatrix2A;
    float4 indirectMatrix2B;
    int destinationAlphaEnabled;
    float destinationAlpha;
    int enableDither;
    int efbPixelFormat;
    int zTextureOp;
    int zTextureFormat;
    int zTextureBias;
    int fogType;
    float fogA;
    int fogBMagnitude;
    int fogBShift;
    float fogC;
    packed_float3 fogColor;
    int fogRangeEnabled;
    float fogRangeCenter;
    float fogViewportWidth;
    _Array_std140_float10 fogRangeK;
};

constant spvUnsafeArray<uint, 4> _324 = spvUnsafeArray<uint, 4>({ 0u, 2u, 3u, 1u });

struct main0_out
{
    float4 entryPointParam_fragmentMain_color [[color(0)]];
    float gl_FragDepth [[depth(any)]];
};

struct main0_in
{
    float4 input_color0 [[user(locn0)]];
    float4 input_color1 [[user(locn1)]];
    float3 input_uv0 [[user(locn2)]];
    float3 input_uv1 [[user(locn3)]];
    float3 input_uv2 [[user(locn4)]];
    float3 input_uv3 [[user(locn5)]];
    float3 input_uv4 [[user(locn6)]];
    float3 input_uv5 [[user(locn7)]];
    float3 input_uv6 [[user(locn8)]];
    float3 input_uv7 [[user(locn9)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant Uniforms_std140& uniforms [[buffer(0)]], texture2d<float> tex0 [[texture(0)]], texture2d<float> tex1 [[texture(1)]], texture2d<float> tex2 [[texture(2)]], texture2d<float> tex3 [[texture(3)]], texture2d<float> tex4 [[texture(4)]], texture2d<float> tex5 [[texture(5)]], texture2d<float> tex6 [[texture(6)]], texture2d<float> tex7 [[texture(7)]], sampler samp0 [[sampler(0)]], sampler samp1 [[sampler(1)]], sampler samp2 [[sampler(2)]], sampler samp3 [[sampler(3)]], sampler samp4 [[sampler(4)]], sampler samp5 [[sampler(5)]], sampler samp6 [[sampler(6)]], sampler samp7 [[sampler(7)]], float4 gl_FragCoord [[position]])
{
    bool _4685 = false;
    bool _4617 = false;
    bool _4578 = false;
    bool _4540 = false;
    bool _4101 = false;
    bool _4072 = false;
    bool _4043 = false;
    bool _4014 = false;
    bool _3967 = false;
    bool _3920 = false;
    bool _3873 = false;
    bool _3826 = false;
    bool _3803 = false;
    bool _3768 = false;
    bool _3745 = false;
    bool _3693 = false;
    bool _3674 = false;
    bool _3655 = false;
    bool _3636 = false;
    bool _3617 = false;
    bool _3590 = false;
    bool _3519 = false;
    bool _3454 = false;
    bool _3435 = false;
    bool _3416 = false;
    bool _3397 = false;
    bool _3378 = false;
    bool _3351 = false;
    bool _3315 = false;
    bool _3257 = false;
    bool _3199 = false;
    bool _2859 = false;
    bool _2822 = false;
    bool _2751 = false;
    main0_out out = {};
    float4 _5020 = uniforms.tevRegister0;
    float4 _5021 = uniforms.tevRegister1;
    float4 _5022 = uniforms.tevRegister2;
    float4 _5023 = uniforms.tevRegister3;
    float2 indirectCoord = float2(0.0);
    float bumpAlpha = 0.0;
    float4 lastTexture = float4(0.0);
    int i = 0;
    float2 _2584;
    bool _2585;
    float3 _2586;
    float _2588;
    float2 _2589;
    float2 _2590;
    float4 _2591;
    float3 _2752;
    float3 _2823;
    float4 _2860;
    float3 _2925;
    float _2950;
    float2 _2983;
    bool _2984;
    float4 _3048;
    float4 _3049;
    float4 _3086;
    float4 _3087;
    float4 _3124;
    float4 _3125;
    float4 _3162;
    float4 _3163;
    float _3200;
    float _3258;
    float4 _3316;
    int4 _3352;
    float _3379;
    float _3398;
    float _3417;
    float _3436;
    float4 _3455;
    float4 _3520;
    int4 _3591;
    float _3618;
    float _3637;
    float _3656;
    float _3675;
    float3 _3694;
    float _3695;
    float4 _3746;
    float _3769;
    float4 _3804;
    float3 _3827;
    float3 _3874;
    float3 _3921;
    float3 _3968;
    float _4015;
    float _4044;
    float _4073;
    float _4102;
    float3 _4131;
    float3 _4132;
    float _4133;
    float _4134;
    float _4135;
    float3 _4136;
    bool _4304;
    bool _4305;
    float _4406;
    float _4407;
    bool _4408;
    for (;;)
    {
        if (!(i < uniforms.tevStageCount))
        {
            break;
        }
        _2751 = false;
        do
        {
            switch (uniforms.tevStages.data[i].texCoord)
            {
                case 0:
                {
                    _2751 = true;
                    _2752 = in.input_uv0;
                    break;
                }
                case 1:
                {
                    _2751 = true;
                    _2752 = in.input_uv1;
                    break;
                }
                case 2:
                {
                    _2751 = true;
                    _2752 = in.input_uv2;
                    break;
                }
                case 3:
                {
                    _2751 = true;
                    _2752 = in.input_uv3;
                    break;
                }
                case 4:
                {
                    _2751 = true;
                    _2752 = in.input_uv4;
                    break;
                }
                case 5:
                {
                    _2751 = true;
                    _2752 = in.input_uv5;
                    break;
                }
                case 6:
                {
                    _2751 = true;
                    _2752 = in.input_uv6;
                    break;
                }
                case 7:
                {
                    _2751 = true;
                    _2752 = in.input_uv7;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2751)
            {
                break;
            }
            _2751 = true;
            _2752 = float3(0.0);
            break;
        } while(false);
        if (_2752.z != 0.0)
        {
            _2584 = _2752.xy / float2(_2752.z);
        }
        else
        {
            _2584 = _2752.xy;
        }
        bool _2611 = uniforms.tevStages.data[i].indirectStage < uniforms.indirectStageCount;
        if (_2611)
        {
            if (uniforms.tevStages.data[i].indirectMatrix != 0)
            {
                _2585 = true;
            }
            else
            {
                _2585 = uniforms.tevStages.data[i].indirectAlpha != 0;
            }
        }
        else
        {
            _2585 = false;
        }
        if (_2585)
        {
            _2822 = false;
            do
            {
                switch (uniforms.indirectStages.data[uniforms.tevStages.data[i].indirectStage].texCoord)
                {
                    case 0:
                    {
                        _2822 = true;
                        _2823 = in.input_uv0;
                        break;
                    }
                    case 1:
                    {
                        _2822 = true;
                        _2823 = in.input_uv1;
                        break;
                    }
                    case 2:
                    {
                        _2822 = true;
                        _2823 = in.input_uv2;
                        break;
                    }
                    case 3:
                    {
                        _2822 = true;
                        _2823 = in.input_uv3;
                        break;
                    }
                    case 4:
                    {
                        _2822 = true;
                        _2823 = in.input_uv4;
                        break;
                    }
                    case 5:
                    {
                        _2822 = true;
                        _2823 = in.input_uv5;
                        break;
                    }
                    case 6:
                    {
                        _2822 = true;
                        _2823 = in.input_uv6;
                        break;
                    }
                    case 7:
                    {
                        _2822 = true;
                        _2823 = in.input_uv7;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_2822)
                {
                    break;
                }
                _2822 = true;
                _2823 = float3(0.0);
                break;
            } while(false);
            float2 _2778 = _2823.xy;
            if (_2823.z != 0.0)
            {
                _2778 /= float2(_2823.z);
            }
            float2 _5086 = _2778;
            _5086.x = _5086.x * exp2(-float(uniforms.indirectStages.data[uniforms.tevStages.data[i].indirectStage].scaleS));
            _5086.y = _5086.y * exp2(-float(uniforms.indirectStages.data[uniforms.tevStages.data[i].indirectStage].scaleT));
            _2778 = _5086;
            _2859 = false;
            do
            {
                switch (uniforms.indirectStages.data[uniforms.tevStages.data[i].indirectStage].texMap)
                {
                    case 0:
                    {
                        _2859 = true;
                        _2860 = tex0.sample(samp0, _5086);
                        break;
                    }
                    case 1:
                    {
                        _2859 = true;
                        _2860 = tex1.sample(samp1, _5086);
                        break;
                    }
                    case 2:
                    {
                        _2859 = true;
                        _2860 = tex2.sample(samp2, _5086);
                        break;
                    }
                    case 3:
                    {
                        _2859 = true;
                        _2860 = tex3.sample(samp3, _5086);
                        break;
                    }
                    case 4:
                    {
                        _2859 = true;
                        _2860 = tex4.sample(samp4, _5086);
                        break;
                    }
                    case 5:
                    {
                        _2859 = true;
                        _2860 = tex5.sample(samp5, _5086);
                        break;
                    }
                    case 6:
                    {
                        _2859 = true;
                        _2860 = tex6.sample(samp6, _5086);
                        break;
                    }
                    case 7:
                    {
                        _2859 = true;
                        _2860 = tex7.sample(samp7, _5086);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_2859)
                {
                    break;
                }
                _2859 = true;
                _2860 = float4(1.0);
                break;
            } while(false);
            _2586 = floor((_2860.wzy * 255.0) + float3(0.5));
        }
        else
        {
            _2586 = float3(0.0);
        }
        if (_2611)
        {
            _2585 = uniforms.tevStages.data[i].indirectAlpha != 0;
        }
        else
        {
            _2585 = false;
        }
        if (_2585)
        {
            float3 _2587 = _2586;
            if (uniforms.tevStages.data[i].indirectFormat == 0)
            {
                _2588 = 0.0;
            }
            else
            {
                _2588 = 6.0 - float(uniforms.tevStages.data[i].indirectFormat);
            }
            float _2648 = _2587[uniforms.tevStages.data[i].indirectAlpha - 1] * exp2(_2588);
            bumpAlpha = floor((_2648 - (256.0 * floor(_2648 * 0.00390625))) * 0.125) * 0.0313725508749485015869140625;
        }
        do
        {
            if (uniforms.tevStages.data[i].indirectFormat == 0)
            {
                _2925 = _2586;
                break;
            }
            if (uniforms.tevStages.data[i].indirectFormat == 1)
            {
                _2925 = floor(_2586 * float3(0.125));
                break;
            }
            if (uniforms.tevStages.data[i].indirectFormat == 2)
            {
                _2925 = floor(_2586 * float3(0.0625));
                break;
            }
            _2925 = floor(_2586 * float3(0.03125));
            break;
        } while(false);
        float3 _2949 = _2925;
        if (uniforms.tevStages.data[i].indirectFormat == 0)
        {
            _2950 = -128.0;
        }
        else
        {
            _2950 = 1.0;
        }
        if ((uniforms.tevStages.data[i].indirectBias & 1) != 0)
        {
            float3 _5092 = _2949;
            _5092.x = _5092.x + _2950;
            _2949 = _5092;
        }
        if ((uniforms.tevStages.data[i].indirectBias & 2) != 0)
        {
            float3 _5095 = _2949;
            _5095.y = _5095.y + _2950;
            _2949 = _5095;
        }
        if ((uniforms.tevStages.data[i].indirectBias & 4) != 0)
        {
            float3 _5098 = _2949;
            _5098.z = _5098.z + _2950;
            _2949 = _5098;
        }
        if (_2611)
        {
            do
            {
                if (uniforms.tevStages.data[i].indirectMatrix == 0)
                {
                    _2983 = float2(0.0);
                    break;
                }
                if (uniforms.tevStages.data[i].indirectMatrix >= 1)
                {
                    _2984 = uniforms.tevStages.data[i].indirectMatrix <= 3;
                }
                else
                {
                    _2984 = false;
                }
                if (_2984)
                {
                    int _2998 = uniforms.tevStages.data[i].indirectMatrix - 1;
                    do
                    {
                        if (_2998 == 0)
                        {
                            if (false)
                            {
                                _3049 = uniforms.indirectMatrix0B;
                            }
                            else
                            {
                                _3049 = uniforms.indirectMatrix0A;
                            }
                            _3048 = _3049;
                            break;
                        }
                        if (_2998 == 1)
                        {
                            if (false)
                            {
                                _3049 = uniforms.indirectMatrix1B;
                            }
                            else
                            {
                                _3049 = uniforms.indirectMatrix1A;
                            }
                            _3048 = _3049;
                            break;
                        }
                        if (false)
                        {
                            _3049 = uniforms.indirectMatrix2B;
                        }
                        else
                        {
                            _3049 = uniforms.indirectMatrix2A;
                        }
                        _3048 = _3049;
                        break;
                    } while(false);
                    do
                    {
                        if (_2998 == 0)
                        {
                            if (true)
                            {
                                _3087 = uniforms.indirectMatrix0B;
                            }
                            else
                            {
                                _3087 = uniforms.indirectMatrix0A;
                            }
                            _3086 = _3087;
                            break;
                        }
                        if (_2998 == 1)
                        {
                            if (true)
                            {
                                _3087 = uniforms.indirectMatrix1B;
                            }
                            else
                            {
                                _3087 = uniforms.indirectMatrix1A;
                            }
                            _3086 = _3087;
                            break;
                        }
                        if (true)
                        {
                            _3087 = uniforms.indirectMatrix2B;
                        }
                        else
                        {
                            _3087 = uniforms.indirectMatrix2A;
                        }
                        _3086 = _3087;
                        break;
                    } while(false);
                    _2983 = (float2(dot(_3048.xyz, _2949), dot(_3086.xyz, _2949)) * exp2(_3048.w)) * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].zw;
                    break;
                }
                if (uniforms.tevStages.data[i].indirectMatrix >= 5)
                {
                    _2984 = uniforms.tevStages.data[i].indirectMatrix <= 7;
                }
                else
                {
                    _2984 = false;
                }
                if (_2984)
                {
                    int _3018 = uniforms.tevStages.data[i].indirectMatrix - 5;
                    do
                    {
                        if (_3018 == 0)
                        {
                            if (false)
                            {
                                _3125 = uniforms.indirectMatrix0B;
                            }
                            else
                            {
                                _3125 = uniforms.indirectMatrix0A;
                            }
                            _3124 = _3125;
                            break;
                        }
                        if (_3018 == 1)
                        {
                            if (false)
                            {
                                _3125 = uniforms.indirectMatrix1B;
                            }
                            else
                            {
                                _3125 = uniforms.indirectMatrix1A;
                            }
                            _3124 = _3125;
                            break;
                        }
                        if (false)
                        {
                            _3125 = uniforms.indirectMatrix2B;
                        }
                        else
                        {
                            _3125 = uniforms.indirectMatrix2A;
                        }
                        _3124 = _3125;
                        break;
                    } while(false);
                    _2983 = ((_2584 * _2949.x) * exp2(_3124.w)) * float2(0.00390625);
                    break;
                }
                if (uniforms.tevStages.data[i].indirectMatrix >= 9)
                {
                    _2984 = uniforms.tevStages.data[i].indirectMatrix <= 11;
                }
                else
                {
                    _2984 = false;
                }
                if (_2984)
                {
                    int _3035 = uniforms.tevStages.data[i].indirectMatrix - 9;
                    do
                    {
                        if (_3035 == 0)
                        {
                            if (false)
                            {
                                _3163 = uniforms.indirectMatrix0B;
                            }
                            else
                            {
                                _3163 = uniforms.indirectMatrix0A;
                            }
                            _3162 = _3163;
                            break;
                        }
                        if (_3035 == 1)
                        {
                            if (false)
                            {
                                _3163 = uniforms.indirectMatrix1B;
                            }
                            else
                            {
                                _3163 = uniforms.indirectMatrix1A;
                            }
                            _3162 = _3163;
                            break;
                        }
                        if (false)
                        {
                            _3163 = uniforms.indirectMatrix2B;
                        }
                        else
                        {
                            _3163 = uniforms.indirectMatrix2A;
                        }
                        _3162 = _3163;
                        break;
                    } while(false);
                    _2983 = ((_2584 * _2949.y) * exp2(_3162.w)) * float2(0.00390625);
                    break;
                }
                _2983 = float2(0.0);
                break;
            } while(false);
            _2589 = _2983;
        }
        else
        {
            _2589 = float2(0.0);
        }
        _3199 = false;
        do
        {
            switch (uniforms.tevStages.data[i].indirectWrapS)
            {
                case 0:
                {
                    _3199 = true;
                    _3200 = _2584.x;
                    break;
                }
                case 1:
                {
                    float _3218 = 256.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _3199 = true;
                    _3200 = _2584.x - (_3218 * floor(_2584.x / _3218));
                    break;
                }
                case 2:
                {
                    float _3215 = 128.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _3199 = true;
                    _3200 = _2584.x - (_3215 * floor(_2584.x / _3215));
                    break;
                }
                case 3:
                {
                    float _3212 = 64.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _3199 = true;
                    _3200 = _2584.x - (_3212 * floor(_2584.x / _3212));
                    break;
                }
                case 4:
                {
                    float _3209 = 32.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _3199 = true;
                    _3200 = _2584.x - (_3209 * floor(_2584.x / _3209));
                    break;
                }
                case 5:
                {
                    float _3206 = 16.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _3199 = true;
                    _3200 = _2584.x - (_3206 * floor(_2584.x / _3206));
                    break;
                }
                case 6:
                {
                    _3199 = true;
                    _3200 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3199)
            {
                break;
            }
            _3199 = true;
            _3200 = _2584.x;
            break;
        } while(false);
        float2 _5101 = _2590;
        _5101.x = _3200;
        _2590 = _5101;
        _3257 = false;
        do
        {
            switch (uniforms.tevStages.data[i].indirectWrapT)
            {
                case 0:
                {
                    _3257 = true;
                    _3258 = _2584.y;
                    break;
                }
                case 1:
                {
                    float _3276 = 256.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _3257 = true;
                    _3258 = _2584.y - (_3276 * floor(_2584.y / _3276));
                    break;
                }
                case 2:
                {
                    float _3273 = 128.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _3257 = true;
                    _3258 = _2584.y - (_3273 * floor(_2584.y / _3273));
                    break;
                }
                case 3:
                {
                    float _3270 = 64.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _3257 = true;
                    _3258 = _2584.y - (_3270 * floor(_2584.y / _3270));
                    break;
                }
                case 4:
                {
                    float _3267 = 32.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _3257 = true;
                    _3258 = _2584.y - (_3267 * floor(_2584.y / _3267));
                    break;
                }
                case 5:
                {
                    float _3264 = 16.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _3257 = true;
                    _3258 = _2584.y - (_3264 * floor(_2584.y / _3264));
                    break;
                }
                case 6:
                {
                    _3257 = true;
                    _3258 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3257)
            {
                break;
            }
            _3257 = true;
            _3258 = _2584.y;
            break;
        } while(false);
        float2 _5103 = _2590;
        _5103.y = _3258;
        _2590 = _5103 + _2589;
        if (uniforms.tevStages.data[i].indirectAddPrevious != 0)
        {
            _2590 += indirectCoord;
        }
        indirectCoord = _2590;
        _3315 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorChannel)
            {
                case 0:
                {
                    _3315 = true;
                    _3316 = in.input_color0;
                    break;
                }
                case 1:
                {
                    _3315 = true;
                    _3316 = in.input_color1;
                    break;
                }
                case 5:
                {
                    _3315 = true;
                    _3316 = float4(bumpAlpha);
                    break;
                }
                case 6:
                {
                    _3315 = true;
                    _3316 = float4(((bumpAlpha * 255.0) + floor(bumpAlpha * 7.96875)) * 0.0039215688593685626983642578125);
                    break;
                }
                default:
                {
                    _3315 = true;
                    _3316 = float4(0.0);
                    break;
                }
            }
            if (_3315)
            {
                break;
            }
            break;
        } while(false);
        _3351 = false;
        do
        {
            switch (uniforms.tevStages.data[i].rasterSwap)
            {
                case 0:
                {
                    _3351 = true;
                    _3352 = int4(uniforms.swapTable0);
                    break;
                }
                case 1:
                {
                    _3351 = true;
                    _3352 = int4(uniforms.swapTable1);
                    break;
                }
                case 2:
                {
                    _3351 = true;
                    _3352 = int4(uniforms.swapTable2);
                    break;
                }
                case 3:
                {
                    _3351 = true;
                    _3352 = int4(uniforms.swapTable3);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3351)
            {
                break;
            }
            _3351 = true;
            _3352 = int4(0, 1, 2, 3);
            break;
        } while(false);
        _3378 = false;
        do
        {
            switch (_3352.x)
            {
                case 0:
                {
                    _3378 = true;
                    _3379 = _3316.x;
                    break;
                }
                case 1:
                {
                    _3378 = true;
                    _3379 = _3316.y;
                    break;
                }
                case 2:
                {
                    _3378 = true;
                    _3379 = _3316.z;
                    break;
                }
                case 3:
                {
                    _3378 = true;
                    _3379 = _3316.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3378)
            {
                break;
            }
            _3378 = true;
            _3379 = 0.0;
            break;
        } while(false);
        _3397 = false;
        do
        {
            switch (_3352.y)
            {
                case 0:
                {
                    _3397 = true;
                    _3398 = _3316.x;
                    break;
                }
                case 1:
                {
                    _3397 = true;
                    _3398 = _3316.y;
                    break;
                }
                case 2:
                {
                    _3397 = true;
                    _3398 = _3316.z;
                    break;
                }
                case 3:
                {
                    _3397 = true;
                    _3398 = _3316.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3397)
            {
                break;
            }
            _3397 = true;
            _3398 = 0.0;
            break;
        } while(false);
        _3416 = false;
        do
        {
            switch (_3352.z)
            {
                case 0:
                {
                    _3416 = true;
                    _3417 = _3316.x;
                    break;
                }
                case 1:
                {
                    _3416 = true;
                    _3417 = _3316.y;
                    break;
                }
                case 2:
                {
                    _3416 = true;
                    _3417 = _3316.z;
                    break;
                }
                case 3:
                {
                    _3416 = true;
                    _3417 = _3316.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3416)
            {
                break;
            }
            _3416 = true;
            _3417 = 0.0;
            break;
        } while(false);
        _3435 = false;
        do
        {
            switch (_3352.w)
            {
                case 0:
                {
                    _3435 = true;
                    _3436 = _3316.x;
                    break;
                }
                case 1:
                {
                    _3435 = true;
                    _3436 = _3316.y;
                    break;
                }
                case 2:
                {
                    _3435 = true;
                    _3436 = _3316.z;
                    break;
                }
                case 3:
                {
                    _3435 = true;
                    _3436 = _3316.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3435)
            {
                break;
            }
            _3435 = true;
            _3436 = 0.0;
            break;
        } while(false);
        float4 _3350 = float4(_3379, _3398, _3417, _3436);
        if (uniforms.tevStages.data[i].textureEnabled != 0)
        {
            if (uniforms.tevStages.data[i].indirectUseOriginalLod != 0)
            {
                float2 _3515 = dfdx(_2584);
                float2 _3518 = dfdy(_2584);
                _3519 = false;
                do
                {
                    switch (uniforms.tevStages.data[i].texMap)
                    {
                        case 0:
                        {
                            _3519 = true;
                            _3520 = tex0.sample(samp0, _2590, gradient2d(_3515, _3518));
                            break;
                        }
                        case 1:
                        {
                            _3519 = true;
                            _3520 = tex1.sample(samp1, _2590, gradient2d(_3515, _3518));
                            break;
                        }
                        case 2:
                        {
                            _3519 = true;
                            _3520 = tex2.sample(samp2, _2590, gradient2d(_3515, _3518));
                            break;
                        }
                        case 3:
                        {
                            _3519 = true;
                            _3520 = tex3.sample(samp3, _2590, gradient2d(_3515, _3518));
                            break;
                        }
                        case 4:
                        {
                            _3519 = true;
                            _3520 = tex4.sample(samp4, _2590, gradient2d(_3515, _3518));
                            break;
                        }
                        case 5:
                        {
                            _3519 = true;
                            _3520 = tex5.sample(samp5, _2590, gradient2d(_3515, _3518));
                            break;
                        }
                        case 6:
                        {
                            _3519 = true;
                            _3520 = tex6.sample(samp6, _2590, gradient2d(_3515, _3518));
                            break;
                        }
                        case 7:
                        {
                            _3519 = true;
                            _3520 = tex7.sample(samp7, _2590, gradient2d(_3515, _3518));
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                    if (_3519)
                    {
                        break;
                    }
                    _3519 = true;
                    _3520 = float4(1.0);
                    break;
                } while(false);
                _2591 = _3520;
            }
            else
            {
                _3454 = false;
                do
                {
                    switch (uniforms.tevStages.data[i].texMap)
                    {
                        case 0:
                        {
                            _3454 = true;
                            _3455 = tex0.sample(samp0, _2590);
                            break;
                        }
                        case 1:
                        {
                            _3454 = true;
                            _3455 = tex1.sample(samp1, _2590);
                            break;
                        }
                        case 2:
                        {
                            _3454 = true;
                            _3455 = tex2.sample(samp2, _2590);
                            break;
                        }
                        case 3:
                        {
                            _3454 = true;
                            _3455 = tex3.sample(samp3, _2590);
                            break;
                        }
                        case 4:
                        {
                            _3454 = true;
                            _3455 = tex4.sample(samp4, _2590);
                            break;
                        }
                        case 5:
                        {
                            _3454 = true;
                            _3455 = tex5.sample(samp5, _2590);
                            break;
                        }
                        case 6:
                        {
                            _3454 = true;
                            _3455 = tex6.sample(samp6, _2590);
                            break;
                        }
                        case 7:
                        {
                            _3454 = true;
                            _3455 = tex7.sample(samp7, _2590);
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                    if (_3454)
                    {
                        break;
                    }
                    _3454 = true;
                    _3455 = float4(1.0);
                    break;
                } while(false);
                _2591 = _3455;
            }
            lastTexture = _2591;
        }
        else
        {
            _2591 = float4(1.0);
        }
        _3590 = false;
        do
        {
            switch (uniforms.tevStages.data[i].textureSwap)
            {
                case 0:
                {
                    _3590 = true;
                    _3591 = int4(uniforms.swapTable0);
                    break;
                }
                case 1:
                {
                    _3590 = true;
                    _3591 = int4(uniforms.swapTable1);
                    break;
                }
                case 2:
                {
                    _3590 = true;
                    _3591 = int4(uniforms.swapTable2);
                    break;
                }
                case 3:
                {
                    _3590 = true;
                    _3591 = int4(uniforms.swapTable3);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3590)
            {
                break;
            }
            _3590 = true;
            _3591 = int4(0, 1, 2, 3);
            break;
        } while(false);
        _3617 = false;
        do
        {
            switch (_3591.x)
            {
                case 0:
                {
                    _3617 = true;
                    _3618 = _2591.x;
                    break;
                }
                case 1:
                {
                    _3617 = true;
                    _3618 = _2591.y;
                    break;
                }
                case 2:
                {
                    _3617 = true;
                    _3618 = _2591.z;
                    break;
                }
                case 3:
                {
                    _3617 = true;
                    _3618 = _2591.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3617)
            {
                break;
            }
            _3617 = true;
            _3618 = 0.0;
            break;
        } while(false);
        _3636 = false;
        do
        {
            switch (_3591.y)
            {
                case 0:
                {
                    _3636 = true;
                    _3637 = _2591.x;
                    break;
                }
                case 1:
                {
                    _3636 = true;
                    _3637 = _2591.y;
                    break;
                }
                case 2:
                {
                    _3636 = true;
                    _3637 = _2591.z;
                    break;
                }
                case 3:
                {
                    _3636 = true;
                    _3637 = _2591.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3636)
            {
                break;
            }
            _3636 = true;
            _3637 = 0.0;
            break;
        } while(false);
        _3655 = false;
        do
        {
            switch (_3591.z)
            {
                case 0:
                {
                    _3655 = true;
                    _3656 = _2591.x;
                    break;
                }
                case 1:
                {
                    _3655 = true;
                    _3656 = _2591.y;
                    break;
                }
                case 2:
                {
                    _3655 = true;
                    _3656 = _2591.z;
                    break;
                }
                case 3:
                {
                    _3655 = true;
                    _3656 = _2591.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3655)
            {
                break;
            }
            _3655 = true;
            _3656 = 0.0;
            break;
        } while(false);
        _3674 = false;
        do
        {
            switch (_3591.w)
            {
                case 0:
                {
                    _3674 = true;
                    _3675 = _2591.x;
                    break;
                }
                case 1:
                {
                    _3674 = true;
                    _3675 = _2591.y;
                    break;
                }
                case 2:
                {
                    _3674 = true;
                    _3675 = _2591.z;
                    break;
                }
                case 3:
                {
                    _3674 = true;
                    _3675 = _2591.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3674)
            {
                break;
            }
            _3674 = true;
            _3675 = 0.0;
            break;
        } while(false);
        float4 _3589 = float4(_3618, _3637, _3656, _3675);
        _3693 = false;
        do
        {
            switch (uniforms.tevStages.data[i].konstColorSel)
            {
                case 0:
                {
                    _3693 = true;
                    _3694 = float3(1.0);
                    break;
                }
                case 1:
                {
                    _3693 = true;
                    _3694 = float3(0.875);
                    break;
                }
                case 2:
                {
                    _3693 = true;
                    _3694 = float3(0.75);
                    break;
                }
                case 3:
                {
                    _3693 = true;
                    _3694 = float3(0.625);
                    break;
                }
                case 4:
                {
                    _3693 = true;
                    _3694 = float3(0.5);
                    break;
                }
                case 5:
                {
                    _3693 = true;
                    _3694 = float3(0.375);
                    break;
                }
                case 6:
                {
                    _3693 = true;
                    _3694 = float3(0.25);
                    break;
                }
                case 7:
                {
                    _3693 = true;
                    _3694 = float3(0.125);
                    break;
                }
                case 8:
                case 9:
                case 10:
                case 11:
                {
                    _3693 = true;
                    _3694 = float3(0.0);
                    break;
                }
                case 12:
                {
                    _3693 = true;
                    _3694 = uniforms.konst0.xyz;
                    break;
                }
                case 13:
                {
                    _3693 = true;
                    _3694 = uniforms.konst1.xyz;
                    break;
                }
                case 14:
                {
                    _3693 = true;
                    _3694 = uniforms.konst2.xyz;
                    break;
                }
                case 15:
                {
                    _3693 = true;
                    _3694 = uniforms.konst3.xyz;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3693)
            {
                break;
            }
            int _3728 = uniforms.tevStages.data[i].konstColorSel - 16;
            _3745 = false;
            do
            {
                switch (_3728 - 4 * (_3728 / 4))
                {
                    case 0:
                    {
                        _3745 = true;
                        _3746 = uniforms.konst0;
                        break;
                    }
                    case 1:
                    {
                        _3745 = true;
                        _3746 = uniforms.konst1;
                        break;
                    }
                    case 2:
                    {
                        _3745 = true;
                        _3746 = uniforms.konst2;
                        break;
                    }
                    case 3:
                    {
                        _3745 = true;
                        _3746 = uniforms.konst3;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_3745)
                {
                    break;
                }
                _3745 = true;
                _3746 = float4(0.0);
                break;
            } while(false);
            switch (_3728 / 4)
            {
                case 0:
                {
                    _3695 = _3746.x;
                    break;
                }
                case 1:
                {
                    _3695 = _3746.y;
                    break;
                }
                case 2:
                {
                    _3695 = _3746.z;
                    break;
                }
                default:
                {
                    _3695 = _3746.w;
                    break;
                }
            }
            _3693 = true;
            _3694 = float3(_3695);
            break;
        } while(false);
        _3768 = false;
        do
        {
            switch (uniforms.tevStages.data[i].konstAlphaSel)
            {
                case 0:
                {
                    _3768 = true;
                    _3769 = 1.0;
                    break;
                }
                case 1:
                {
                    _3768 = true;
                    _3769 = 0.875;
                    break;
                }
                case 2:
                {
                    _3768 = true;
                    _3769 = 0.75;
                    break;
                }
                case 3:
                {
                    _3768 = true;
                    _3769 = 0.625;
                    break;
                }
                case 4:
                {
                    _3768 = true;
                    _3769 = 0.5;
                    break;
                }
                case 5:
                {
                    _3768 = true;
                    _3769 = 0.375;
                    break;
                }
                case 6:
                {
                    _3768 = true;
                    _3769 = 0.25;
                    break;
                }
                case 7:
                {
                    _3768 = true;
                    _3769 = 0.125;
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
                    _3768 = true;
                    _3769 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3768)
            {
                break;
            }
            int _3786 = uniforms.tevStages.data[i].konstAlphaSel - 16;
            _3803 = false;
            do
            {
                switch (_3786 - 4 * (_3786 / 4))
                {
                    case 0:
                    {
                        _3803 = true;
                        _3804 = uniforms.konst0;
                        break;
                    }
                    case 1:
                    {
                        _3803 = true;
                        _3804 = uniforms.konst1;
                        break;
                    }
                    case 2:
                    {
                        _3803 = true;
                        _3804 = uniforms.konst2;
                        break;
                    }
                    case 3:
                    {
                        _3803 = true;
                        _3804 = uniforms.konst3;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_3803)
                {
                    break;
                }
                _3803 = true;
                _3804 = float4(0.0);
                break;
            } while(false);
            switch (_3786 / 4)
            {
                case 0:
                {
                    _3768 = true;
                    _3769 = _3804.x;
                    break;
                }
                case 1:
                {
                    _3768 = true;
                    _3769 = _3804.y;
                    break;
                }
                case 2:
                {
                    _3768 = true;
                    _3769 = _3804.z;
                    break;
                }
                default:
                {
                    _3768 = true;
                    _3769 = _3804.w;
                    break;
                }
            }
            if (_3768)
            {
                break;
            }
            break;
        } while(false);
        _3826 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorA)
            {
                case 0:
                {
                    _3826 = true;
                    _3827 = _5020.xyz;
                    break;
                }
                case 1:
                {
                    _3826 = true;
                    _3827 = _5020.www;
                    break;
                }
                case 2:
                {
                    _3826 = true;
                    _3827 = _5021.xyz;
                    break;
                }
                case 3:
                {
                    _3826 = true;
                    _3827 = _5021.www;
                    break;
                }
                case 4:
                {
                    _3826 = true;
                    _3827 = _5022.xyz;
                    break;
                }
                case 5:
                {
                    _3826 = true;
                    _3827 = _5022.www;
                    break;
                }
                case 6:
                {
                    _3826 = true;
                    _3827 = _5023.xyz;
                    break;
                }
                case 7:
                {
                    _3826 = true;
                    _3827 = _5023.www;
                    break;
                }
                case 8:
                {
                    _3826 = true;
                    _3827 = _3589.xyz;
                    break;
                }
                case 9:
                {
                    _3826 = true;
                    _3827 = _3589.www;
                    break;
                }
                case 10:
                {
                    _3826 = true;
                    _3827 = _3350.xyz;
                    break;
                }
                case 11:
                {
                    _3826 = true;
                    _3827 = _3350.www;
                    break;
                }
                case 12:
                {
                    _3826 = true;
                    _3827 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _3826 = true;
                    _3827 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _3826 = true;
                    _3827 = _3694;
                    break;
                }
                case 15:
                {
                    _3826 = true;
                    _3827 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3826)
            {
                break;
            }
            _3826 = true;
            _3827 = float3(0.0);
            break;
        } while(false);
        _3873 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorB)
            {
                case 0:
                {
                    _3873 = true;
                    _3874 = _5020.xyz;
                    break;
                }
                case 1:
                {
                    _3873 = true;
                    _3874 = _5020.www;
                    break;
                }
                case 2:
                {
                    _3873 = true;
                    _3874 = _5021.xyz;
                    break;
                }
                case 3:
                {
                    _3873 = true;
                    _3874 = _5021.www;
                    break;
                }
                case 4:
                {
                    _3873 = true;
                    _3874 = _5022.xyz;
                    break;
                }
                case 5:
                {
                    _3873 = true;
                    _3874 = _5022.www;
                    break;
                }
                case 6:
                {
                    _3873 = true;
                    _3874 = _5023.xyz;
                    break;
                }
                case 7:
                {
                    _3873 = true;
                    _3874 = _5023.www;
                    break;
                }
                case 8:
                {
                    _3873 = true;
                    _3874 = _3589.xyz;
                    break;
                }
                case 9:
                {
                    _3873 = true;
                    _3874 = _3589.www;
                    break;
                }
                case 10:
                {
                    _3873 = true;
                    _3874 = _3350.xyz;
                    break;
                }
                case 11:
                {
                    _3873 = true;
                    _3874 = _3350.www;
                    break;
                }
                case 12:
                {
                    _3873 = true;
                    _3874 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _3873 = true;
                    _3874 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _3873 = true;
                    _3874 = _3694;
                    break;
                }
                case 15:
                {
                    _3873 = true;
                    _3874 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3873)
            {
                break;
            }
            _3873 = true;
            _3874 = float3(0.0);
            break;
        } while(false);
        _3920 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorC)
            {
                case 0:
                {
                    _3920 = true;
                    _3921 = _5020.xyz;
                    break;
                }
                case 1:
                {
                    _3920 = true;
                    _3921 = _5020.www;
                    break;
                }
                case 2:
                {
                    _3920 = true;
                    _3921 = _5021.xyz;
                    break;
                }
                case 3:
                {
                    _3920 = true;
                    _3921 = _5021.www;
                    break;
                }
                case 4:
                {
                    _3920 = true;
                    _3921 = _5022.xyz;
                    break;
                }
                case 5:
                {
                    _3920 = true;
                    _3921 = _5022.www;
                    break;
                }
                case 6:
                {
                    _3920 = true;
                    _3921 = _5023.xyz;
                    break;
                }
                case 7:
                {
                    _3920 = true;
                    _3921 = _5023.www;
                    break;
                }
                case 8:
                {
                    _3920 = true;
                    _3921 = _3589.xyz;
                    break;
                }
                case 9:
                {
                    _3920 = true;
                    _3921 = _3589.www;
                    break;
                }
                case 10:
                {
                    _3920 = true;
                    _3921 = _3350.xyz;
                    break;
                }
                case 11:
                {
                    _3920 = true;
                    _3921 = _3350.www;
                    break;
                }
                case 12:
                {
                    _3920 = true;
                    _3921 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _3920 = true;
                    _3921 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _3920 = true;
                    _3921 = _3694;
                    break;
                }
                case 15:
                {
                    _3920 = true;
                    _3921 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3920)
            {
                break;
            }
            _3920 = true;
            _3921 = float3(0.0);
            break;
        } while(false);
        _3967 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorD)
            {
                case 0:
                {
                    _3967 = true;
                    _3968 = _5020.xyz;
                    break;
                }
                case 1:
                {
                    _3967 = true;
                    _3968 = _5020.www;
                    break;
                }
                case 2:
                {
                    _3967 = true;
                    _3968 = _5021.xyz;
                    break;
                }
                case 3:
                {
                    _3967 = true;
                    _3968 = _5021.www;
                    break;
                }
                case 4:
                {
                    _3967 = true;
                    _3968 = _5022.xyz;
                    break;
                }
                case 5:
                {
                    _3967 = true;
                    _3968 = _5022.www;
                    break;
                }
                case 6:
                {
                    _3967 = true;
                    _3968 = _5023.xyz;
                    break;
                }
                case 7:
                {
                    _3967 = true;
                    _3968 = _5023.www;
                    break;
                }
                case 8:
                {
                    _3967 = true;
                    _3968 = _3589.xyz;
                    break;
                }
                case 9:
                {
                    _3967 = true;
                    _3968 = _3589.www;
                    break;
                }
                case 10:
                {
                    _3967 = true;
                    _3968 = _3350.xyz;
                    break;
                }
                case 11:
                {
                    _3967 = true;
                    _3968 = _3350.www;
                    break;
                }
                case 12:
                {
                    _3967 = true;
                    _3968 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _3967 = true;
                    _3968 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _3967 = true;
                    _3968 = _3694;
                    break;
                }
                case 15:
                {
                    _3967 = true;
                    _3968 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3967)
            {
                break;
            }
            _3967 = true;
            _3968 = float3(0.0);
            break;
        } while(false);
        _4014 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaA)
            {
                case 0:
                {
                    _4014 = true;
                    _4015 = _5020.w;
                    break;
                }
                case 1:
                {
                    _4014 = true;
                    _4015 = _5021.w;
                    break;
                }
                case 2:
                {
                    _4014 = true;
                    _4015 = _5022.w;
                    break;
                }
                case 3:
                {
                    _4014 = true;
                    _4015 = _5023.w;
                    break;
                }
                case 4:
                {
                    _4014 = true;
                    _4015 = _3589.w;
                    break;
                }
                case 5:
                {
                    _4014 = true;
                    _4015 = _3350.w;
                    break;
                }
                case 6:
                {
                    _4014 = true;
                    _4015 = _3769;
                    break;
                }
                case 7:
                {
                    _4014 = true;
                    _4015 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4014)
            {
                break;
            }
            _4014 = true;
            _4015 = 0.0;
            break;
        } while(false);
        _4043 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaB)
            {
                case 0:
                {
                    _4043 = true;
                    _4044 = _5020.w;
                    break;
                }
                case 1:
                {
                    _4043 = true;
                    _4044 = _5021.w;
                    break;
                }
                case 2:
                {
                    _4043 = true;
                    _4044 = _5022.w;
                    break;
                }
                case 3:
                {
                    _4043 = true;
                    _4044 = _5023.w;
                    break;
                }
                case 4:
                {
                    _4043 = true;
                    _4044 = _3589.w;
                    break;
                }
                case 5:
                {
                    _4043 = true;
                    _4044 = _3350.w;
                    break;
                }
                case 6:
                {
                    _4043 = true;
                    _4044 = _3769;
                    break;
                }
                case 7:
                {
                    _4043 = true;
                    _4044 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4043)
            {
                break;
            }
            _4043 = true;
            _4044 = 0.0;
            break;
        } while(false);
        _4072 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaC)
            {
                case 0:
                {
                    _4072 = true;
                    _4073 = _5020.w;
                    break;
                }
                case 1:
                {
                    _4072 = true;
                    _4073 = _5021.w;
                    break;
                }
                case 2:
                {
                    _4072 = true;
                    _4073 = _5022.w;
                    break;
                }
                case 3:
                {
                    _4072 = true;
                    _4073 = _5023.w;
                    break;
                }
                case 4:
                {
                    _4072 = true;
                    _4073 = _3589.w;
                    break;
                }
                case 5:
                {
                    _4072 = true;
                    _4073 = _3350.w;
                    break;
                }
                case 6:
                {
                    _4072 = true;
                    _4073 = _3769;
                    break;
                }
                case 7:
                {
                    _4072 = true;
                    _4073 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4072)
            {
                break;
            }
            _4072 = true;
            _4073 = 0.0;
            break;
        } while(false);
        _4101 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaD)
            {
                case 0:
                {
                    _4101 = true;
                    _4102 = _5020.w;
                    break;
                }
                case 1:
                {
                    _4101 = true;
                    _4102 = _5021.w;
                    break;
                }
                case 2:
                {
                    _4101 = true;
                    _4102 = _5022.w;
                    break;
                }
                case 3:
                {
                    _4101 = true;
                    _4102 = _5023.w;
                    break;
                }
                case 4:
                {
                    _4101 = true;
                    _4102 = _3589.w;
                    break;
                }
                case 5:
                {
                    _4101 = true;
                    _4102 = _3350.w;
                    break;
                }
                case 6:
                {
                    _4101 = true;
                    _4102 = _3769;
                    break;
                }
                case 7:
                {
                    _4101 = true;
                    _4102 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4101)
            {
                break;
            }
            _4101 = true;
            _4102 = 0.0;
            break;
        } while(false);
        do
        {
            if (uniforms.tevStages.data[i].colorBias == 3)
            {
                if (uniforms.tevStages.data[i].colorScale == 3)
                {
                    uint _4269 = uint(round(fast::clamp(_3827.x, 0.0, 1.0) * 255.0));
                    uint _4275 = uint(round(fast::clamp(_3827.y, 0.0, 1.0) * 255.0));
                    uint _4281 = uint(round(fast::clamp(_3827.z, 0.0, 1.0) * 255.0));
                    uint _4287 = uint(round(fast::clamp(_3874.x, 0.0, 1.0) * 255.0));
                    uint _4293 = uint(round(fast::clamp(_3874.y, 0.0, 1.0) * 255.0));
                    uint _4299 = uint(round(fast::clamp(_3874.z, 0.0, 1.0) * 255.0));
                    if (uniforms.tevStages.data[i].colorOp != 0)
                    {
                        if (_4269 == _4287)
                        {
                            _4133 = 1.0;
                        }
                        else
                        {
                            _4133 = 0.0;
                        }
                        if (_4275 == _4293)
                        {
                            _4134 = 1.0;
                        }
                        else
                        {
                            _4134 = 0.0;
                        }
                        if (_4281 == _4299)
                        {
                            _4135 = 1.0;
                        }
                        else
                        {
                            _4135 = 0.0;
                        }
                        _4132 = float3(_4133, _4134, _4135);
                    }
                    else
                    {
                        if (_4269 > _4287)
                        {
                            _4133 = 1.0;
                        }
                        else
                        {
                            _4133 = 0.0;
                        }
                        if (_4275 > _4293)
                        {
                            _4134 = 1.0;
                        }
                        else
                        {
                            _4134 = 0.0;
                        }
                        if (_4281 > _4299)
                        {
                            _4135 = 1.0;
                        }
                        else
                        {
                            _4135 = 0.0;
                        }
                        _4132 = float3(_4133, _4134, _4135);
                    }
                    float3 _4197 = _3968 + (_3921 * _4132);
                    if (uniforms.tevStages.data[i].colorClamp != 0)
                    {
                        _4136 = fast::clamp(_4197, float3(0.0), float3(1.0));
                    }
                    else
                    {
                        _4136 = _4197;
                    }
                    _4131 = _4136;
                    break;
                }
                do
                {
                    uint _4368 = uint(round(fast::clamp(_3827.x, 0.0, 1.0) * 255.0));
                    uint _4374 = uint(round(fast::clamp(_3827.y, 0.0, 1.0) * 255.0));
                    uint _4386 = uint(round(fast::clamp(_3874.x, 0.0, 1.0) * 255.0));
                    uint _4392 = uint(round(fast::clamp(_3874.y, 0.0, 1.0) * 255.0));
                    bool _4321 = uniforms.tevStages.data[i].colorOp != 0;
                    if (uniforms.tevStages.data[i].colorScale == 0)
                    {
                        if (_4321)
                        {
                            _4305 = _4368 == _4386;
                        }
                        else
                        {
                            _4305 = _4368 > _4386;
                        }
                        _4304 = _4305;
                        break;
                    }
                    if (uniforms.tevStages.data[i].colorScale == 1)
                    {
                        uint _4334 = (_4374 << uint(8)) | _4368;
                        uint _4336 = (_4392 << uint(8)) | _4386;
                        if (_4321)
                        {
                            _4305 = _4334 == _4336;
                        }
                        else
                        {
                            _4305 = _4334 > _4336;
                        }
                        _4304 = _4305;
                        break;
                    }
                    if (uniforms.tevStages.data[i].colorScale == 2)
                    {
                        uint _4349 = ((uint(round(fast::clamp(_3827.z, 0.0, 1.0) * 255.0)) << uint(16)) | (_4374 << uint(8))) | _4368;
                        uint _4353 = ((uint(round(fast::clamp(_3874.z, 0.0, 1.0) * 255.0)) << uint(16)) | (_4392 << uint(8))) | _4386;
                        if (_4321)
                        {
                            _4305 = _4349 == _4353;
                        }
                        else
                        {
                            _4305 = _4349 > _4353;
                        }
                        _4304 = _4305;
                        break;
                    }
                    _4304 = false;
                    break;
                } while(false);
                if (_4304)
                {
                    _4132 = _3921;
                }
                else
                {
                    _4132 = float3(0.0);
                }
                float3 _4211 = _4132;
                float3 _4212 = _3968 + _4211;
                if (uniforms.tevStages.data[i].colorClamp != 0)
                {
                    _4132 = fast::clamp(_4212, float3(0.0), float3(1.0));
                }
                else
                {
                    _4132 = _4212;
                }
                _4131 = _4132;
                break;
            }
            float3 _4225 = (_3827 * (float3(1.0) - _3921)) + (_3874 * _3921);
            if (uniforms.tevStages.data[i].colorOp == 0)
            {
                _4132 = _3968 + _4225;
            }
            else
            {
                _4132 = _3968 - _4225;
            }
            switch (uniforms.tevStages.data[i].colorBias)
            {
                case 1:
                {
                    _4132 += float3(0.5);
                    break;
                }
                case 2:
                {
                    _4132 -= float3(0.5);
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
                    _4132 *= 2.0;
                    break;
                }
                case 2:
                {
                    _4132 *= 4.0;
                    break;
                }
                case 3:
                {
                    _4132 *= 0.5;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (uniforms.tevStages.data[i].colorClamp != 0)
            {
                _4132 = fast::clamp(_4132, float3(0.0), float3(1.0));
            }
            _4131 = _4132;
            break;
        } while(false);
        do
        {
            if (uniforms.tevStages.data[i].alphaBias == 3)
            {
                uint _4483 = uint(round(fast::clamp(_4015, 0.0, 1.0) * 255.0));
                uint _4489 = uint(round(fast::clamp(_4044, 0.0, 1.0) * 255.0));
                if (uniforms.tevStages.data[i].alphaOp != 0)
                {
                    _4408 = _4483 == _4489;
                }
                else
                {
                    _4408 = _4483 > _4489;
                }
                if (_4408)
                {
                    _4407 = _4073;
                }
                else
                {
                    _4407 = 0.0;
                }
                float _4428 = _4407;
                float _4429 = _4102 + _4428;
                if (uniforms.tevStages.data[i].alphaClamp != 0)
                {
                    _4407 = fast::clamp(_4429, 0.0, 1.0);
                }
                else
                {
                    _4407 = _4429;
                }
                _4406 = _4407;
                break;
            }
            float _4441 = (_4015 * (1.0 - _4073)) + (_4044 * _4073);
            if (uniforms.tevStages.data[i].alphaOp == 0)
            {
                _4407 = _4102 + _4441;
            }
            else
            {
                _4407 = _4102 - _4441;
            }
            switch (uniforms.tevStages.data[i].alphaBias)
            {
                case 1:
                {
                    _4407 += 0.5;
                    break;
                }
                case 2:
                {
                    _4407 -= 0.5;
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
                    _4407 *= 2.0;
                    break;
                }
                case 2:
                {
                    _4407 *= 4.0;
                    break;
                }
                case 3:
                {
                    _4407 *= 0.5;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (uniforms.tevStages.data[i].alphaClamp != 0)
            {
                _4407 = fast::clamp(_4407, 0.0, 1.0);
            }
            _4406 = _4407;
            break;
        } while(false);
        switch (uniforms.tevStages.data[i].colorOutput)
        {
            case 0:
            {
                float4 _5123 = _5020;
                _5123.x = _4131.x;
                _5123.y = _4131.y;
                _5123.z = _4131.z;
                _5020 = _5123;
                break;
            }
            case 1:
            {
                float4 _5117 = _5021;
                _5117.x = _4131.x;
                _5117.y = _4131.y;
                _5117.z = _4131.z;
                _5021 = _5117;
                break;
            }
            case 2:
            {
                float4 _5111 = _5022;
                _5111.x = _4131.x;
                _5111.y = _4131.y;
                _5111.z = _4131.z;
                _5022 = _5111;
                break;
            }
            case 3:
            {
                float4 _5105 = _5023;
                _5105.x = _4131.x;
                _5105.y = _4131.y;
                _5105.z = _4131.z;
                _5023 = _5105;
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
                float4 _5135 = _5020;
                _5135.w = _4406;
                _5020 = _5135;
                break;
            }
            case 1:
            {
                float4 _5133 = _5021;
                _5133.w = _4406;
                _5021 = _5133;
                break;
            }
            case 2:
            {
                float4 _5131 = _5022;
                _5131.w = _4406;
                _5022 = _5131;
                break;
            }
            case 3:
            {
                float4 _5129 = _5023;
                _5129.w = _4406;
                _5023 = _5129;
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
    float4 color = _5020;
    _4540 = false;
    bool _4541;
    do
    {
        _4578 = false;
        bool _4579;
        do
        {
            uint _4610 = uint(round(fast::clamp(_5020.w, 0.0, 1.0) * 255.0));
            uint _4616 = uint(round(fast::clamp(uniforms.alphaRef0, 0.0, 1.0) * 255.0));
            switch (uniforms.alphaComp0)
            {
                case 0:
                {
                    _4578 = true;
                    _4579 = false;
                    break;
                }
                case 1:
                {
                    _4578 = true;
                    _4579 = _4610 < _4616;
                    break;
                }
                case 2:
                {
                    _4578 = true;
                    _4579 = _4610 == _4616;
                    break;
                }
                case 3:
                {
                    _4578 = true;
                    _4579 = _4610 <= _4616;
                    break;
                }
                case 4:
                {
                    _4578 = true;
                    _4579 = _4610 > _4616;
                    break;
                }
                case 5:
                {
                    _4578 = true;
                    _4579 = _4610 != _4616;
                    break;
                }
                case 6:
                {
                    _4578 = true;
                    _4579 = _4610 >= _4616;
                    break;
                }
                case 7:
                {
                    _4578 = true;
                    _4579 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4578)
            {
                break;
            }
            _4578 = true;
            _4579 = true;
            break;
        } while(false);
        _4617 = false;
        bool _4618;
        do
        {
            uint _4649 = uint(round(fast::clamp(_5020.w, 0.0, 1.0) * 255.0));
            uint _4655 = uint(round(fast::clamp(uniforms.alphaRef1, 0.0, 1.0) * 255.0));
            switch (uniforms.alphaComp1)
            {
                case 0:
                {
                    _4617 = true;
                    _4618 = false;
                    break;
                }
                case 1:
                {
                    _4617 = true;
                    _4618 = _4649 < _4655;
                    break;
                }
                case 2:
                {
                    _4617 = true;
                    _4618 = _4649 == _4655;
                    break;
                }
                case 3:
                {
                    _4617 = true;
                    _4618 = _4649 <= _4655;
                    break;
                }
                case 4:
                {
                    _4617 = true;
                    _4618 = _4649 > _4655;
                    break;
                }
                case 5:
                {
                    _4617 = true;
                    _4618 = _4649 != _4655;
                    break;
                }
                case 6:
                {
                    _4617 = true;
                    _4618 = _4649 >= _4655;
                    break;
                }
                case 7:
                {
                    _4617 = true;
                    _4618 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4617)
            {
                break;
            }
            _4617 = true;
            _4618 = true;
            break;
        } while(false);
        bool _4542;
        switch (uniforms.alphaLogic)
        {
            case 0:
            {
                if (_4579)
                {
                    _4542 = _4618;
                }
                else
                {
                    _4542 = false;
                }
                _4540 = true;
                _4541 = _4542;
                break;
            }
            case 1:
            {
                if (_4579)
                {
                    _4542 = true;
                }
                else
                {
                    _4542 = _4618;
                }
                _4540 = true;
                _4541 = _4542;
                break;
            }
            case 2:
            {
                _4540 = true;
                _4541 = _4579 != _4618;
                break;
            }
            case 3:
            {
                _4540 = true;
                _4541 = _4579 == _4618;
                break;
            }
            default:
            {
                break;
            }
        }
        if (_4540)
        {
            break;
        }
        _4540 = true;
        _4541 = true;
        break;
    } while(false);
    if (!_4541)
    {
        discard_fragment();
    }
    if (uniforms.destinationAlphaEnabled != 0)
    {
        float4 _5137 = color;
        _5137.w = uniforms.destinationAlpha;
        color = _5137;
    }
    if (uniforms.enableDither != 0)
    {
        float4 _286 = color;
        uint3 _4670 = uint3(round(fast::clamp(_286.xyz, float3(0.0), float3(1.0)) * 255.0));
        float3 _4681 = float3(min(((_4670 - (_4670 >> uint3(6u))) + uint3(_324[(int(gl_FragCoord.x) & 1) | ((int(gl_FragCoord.y) & 1) << 1)])), uint3(255u))) * float3(0.0039215688593685626983642578125);
        float4 _5140 = _286;
        _5140.x = _4681.x;
        _5140.y = _4681.y;
        _5140.z = _4681.z;
        color = _5140;
    }
    float depth;
    if (uniforms.zTextureOp != 0)
    {
        _4685 = false;
        uint _4686;
        do
        {
            uint4 _4693 = uint4(round(fast::clamp(lastTexture, float4(0.0), float4(1.0)) * 255.0));
            switch (uniforms.zTextureFormat)
            {
                case 0:
                {
                    _4685 = true;
                    _4686 = _4693.w;
                    break;
                }
                case 1:
                {
                    _4685 = true;
                    _4686 = _4693.x | (_4693.w << 8u);
                    break;
                }
                case 2:
                {
                    _4685 = true;
                    _4686 = ((_4693.x << 16u) | (_4693.y << 8u)) | _4693.z;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4685)
            {
                break;
            }
            _4685 = true;
            _4686 = 0u;
            break;
        } while(false);
        uint bias = uint(uniforms.zTextureBias);
        uint finalZ;
        if (uniforms.zTextureOp == 1)
        {
            finalZ = (uint(fast::clamp(gl_FragCoord.z, 0.0, 1.0) * 16777215.0) + _4686) + bias;
        }
        else
        {
            finalZ = _4686 + bias;
        }
        depth = float(finalZ & 16777215u) * 5.9604651880817982601001858711243e-08;
    }
    else
    {
        depth = gl_FragCoord.z;
    }
    float _4719;
    do
    {
        if (uniforms.fogType == 0)
        {
            _4719 = 0.0;
            break;
        }
        uint _4734 = min(uint(round(fast::clamp(depth, 0.0, 1.0) * 16777216.0)), 16777215u);
        float _4720;
        if ((uniforms.fogType & 8) != 0)
        {
            _4720 = (uniforms.fogA * float(_4734)) * 5.9604644775390625e-08;
        }
        else
        {
            int _4745 = uniforms.fogBMagnitude - int(_4734 >> uint(uniforms.fogBShift));
            if (_4745 == 0)
            {
                if (uniforms.fogA < 0.0)
                {
                    _4720 = -1000000015047466219876688855040.0;
                }
                else
                {
                    _4720 = 1000000015047466219876688855040.0;
                }
            }
            else
            {
                _4720 = (uniforms.fogA * 16777216.0) / float(_4745);
            }
        }
        if (uniforms.fogRangeEnabled != 0)
        {
            float _4780 = (((2.0 * gl_FragCoord.x) / uniforms.fogViewportWidth) - 1.0) - uniforms.fogRangeCenter;
            float _4784 = fast::clamp(9.0 - (abs(_4780) * 9.0), 0.0, 9.0);
            uint _4785 = uint(_4784);
            float _4795 = mix(uniforms.fogRangeK.data[_4785].x, uniforms.fogRangeK.data[min((_4785 + 1u), 9u)].x, fract(_4784));
            _4720 *= (sqrt((_4780 * _4780) + (_4795 * _4795)) / fast::max(_4795, 9.9999999747524270787835121154785e-07));
        }
        float _4809 = fast::clamp(_4720 - uniforms.fogC, 0.0, 1.0);
        float _4721;
        switch (uniforms.fogType & 7)
        {
            case 2:
            {
                _4721 = _4809;
                break;
            }
            case 4:
            {
                _4721 = 1.0 - exp2((-8.0) * _4809);
                break;
            }
            case 5:
            {
                _4721 = 1.0 - exp2(((-8.0) * _4809) * _4809);
                break;
            }
            case 6:
            {
                _4721 = exp2((-8.0) * (1.0 - _4809));
                break;
            }
            case 7:
            {
                float _4813 = 1.0 - _4809;
                _4721 = exp2(((-8.0) * _4813) * _4813);
                break;
            }
            default:
            {
                _4721 = _4809;
                break;
            }
        }
        _4719 = fast::clamp(_4721, 0.0, 1.0);
        break;
    } while(false);
    float4 _602 = color;
    float3 _609 = mix(_602.xyz, float3(uniforms.fogColor), float3(_4719));
    float4 _5146 = _602;
    _5146.x = _609.x;
    _5146.y = _609.y;
    _5146.z = _609.z;
    color = _5146;
    float4 _4838;
    do
    {
        float4 _4839 = _5146;
        if (uniforms.efbPixelFormat == 1)
        {
            _4838 = float4(uint4(round(fast::clamp(_4839, float4(0.0), float4(1.0)) * 63.0))) * float4(0.01587301678955554962158203125);
            break;
        }
        if (uniforms.efbPixelFormat == 2)
        {
            _4838 = float4(float(uint(round(fast::clamp(_4839.x, 0.0, 1.0) * 31.0))) * 0.0322580635547637939453125, float(uint(round(fast::clamp(_4839.y, 0.0, 1.0) * 63.0))) * 0.01587301678955554962158203125, float(uint(round(fast::clamp(_4839.z, 0.0, 1.0) * 31.0))) * 0.0322580635547637939453125, 1.0);
            break;
        }
        if (uniforms.efbPixelFormat == 0)
        {
            float4 _5151 = _4839;
            _5151.w = 1.0;
            _4839 = _5151;
        }
        _4838 = _4839;
        break;
    } while(false);
    color = _4838;
    out.entryPointParam_fragmentMain_color = _4838;
    out.gl_FragDepth = depth;
    return out;
}

)SHDR3";

#endif
