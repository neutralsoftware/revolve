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

constant spvUnsafeArray<uint, 4> _319 = spvUnsafeArray<uint, 4>({ 0u, 2u, 3u, 1u });

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
    bool _4656 = false;
    bool _4588 = false;
    bool _4549 = false;
    bool _4511 = false;
    bool _4071 = false;
    bool _4042 = false;
    bool _4013 = false;
    bool _3984 = false;
    bool _3937 = false;
    bool _3890 = false;
    bool _3843 = false;
    bool _3796 = false;
    bool _3773 = false;
    bool _3738 = false;
    bool _3715 = false;
    bool _3663 = false;
    bool _3644 = false;
    bool _3625 = false;
    bool _3606 = false;
    bool _3587 = false;
    bool _3560 = false;
    bool _3489 = false;
    bool _3424 = false;
    bool _3405 = false;
    bool _3386 = false;
    bool _3367 = false;
    bool _3348 = false;
    bool _3321 = false;
    bool _3285 = false;
    bool _3227 = false;
    bool _3169 = false;
    bool _2829 = false;
    bool _2792 = false;
    bool _2727 = false;
    main0_out out = {};
    float4 _4872 = uniforms.tevRegister0;
    float4 _4873 = uniforms.tevRegister1;
    float4 _4874 = uniforms.tevRegister2;
    float4 _4875 = uniforms.tevRegister3;
    float2 indirectCoord = float2(0.0);
    float bumpAlpha = 0.0;
    float4 lastTexture = float4(0.0);
    int i = 0;
    float2 _2560;
    bool _2561;
    float3 _2562;
    float _2564;
    float2 _2565;
    float2 _2566;
    float4 _2567;
    float3 _2728;
    float3 _2793;
    float4 _2830;
    float3 _2895;
    float _2920;
    float2 _2953;
    bool _2954;
    float4 _3018;
    float4 _3019;
    float4 _3056;
    float4 _3057;
    float4 _3094;
    float4 _3095;
    float4 _3132;
    float4 _3133;
    float _3170;
    float _3228;
    float4 _3286;
    int4 _3322;
    float _3349;
    float _3368;
    float _3387;
    float _3406;
    float4 _3425;
    float4 _3490;
    int4 _3561;
    float _3588;
    float _3607;
    float _3626;
    float _3645;
    float3 _3664;
    float _3665;
    float4 _3716;
    float _3739;
    float4 _3774;
    float3 _3797;
    float3 _3844;
    float3 _3891;
    float3 _3938;
    float _3985;
    float _4014;
    float _4043;
    float _4072;
    float3 _4101;
    float3 _4102;
    float _4103;
    float _4104;
    float _4105;
    float3 _4106;
    bool _4275;
    bool _4276;
    float _4377;
    float _4378;
    bool _4379;
    for (;;)
    {
        if (!(i < uniforms.tevStageCount))
        {
            break;
        }
        _2727 = false;
        do
        {
            switch (uniforms.tevStages.data[i].texCoord)
            {
                case 0:
                {
                    _2727 = true;
                    _2728 = in.input_uv0;
                    break;
                }
                case 1:
                {
                    _2727 = true;
                    _2728 = in.input_uv1;
                    break;
                }
                case 2:
                {
                    _2727 = true;
                    _2728 = in.input_uv2;
                    break;
                }
                case 3:
                {
                    _2727 = true;
                    _2728 = in.input_uv3;
                    break;
                }
                case 4:
                {
                    _2727 = true;
                    _2728 = in.input_uv4;
                    break;
                }
                case 5:
                {
                    _2727 = true;
                    _2728 = in.input_uv5;
                    break;
                }
                case 6:
                {
                    _2727 = true;
                    _2728 = in.input_uv6;
                    break;
                }
                case 7:
                {
                    _2727 = true;
                    _2728 = in.input_uv7;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2727)
            {
                break;
            }
            _2727 = true;
            _2728 = float3(0.0);
            break;
        } while(false);
        if (_2728.z != 0.0)
        {
            _2560 = _2728.xy / float2(_2728.z);
        }
        else
        {
            _2560 = _2728.xy;
        }
        bool _2587 = uniforms.tevStages.data[i].indirectStage < uniforms.indirectStageCount;
        if (_2587)
        {
            if (uniforms.tevStages.data[i].indirectMatrix != 0)
            {
                _2561 = true;
            }
            else
            {
                _2561 = uniforms.tevStages.data[i].indirectAlpha != 0;
            }
        }
        else
        {
            _2561 = false;
        }
        if (_2561)
        {
            _2792 = false;
            do
            {
                switch (uniforms.indirectStages.data[uniforms.tevStages.data[i].indirectStage].texCoord)
                {
                    case 0:
                    {
                        _2792 = true;
                        _2793 = in.input_uv0;
                        break;
                    }
                    case 1:
                    {
                        _2792 = true;
                        _2793 = in.input_uv1;
                        break;
                    }
                    case 2:
                    {
                        _2792 = true;
                        _2793 = in.input_uv2;
                        break;
                    }
                    case 3:
                    {
                        _2792 = true;
                        _2793 = in.input_uv3;
                        break;
                    }
                    case 4:
                    {
                        _2792 = true;
                        _2793 = in.input_uv4;
                        break;
                    }
                    case 5:
                    {
                        _2792 = true;
                        _2793 = in.input_uv5;
                        break;
                    }
                    case 6:
                    {
                        _2792 = true;
                        _2793 = in.input_uv6;
                        break;
                    }
                    case 7:
                    {
                        _2792 = true;
                        _2793 = in.input_uv7;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_2792)
                {
                    break;
                }
                _2792 = true;
                _2793 = float3(0.0);
                break;
            } while(false);
            float2 _2754 = _2793.xy;
            if (_2793.z != 0.0)
            {
                _2754 /= float2(_2793.z);
            }
            float2 _4938 = _2754;
            _4938.x = _4938.x * exp2(-float(uniforms.indirectStages.data[uniforms.tevStages.data[i].indirectStage].scaleS));
            _4938.y = _4938.y * exp2(-float(uniforms.indirectStages.data[uniforms.tevStages.data[i].indirectStage].scaleT));
            _2754 = _4938;
            _2829 = false;
            do
            {
                switch (uniforms.indirectStages.data[uniforms.tevStages.data[i].indirectStage].texMap)
                {
                    case 0:
                    {
                        _2829 = true;
                        _2830 = tex0.sample(samp0, _4938);
                        break;
                    }
                    case 1:
                    {
                        _2829 = true;
                        _2830 = tex1.sample(samp1, _4938);
                        break;
                    }
                    case 2:
                    {
                        _2829 = true;
                        _2830 = tex2.sample(samp2, _4938);
                        break;
                    }
                    case 3:
                    {
                        _2829 = true;
                        _2830 = tex3.sample(samp3, _4938);
                        break;
                    }
                    case 4:
                    {
                        _2829 = true;
                        _2830 = tex4.sample(samp4, _4938);
                        break;
                    }
                    case 5:
                    {
                        _2829 = true;
                        _2830 = tex5.sample(samp5, _4938);
                        break;
                    }
                    case 6:
                    {
                        _2829 = true;
                        _2830 = tex6.sample(samp6, _4938);
                        break;
                    }
                    case 7:
                    {
                        _2829 = true;
                        _2830 = tex7.sample(samp7, _4938);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_2829)
                {
                    break;
                }
                _2829 = true;
                _2830 = float4(1.0);
                break;
            } while(false);
            _2562 = floor((_2830.wzy * 255.0) + float3(0.5));
        }
        else
        {
            _2562 = float3(0.0);
        }
        if (_2587)
        {
            _2561 = uniforms.tevStages.data[i].indirectAlpha != 0;
        }
        else
        {
            _2561 = false;
        }
        if (_2561)
        {
            float3 _2563 = _2562;
            if (uniforms.tevStages.data[i].indirectFormat == 0)
            {
                _2564 = 0.0;
            }
            else
            {
                _2564 = 6.0 - float(uniforms.tevStages.data[i].indirectFormat);
            }
            float _2624 = _2563[uniforms.tevStages.data[i].indirectAlpha - 1] * exp2(_2564);
            bumpAlpha = floor((_2624 - (256.0 * floor(_2624 * 0.00390625))) * 0.125) * 0.0313725508749485015869140625;
        }
        do
        {
            if (uniforms.tevStages.data[i].indirectFormat == 0)
            {
                _2895 = _2562;
                break;
            }
            if (uniforms.tevStages.data[i].indirectFormat == 1)
            {
                _2895 = floor(_2562 * float3(0.125));
                break;
            }
            if (uniforms.tevStages.data[i].indirectFormat == 2)
            {
                _2895 = floor(_2562 * float3(0.0625));
                break;
            }
            _2895 = floor(_2562 * float3(0.03125));
            break;
        } while(false);
        float3 _2919 = _2895;
        if (uniforms.tevStages.data[i].indirectFormat == 0)
        {
            _2920 = -128.0;
        }
        else
        {
            _2920 = 1.0;
        }
        if ((uniforms.tevStages.data[i].indirectBias & 1) != 0)
        {
            float3 _4944 = _2919;
            _4944.x = _4944.x + _2920;
            _2919 = _4944;
        }
        if ((uniforms.tevStages.data[i].indirectBias & 2) != 0)
        {
            float3 _4947 = _2919;
            _4947.y = _4947.y + _2920;
            _2919 = _4947;
        }
        if ((uniforms.tevStages.data[i].indirectBias & 4) != 0)
        {
            float3 _4950 = _2919;
            _4950.z = _4950.z + _2920;
            _2919 = _4950;
        }
        if (_2587)
        {
            do
            {
                if (uniforms.tevStages.data[i].indirectMatrix == 0)
                {
                    _2953 = float2(0.0);
                    break;
                }
                if (uniforms.tevStages.data[i].indirectMatrix >= 1)
                {
                    _2954 = uniforms.tevStages.data[i].indirectMatrix <= 3;
                }
                else
                {
                    _2954 = false;
                }
                if (_2954)
                {
                    int _2968 = uniforms.tevStages.data[i].indirectMatrix - 1;
                    do
                    {
                        if (_2968 == 0)
                        {
                            if (false)
                            {
                                _3019 = uniforms.indirectMatrix0B;
                            }
                            else
                            {
                                _3019 = uniforms.indirectMatrix0A;
                            }
                            _3018 = _3019;
                            break;
                        }
                        if (_2968 == 1)
                        {
                            if (false)
                            {
                                _3019 = uniforms.indirectMatrix1B;
                            }
                            else
                            {
                                _3019 = uniforms.indirectMatrix1A;
                            }
                            _3018 = _3019;
                            break;
                        }
                        if (false)
                        {
                            _3019 = uniforms.indirectMatrix2B;
                        }
                        else
                        {
                            _3019 = uniforms.indirectMatrix2A;
                        }
                        _3018 = _3019;
                        break;
                    } while(false);
                    do
                    {
                        if (_2968 == 0)
                        {
                            if (true)
                            {
                                _3057 = uniforms.indirectMatrix0B;
                            }
                            else
                            {
                                _3057 = uniforms.indirectMatrix0A;
                            }
                            _3056 = _3057;
                            break;
                        }
                        if (_2968 == 1)
                        {
                            if (true)
                            {
                                _3057 = uniforms.indirectMatrix1B;
                            }
                            else
                            {
                                _3057 = uniforms.indirectMatrix1A;
                            }
                            _3056 = _3057;
                            break;
                        }
                        if (true)
                        {
                            _3057 = uniforms.indirectMatrix2B;
                        }
                        else
                        {
                            _3057 = uniforms.indirectMatrix2A;
                        }
                        _3056 = _3057;
                        break;
                    } while(false);
                    _2953 = (float2(dot(_3018.xyz, _2919), dot(_3056.xyz, _2919)) * exp2(_3018.w)) * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].zw;
                    break;
                }
                if (uniforms.tevStages.data[i].indirectMatrix >= 5)
                {
                    _2954 = uniforms.tevStages.data[i].indirectMatrix <= 7;
                }
                else
                {
                    _2954 = false;
                }
                if (_2954)
                {
                    int _2988 = uniforms.tevStages.data[i].indirectMatrix - 5;
                    do
                    {
                        if (_2988 == 0)
                        {
                            if (false)
                            {
                                _3095 = uniforms.indirectMatrix0B;
                            }
                            else
                            {
                                _3095 = uniforms.indirectMatrix0A;
                            }
                            _3094 = _3095;
                            break;
                        }
                        if (_2988 == 1)
                        {
                            if (false)
                            {
                                _3095 = uniforms.indirectMatrix1B;
                            }
                            else
                            {
                                _3095 = uniforms.indirectMatrix1A;
                            }
                            _3094 = _3095;
                            break;
                        }
                        if (false)
                        {
                            _3095 = uniforms.indirectMatrix2B;
                        }
                        else
                        {
                            _3095 = uniforms.indirectMatrix2A;
                        }
                        _3094 = _3095;
                        break;
                    } while(false);
                    _2953 = ((_2560 * _2919.x) * exp2(_3094.w)) * float2(0.00390625);
                    break;
                }
                if (uniforms.tevStages.data[i].indirectMatrix >= 9)
                {
                    _2954 = uniforms.tevStages.data[i].indirectMatrix <= 11;
                }
                else
                {
                    _2954 = false;
                }
                if (_2954)
                {
                    int _3005 = uniforms.tevStages.data[i].indirectMatrix - 9;
                    do
                    {
                        if (_3005 == 0)
                        {
                            if (false)
                            {
                                _3133 = uniforms.indirectMatrix0B;
                            }
                            else
                            {
                                _3133 = uniforms.indirectMatrix0A;
                            }
                            _3132 = _3133;
                            break;
                        }
                        if (_3005 == 1)
                        {
                            if (false)
                            {
                                _3133 = uniforms.indirectMatrix1B;
                            }
                            else
                            {
                                _3133 = uniforms.indirectMatrix1A;
                            }
                            _3132 = _3133;
                            break;
                        }
                        if (false)
                        {
                            _3133 = uniforms.indirectMatrix2B;
                        }
                        else
                        {
                            _3133 = uniforms.indirectMatrix2A;
                        }
                        _3132 = _3133;
                        break;
                    } while(false);
                    _2953 = ((_2560 * _2919.y) * exp2(_3132.w)) * float2(0.00390625);
                    break;
                }
                _2953 = float2(0.0);
                break;
            } while(false);
            _2565 = _2953;
        }
        else
        {
            _2565 = float2(0.0);
        }
        _3169 = false;
        do
        {
            switch (uniforms.tevStages.data[i].indirectWrapS)
            {
                case 0:
                {
                    _3169 = true;
                    _3170 = _2560.x;
                    break;
                }
                case 1:
                {
                    float _3188 = 256.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _3169 = true;
                    _3170 = _2560.x - (_3188 * floor(_2560.x / _3188));
                    break;
                }
                case 2:
                {
                    float _3185 = 128.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _3169 = true;
                    _3170 = _2560.x - (_3185 * floor(_2560.x / _3185));
                    break;
                }
                case 3:
                {
                    float _3182 = 64.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _3169 = true;
                    _3170 = _2560.x - (_3182 * floor(_2560.x / _3182));
                    break;
                }
                case 4:
                {
                    float _3179 = 32.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _3169 = true;
                    _3170 = _2560.x - (_3179 * floor(_2560.x / _3179));
                    break;
                }
                case 5:
                {
                    float _3176 = 16.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _3169 = true;
                    _3170 = _2560.x - (_3176 * floor(_2560.x / _3176));
                    break;
                }
                case 6:
                {
                    _3169 = true;
                    _3170 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3169)
            {
                break;
            }
            _3169 = true;
            _3170 = _2560.x;
            break;
        } while(false);
        float2 _4953 = _2566;
        _4953.x = _3170;
        _2566 = _4953;
        _3227 = false;
        do
        {
            switch (uniforms.tevStages.data[i].indirectWrapT)
            {
                case 0:
                {
                    _3227 = true;
                    _3228 = _2560.y;
                    break;
                }
                case 1:
                {
                    float _3246 = 256.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _3227 = true;
                    _3228 = _2560.y - (_3246 * floor(_2560.y / _3246));
                    break;
                }
                case 2:
                {
                    float _3243 = 128.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _3227 = true;
                    _3228 = _2560.y - (_3243 * floor(_2560.y / _3243));
                    break;
                }
                case 3:
                {
                    float _3240 = 64.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _3227 = true;
                    _3228 = _2560.y - (_3240 * floor(_2560.y / _3240));
                    break;
                }
                case 4:
                {
                    float _3237 = 32.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _3227 = true;
                    _3228 = _2560.y - (_3237 * floor(_2560.y / _3237));
                    break;
                }
                case 5:
                {
                    float _3234 = 16.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _3227 = true;
                    _3228 = _2560.y - (_3234 * floor(_2560.y / _3234));
                    break;
                }
                case 6:
                {
                    _3227 = true;
                    _3228 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3227)
            {
                break;
            }
            _3227 = true;
            _3228 = _2560.y;
            break;
        } while(false);
        float2 _4955 = _2566;
        _4955.y = _3228;
        _2566 = _4955 + _2565;
        if (uniforms.tevStages.data[i].indirectAddPrevious != 0)
        {
            _2566 += indirectCoord;
        }
        indirectCoord = _2566;
        _3285 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorChannel)
            {
                case 0:
                {
                    _3285 = true;
                    _3286 = in.input_color0;
                    break;
                }
                case 1:
                {
                    _3285 = true;
                    _3286 = in.input_color1;
                    break;
                }
                case 5:
                {
                    _3285 = true;
                    _3286 = float4(bumpAlpha);
                    break;
                }
                case 6:
                {
                    _3285 = true;
                    _3286 = float4(((bumpAlpha * 255.0) + floor(bumpAlpha * 7.96875)) * 0.0039215688593685626983642578125);
                    break;
                }
                default:
                {
                    _3285 = true;
                    _3286 = float4(0.0);
                    break;
                }
            }
            if (_3285)
            {
                break;
            }
            break;
        } while(false);
        _3321 = false;
        do
        {
            switch (uniforms.tevStages.data[i].rasterSwap)
            {
                case 0:
                {
                    _3321 = true;
                    _3322 = int4(uniforms.swapTable0);
                    break;
                }
                case 1:
                {
                    _3321 = true;
                    _3322 = int4(uniforms.swapTable1);
                    break;
                }
                case 2:
                {
                    _3321 = true;
                    _3322 = int4(uniforms.swapTable2);
                    break;
                }
                case 3:
                {
                    _3321 = true;
                    _3322 = int4(uniforms.swapTable3);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3321)
            {
                break;
            }
            _3321 = true;
            _3322 = int4(0, 1, 2, 3);
            break;
        } while(false);
        _3348 = false;
        do
        {
            switch (_3322.x)
            {
                case 0:
                {
                    _3348 = true;
                    _3349 = _3286.x;
                    break;
                }
                case 1:
                {
                    _3348 = true;
                    _3349 = _3286.y;
                    break;
                }
                case 2:
                {
                    _3348 = true;
                    _3349 = _3286.z;
                    break;
                }
                case 3:
                {
                    _3348 = true;
                    _3349 = _3286.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3348)
            {
                break;
            }
            _3348 = true;
            _3349 = 0.0;
            break;
        } while(false);
        _3367 = false;
        do
        {
            switch (_3322.y)
            {
                case 0:
                {
                    _3367 = true;
                    _3368 = _3286.x;
                    break;
                }
                case 1:
                {
                    _3367 = true;
                    _3368 = _3286.y;
                    break;
                }
                case 2:
                {
                    _3367 = true;
                    _3368 = _3286.z;
                    break;
                }
                case 3:
                {
                    _3367 = true;
                    _3368 = _3286.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3367)
            {
                break;
            }
            _3367 = true;
            _3368 = 0.0;
            break;
        } while(false);
        _3386 = false;
        do
        {
            switch (_3322.z)
            {
                case 0:
                {
                    _3386 = true;
                    _3387 = _3286.x;
                    break;
                }
                case 1:
                {
                    _3386 = true;
                    _3387 = _3286.y;
                    break;
                }
                case 2:
                {
                    _3386 = true;
                    _3387 = _3286.z;
                    break;
                }
                case 3:
                {
                    _3386 = true;
                    _3387 = _3286.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3386)
            {
                break;
            }
            _3386 = true;
            _3387 = 0.0;
            break;
        } while(false);
        _3405 = false;
        do
        {
            switch (_3322.w)
            {
                case 0:
                {
                    _3405 = true;
                    _3406 = _3286.x;
                    break;
                }
                case 1:
                {
                    _3405 = true;
                    _3406 = _3286.y;
                    break;
                }
                case 2:
                {
                    _3405 = true;
                    _3406 = _3286.z;
                    break;
                }
                case 3:
                {
                    _3405 = true;
                    _3406 = _3286.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3405)
            {
                break;
            }
            _3405 = true;
            _3406 = 0.0;
            break;
        } while(false);
        float4 _3320 = float4(_3349, _3368, _3387, _3406);
        if (uniforms.tevStages.data[i].textureEnabled != 0)
        {
            if (uniforms.tevStages.data[i].indirectUseOriginalLod != 0)
            {
                float2 _3485 = dfdx(_2560);
                float2 _3488 = dfdy(_2560);
                _3489 = false;
                do
                {
                    switch (uniforms.tevStages.data[i].texMap)
                    {
                        case 0:
                        {
                            _3489 = true;
                            _3490 = tex0.sample(samp0, _2566, gradient2d(_3485, _3488));
                            break;
                        }
                        case 1:
                        {
                            _3489 = true;
                            _3490 = tex1.sample(samp1, _2566, gradient2d(_3485, _3488));
                            break;
                        }
                        case 2:
                        {
                            _3489 = true;
                            _3490 = tex2.sample(samp2, _2566, gradient2d(_3485, _3488));
                            break;
                        }
                        case 3:
                        {
                            _3489 = true;
                            _3490 = tex3.sample(samp3, _2566, gradient2d(_3485, _3488));
                            break;
                        }
                        case 4:
                        {
                            _3489 = true;
                            _3490 = tex4.sample(samp4, _2566, gradient2d(_3485, _3488));
                            break;
                        }
                        case 5:
                        {
                            _3489 = true;
                            _3490 = tex5.sample(samp5, _2566, gradient2d(_3485, _3488));
                            break;
                        }
                        case 6:
                        {
                            _3489 = true;
                            _3490 = tex6.sample(samp6, _2566, gradient2d(_3485, _3488));
                            break;
                        }
                        case 7:
                        {
                            _3489 = true;
                            _3490 = tex7.sample(samp7, _2566, gradient2d(_3485, _3488));
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                    if (_3489)
                    {
                        break;
                    }
                    _3489 = true;
                    _3490 = float4(1.0);
                    break;
                } while(false);
                _2567 = _3490;
            }
            else
            {
                _3424 = false;
                do
                {
                    switch (uniforms.tevStages.data[i].texMap)
                    {
                        case 0:
                        {
                            _3424 = true;
                            _3425 = tex0.sample(samp0, _2566);
                            break;
                        }
                        case 1:
                        {
                            _3424 = true;
                            _3425 = tex1.sample(samp1, _2566);
                            break;
                        }
                        case 2:
                        {
                            _3424 = true;
                            _3425 = tex2.sample(samp2, _2566);
                            break;
                        }
                        case 3:
                        {
                            _3424 = true;
                            _3425 = tex3.sample(samp3, _2566);
                            break;
                        }
                        case 4:
                        {
                            _3424 = true;
                            _3425 = tex4.sample(samp4, _2566);
                            break;
                        }
                        case 5:
                        {
                            _3424 = true;
                            _3425 = tex5.sample(samp5, _2566);
                            break;
                        }
                        case 6:
                        {
                            _3424 = true;
                            _3425 = tex6.sample(samp6, _2566);
                            break;
                        }
                        case 7:
                        {
                            _3424 = true;
                            _3425 = tex7.sample(samp7, _2566);
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                    if (_3424)
                    {
                        break;
                    }
                    _3424 = true;
                    _3425 = float4(1.0);
                    break;
                } while(false);
                _2567 = _3425;
            }
            lastTexture = _2567;
        }
        else
        {
            _2567 = float4(1.0);
        }
        _3560 = false;
        do
        {
            switch (uniforms.tevStages.data[i].textureSwap)
            {
                case 0:
                {
                    _3560 = true;
                    _3561 = int4(uniforms.swapTable0);
                    break;
                }
                case 1:
                {
                    _3560 = true;
                    _3561 = int4(uniforms.swapTable1);
                    break;
                }
                case 2:
                {
                    _3560 = true;
                    _3561 = int4(uniforms.swapTable2);
                    break;
                }
                case 3:
                {
                    _3560 = true;
                    _3561 = int4(uniforms.swapTable3);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3560)
            {
                break;
            }
            _3560 = true;
            _3561 = int4(0, 1, 2, 3);
            break;
        } while(false);
        _3587 = false;
        do
        {
            switch (_3561.x)
            {
                case 0:
                {
                    _3587 = true;
                    _3588 = _2567.x;
                    break;
                }
                case 1:
                {
                    _3587 = true;
                    _3588 = _2567.y;
                    break;
                }
                case 2:
                {
                    _3587 = true;
                    _3588 = _2567.z;
                    break;
                }
                case 3:
                {
                    _3587 = true;
                    _3588 = _2567.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3587)
            {
                break;
            }
            _3587 = true;
            _3588 = 0.0;
            break;
        } while(false);
        _3606 = false;
        do
        {
            switch (_3561.y)
            {
                case 0:
                {
                    _3606 = true;
                    _3607 = _2567.x;
                    break;
                }
                case 1:
                {
                    _3606 = true;
                    _3607 = _2567.y;
                    break;
                }
                case 2:
                {
                    _3606 = true;
                    _3607 = _2567.z;
                    break;
                }
                case 3:
                {
                    _3606 = true;
                    _3607 = _2567.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3606)
            {
                break;
            }
            _3606 = true;
            _3607 = 0.0;
            break;
        } while(false);
        _3625 = false;
        do
        {
            switch (_3561.z)
            {
                case 0:
                {
                    _3625 = true;
                    _3626 = _2567.x;
                    break;
                }
                case 1:
                {
                    _3625 = true;
                    _3626 = _2567.y;
                    break;
                }
                case 2:
                {
                    _3625 = true;
                    _3626 = _2567.z;
                    break;
                }
                case 3:
                {
                    _3625 = true;
                    _3626 = _2567.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3625)
            {
                break;
            }
            _3625 = true;
            _3626 = 0.0;
            break;
        } while(false);
        _3644 = false;
        do
        {
            switch (_3561.w)
            {
                case 0:
                {
                    _3644 = true;
                    _3645 = _2567.x;
                    break;
                }
                case 1:
                {
                    _3644 = true;
                    _3645 = _2567.y;
                    break;
                }
                case 2:
                {
                    _3644 = true;
                    _3645 = _2567.z;
                    break;
                }
                case 3:
                {
                    _3644 = true;
                    _3645 = _2567.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3644)
            {
                break;
            }
            _3644 = true;
            _3645 = 0.0;
            break;
        } while(false);
        float4 _3559 = float4(_3588, _3607, _3626, _3645);
        _3663 = false;
        do
        {
            switch (uniforms.tevStages.data[i].konstColorSel)
            {
                case 0:
                {
                    _3663 = true;
                    _3664 = float3(1.0);
                    break;
                }
                case 1:
                {
                    _3663 = true;
                    _3664 = float3(0.875);
                    break;
                }
                case 2:
                {
                    _3663 = true;
                    _3664 = float3(0.75);
                    break;
                }
                case 3:
                {
                    _3663 = true;
                    _3664 = float3(0.625);
                    break;
                }
                case 4:
                {
                    _3663 = true;
                    _3664 = float3(0.5);
                    break;
                }
                case 5:
                {
                    _3663 = true;
                    _3664 = float3(0.375);
                    break;
                }
                case 6:
                {
                    _3663 = true;
                    _3664 = float3(0.25);
                    break;
                }
                case 7:
                {
                    _3663 = true;
                    _3664 = float3(0.125);
                    break;
                }
                case 8:
                case 9:
                case 10:
                case 11:
                {
                    _3663 = true;
                    _3664 = float3(0.0);
                    break;
                }
                case 12:
                {
                    _3663 = true;
                    _3664 = uniforms.konst0.xyz;
                    break;
                }
                case 13:
                {
                    _3663 = true;
                    _3664 = uniforms.konst1.xyz;
                    break;
                }
                case 14:
                {
                    _3663 = true;
                    _3664 = uniforms.konst2.xyz;
                    break;
                }
                case 15:
                {
                    _3663 = true;
                    _3664 = uniforms.konst3.xyz;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3663)
            {
                break;
            }
            int _3698 = uniforms.tevStages.data[i].konstColorSel - 16;
            _3715 = false;
            do
            {
                switch (_3698 - 4 * (_3698 / 4))
                {
                    case 0:
                    {
                        _3715 = true;
                        _3716 = uniforms.konst0;
                        break;
                    }
                    case 1:
                    {
                        _3715 = true;
                        _3716 = uniforms.konst1;
                        break;
                    }
                    case 2:
                    {
                        _3715 = true;
                        _3716 = uniforms.konst2;
                        break;
                    }
                    case 3:
                    {
                        _3715 = true;
                        _3716 = uniforms.konst3;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_3715)
                {
                    break;
                }
                _3715 = true;
                _3716 = float4(0.0);
                break;
            } while(false);
            switch (_3698 / 4)
            {
                case 0:
                {
                    _3665 = _3716.x;
                    break;
                }
                case 1:
                {
                    _3665 = _3716.y;
                    break;
                }
                case 2:
                {
                    _3665 = _3716.z;
                    break;
                }
                default:
                {
                    _3665 = _3716.w;
                    break;
                }
            }
            _3663 = true;
            _3664 = float3(_3665);
            break;
        } while(false);
        _3738 = false;
        do
        {
            switch (uniforms.tevStages.data[i].konstAlphaSel)
            {
                case 0:
                {
                    _3738 = true;
                    _3739 = 1.0;
                    break;
                }
                case 1:
                {
                    _3738 = true;
                    _3739 = 0.875;
                    break;
                }
                case 2:
                {
                    _3738 = true;
                    _3739 = 0.75;
                    break;
                }
                case 3:
                {
                    _3738 = true;
                    _3739 = 0.625;
                    break;
                }
                case 4:
                {
                    _3738 = true;
                    _3739 = 0.5;
                    break;
                }
                case 5:
                {
                    _3738 = true;
                    _3739 = 0.375;
                    break;
                }
                case 6:
                {
                    _3738 = true;
                    _3739 = 0.25;
                    break;
                }
                case 7:
                {
                    _3738 = true;
                    _3739 = 0.125;
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
                    _3738 = true;
                    _3739 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3738)
            {
                break;
            }
            int _3756 = uniforms.tevStages.data[i].konstAlphaSel - 16;
            _3773 = false;
            do
            {
                switch (_3756 - 4 * (_3756 / 4))
                {
                    case 0:
                    {
                        _3773 = true;
                        _3774 = uniforms.konst0;
                        break;
                    }
                    case 1:
                    {
                        _3773 = true;
                        _3774 = uniforms.konst1;
                        break;
                    }
                    case 2:
                    {
                        _3773 = true;
                        _3774 = uniforms.konst2;
                        break;
                    }
                    case 3:
                    {
                        _3773 = true;
                        _3774 = uniforms.konst3;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_3773)
                {
                    break;
                }
                _3773 = true;
                _3774 = float4(0.0);
                break;
            } while(false);
            switch (_3756 / 4)
            {
                case 0:
                {
                    _3738 = true;
                    _3739 = _3774.x;
                    break;
                }
                case 1:
                {
                    _3738 = true;
                    _3739 = _3774.y;
                    break;
                }
                case 2:
                {
                    _3738 = true;
                    _3739 = _3774.z;
                    break;
                }
                default:
                {
                    _3738 = true;
                    _3739 = _3774.w;
                    break;
                }
            }
            if (_3738)
            {
                break;
            }
            break;
        } while(false);
        _3796 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorA)
            {
                case 0:
                {
                    _3796 = true;
                    _3797 = _4872.xyz;
                    break;
                }
                case 1:
                {
                    _3796 = true;
                    _3797 = _4872.www;
                    break;
                }
                case 2:
                {
                    _3796 = true;
                    _3797 = _4873.xyz;
                    break;
                }
                case 3:
                {
                    _3796 = true;
                    _3797 = _4873.www;
                    break;
                }
                case 4:
                {
                    _3796 = true;
                    _3797 = _4874.xyz;
                    break;
                }
                case 5:
                {
                    _3796 = true;
                    _3797 = _4874.www;
                    break;
                }
                case 6:
                {
                    _3796 = true;
                    _3797 = _4875.xyz;
                    break;
                }
                case 7:
                {
                    _3796 = true;
                    _3797 = _4875.www;
                    break;
                }
                case 8:
                {
                    _3796 = true;
                    _3797 = _3559.xyz;
                    break;
                }
                case 9:
                {
                    _3796 = true;
                    _3797 = _3559.www;
                    break;
                }
                case 10:
                {
                    _3796 = true;
                    _3797 = _3320.xyz;
                    break;
                }
                case 11:
                {
                    _3796 = true;
                    _3797 = _3320.www;
                    break;
                }
                case 12:
                {
                    _3796 = true;
                    _3797 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _3796 = true;
                    _3797 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _3796 = true;
                    _3797 = _3664;
                    break;
                }
                case 15:
                {
                    _3796 = true;
                    _3797 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3796)
            {
                break;
            }
            _3796 = true;
            _3797 = float3(0.0);
            break;
        } while(false);
        _3843 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorB)
            {
                case 0:
                {
                    _3843 = true;
                    _3844 = _4872.xyz;
                    break;
                }
                case 1:
                {
                    _3843 = true;
                    _3844 = _4872.www;
                    break;
                }
                case 2:
                {
                    _3843 = true;
                    _3844 = _4873.xyz;
                    break;
                }
                case 3:
                {
                    _3843 = true;
                    _3844 = _4873.www;
                    break;
                }
                case 4:
                {
                    _3843 = true;
                    _3844 = _4874.xyz;
                    break;
                }
                case 5:
                {
                    _3843 = true;
                    _3844 = _4874.www;
                    break;
                }
                case 6:
                {
                    _3843 = true;
                    _3844 = _4875.xyz;
                    break;
                }
                case 7:
                {
                    _3843 = true;
                    _3844 = _4875.www;
                    break;
                }
                case 8:
                {
                    _3843 = true;
                    _3844 = _3559.xyz;
                    break;
                }
                case 9:
                {
                    _3843 = true;
                    _3844 = _3559.www;
                    break;
                }
                case 10:
                {
                    _3843 = true;
                    _3844 = _3320.xyz;
                    break;
                }
                case 11:
                {
                    _3843 = true;
                    _3844 = _3320.www;
                    break;
                }
                case 12:
                {
                    _3843 = true;
                    _3844 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _3843 = true;
                    _3844 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _3843 = true;
                    _3844 = _3664;
                    break;
                }
                case 15:
                {
                    _3843 = true;
                    _3844 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3843)
            {
                break;
            }
            _3843 = true;
            _3844 = float3(0.0);
            break;
        } while(false);
        _3890 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorC)
            {
                case 0:
                {
                    _3890 = true;
                    _3891 = _4872.xyz;
                    break;
                }
                case 1:
                {
                    _3890 = true;
                    _3891 = _4872.www;
                    break;
                }
                case 2:
                {
                    _3890 = true;
                    _3891 = _4873.xyz;
                    break;
                }
                case 3:
                {
                    _3890 = true;
                    _3891 = _4873.www;
                    break;
                }
                case 4:
                {
                    _3890 = true;
                    _3891 = _4874.xyz;
                    break;
                }
                case 5:
                {
                    _3890 = true;
                    _3891 = _4874.www;
                    break;
                }
                case 6:
                {
                    _3890 = true;
                    _3891 = _4875.xyz;
                    break;
                }
                case 7:
                {
                    _3890 = true;
                    _3891 = _4875.www;
                    break;
                }
                case 8:
                {
                    _3890 = true;
                    _3891 = _3559.xyz;
                    break;
                }
                case 9:
                {
                    _3890 = true;
                    _3891 = _3559.www;
                    break;
                }
                case 10:
                {
                    _3890 = true;
                    _3891 = _3320.xyz;
                    break;
                }
                case 11:
                {
                    _3890 = true;
                    _3891 = _3320.www;
                    break;
                }
                case 12:
                {
                    _3890 = true;
                    _3891 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _3890 = true;
                    _3891 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _3890 = true;
                    _3891 = _3664;
                    break;
                }
                case 15:
                {
                    _3890 = true;
                    _3891 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3890)
            {
                break;
            }
            _3890 = true;
            _3891 = float3(0.0);
            break;
        } while(false);
        _3937 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorD)
            {
                case 0:
                {
                    _3937 = true;
                    _3938 = _4872.xyz;
                    break;
                }
                case 1:
                {
                    _3937 = true;
                    _3938 = _4872.www;
                    break;
                }
                case 2:
                {
                    _3937 = true;
                    _3938 = _4873.xyz;
                    break;
                }
                case 3:
                {
                    _3937 = true;
                    _3938 = _4873.www;
                    break;
                }
                case 4:
                {
                    _3937 = true;
                    _3938 = _4874.xyz;
                    break;
                }
                case 5:
                {
                    _3937 = true;
                    _3938 = _4874.www;
                    break;
                }
                case 6:
                {
                    _3937 = true;
                    _3938 = _4875.xyz;
                    break;
                }
                case 7:
                {
                    _3937 = true;
                    _3938 = _4875.www;
                    break;
                }
                case 8:
                {
                    _3937 = true;
                    _3938 = _3559.xyz;
                    break;
                }
                case 9:
                {
                    _3937 = true;
                    _3938 = _3559.www;
                    break;
                }
                case 10:
                {
                    _3937 = true;
                    _3938 = _3320.xyz;
                    break;
                }
                case 11:
                {
                    _3937 = true;
                    _3938 = _3320.www;
                    break;
                }
                case 12:
                {
                    _3937 = true;
                    _3938 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _3937 = true;
                    _3938 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _3937 = true;
                    _3938 = _3664;
                    break;
                }
                case 15:
                {
                    _3937 = true;
                    _3938 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3937)
            {
                break;
            }
            _3937 = true;
            _3938 = float3(0.0);
            break;
        } while(false);
        _3984 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaA)
            {
                case 0:
                {
                    _3984 = true;
                    _3985 = _4872.w;
                    break;
                }
                case 1:
                {
                    _3984 = true;
                    _3985 = _4873.w;
                    break;
                }
                case 2:
                {
                    _3984 = true;
                    _3985 = _4874.w;
                    break;
                }
                case 3:
                {
                    _3984 = true;
                    _3985 = _4875.w;
                    break;
                }
                case 4:
                {
                    _3984 = true;
                    _3985 = _3559.w;
                    break;
                }
                case 5:
                {
                    _3984 = true;
                    _3985 = _3320.w;
                    break;
                }
                case 6:
                {
                    _3984 = true;
                    _3985 = _3739;
                    break;
                }
                case 7:
                {
                    _3984 = true;
                    _3985 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3984)
            {
                break;
            }
            _3984 = true;
            _3985 = 0.0;
            break;
        } while(false);
        _4013 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaB)
            {
                case 0:
                {
                    _4013 = true;
                    _4014 = _4872.w;
                    break;
                }
                case 1:
                {
                    _4013 = true;
                    _4014 = _4873.w;
                    break;
                }
                case 2:
                {
                    _4013 = true;
                    _4014 = _4874.w;
                    break;
                }
                case 3:
                {
                    _4013 = true;
                    _4014 = _4875.w;
                    break;
                }
                case 4:
                {
                    _4013 = true;
                    _4014 = _3559.w;
                    break;
                }
                case 5:
                {
                    _4013 = true;
                    _4014 = _3320.w;
                    break;
                }
                case 6:
                {
                    _4013 = true;
                    _4014 = _3739;
                    break;
                }
                case 7:
                {
                    _4013 = true;
                    _4014 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4013)
            {
                break;
            }
            _4013 = true;
            _4014 = 0.0;
            break;
        } while(false);
        _4042 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaC)
            {
                case 0:
                {
                    _4042 = true;
                    _4043 = _4872.w;
                    break;
                }
                case 1:
                {
                    _4042 = true;
                    _4043 = _4873.w;
                    break;
                }
                case 2:
                {
                    _4042 = true;
                    _4043 = _4874.w;
                    break;
                }
                case 3:
                {
                    _4042 = true;
                    _4043 = _4875.w;
                    break;
                }
                case 4:
                {
                    _4042 = true;
                    _4043 = _3559.w;
                    break;
                }
                case 5:
                {
                    _4042 = true;
                    _4043 = _3320.w;
                    break;
                }
                case 6:
                {
                    _4042 = true;
                    _4043 = _3739;
                    break;
                }
                case 7:
                {
                    _4042 = true;
                    _4043 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4042)
            {
                break;
            }
            _4042 = true;
            _4043 = 0.0;
            break;
        } while(false);
        _4071 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaD)
            {
                case 0:
                {
                    _4071 = true;
                    _4072 = _4872.w;
                    break;
                }
                case 1:
                {
                    _4071 = true;
                    _4072 = _4873.w;
                    break;
                }
                case 2:
                {
                    _4071 = true;
                    _4072 = _4874.w;
                    break;
                }
                case 3:
                {
                    _4071 = true;
                    _4072 = _4875.w;
                    break;
                }
                case 4:
                {
                    _4071 = true;
                    _4072 = _3559.w;
                    break;
                }
                case 5:
                {
                    _4071 = true;
                    _4072 = _3320.w;
                    break;
                }
                case 6:
                {
                    _4071 = true;
                    _4072 = _3739;
                    break;
                }
                case 7:
                {
                    _4071 = true;
                    _4072 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4071)
            {
                break;
            }
            _4071 = true;
            _4072 = 0.0;
            break;
        } while(false);
        do
        {
            if (uniforms.tevStages.data[i].colorBias == 3)
            {
                if (uniforms.tevStages.data[i].colorScale == 3)
                {
                    uint _4240 = uint(round(fast::clamp(_3797.x, 0.0, 1.0) * 255.0));
                    uint _4246 = uint(round(fast::clamp(_3797.y, 0.0, 1.0) * 255.0));
                    uint _4252 = uint(round(fast::clamp(_3797.z, 0.0, 1.0) * 255.0));
                    uint _4258 = uint(round(fast::clamp(_3844.x, 0.0, 1.0) * 255.0));
                    uint _4264 = uint(round(fast::clamp(_3844.y, 0.0, 1.0) * 255.0));
                    uint _4270 = uint(round(fast::clamp(_3844.z, 0.0, 1.0) * 255.0));
                    if (uniforms.tevStages.data[i].colorOp != 0)
                    {
                        if (_4240 == _4258)
                        {
                            _4103 = 1.0;
                        }
                        else
                        {
                            _4103 = 0.0;
                        }
                        if (_4246 == _4264)
                        {
                            _4104 = 1.0;
                        }
                        else
                        {
                            _4104 = 0.0;
                        }
                        if (_4252 == _4270)
                        {
                            _4105 = 1.0;
                        }
                        else
                        {
                            _4105 = 0.0;
                        }
                        _4102 = float3(_4103, _4104, _4105);
                    }
                    else
                    {
                        if (_4240 > _4258)
                        {
                            _4103 = 1.0;
                        }
                        else
                        {
                            _4103 = 0.0;
                        }
                        if (_4246 > _4264)
                        {
                            _4104 = 1.0;
                        }
                        else
                        {
                            _4104 = 0.0;
                        }
                        if (_4252 > _4270)
                        {
                            _4105 = 1.0;
                        }
                        else
                        {
                            _4105 = 0.0;
                        }
                        _4102 = float3(_4103, _4104, _4105);
                    }
                    float3 _4167 = _3938 + (_3891 * _4102);
                    if (uniforms.tevStages.data[i].colorClamp != 0)
                    {
                        _4106 = fast::clamp(_4167, float3(0.0), float3(1.0));
                    }
                    else
                    {
                        _4106 = _4167;
                    }
                    _4101 = _4106;
                    break;
                }
                do
                {
                    uint _4339 = uint(round(fast::clamp(_3797.x, 0.0, 1.0) * 255.0));
                    uint _4345 = uint(round(fast::clamp(_3797.y, 0.0, 1.0) * 255.0));
                    uint _4357 = uint(round(fast::clamp(_3844.x, 0.0, 1.0) * 255.0));
                    uint _4363 = uint(round(fast::clamp(_3844.y, 0.0, 1.0) * 255.0));
                    bool _4292 = uniforms.tevStages.data[i].colorOp != 0;
                    if (uniforms.tevStages.data[i].colorScale == 0)
                    {
                        if (_4292)
                        {
                            _4276 = _4339 == _4357;
                        }
                        else
                        {
                            _4276 = _4339 > _4357;
                        }
                        _4275 = _4276;
                        break;
                    }
                    if (uniforms.tevStages.data[i].colorScale == 1)
                    {
                        uint _4305 = (_4345 << uint(8)) | _4339;
                        uint _4307 = (_4363 << uint(8)) | _4357;
                        if (_4292)
                        {
                            _4276 = _4305 == _4307;
                        }
                        else
                        {
                            _4276 = _4305 > _4307;
                        }
                        _4275 = _4276;
                        break;
                    }
                    if (uniforms.tevStages.data[i].colorScale == 2)
                    {
                        uint _4320 = ((uint(round(fast::clamp(_3797.z, 0.0, 1.0) * 255.0)) << uint(16)) | (_4345 << uint(8))) | _4339;
                        uint _4324 = ((uint(round(fast::clamp(_3844.z, 0.0, 1.0) * 255.0)) << uint(16)) | (_4363 << uint(8))) | _4357;
                        if (_4292)
                        {
                            _4276 = _4320 == _4324;
                        }
                        else
                        {
                            _4276 = _4320 > _4324;
                        }
                        _4275 = _4276;
                        break;
                    }
                    _4275 = false;
                    break;
                } while(false);
                if (_4275)
                {
                    _4102 = _3891;
                }
                else
                {
                    _4102 = float3(0.0);
                }
                float3 _4182 = _4102;
                float3 _4183 = _3938 + _4182;
                if (uniforms.tevStages.data[i].colorClamp != 0)
                {
                    _4102 = fast::clamp(_4183, float3(0.0), float3(1.0));
                }
                else
                {
                    _4102 = _4183;
                }
                _4101 = _4102;
                break;
            }
            float3 _4196 = (_3797 * (float3(1.0) - _3891)) + (_3844 * _3891);
            if (uniforms.tevStages.data[i].colorOp == 0)
            {
                _4102 = _3938 + _4196;
            }
            else
            {
                _4102 = _3938 - _4196;
            }
            switch (uniforms.tevStages.data[i].colorBias)
            {
                case 1:
                {
                    _4102 += float3(0.5);
                    break;
                }
                case 2:
                {
                    _4102 -= float3(0.5);
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
                    _4102 *= 2.0;
                    break;
                }
                case 2:
                {
                    _4102 *= 4.0;
                    break;
                }
                case 3:
                {
                    _4102 *= 0.5;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (uniforms.tevStages.data[i].colorClamp != 0)
            {
                _4102 = fast::clamp(_4102, float3(0.0), float3(1.0));
            }
            _4101 = _4102;
            break;
        } while(false);
        do
        {
            if (uniforms.tevStages.data[i].alphaBias == 3)
            {
                uint _4454 = uint(round(fast::clamp(_3985, 0.0, 1.0) * 255.0));
                uint _4460 = uint(round(fast::clamp(_4014, 0.0, 1.0) * 255.0));
                if (uniforms.tevStages.data[i].alphaOp != 0)
                {
                    _4379 = _4454 == _4460;
                }
                else
                {
                    _4379 = _4454 > _4460;
                }
                if (_4379)
                {
                    _4378 = _4043;
                }
                else
                {
                    _4378 = 0.0;
                }
                float _4399 = _4378;
                float _4400 = _4072 + _4399;
                if (uniforms.tevStages.data[i].alphaClamp != 0)
                {
                    _4378 = fast::clamp(_4400, 0.0, 1.0);
                }
                else
                {
                    _4378 = _4400;
                }
                _4377 = _4378;
                break;
            }
            float _4412 = (_3985 * (1.0 - _4043)) + (_4014 * _4043);
            if (uniforms.tevStages.data[i].alphaOp == 0)
            {
                _4378 = _4072 + _4412;
            }
            else
            {
                _4378 = _4072 - _4412;
            }
            switch (uniforms.tevStages.data[i].alphaBias)
            {
                case 1:
                {
                    _4378 += 0.5;
                    break;
                }
                case 2:
                {
                    _4378 -= 0.5;
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
                    _4378 *= 2.0;
                    break;
                }
                case 2:
                {
                    _4378 *= 4.0;
                    break;
                }
                case 3:
                {
                    _4378 *= 0.5;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (uniforms.tevStages.data[i].alphaClamp != 0)
            {
                _4378 = fast::clamp(_4378, 0.0, 1.0);
            }
            _4377 = _4378;
            break;
        } while(false);
        switch (uniforms.tevStages.data[i].colorOutput)
        {
            case 0:
            {
                float4 _4975 = _4872;
                _4975.x = _4101.x;
                _4975.y = _4101.y;
                _4975.z = _4101.z;
                _4872 = _4975;
                break;
            }
            case 1:
            {
                float4 _4969 = _4873;
                _4969.x = _4101.x;
                _4969.y = _4101.y;
                _4969.z = _4101.z;
                _4873 = _4969;
                break;
            }
            case 2:
            {
                float4 _4963 = _4874;
                _4963.x = _4101.x;
                _4963.y = _4101.y;
                _4963.z = _4101.z;
                _4874 = _4963;
                break;
            }
            case 3:
            {
                float4 _4957 = _4875;
                _4957.x = _4101.x;
                _4957.y = _4101.y;
                _4957.z = _4101.z;
                _4875 = _4957;
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
                float4 _4987 = _4872;
                _4987.w = _4377;
                _4872 = _4987;
                break;
            }
            case 1:
            {
                float4 _4985 = _4873;
                _4985.w = _4377;
                _4873 = _4985;
                break;
            }
            case 2:
            {
                float4 _4983 = _4874;
                _4983.w = _4377;
                _4874 = _4983;
                break;
            }
            case 3:
            {
                float4 _4981 = _4875;
                _4981.w = _4377;
                _4875 = _4981;
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
    float4 color = _4872;
    _4511 = false;
    bool _4512;
    do
    {
        _4549 = false;
        bool _4550;
        do
        {
            uint _4581 = uint(round(fast::clamp(_4872.w, 0.0, 1.0) * 255.0));
            uint _4587 = uint(round(fast::clamp(uniforms.alphaRef0, 0.0, 1.0) * 255.0));
            switch (uniforms.alphaComp0)
            {
                case 0:
                {
                    _4549 = true;
                    _4550 = false;
                    break;
                }
                case 1:
                {
                    _4549 = true;
                    _4550 = _4581 < _4587;
                    break;
                }
                case 2:
                {
                    _4549 = true;
                    _4550 = _4581 == _4587;
                    break;
                }
                case 3:
                {
                    _4549 = true;
                    _4550 = _4581 <= _4587;
                    break;
                }
                case 4:
                {
                    _4549 = true;
                    _4550 = _4581 > _4587;
                    break;
                }
                case 5:
                {
                    _4549 = true;
                    _4550 = _4581 != _4587;
                    break;
                }
                case 6:
                {
                    _4549 = true;
                    _4550 = _4581 >= _4587;
                    break;
                }
                case 7:
                {
                    _4549 = true;
                    _4550 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4549)
            {
                break;
            }
            _4549 = true;
            _4550 = true;
            break;
        } while(false);
        _4588 = false;
        bool _4589;
        do
        {
            uint _4620 = uint(round(fast::clamp(_4872.w, 0.0, 1.0) * 255.0));
            uint _4626 = uint(round(fast::clamp(uniforms.alphaRef1, 0.0, 1.0) * 255.0));
            switch (uniforms.alphaComp1)
            {
                case 0:
                {
                    _4588 = true;
                    _4589 = false;
                    break;
                }
                case 1:
                {
                    _4588 = true;
                    _4589 = _4620 < _4626;
                    break;
                }
                case 2:
                {
                    _4588 = true;
                    _4589 = _4620 == _4626;
                    break;
                }
                case 3:
                {
                    _4588 = true;
                    _4589 = _4620 <= _4626;
                    break;
                }
                case 4:
                {
                    _4588 = true;
                    _4589 = _4620 > _4626;
                    break;
                }
                case 5:
                {
                    _4588 = true;
                    _4589 = _4620 != _4626;
                    break;
                }
                case 6:
                {
                    _4588 = true;
                    _4589 = _4620 >= _4626;
                    break;
                }
                case 7:
                {
                    _4588 = true;
                    _4589 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4588)
            {
                break;
            }
            _4588 = true;
            _4589 = true;
            break;
        } while(false);
        bool _4513;
        switch (uniforms.alphaLogic)
        {
            case 0:
            {
                if (_4550)
                {
                    _4513 = _4589;
                }
                else
                {
                    _4513 = false;
                }
                _4511 = true;
                _4512 = _4513;
                break;
            }
            case 1:
            {
                if (_4550)
                {
                    _4513 = true;
                }
                else
                {
                    _4513 = _4589;
                }
                _4511 = true;
                _4512 = _4513;
                break;
            }
            case 2:
            {
                _4511 = true;
                _4512 = _4550 != _4589;
                break;
            }
            case 3:
            {
                _4511 = true;
                _4512 = _4550 == _4589;
                break;
            }
            default:
            {
                break;
            }
        }
        if (_4511)
        {
            break;
        }
        _4511 = true;
        _4512 = true;
        break;
    } while(false);
    if (!_4512)
    {
        discard_fragment();
    }
    if (uniforms.destinationAlphaEnabled != 0)
    {
        float4 _4989 = color;
        _4989.w = uniforms.destinationAlpha;
        color = _4989;
    }
    if (uniforms.enableDither != 0)
    {
        float4 _282 = color;
        uint3 _4641 = uint3(round(fast::clamp(_282.xyz, float3(0.0), float3(1.0)) * 255.0));
        float3 _4652 = float3(min(((_4641 - (_4641 >> uint3(6u))) + uint3(_319[(int(gl_FragCoord.x) & 1) | ((int(gl_FragCoord.y) & 1) << 1)])), uint3(255u))) * float3(0.0039215688593685626983642578125);
        float4 _4992 = _282;
        _4992.x = _4652.x;
        _4992.y = _4652.y;
        _4992.z = _4652.z;
        color = _4992;
    }
    float depth;
    if (uniforms.zTextureOp != 0)
    {
        _4656 = false;
        uint _4657;
        do
        {
            uint4 _4664 = uint4(round(fast::clamp(lastTexture, float4(0.0), float4(1.0)) * 255.0));
            switch (uniforms.zTextureFormat)
            {
                case 0:
                {
                    _4656 = true;
                    _4657 = _4664.w;
                    break;
                }
                case 1:
                {
                    _4656 = true;
                    _4657 = _4664.x | (_4664.w << 8u);
                    break;
                }
                case 2:
                {
                    _4656 = true;
                    _4657 = ((_4664.x << 16u) | (_4664.y << 8u)) | _4664.z;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4656)
            {
                break;
            }
            _4656 = true;
            _4657 = 0u;
            break;
        } while(false);
        uint bias = uint(uniforms.zTextureBias);
        uint finalZ;
        if (uniforms.zTextureOp == 1)
        {
            finalZ = (uint(fast::clamp(gl_FragCoord.z, 0.0, 1.0) * 16777215.0) + _4657) + bias;
        }
        else
        {
            finalZ = _4657 + bias;
        }
        depth = float(finalZ & 16777215u) * 5.9604651880817982601001858711243e-08;
    }
    else
    {
        depth = gl_FragCoord.z;
    }
    float _4690;
    do
    {
        if (uniforms.fogType == 0)
        {
            _4690 = 0.0;
            break;
        }
        uint _4705 = min(uint(round(fast::clamp(depth, 0.0, 1.0) * 16777216.0)), 16777215u);
        float _4691;
        if ((uniforms.fogType & 8) != 0)
        {
            _4691 = (uniforms.fogA * float(_4705)) * 5.9604644775390625e-08;
        }
        else
        {
            int _4716 = uniforms.fogBMagnitude - int(_4705 >> uint(uniforms.fogBShift));
            if (_4716 == 0)
            {
                if (uniforms.fogA < 0.0)
                {
                    _4691 = -1000000015047466219876688855040.0;
                }
                else
                {
                    _4691 = 1000000015047466219876688855040.0;
                }
            }
            else
            {
                _4691 = (uniforms.fogA * 16777216.0) / float(_4716);
            }
        }
        if (uniforms.fogRangeEnabled != 0)
        {
            float _4751 = (((2.0 * gl_FragCoord.x) / uniforms.fogViewportWidth) - 1.0) - uniforms.fogRangeCenter;
            float _4755 = fast::clamp(9.0 - (abs(_4751) * 9.0), 0.0, 9.0);
            uint _4756 = uint(_4755);
            float _4766 = mix(uniforms.fogRangeK.data[_4756].x, uniforms.fogRangeK.data[min((_4756 + 1u), 9u)].x, fract(_4755));
            _4691 *= (sqrt((_4751 * _4751) + (_4766 * _4766)) / fast::max(_4766, 9.9999999747524270787835121154785e-07));
        }
        float _4780 = fast::clamp(_4691 - uniforms.fogC, 0.0, 1.0);
        float _4692;
        switch (uniforms.fogType & 7)
        {
            case 2:
            {
                _4692 = _4780;
                break;
            }
            case 4:
            {
                _4692 = 1.0 - exp2((-8.0) * _4780);
                break;
            }
            case 5:
            {
                _4692 = 1.0 - exp2(((-8.0) * _4780) * _4780);
                break;
            }
            case 6:
            {
                _4692 = exp2((-8.0) * (1.0 - _4780));
                break;
            }
            case 7:
            {
                float _4784 = 1.0 - _4780;
                _4692 = exp2(((-8.0) * _4784) * _4784);
                break;
            }
            default:
            {
                _4692 = _4780;
                break;
            }
        }
        _4690 = fast::clamp(_4692, 0.0, 1.0);
        break;
    } while(false);
    float4 _596 = color;
    float3 _603 = mix(_596.xyz, float3(uniforms.fogColor), float3(_4690));
    float4 _4998 = _596;
    _4998.x = _603.x;
    _4998.y = _603.y;
    _4998.z = _603.z;
    color = _4998;
    float4 _4809;
    do
    {
        float4 _4810 = _4998;
        if (uniforms.efbPixelFormat == 1)
        {
            _4809 = float4(uint4(round(fast::clamp(_4810, float4(0.0), float4(1.0)) * 63.0))) * float4(0.01587301678955554962158203125);
            break;
        }
        if (uniforms.efbPixelFormat == 2)
        {
            _4809 = float4(float(uint(round(fast::clamp(_4810.x, 0.0, 1.0) * 31.0))) * 0.0322580635547637939453125, float(uint(round(fast::clamp(_4810.y, 0.0, 1.0) * 63.0))) * 0.01587301678955554962158203125, float(uint(round(fast::clamp(_4810.z, 0.0, 1.0) * 31.0))) * 0.0322580635547637939453125, 1.0);
            break;
        }
        if (uniforms.efbPixelFormat == 0)
        {
            float4 _5003 = _4810;
            _5003.w = 1.0;
            _4810 = _5003;
        }
        _4809 = _4810;
        break;
    } while(false);
    color = _4809;
    out.entryPointParam_fragmentMain_color = _4809;
    out.gl_FragDepth = depth;
    return out;
}

)SHDR3";

#endif
