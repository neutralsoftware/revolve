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
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

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
};

struct main0_out
{
    float4 entryPointParam_fragmentMain [[color(0)]];
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

fragment main0_out main0(main0_in in [[stage_in]], constant Uniforms_std140& uniforms [[buffer(0)]], texture2d<float> tex0 [[texture(0)]], texture2d<float> tex1 [[texture(1)]], texture2d<float> tex2 [[texture(2)]], texture2d<float> tex3 [[texture(3)]], texture2d<float> tex4 [[texture(4)]], texture2d<float> tex5 [[texture(5)]], texture2d<float> tex6 [[texture(6)]], texture2d<float> tex7 [[texture(7)]], sampler samp0 [[sampler(0)]], sampler samp1 [[sampler(1)]], sampler samp2 [[sampler(2)]], sampler samp3 [[sampler(3)]], sampler samp4 [[sampler(4)]], sampler samp5 [[sampler(5)]], sampler samp6 [[sampler(6)]], sampler samp7 [[sampler(7)]])
{
    bool _4142 = false;
    bool _4117 = false;
    bool _4079 = false;
    bool _3639 = false;
    bool _3610 = false;
    bool _3581 = false;
    bool _3552 = false;
    bool _3505 = false;
    bool _3458 = false;
    bool _3411 = false;
    bool _3364 = false;
    bool _3341 = false;
    bool _3306 = false;
    bool _3283 = false;
    bool _3231 = false;
    bool _3212 = false;
    bool _3193 = false;
    bool _3174 = false;
    bool _3155 = false;
    bool _3128 = false;
    bool _3057 = false;
    bool _2992 = false;
    bool _2973 = false;
    bool _2954 = false;
    bool _2935 = false;
    bool _2916 = false;
    bool _2889 = false;
    bool _2853 = false;
    bool _2795 = false;
    bool _2737 = false;
    bool _2397 = false;
    bool _2360 = false;
    bool _2289 = false;
    main0_out out = {};
    float4 _4286 = uniforms.tevRegister0;
    float4 _4287 = uniforms.tevRegister1;
    float4 _4288 = uniforms.tevRegister2;
    float4 _4289 = uniforms.tevRegister3;
    float2 indirectCoord = float2(0.0);
    float bumpAlpha = 0.0;
    int i = 0;
    float2 _2123;
    bool _2124;
    float3 _2125;
    float _2127;
    float2 _2128;
    float2 _2129;
    float4 _2130;
    float3 _2290;
    float3 _2361;
    float4 _2398;
    float3 _2463;
    float _2488;
    float2 _2521;
    bool _2522;
    float4 _2586;
    float4 _2587;
    float4 _2624;
    float4 _2625;
    float4 _2662;
    float4 _2663;
    float4 _2700;
    float4 _2701;
    float _2738;
    float _2796;
    float4 _2854;
    int4 _2890;
    float _2917;
    float _2936;
    float _2955;
    float _2974;
    float4 _2993;
    float4 _3058;
    int4 _3129;
    float _3156;
    float _3175;
    float _3194;
    float _3213;
    float3 _3232;
    float _3233;
    float4 _3284;
    float _3307;
    float4 _3342;
    float3 _3365;
    float3 _3412;
    float3 _3459;
    float3 _3506;
    float _3553;
    float _3582;
    float _3611;
    float _3640;
    float3 _3669;
    float3 _3670;
    float _3671;
    float _3672;
    float _3673;
    float3 _3674;
    bool _3843;
    bool _3844;
    float _3945;
    float _3946;
    bool _3947;
    for (;;)
    {
        if (!(i < uniforms.tevStageCount))
        {
            break;
        }
        _2289 = false;
        do
        {
            switch (uniforms.tevStages.data[i].texCoord)
            {
                case 0:
                {
                    _2289 = true;
                    _2290 = in.input_uv0;
                    break;
                }
                case 1:
                {
                    _2289 = true;
                    _2290 = in.input_uv1;
                    break;
                }
                case 2:
                {
                    _2289 = true;
                    _2290 = in.input_uv2;
                    break;
                }
                case 3:
                {
                    _2289 = true;
                    _2290 = in.input_uv3;
                    break;
                }
                case 4:
                {
                    _2289 = true;
                    _2290 = in.input_uv4;
                    break;
                }
                case 5:
                {
                    _2289 = true;
                    _2290 = in.input_uv5;
                    break;
                }
                case 6:
                {
                    _2289 = true;
                    _2290 = in.input_uv6;
                    break;
                }
                case 7:
                {
                    _2289 = true;
                    _2290 = in.input_uv7;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2289)
            {
                break;
            }
            _2289 = true;
            _2290 = float3(0.0);
            break;
        } while(false);
        if (_2290.z != 0.0)
        {
            _2123 = _2290.xy / float2(_2290.z);
        }
        else
        {
            _2123 = _2290.xy;
        }
        bool _2150 = uniforms.tevStages.data[i].indirectStage < uniforms.indirectStageCount;
        if (_2150)
        {
            if (uniforms.tevStages.data[i].indirectMatrix != 0)
            {
                _2124 = true;
            }
            else
            {
                _2124 = uniforms.tevStages.data[i].indirectAlpha != 0;
            }
        }
        else
        {
            _2124 = false;
        }
        if (_2124)
        {
            _2360 = false;
            do
            {
                switch (uniforms.indirectStages.data[uniforms.tevStages.data[i].indirectStage].texCoord)
                {
                    case 0:
                    {
                        _2360 = true;
                        _2361 = in.input_uv0;
                        break;
                    }
                    case 1:
                    {
                        _2360 = true;
                        _2361 = in.input_uv1;
                        break;
                    }
                    case 2:
                    {
                        _2360 = true;
                        _2361 = in.input_uv2;
                        break;
                    }
                    case 3:
                    {
                        _2360 = true;
                        _2361 = in.input_uv3;
                        break;
                    }
                    case 4:
                    {
                        _2360 = true;
                        _2361 = in.input_uv4;
                        break;
                    }
                    case 5:
                    {
                        _2360 = true;
                        _2361 = in.input_uv5;
                        break;
                    }
                    case 6:
                    {
                        _2360 = true;
                        _2361 = in.input_uv6;
                        break;
                    }
                    case 7:
                    {
                        _2360 = true;
                        _2361 = in.input_uv7;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_2360)
                {
                    break;
                }
                _2360 = true;
                _2361 = float3(0.0);
                break;
            } while(false);
            float2 _2316 = _2361.xy;
            if (_2361.z != 0.0)
            {
                _2316 /= float2(_2361.z);
            }
            float2 _4343 = _2316;
            _4343.x = _4343.x * exp2(-float(uniforms.indirectStages.data[uniforms.tevStages.data[i].indirectStage].scaleS));
            _4343.y = _4343.y * exp2(-float(uniforms.indirectStages.data[uniforms.tevStages.data[i].indirectStage].scaleT));
            _2316 = _4343;
            _2397 = false;
            do
            {
                switch (uniforms.indirectStages.data[uniforms.tevStages.data[i].indirectStage].texMap)
                {
                    case 0:
                    {
                        _2397 = true;
                        _2398 = tex0.sample(samp0, _4343);
                        break;
                    }
                    case 1:
                    {
                        _2397 = true;
                        _2398 = tex1.sample(samp1, _4343);
                        break;
                    }
                    case 2:
                    {
                        _2397 = true;
                        _2398 = tex2.sample(samp2, _4343);
                        break;
                    }
                    case 3:
                    {
                        _2397 = true;
                        _2398 = tex3.sample(samp3, _4343);
                        break;
                    }
                    case 4:
                    {
                        _2397 = true;
                        _2398 = tex4.sample(samp4, _4343);
                        break;
                    }
                    case 5:
                    {
                        _2397 = true;
                        _2398 = tex5.sample(samp5, _4343);
                        break;
                    }
                    case 6:
                    {
                        _2397 = true;
                        _2398 = tex6.sample(samp6, _4343);
                        break;
                    }
                    case 7:
                    {
                        _2397 = true;
                        _2398 = tex7.sample(samp7, _4343);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_2397)
                {
                    break;
                }
                _2397 = true;
                _2398 = float4(1.0);
                break;
            } while(false);
            _2125 = floor((_2398.wzy * 255.0) + float3(0.5));
        }
        else
        {
            _2125 = float3(0.0);
        }
        if (_2150)
        {
            _2124 = uniforms.tevStages.data[i].indirectAlpha != 0;
        }
        else
        {
            _2124 = false;
        }
        if (_2124)
        {
            float3 _2126 = _2125;
            if (uniforms.tevStages.data[i].indirectFormat == 0)
            {
                _2127 = 0.0;
            }
            else
            {
                _2127 = 6.0 - float(uniforms.tevStages.data[i].indirectFormat);
            }
            float _2187 = _2126[uniforms.tevStages.data[i].indirectAlpha - 1] * exp2(_2127);
            bumpAlpha = floor((_2187 - (256.0 * floor(_2187 * 0.00390625))) * 0.125) * 0.0313725508749485015869140625;
        }
        do
        {
            if (uniforms.tevStages.data[i].indirectFormat == 0)
            {
                _2463 = _2125;
                break;
            }
            if (uniforms.tevStages.data[i].indirectFormat == 1)
            {
                _2463 = floor(_2125 * float3(0.125));
                break;
            }
            if (uniforms.tevStages.data[i].indirectFormat == 2)
            {
                _2463 = floor(_2125 * float3(0.0625));
                break;
            }
            _2463 = floor(_2125 * float3(0.03125));
            break;
        } while(false);
        float3 _2487 = _2463;
        if (uniforms.tevStages.data[i].indirectFormat == 0)
        {
            _2488 = -128.0;
        }
        else
        {
            _2488 = 1.0;
        }
        if ((uniforms.tevStages.data[i].indirectBias & 1) != 0)
        {
            float3 _4349 = _2487;
            _4349.x = _4349.x + _2488;
            _2487 = _4349;
        }
        if ((uniforms.tevStages.data[i].indirectBias & 2) != 0)
        {
            float3 _4352 = _2487;
            _4352.y = _4352.y + _2488;
            _2487 = _4352;
        }
        if ((uniforms.tevStages.data[i].indirectBias & 4) != 0)
        {
            float3 _4355 = _2487;
            _4355.z = _4355.z + _2488;
            _2487 = _4355;
        }
        if (_2150)
        {
            do
            {
                if (uniforms.tevStages.data[i].indirectMatrix == 0)
                {
                    _2521 = float2(0.0);
                    break;
                }
                if (uniforms.tevStages.data[i].indirectMatrix >= 1)
                {
                    _2522 = uniforms.tevStages.data[i].indirectMatrix <= 3;
                }
                else
                {
                    _2522 = false;
                }
                if (_2522)
                {
                    int _2536 = uniforms.tevStages.data[i].indirectMatrix - 1;
                    do
                    {
                        if (_2536 == 0)
                        {
                            if (false)
                            {
                                _2587 = uniforms.indirectMatrix0B;
                            }
                            else
                            {
                                _2587 = uniforms.indirectMatrix0A;
                            }
                            _2586 = _2587;
                            break;
                        }
                        if (_2536 == 1)
                        {
                            if (false)
                            {
                                _2587 = uniforms.indirectMatrix1B;
                            }
                            else
                            {
                                _2587 = uniforms.indirectMatrix1A;
                            }
                            _2586 = _2587;
                            break;
                        }
                        if (false)
                        {
                            _2587 = uniforms.indirectMatrix2B;
                        }
                        else
                        {
                            _2587 = uniforms.indirectMatrix2A;
                        }
                        _2586 = _2587;
                        break;
                    } while(false);
                    do
                    {
                        if (_2536 == 0)
                        {
                            if (true)
                            {
                                _2625 = uniforms.indirectMatrix0B;
                            }
                            else
                            {
                                _2625 = uniforms.indirectMatrix0A;
                            }
                            _2624 = _2625;
                            break;
                        }
                        if (_2536 == 1)
                        {
                            if (true)
                            {
                                _2625 = uniforms.indirectMatrix1B;
                            }
                            else
                            {
                                _2625 = uniforms.indirectMatrix1A;
                            }
                            _2624 = _2625;
                            break;
                        }
                        if (true)
                        {
                            _2625 = uniforms.indirectMatrix2B;
                        }
                        else
                        {
                            _2625 = uniforms.indirectMatrix2A;
                        }
                        _2624 = _2625;
                        break;
                    } while(false);
                    _2521 = (float2(dot(_2586.xyz, _2487), dot(_2624.xyz, _2487)) * exp2(_2586.w)) * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].zw;
                    break;
                }
                if (uniforms.tevStages.data[i].indirectMatrix >= 5)
                {
                    _2522 = uniforms.tevStages.data[i].indirectMatrix <= 7;
                }
                else
                {
                    _2522 = false;
                }
                if (_2522)
                {
                    int _2556 = uniforms.tevStages.data[i].indirectMatrix - 5;
                    do
                    {
                        if (_2556 == 0)
                        {
                            if (false)
                            {
                                _2663 = uniforms.indirectMatrix0B;
                            }
                            else
                            {
                                _2663 = uniforms.indirectMatrix0A;
                            }
                            _2662 = _2663;
                            break;
                        }
                        if (_2556 == 1)
                        {
                            if (false)
                            {
                                _2663 = uniforms.indirectMatrix1B;
                            }
                            else
                            {
                                _2663 = uniforms.indirectMatrix1A;
                            }
                            _2662 = _2663;
                            break;
                        }
                        if (false)
                        {
                            _2663 = uniforms.indirectMatrix2B;
                        }
                        else
                        {
                            _2663 = uniforms.indirectMatrix2A;
                        }
                        _2662 = _2663;
                        break;
                    } while(false);
                    _2521 = ((_2123 * _2487.x) * exp2(_2662.w)) * float2(0.00390625);
                    break;
                }
                if (uniforms.tevStages.data[i].indirectMatrix >= 9)
                {
                    _2522 = uniforms.tevStages.data[i].indirectMatrix <= 11;
                }
                else
                {
                    _2522 = false;
                }
                if (_2522)
                {
                    int _2573 = uniforms.tevStages.data[i].indirectMatrix - 9;
                    do
                    {
                        if (_2573 == 0)
                        {
                            if (false)
                            {
                                _2701 = uniforms.indirectMatrix0B;
                            }
                            else
                            {
                                _2701 = uniforms.indirectMatrix0A;
                            }
                            _2700 = _2701;
                            break;
                        }
                        if (_2573 == 1)
                        {
                            if (false)
                            {
                                _2701 = uniforms.indirectMatrix1B;
                            }
                            else
                            {
                                _2701 = uniforms.indirectMatrix1A;
                            }
                            _2700 = _2701;
                            break;
                        }
                        if (false)
                        {
                            _2701 = uniforms.indirectMatrix2B;
                        }
                        else
                        {
                            _2701 = uniforms.indirectMatrix2A;
                        }
                        _2700 = _2701;
                        break;
                    } while(false);
                    _2521 = ((_2123 * _2487.y) * exp2(_2700.w)) * float2(0.00390625);
                    break;
                }
                _2521 = float2(0.0);
                break;
            } while(false);
            _2128 = _2521;
        }
        else
        {
            _2128 = float2(0.0);
        }
        _2737 = false;
        do
        {
            switch (uniforms.tevStages.data[i].indirectWrapS)
            {
                case 0:
                {
                    _2737 = true;
                    _2738 = _2123.x;
                    break;
                }
                case 1:
                {
                    float _2756 = 256.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _2737 = true;
                    _2738 = _2123.x - (_2756 * floor(_2123.x / _2756));
                    break;
                }
                case 2:
                {
                    float _2753 = 128.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _2737 = true;
                    _2738 = _2123.x - (_2753 * floor(_2123.x / _2753));
                    break;
                }
                case 3:
                {
                    float _2750 = 64.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _2737 = true;
                    _2738 = _2123.x - (_2750 * floor(_2123.x / _2750));
                    break;
                }
                case 4:
                {
                    float _2747 = 32.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _2737 = true;
                    _2738 = _2123.x - (_2747 * floor(_2123.x / _2747));
                    break;
                }
                case 5:
                {
                    float _2744 = 16.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].z;
                    _2737 = true;
                    _2738 = _2123.x - (_2744 * floor(_2123.x / _2744));
                    break;
                }
                case 6:
                {
                    _2737 = true;
                    _2738 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2737)
            {
                break;
            }
            _2737 = true;
            _2738 = _2123.x;
            break;
        } while(false);
        float2 _4358 = _2129;
        _4358.x = _2738;
        _2129 = _4358;
        _2795 = false;
        do
        {
            switch (uniforms.tevStages.data[i].indirectWrapT)
            {
                case 0:
                {
                    _2795 = true;
                    _2796 = _2123.y;
                    break;
                }
                case 1:
                {
                    float _2814 = 256.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _2795 = true;
                    _2796 = _2123.y - (_2814 * floor(_2123.y / _2814));
                    break;
                }
                case 2:
                {
                    float _2811 = 128.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _2795 = true;
                    _2796 = _2123.y - (_2811 * floor(_2123.y / _2811));
                    break;
                }
                case 3:
                {
                    float _2808 = 64.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _2795 = true;
                    _2796 = _2123.y - (_2808 * floor(_2123.y / _2808));
                    break;
                }
                case 4:
                {
                    float _2805 = 32.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _2795 = true;
                    _2796 = _2123.y - (_2805 * floor(_2123.y / _2805));
                    break;
                }
                case 5:
                {
                    float _2802 = 16.0 * uniforms.textureSizes.data[uniforms.tevStages.data[i].texMap].w;
                    _2795 = true;
                    _2796 = _2123.y - (_2802 * floor(_2123.y / _2802));
                    break;
                }
                case 6:
                {
                    _2795 = true;
                    _2796 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2795)
            {
                break;
            }
            _2795 = true;
            _2796 = _2123.y;
            break;
        } while(false);
        float2 _4360 = _2129;
        _4360.y = _2796;
        _2129 = _4360 + _2128;
        if (uniforms.tevStages.data[i].indirectAddPrevious != 0)
        {
            _2129 += indirectCoord;
        }
        indirectCoord = _2129;
        _2853 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorChannel)
            {
                case 0:
                {
                    _2853 = true;
                    _2854 = in.input_color0;
                    break;
                }
                case 1:
                {
                    _2853 = true;
                    _2854 = in.input_color1;
                    break;
                }
                case 5:
                {
                    _2853 = true;
                    _2854 = float4(bumpAlpha);
                    break;
                }
                case 6:
                {
                    _2853 = true;
                    _2854 = float4(((bumpAlpha * 255.0) + floor(bumpAlpha * 7.96875)) * 0.0039215688593685626983642578125);
                    break;
                }
                default:
                {
                    _2853 = true;
                    _2854 = float4(0.0);
                    break;
                }
            }
            if (_2853)
            {
                break;
            }
            break;
        } while(false);
        _2889 = false;
        do
        {
            switch (uniforms.tevStages.data[i].rasterSwap)
            {
                case 0:
                {
                    _2889 = true;
                    _2890 = int4(uniforms.swapTable0);
                    break;
                }
                case 1:
                {
                    _2889 = true;
                    _2890 = int4(uniforms.swapTable1);
                    break;
                }
                case 2:
                {
                    _2889 = true;
                    _2890 = int4(uniforms.swapTable2);
                    break;
                }
                case 3:
                {
                    _2889 = true;
                    _2890 = int4(uniforms.swapTable3);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2889)
            {
                break;
            }
            _2889 = true;
            _2890 = int4(0, 1, 2, 3);
            break;
        } while(false);
        _2916 = false;
        do
        {
            switch (_2890.x)
            {
                case 0:
                {
                    _2916 = true;
                    _2917 = _2854.x;
                    break;
                }
                case 1:
                {
                    _2916 = true;
                    _2917 = _2854.y;
                    break;
                }
                case 2:
                {
                    _2916 = true;
                    _2917 = _2854.z;
                    break;
                }
                case 3:
                {
                    _2916 = true;
                    _2917 = _2854.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2916)
            {
                break;
            }
            _2916 = true;
            _2917 = 0.0;
            break;
        } while(false);
        _2935 = false;
        do
        {
            switch (_2890.y)
            {
                case 0:
                {
                    _2935 = true;
                    _2936 = _2854.x;
                    break;
                }
                case 1:
                {
                    _2935 = true;
                    _2936 = _2854.y;
                    break;
                }
                case 2:
                {
                    _2935 = true;
                    _2936 = _2854.z;
                    break;
                }
                case 3:
                {
                    _2935 = true;
                    _2936 = _2854.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2935)
            {
                break;
            }
            _2935 = true;
            _2936 = 0.0;
            break;
        } while(false);
        _2954 = false;
        do
        {
            switch (_2890.z)
            {
                case 0:
                {
                    _2954 = true;
                    _2955 = _2854.x;
                    break;
                }
                case 1:
                {
                    _2954 = true;
                    _2955 = _2854.y;
                    break;
                }
                case 2:
                {
                    _2954 = true;
                    _2955 = _2854.z;
                    break;
                }
                case 3:
                {
                    _2954 = true;
                    _2955 = _2854.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2954)
            {
                break;
            }
            _2954 = true;
            _2955 = 0.0;
            break;
        } while(false);
        _2973 = false;
        do
        {
            switch (_2890.w)
            {
                case 0:
                {
                    _2973 = true;
                    _2974 = _2854.x;
                    break;
                }
                case 1:
                {
                    _2973 = true;
                    _2974 = _2854.y;
                    break;
                }
                case 2:
                {
                    _2973 = true;
                    _2974 = _2854.z;
                    break;
                }
                case 3:
                {
                    _2973 = true;
                    _2974 = _2854.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_2973)
            {
                break;
            }
            _2973 = true;
            _2974 = 0.0;
            break;
        } while(false);
        float4 _2888 = float4(_2917, _2936, _2955, _2974);
        if (uniforms.tevStages.data[i].textureEnabled != 0)
        {
            if (uniforms.tevStages.data[i].indirectUseOriginalLod != 0)
            {
                float2 _3053 = dfdx(_2123);
                float2 _3056 = dfdy(_2123);
                _3057 = false;
                do
                {
                    switch (uniforms.tevStages.data[i].texMap)
                    {
                        case 0:
                        {
                            _3057 = true;
                            _3058 = tex0.sample(samp0, _2129, gradient2d(_3053, _3056));
                            break;
                        }
                        case 1:
                        {
                            _3057 = true;
                            _3058 = tex1.sample(samp1, _2129, gradient2d(_3053, _3056));
                            break;
                        }
                        case 2:
                        {
                            _3057 = true;
                            _3058 = tex2.sample(samp2, _2129, gradient2d(_3053, _3056));
                            break;
                        }
                        case 3:
                        {
                            _3057 = true;
                            _3058 = tex3.sample(samp3, _2129, gradient2d(_3053, _3056));
                            break;
                        }
                        case 4:
                        {
                            _3057 = true;
                            _3058 = tex4.sample(samp4, _2129, gradient2d(_3053, _3056));
                            break;
                        }
                        case 5:
                        {
                            _3057 = true;
                            _3058 = tex5.sample(samp5, _2129, gradient2d(_3053, _3056));
                            break;
                        }
                        case 6:
                        {
                            _3057 = true;
                            _3058 = tex6.sample(samp6, _2129, gradient2d(_3053, _3056));
                            break;
                        }
                        case 7:
                        {
                            _3057 = true;
                            _3058 = tex7.sample(samp7, _2129, gradient2d(_3053, _3056));
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                    if (_3057)
                    {
                        break;
                    }
                    _3057 = true;
                    _3058 = float4(1.0);
                    break;
                } while(false);
                _2130 = _3058;
            }
            else
            {
                _2992 = false;
                do
                {
                    switch (uniforms.tevStages.data[i].texMap)
                    {
                        case 0:
                        {
                            _2992 = true;
                            _2993 = tex0.sample(samp0, _2129);
                            break;
                        }
                        case 1:
                        {
                            _2992 = true;
                            _2993 = tex1.sample(samp1, _2129);
                            break;
                        }
                        case 2:
                        {
                            _2992 = true;
                            _2993 = tex2.sample(samp2, _2129);
                            break;
                        }
                        case 3:
                        {
                            _2992 = true;
                            _2993 = tex3.sample(samp3, _2129);
                            break;
                        }
                        case 4:
                        {
                            _2992 = true;
                            _2993 = tex4.sample(samp4, _2129);
                            break;
                        }
                        case 5:
                        {
                            _2992 = true;
                            _2993 = tex5.sample(samp5, _2129);
                            break;
                        }
                        case 6:
                        {
                            _2992 = true;
                            _2993 = tex6.sample(samp6, _2129);
                            break;
                        }
                        case 7:
                        {
                            _2992 = true;
                            _2993 = tex7.sample(samp7, _2129);
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                    if (_2992)
                    {
                        break;
                    }
                    _2992 = true;
                    _2993 = float4(1.0);
                    break;
                } while(false);
                _2130 = _2993;
            }
        }
        else
        {
            _2130 = float4(1.0);
        }
        _3128 = false;
        do
        {
            switch (uniforms.tevStages.data[i].textureSwap)
            {
                case 0:
                {
                    _3128 = true;
                    _3129 = int4(uniforms.swapTable0);
                    break;
                }
                case 1:
                {
                    _3128 = true;
                    _3129 = int4(uniforms.swapTable1);
                    break;
                }
                case 2:
                {
                    _3128 = true;
                    _3129 = int4(uniforms.swapTable2);
                    break;
                }
                case 3:
                {
                    _3128 = true;
                    _3129 = int4(uniforms.swapTable3);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3128)
            {
                break;
            }
            _3128 = true;
            _3129 = int4(0, 1, 2, 3);
            break;
        } while(false);
        _3155 = false;
        do
        {
            switch (_3129.x)
            {
                case 0:
                {
                    _3155 = true;
                    _3156 = _2130.x;
                    break;
                }
                case 1:
                {
                    _3155 = true;
                    _3156 = _2130.y;
                    break;
                }
                case 2:
                {
                    _3155 = true;
                    _3156 = _2130.z;
                    break;
                }
                case 3:
                {
                    _3155 = true;
                    _3156 = _2130.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3155)
            {
                break;
            }
            _3155 = true;
            _3156 = 0.0;
            break;
        } while(false);
        _3174 = false;
        do
        {
            switch (_3129.y)
            {
                case 0:
                {
                    _3174 = true;
                    _3175 = _2130.x;
                    break;
                }
                case 1:
                {
                    _3174 = true;
                    _3175 = _2130.y;
                    break;
                }
                case 2:
                {
                    _3174 = true;
                    _3175 = _2130.z;
                    break;
                }
                case 3:
                {
                    _3174 = true;
                    _3175 = _2130.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3174)
            {
                break;
            }
            _3174 = true;
            _3175 = 0.0;
            break;
        } while(false);
        _3193 = false;
        do
        {
            switch (_3129.z)
            {
                case 0:
                {
                    _3193 = true;
                    _3194 = _2130.x;
                    break;
                }
                case 1:
                {
                    _3193 = true;
                    _3194 = _2130.y;
                    break;
                }
                case 2:
                {
                    _3193 = true;
                    _3194 = _2130.z;
                    break;
                }
                case 3:
                {
                    _3193 = true;
                    _3194 = _2130.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3193)
            {
                break;
            }
            _3193 = true;
            _3194 = 0.0;
            break;
        } while(false);
        _3212 = false;
        do
        {
            switch (_3129.w)
            {
                case 0:
                {
                    _3212 = true;
                    _3213 = _2130.x;
                    break;
                }
                case 1:
                {
                    _3212 = true;
                    _3213 = _2130.y;
                    break;
                }
                case 2:
                {
                    _3212 = true;
                    _3213 = _2130.z;
                    break;
                }
                case 3:
                {
                    _3212 = true;
                    _3213 = _2130.w;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3212)
            {
                break;
            }
            _3212 = true;
            _3213 = 0.0;
            break;
        } while(false);
        float4 _3127 = float4(_3156, _3175, _3194, _3213);
        _3231 = false;
        do
        {
            switch (uniforms.tevStages.data[i].konstColorSel)
            {
                case 0:
                {
                    _3231 = true;
                    _3232 = float3(1.0);
                    break;
                }
                case 1:
                {
                    _3231 = true;
                    _3232 = float3(0.875);
                    break;
                }
                case 2:
                {
                    _3231 = true;
                    _3232 = float3(0.75);
                    break;
                }
                case 3:
                {
                    _3231 = true;
                    _3232 = float3(0.625);
                    break;
                }
                case 4:
                {
                    _3231 = true;
                    _3232 = float3(0.5);
                    break;
                }
                case 5:
                {
                    _3231 = true;
                    _3232 = float3(0.375);
                    break;
                }
                case 6:
                {
                    _3231 = true;
                    _3232 = float3(0.25);
                    break;
                }
                case 7:
                {
                    _3231 = true;
                    _3232 = float3(0.125);
                    break;
                }
                case 8:
                case 9:
                case 10:
                case 11:
                {
                    _3231 = true;
                    _3232 = float3(0.0);
                    break;
                }
                case 12:
                {
                    _3231 = true;
                    _3232 = uniforms.konst0.xyz;
                    break;
                }
                case 13:
                {
                    _3231 = true;
                    _3232 = uniforms.konst1.xyz;
                    break;
                }
                case 14:
                {
                    _3231 = true;
                    _3232 = uniforms.konst2.xyz;
                    break;
                }
                case 15:
                {
                    _3231 = true;
                    _3232 = uniforms.konst3.xyz;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3231)
            {
                break;
            }
            int _3266 = uniforms.tevStages.data[i].konstColorSel - 16;
            _3283 = false;
            do
            {
                switch (_3266 - 4 * (_3266 / 4))
                {
                    case 0:
                    {
                        _3283 = true;
                        _3284 = uniforms.konst0;
                        break;
                    }
                    case 1:
                    {
                        _3283 = true;
                        _3284 = uniforms.konst1;
                        break;
                    }
                    case 2:
                    {
                        _3283 = true;
                        _3284 = uniforms.konst2;
                        break;
                    }
                    case 3:
                    {
                        _3283 = true;
                        _3284 = uniforms.konst3;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_3283)
                {
                    break;
                }
                _3283 = true;
                _3284 = float4(0.0);
                break;
            } while(false);
            switch (_3266 / 4)
            {
                case 0:
                {
                    _3233 = _3284.x;
                    break;
                }
                case 1:
                {
                    _3233 = _3284.y;
                    break;
                }
                case 2:
                {
                    _3233 = _3284.z;
                    break;
                }
                default:
                {
                    _3233 = _3284.w;
                    break;
                }
            }
            _3231 = true;
            _3232 = float3(_3233);
            break;
        } while(false);
        _3306 = false;
        do
        {
            switch (uniforms.tevStages.data[i].konstAlphaSel)
            {
                case 0:
                {
                    _3306 = true;
                    _3307 = 1.0;
                    break;
                }
                case 1:
                {
                    _3306 = true;
                    _3307 = 0.875;
                    break;
                }
                case 2:
                {
                    _3306 = true;
                    _3307 = 0.75;
                    break;
                }
                case 3:
                {
                    _3306 = true;
                    _3307 = 0.625;
                    break;
                }
                case 4:
                {
                    _3306 = true;
                    _3307 = 0.5;
                    break;
                }
                case 5:
                {
                    _3306 = true;
                    _3307 = 0.375;
                    break;
                }
                case 6:
                {
                    _3306 = true;
                    _3307 = 0.25;
                    break;
                }
                case 7:
                {
                    _3306 = true;
                    _3307 = 0.125;
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
                    _3306 = true;
                    _3307 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3306)
            {
                break;
            }
            int _3324 = uniforms.tevStages.data[i].konstAlphaSel - 16;
            _3341 = false;
            do
            {
                switch (_3324 - 4 * (_3324 / 4))
                {
                    case 0:
                    {
                        _3341 = true;
                        _3342 = uniforms.konst0;
                        break;
                    }
                    case 1:
                    {
                        _3341 = true;
                        _3342 = uniforms.konst1;
                        break;
                    }
                    case 2:
                    {
                        _3341 = true;
                        _3342 = uniforms.konst2;
                        break;
                    }
                    case 3:
                    {
                        _3341 = true;
                        _3342 = uniforms.konst3;
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
                if (_3341)
                {
                    break;
                }
                _3341 = true;
                _3342 = float4(0.0);
                break;
            } while(false);
            switch (_3324 / 4)
            {
                case 0:
                {
                    _3306 = true;
                    _3307 = _3342.x;
                    break;
                }
                case 1:
                {
                    _3306 = true;
                    _3307 = _3342.y;
                    break;
                }
                case 2:
                {
                    _3306 = true;
                    _3307 = _3342.z;
                    break;
                }
                default:
                {
                    _3306 = true;
                    _3307 = _3342.w;
                    break;
                }
            }
            if (_3306)
            {
                break;
            }
            break;
        } while(false);
        _3364 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorA)
            {
                case 0:
                {
                    _3364 = true;
                    _3365 = _4286.xyz;
                    break;
                }
                case 1:
                {
                    _3364 = true;
                    _3365 = _4286.www;
                    break;
                }
                case 2:
                {
                    _3364 = true;
                    _3365 = _4287.xyz;
                    break;
                }
                case 3:
                {
                    _3364 = true;
                    _3365 = _4287.www;
                    break;
                }
                case 4:
                {
                    _3364 = true;
                    _3365 = _4288.xyz;
                    break;
                }
                case 5:
                {
                    _3364 = true;
                    _3365 = _4288.www;
                    break;
                }
                case 6:
                {
                    _3364 = true;
                    _3365 = _4289.xyz;
                    break;
                }
                case 7:
                {
                    _3364 = true;
                    _3365 = _4289.www;
                    break;
                }
                case 8:
                {
                    _3364 = true;
                    _3365 = _3127.xyz;
                    break;
                }
                case 9:
                {
                    _3364 = true;
                    _3365 = _3127.www;
                    break;
                }
                case 10:
                {
                    _3364 = true;
                    _3365 = _2888.xyz;
                    break;
                }
                case 11:
                {
                    _3364 = true;
                    _3365 = _2888.www;
                    break;
                }
                case 12:
                {
                    _3364 = true;
                    _3365 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _3364 = true;
                    _3365 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _3364 = true;
                    _3365 = _3232;
                    break;
                }
                case 15:
                {
                    _3364 = true;
                    _3365 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3364)
            {
                break;
            }
            _3364 = true;
            _3365 = float3(0.0);
            break;
        } while(false);
        _3411 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorB)
            {
                case 0:
                {
                    _3411 = true;
                    _3412 = _4286.xyz;
                    break;
                }
                case 1:
                {
                    _3411 = true;
                    _3412 = _4286.www;
                    break;
                }
                case 2:
                {
                    _3411 = true;
                    _3412 = _4287.xyz;
                    break;
                }
                case 3:
                {
                    _3411 = true;
                    _3412 = _4287.www;
                    break;
                }
                case 4:
                {
                    _3411 = true;
                    _3412 = _4288.xyz;
                    break;
                }
                case 5:
                {
                    _3411 = true;
                    _3412 = _4288.www;
                    break;
                }
                case 6:
                {
                    _3411 = true;
                    _3412 = _4289.xyz;
                    break;
                }
                case 7:
                {
                    _3411 = true;
                    _3412 = _4289.www;
                    break;
                }
                case 8:
                {
                    _3411 = true;
                    _3412 = _3127.xyz;
                    break;
                }
                case 9:
                {
                    _3411 = true;
                    _3412 = _3127.www;
                    break;
                }
                case 10:
                {
                    _3411 = true;
                    _3412 = _2888.xyz;
                    break;
                }
                case 11:
                {
                    _3411 = true;
                    _3412 = _2888.www;
                    break;
                }
                case 12:
                {
                    _3411 = true;
                    _3412 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _3411 = true;
                    _3412 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _3411 = true;
                    _3412 = _3232;
                    break;
                }
                case 15:
                {
                    _3411 = true;
                    _3412 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3411)
            {
                break;
            }
            _3411 = true;
            _3412 = float3(0.0);
            break;
        } while(false);
        _3458 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorC)
            {
                case 0:
                {
                    _3458 = true;
                    _3459 = _4286.xyz;
                    break;
                }
                case 1:
                {
                    _3458 = true;
                    _3459 = _4286.www;
                    break;
                }
                case 2:
                {
                    _3458 = true;
                    _3459 = _4287.xyz;
                    break;
                }
                case 3:
                {
                    _3458 = true;
                    _3459 = _4287.www;
                    break;
                }
                case 4:
                {
                    _3458 = true;
                    _3459 = _4288.xyz;
                    break;
                }
                case 5:
                {
                    _3458 = true;
                    _3459 = _4288.www;
                    break;
                }
                case 6:
                {
                    _3458 = true;
                    _3459 = _4289.xyz;
                    break;
                }
                case 7:
                {
                    _3458 = true;
                    _3459 = _4289.www;
                    break;
                }
                case 8:
                {
                    _3458 = true;
                    _3459 = _3127.xyz;
                    break;
                }
                case 9:
                {
                    _3458 = true;
                    _3459 = _3127.www;
                    break;
                }
                case 10:
                {
                    _3458 = true;
                    _3459 = _2888.xyz;
                    break;
                }
                case 11:
                {
                    _3458 = true;
                    _3459 = _2888.www;
                    break;
                }
                case 12:
                {
                    _3458 = true;
                    _3459 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _3458 = true;
                    _3459 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _3458 = true;
                    _3459 = _3232;
                    break;
                }
                case 15:
                {
                    _3458 = true;
                    _3459 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3458)
            {
                break;
            }
            _3458 = true;
            _3459 = float3(0.0);
            break;
        } while(false);
        _3505 = false;
        do
        {
            switch (uniforms.tevStages.data[i].colorD)
            {
                case 0:
                {
                    _3505 = true;
                    _3506 = _4286.xyz;
                    break;
                }
                case 1:
                {
                    _3505 = true;
                    _3506 = _4286.www;
                    break;
                }
                case 2:
                {
                    _3505 = true;
                    _3506 = _4287.xyz;
                    break;
                }
                case 3:
                {
                    _3505 = true;
                    _3506 = _4287.www;
                    break;
                }
                case 4:
                {
                    _3505 = true;
                    _3506 = _4288.xyz;
                    break;
                }
                case 5:
                {
                    _3505 = true;
                    _3506 = _4288.www;
                    break;
                }
                case 6:
                {
                    _3505 = true;
                    _3506 = _4289.xyz;
                    break;
                }
                case 7:
                {
                    _3505 = true;
                    _3506 = _4289.www;
                    break;
                }
                case 8:
                {
                    _3505 = true;
                    _3506 = _3127.xyz;
                    break;
                }
                case 9:
                {
                    _3505 = true;
                    _3506 = _3127.www;
                    break;
                }
                case 10:
                {
                    _3505 = true;
                    _3506 = _2888.xyz;
                    break;
                }
                case 11:
                {
                    _3505 = true;
                    _3506 = _2888.www;
                    break;
                }
                case 12:
                {
                    _3505 = true;
                    _3506 = float3(1.0);
                    break;
                }
                case 13:
                {
                    _3505 = true;
                    _3506 = float3(0.5);
                    break;
                }
                case 14:
                {
                    _3505 = true;
                    _3506 = _3232;
                    break;
                }
                case 15:
                {
                    _3505 = true;
                    _3506 = float3(0.0);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3505)
            {
                break;
            }
            _3505 = true;
            _3506 = float3(0.0);
            break;
        } while(false);
        _3552 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaA)
            {
                case 0:
                {
                    _3552 = true;
                    _3553 = _4286.w;
                    break;
                }
                case 1:
                {
                    _3552 = true;
                    _3553 = _4287.w;
                    break;
                }
                case 2:
                {
                    _3552 = true;
                    _3553 = _4288.w;
                    break;
                }
                case 3:
                {
                    _3552 = true;
                    _3553 = _4289.w;
                    break;
                }
                case 4:
                {
                    _3552 = true;
                    _3553 = _3127.w;
                    break;
                }
                case 5:
                {
                    _3552 = true;
                    _3553 = _2888.w;
                    break;
                }
                case 6:
                {
                    _3552 = true;
                    _3553 = _3307;
                    break;
                }
                case 7:
                {
                    _3552 = true;
                    _3553 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3552)
            {
                break;
            }
            _3552 = true;
            _3553 = 0.0;
            break;
        } while(false);
        _3581 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaB)
            {
                case 0:
                {
                    _3581 = true;
                    _3582 = _4286.w;
                    break;
                }
                case 1:
                {
                    _3581 = true;
                    _3582 = _4287.w;
                    break;
                }
                case 2:
                {
                    _3581 = true;
                    _3582 = _4288.w;
                    break;
                }
                case 3:
                {
                    _3581 = true;
                    _3582 = _4289.w;
                    break;
                }
                case 4:
                {
                    _3581 = true;
                    _3582 = _3127.w;
                    break;
                }
                case 5:
                {
                    _3581 = true;
                    _3582 = _2888.w;
                    break;
                }
                case 6:
                {
                    _3581 = true;
                    _3582 = _3307;
                    break;
                }
                case 7:
                {
                    _3581 = true;
                    _3582 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3581)
            {
                break;
            }
            _3581 = true;
            _3582 = 0.0;
            break;
        } while(false);
        _3610 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaC)
            {
                case 0:
                {
                    _3610 = true;
                    _3611 = _4286.w;
                    break;
                }
                case 1:
                {
                    _3610 = true;
                    _3611 = _4287.w;
                    break;
                }
                case 2:
                {
                    _3610 = true;
                    _3611 = _4288.w;
                    break;
                }
                case 3:
                {
                    _3610 = true;
                    _3611 = _4289.w;
                    break;
                }
                case 4:
                {
                    _3610 = true;
                    _3611 = _3127.w;
                    break;
                }
                case 5:
                {
                    _3610 = true;
                    _3611 = _2888.w;
                    break;
                }
                case 6:
                {
                    _3610 = true;
                    _3611 = _3307;
                    break;
                }
                case 7:
                {
                    _3610 = true;
                    _3611 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3610)
            {
                break;
            }
            _3610 = true;
            _3611 = 0.0;
            break;
        } while(false);
        _3639 = false;
        do
        {
            switch (uniforms.tevStages.data[i].alphaD)
            {
                case 0:
                {
                    _3639 = true;
                    _3640 = _4286.w;
                    break;
                }
                case 1:
                {
                    _3639 = true;
                    _3640 = _4287.w;
                    break;
                }
                case 2:
                {
                    _3639 = true;
                    _3640 = _4288.w;
                    break;
                }
                case 3:
                {
                    _3639 = true;
                    _3640 = _4289.w;
                    break;
                }
                case 4:
                {
                    _3639 = true;
                    _3640 = _3127.w;
                    break;
                }
                case 5:
                {
                    _3639 = true;
                    _3640 = _2888.w;
                    break;
                }
                case 6:
                {
                    _3639 = true;
                    _3640 = _3307;
                    break;
                }
                case 7:
                {
                    _3639 = true;
                    _3640 = 0.0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_3639)
            {
                break;
            }
            _3639 = true;
            _3640 = 0.0;
            break;
        } while(false);
        do
        {
            if (uniforms.tevStages.data[i].colorBias == 3)
            {
                if (uniforms.tevStages.data[i].colorScale == 3)
                {
                    uint _3808 = uint(round(fast::clamp(_3365.x, 0.0, 1.0) * 255.0));
                    uint _3814 = uint(round(fast::clamp(_3365.y, 0.0, 1.0) * 255.0));
                    uint _3820 = uint(round(fast::clamp(_3365.z, 0.0, 1.0) * 255.0));
                    uint _3826 = uint(round(fast::clamp(_3412.x, 0.0, 1.0) * 255.0));
                    uint _3832 = uint(round(fast::clamp(_3412.y, 0.0, 1.0) * 255.0));
                    uint _3838 = uint(round(fast::clamp(_3412.z, 0.0, 1.0) * 255.0));
                    if (uniforms.tevStages.data[i].colorOp != 0)
                    {
                        if (_3808 == _3826)
                        {
                            _3671 = 1.0;
                        }
                        else
                        {
                            _3671 = 0.0;
                        }
                        if (_3814 == _3832)
                        {
                            _3672 = 1.0;
                        }
                        else
                        {
                            _3672 = 0.0;
                        }
                        if (_3820 == _3838)
                        {
                            _3673 = 1.0;
                        }
                        else
                        {
                            _3673 = 0.0;
                        }
                        _3670 = float3(_3671, _3672, _3673);
                    }
                    else
                    {
                        if (_3808 > _3826)
                        {
                            _3671 = 1.0;
                        }
                        else
                        {
                            _3671 = 0.0;
                        }
                        if (_3814 > _3832)
                        {
                            _3672 = 1.0;
                        }
                        else
                        {
                            _3672 = 0.0;
                        }
                        if (_3820 > _3838)
                        {
                            _3673 = 1.0;
                        }
                        else
                        {
                            _3673 = 0.0;
                        }
                        _3670 = float3(_3671, _3672, _3673);
                    }
                    float3 _3735 = _3506 + (_3459 * _3670);
                    if (uniforms.tevStages.data[i].colorClamp != 0)
                    {
                        _3674 = fast::clamp(_3735, float3(0.0), float3(1.0));
                    }
                    else
                    {
                        _3674 = _3735;
                    }
                    _3669 = _3674;
                    break;
                }
                do
                {
                    uint _3907 = uint(round(fast::clamp(_3365.x, 0.0, 1.0) * 255.0));
                    uint _3913 = uint(round(fast::clamp(_3365.y, 0.0, 1.0) * 255.0));
                    uint _3925 = uint(round(fast::clamp(_3412.x, 0.0, 1.0) * 255.0));
                    uint _3931 = uint(round(fast::clamp(_3412.y, 0.0, 1.0) * 255.0));
                    bool _3860 = uniforms.tevStages.data[i].colorOp != 0;
                    if (uniforms.tevStages.data[i].colorScale == 0)
                    {
                        if (_3860)
                        {
                            _3844 = _3907 == _3925;
                        }
                        else
                        {
                            _3844 = _3907 > _3925;
                        }
                        _3843 = _3844;
                        break;
                    }
                    if (uniforms.tevStages.data[i].colorScale == 1)
                    {
                        uint _3873 = (_3913 << uint(8)) | _3907;
                        uint _3875 = (_3931 << uint(8)) | _3925;
                        if (_3860)
                        {
                            _3844 = _3873 == _3875;
                        }
                        else
                        {
                            _3844 = _3873 > _3875;
                        }
                        _3843 = _3844;
                        break;
                    }
                    if (uniforms.tevStages.data[i].colorScale == 2)
                    {
                        uint _3888 = ((uint(round(fast::clamp(_3365.z, 0.0, 1.0) * 255.0)) << uint(16)) | (_3913 << uint(8))) | _3907;
                        uint _3892 = ((uint(round(fast::clamp(_3412.z, 0.0, 1.0) * 255.0)) << uint(16)) | (_3931 << uint(8))) | _3925;
                        if (_3860)
                        {
                            _3844 = _3888 == _3892;
                        }
                        else
                        {
                            _3844 = _3888 > _3892;
                        }
                        _3843 = _3844;
                        break;
                    }
                    _3843 = false;
                    break;
                } while(false);
                if (_3843)
                {
                    _3670 = _3459;
                }
                else
                {
                    _3670 = float3(0.0);
                }
                float3 _3749 = _3670;
                float3 _3750 = _3506 + _3749;
                if (uniforms.tevStages.data[i].colorClamp != 0)
                {
                    _3670 = fast::clamp(_3750, float3(0.0), float3(1.0));
                }
                else
                {
                    _3670 = _3750;
                }
                _3669 = _3670;
                break;
            }
            float3 _3763 = (_3365 * (float3(1.0) - _3459)) + (_3412 * _3459);
            if (uniforms.tevStages.data[i].colorOp == 0)
            {
                _3670 = _3506 + _3763;
            }
            else
            {
                _3670 = _3506 - _3763;
            }
            switch (uniforms.tevStages.data[i].colorBias)
            {
                case 1:
                {
                    _3670 += float3(0.5);
                    break;
                }
                case 2:
                {
                    _3670 -= float3(0.5);
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
                    _3670 *= 2.0;
                    break;
                }
                case 2:
                {
                    _3670 *= 4.0;
                    break;
                }
                case 3:
                {
                    _3670 *= 0.5;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (uniforms.tevStages.data[i].colorClamp != 0)
            {
                _3670 = fast::clamp(_3670, float3(0.0), float3(1.0));
            }
            _3669 = _3670;
            break;
        } while(false);
        do
        {
            if (uniforms.tevStages.data[i].alphaBias == 3)
            {
                uint _4022 = uint(round(fast::clamp(_3553, 0.0, 1.0) * 255.0));
                uint _4028 = uint(round(fast::clamp(_3582, 0.0, 1.0) * 255.0));
                if (uniforms.tevStages.data[i].alphaOp != 0)
                {
                    _3947 = _4022 == _4028;
                }
                else
                {
                    _3947 = _4022 > _4028;
                }
                if (_3947)
                {
                    _3946 = _3611;
                }
                else
                {
                    _3946 = 0.0;
                }
                float _3967 = _3946;
                float _3968 = _3640 + _3967;
                if (uniforms.tevStages.data[i].alphaClamp != 0)
                {
                    _3946 = fast::clamp(_3968, 0.0, 1.0);
                }
                else
                {
                    _3946 = _3968;
                }
                _3945 = _3946;
                break;
            }
            float _3980 = (_3553 * (1.0 - _3611)) + (_3582 * _3611);
            if (uniforms.tevStages.data[i].alphaOp == 0)
            {
                _3946 = _3640 + _3980;
            }
            else
            {
                _3946 = _3640 - _3980;
            }
            switch (uniforms.tevStages.data[i].alphaBias)
            {
                case 1:
                {
                    _3946 += 0.5;
                    break;
                }
                case 2:
                {
                    _3946 -= 0.5;
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
                    _3946 *= 2.0;
                    break;
                }
                case 2:
                {
                    _3946 *= 4.0;
                    break;
                }
                case 3:
                {
                    _3946 *= 0.5;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (uniforms.tevStages.data[i].alphaClamp != 0)
            {
                _3946 = fast::clamp(_3946, 0.0, 1.0);
            }
            _3945 = _3946;
            break;
        } while(false);
        switch (uniforms.tevStages.data[i].colorOutput)
        {
            case 0:
            {
                float4 _4380 = _4286;
                _4380.x = _3669.x;
                _4380.y = _3669.y;
                _4380.z = _3669.z;
                _4286 = _4380;
                break;
            }
            case 1:
            {
                float4 _4374 = _4287;
                _4374.x = _3669.x;
                _4374.y = _3669.y;
                _4374.z = _3669.z;
                _4287 = _4374;
                break;
            }
            case 2:
            {
                float4 _4368 = _4288;
                _4368.x = _3669.x;
                _4368.y = _3669.y;
                _4368.z = _3669.z;
                _4288 = _4368;
                break;
            }
            case 3:
            {
                float4 _4362 = _4289;
                _4362.x = _3669.x;
                _4362.y = _3669.y;
                _4362.z = _3669.z;
                _4289 = _4362;
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
                float4 _4392 = _4286;
                _4392.w = _3945;
                _4286 = _4392;
                break;
            }
            case 1:
            {
                float4 _4390 = _4287;
                _4390.w = _3945;
                _4287 = _4390;
                break;
            }
            case 2:
            {
                float4 _4388 = _4288;
                _4388.w = _3945;
                _4288 = _4388;
                break;
            }
            case 3:
            {
                float4 _4386 = _4289;
                _4386.w = _3945;
                _4289 = _4386;
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
    _4079 = false;
    bool _4080;
    do
    {
        _4117 = false;
        bool _4118;
        do
        {
            switch (uniforms.alphaComp0)
            {
                case 0:
                {
                    _4117 = true;
                    _4118 = false;
                    break;
                }
                case 1:
                {
                    _4117 = true;
                    _4118 = _4286.w < uniforms.alphaRef0;
                    break;
                }
                case 2:
                {
                    _4117 = true;
                    _4118 = _4286.w == uniforms.alphaRef0;
                    break;
                }
                case 3:
                {
                    _4117 = true;
                    _4118 = _4286.w <= uniforms.alphaRef0;
                    break;
                }
                case 4:
                {
                    _4117 = true;
                    _4118 = _4286.w > uniforms.alphaRef0;
                    break;
                }
                case 5:
                {
                    _4117 = true;
                    _4118 = _4286.w != uniforms.alphaRef0;
                    break;
                }
                case 6:
                {
                    _4117 = true;
                    _4118 = _4286.w >= uniforms.alphaRef0;
                    break;
                }
                case 7:
                {
                    _4117 = true;
                    _4118 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4117)
            {
                break;
            }
            _4117 = true;
            _4118 = true;
            break;
        } while(false);
        _4142 = false;
        bool _4143;
        do
        {
            switch (uniforms.alphaComp1)
            {
                case 0:
                {
                    _4142 = true;
                    _4143 = false;
                    break;
                }
                case 1:
                {
                    _4142 = true;
                    _4143 = _4286.w < uniforms.alphaRef1;
                    break;
                }
                case 2:
                {
                    _4142 = true;
                    _4143 = _4286.w == uniforms.alphaRef1;
                    break;
                }
                case 3:
                {
                    _4142 = true;
                    _4143 = _4286.w <= uniforms.alphaRef1;
                    break;
                }
                case 4:
                {
                    _4142 = true;
                    _4143 = _4286.w > uniforms.alphaRef1;
                    break;
                }
                case 5:
                {
                    _4142 = true;
                    _4143 = _4286.w != uniforms.alphaRef1;
                    break;
                }
                case 6:
                {
                    _4142 = true;
                    _4143 = _4286.w >= uniforms.alphaRef1;
                    break;
                }
                case 7:
                {
                    _4142 = true;
                    _4143 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_4142)
            {
                break;
            }
            _4142 = true;
            _4143 = true;
            break;
        } while(false);
        bool _4081;
        switch (uniforms.alphaLogic)
        {
            case 0:
            {
                if (_4118)
                {
                    _4081 = _4143;
                }
                else
                {
                    _4081 = false;
                }
                _4079 = true;
                _4080 = _4081;
                break;
            }
            case 1:
            {
                if (_4118)
                {
                    _4081 = true;
                }
                else
                {
                    _4081 = _4143;
                }
                _4079 = true;
                _4080 = _4081;
                break;
            }
            case 2:
            {
                _4079 = true;
                _4080 = _4118 != _4143;
                break;
            }
            case 3:
            {
                _4079 = true;
                _4080 = _4118 == _4143;
                break;
            }
            default:
            {
                break;
            }
        }
        if (_4079)
        {
            break;
        }
        _4079 = true;
        _4080 = true;
        break;
    } while(false);
    if (!_4080)
    {
        discard_fragment();
    }
    out.entryPointParam_fragmentMain = _4286;
    return out;
}

)SHDR3";

#endif
