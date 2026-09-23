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

struct Uniforms_std140
{
    float alphaRef0;
    float alphaRef1;
    int alphaComp0;
    int alphaComp1;
    int alphaLogic;
    int textureEnabled;
    int activeTexture;
    int activeTexCoord;
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
    bool _381 = false;
    bool _356 = false;
    bool _318 = false;
    bool _259 = false;
    main0_out out = {};
    float4 color;
    if (uniforms.textureEnabled != 0)
    {
        _259 = false;
        float4 _260;
        do
        {
            switch (uniforms.activeTexture)
            {
                case 0:
                {
                    _259 = true;
                    _260 = tex0.sample(samp0, in.input_uv);
                    break;
                }
                case 1:
                {
                    _259 = true;
                    _260 = tex1.sample(samp1, in.input_uv);
                    break;
                }
                case 2:
                {
                    _259 = true;
                    _260 = tex2.sample(samp2, in.input_uv);
                    break;
                }
                case 3:
                {
                    _259 = true;
                    _260 = tex3.sample(samp3, in.input_uv);
                    break;
                }
                case 4:
                {
                    _259 = true;
                    _260 = tex4.sample(samp4, in.input_uv);
                    break;
                }
                case 5:
                {
                    _259 = true;
                    _260 = tex5.sample(samp5, in.input_uv);
                    break;
                }
                case 6:
                {
                    _259 = true;
                    _260 = tex6.sample(samp6, in.input_uv);
                    break;
                }
                case 7:
                {
                    _259 = true;
                    _260 = tex7.sample(samp7, in.input_uv);
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_259)
            {
                break;
            }
            _259 = true;
            _260 = float4(1.0);
            break;
        } while(false);
        color = _260;
    }
    else
    {
        color = in.input_color;
    }
    _318 = false;
    bool _319;
    do
    {
        _356 = false;
        bool _357;
        do
        {
            switch (uniforms.alphaComp0)
            {
                case 0:
                {
                    _356 = true;
                    _357 = false;
                    break;
                }
                case 1:
                {
                    _356 = true;
                    _357 = color.w < uniforms.alphaRef0;
                    break;
                }
                case 2:
                {
                    _356 = true;
                    _357 = color.w == uniforms.alphaRef0;
                    break;
                }
                case 3:
                {
                    _356 = true;
                    _357 = color.w <= uniforms.alphaRef0;
                    break;
                }
                case 4:
                {
                    _356 = true;
                    _357 = color.w > uniforms.alphaRef0;
                    break;
                }
                case 5:
                {
                    _356 = true;
                    _357 = color.w != uniforms.alphaRef0;
                    break;
                }
                case 6:
                {
                    _356 = true;
                    _357 = color.w >= uniforms.alphaRef0;
                    break;
                }
                case 7:
                {
                    _356 = true;
                    _357 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_356)
            {
                break;
            }
            _356 = true;
            _357 = true;
            break;
        } while(false);
        _381 = false;
        bool _382;
        do
        {
            switch (uniforms.alphaComp1)
            {
                case 0:
                {
                    _381 = true;
                    _382 = false;
                    break;
                }
                case 1:
                {
                    _381 = true;
                    _382 = color.w < uniforms.alphaRef1;
                    break;
                }
                case 2:
                {
                    _381 = true;
                    _382 = color.w == uniforms.alphaRef1;
                    break;
                }
                case 3:
                {
                    _381 = true;
                    _382 = color.w <= uniforms.alphaRef1;
                    break;
                }
                case 4:
                {
                    _381 = true;
                    _382 = color.w > uniforms.alphaRef1;
                    break;
                }
                case 5:
                {
                    _381 = true;
                    _382 = color.w != uniforms.alphaRef1;
                    break;
                }
                case 6:
                {
                    _381 = true;
                    _382 = color.w >= uniforms.alphaRef1;
                    break;
                }
                case 7:
                {
                    _381 = true;
                    _382 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_381)
            {
                break;
            }
            _381 = true;
            _382 = true;
            break;
        } while(false);
        bool _320;
        switch (uniforms.alphaLogic)
        {
            case 0:
            {
                if (_357)
                {
                    _320 = _382;
                }
                else
                {
                    _320 = false;
                }
                _318 = true;
                _319 = _320;
                break;
            }
            case 1:
            {
                if (_357)
                {
                    _320 = true;
                }
                else
                {
                    _320 = _382;
                }
                _318 = true;
                _319 = _320;
                break;
            }
            case 2:
            {
                _318 = true;
                _319 = _357 != _382;
                break;
            }
            case 3:
            {
                _318 = true;
                _319 = _357 == _382;
                break;
            }
            default:
            {
                break;
            }
        }
        if (_318)
        {
            break;
        }
        _318 = true;
        _319 = true;
        break;
    } while(false);
    if (!_319)
    {
        discard_fragment();
    }
    out.entryPointParam_fragmentMain = color;
    return out;
}

)SHDR3";

#endif
