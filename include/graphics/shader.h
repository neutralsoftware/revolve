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
    int activeTexture;
};

struct main0_out
{
    float4 entryPointParam_fragmentMain [[color(0)]];
};

struct main0_in
{
    float2 input_uv [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant Uniforms_std140& uniforms [[buffer(0)]], texture2d<float> tex0 [[texture(0)]], texture2d<float> tex1 [[texture(1)]], texture2d<float> tex2 [[texture(2)]], texture2d<float> tex3 [[texture(3)]], texture2d<float> tex4 [[texture(4)]], texture2d<float> tex5 [[texture(5)]], texture2d<float> tex6 [[texture(6)]], texture2d<float> tex7 [[texture(7)]], sampler samp0 [[sampler(0)]], sampler samp1 [[sampler(1)]], sampler samp2 [[sampler(2)]], sampler samp3 [[sampler(3)]], sampler samp4 [[sampler(4)]], sampler samp5 [[sampler(5)]], sampler samp6 [[sampler(6)]], sampler samp7 [[sampler(7)]])
{
    bool _365 = false;
    bool _340 = false;
    bool _302 = false;
    bool _243 = false;
    main0_out out = {};
    _243 = false;
    float4 _244;
    do
    {
        switch (uniforms.activeTexture)
        {
            case 0:
            {
                _243 = true;
                _244 = tex0.sample(samp0, in.input_uv);
                break;
            }
            case 1:
            {
                _243 = true;
                _244 = tex1.sample(samp1, in.input_uv);
                break;
            }
            case 2:
            {
                _243 = true;
                _244 = tex2.sample(samp2, in.input_uv);
                break;
            }
            case 3:
            {
                _243 = true;
                _244 = tex3.sample(samp3, in.input_uv);
                break;
            }
            case 4:
            {
                _243 = true;
                _244 = tex4.sample(samp4, in.input_uv);
                break;
            }
            case 5:
            {
                _243 = true;
                _244 = tex5.sample(samp5, in.input_uv);
                break;
            }
            case 6:
            {
                _243 = true;
                _244 = tex6.sample(samp6, in.input_uv);
                break;
            }
            case 7:
            {
                _243 = true;
                _244 = tex7.sample(samp7, in.input_uv);
                break;
            }
            default:
            {
                break;
            }
        }
        if (_243)
        {
            break;
        }
        _243 = true;
        _244 = float4(1.0);
        break;
    } while(false);
    _302 = false;
    bool _303;
    do
    {
        _340 = false;
        bool _341;
        do
        {
            switch (uniforms.alphaComp0)
            {
                case 0:
                {
                    _340 = true;
                    _341 = false;
                    break;
                }
                case 1:
                {
                    _340 = true;
                    _341 = _244.w < uniforms.alphaRef0;
                    break;
                }
                case 2:
                {
                    _340 = true;
                    _341 = _244.w == uniforms.alphaRef0;
                    break;
                }
                case 3:
                {
                    _340 = true;
                    _341 = _244.w <= uniforms.alphaRef0;
                    break;
                }
                case 4:
                {
                    _340 = true;
                    _341 = _244.w > uniforms.alphaRef0;
                    break;
                }
                case 5:
                {
                    _340 = true;
                    _341 = _244.w != uniforms.alphaRef0;
                    break;
                }
                case 6:
                {
                    _340 = true;
                    _341 = _244.w >= uniforms.alphaRef0;
                    break;
                }
                case 7:
                {
                    _340 = true;
                    _341 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_340)
            {
                break;
            }
            _340 = true;
            _341 = true;
            break;
        } while(false);
        _365 = false;
        bool _366;
        do
        {
            switch (uniforms.alphaComp1)
            {
                case 0:
                {
                    _365 = true;
                    _366 = false;
                    break;
                }
                case 1:
                {
                    _365 = true;
                    _366 = _244.w < uniforms.alphaRef1;
                    break;
                }
                case 2:
                {
                    _365 = true;
                    _366 = _244.w == uniforms.alphaRef1;
                    break;
                }
                case 3:
                {
                    _365 = true;
                    _366 = _244.w <= uniforms.alphaRef1;
                    break;
                }
                case 4:
                {
                    _365 = true;
                    _366 = _244.w > uniforms.alphaRef1;
                    break;
                }
                case 5:
                {
                    _365 = true;
                    _366 = _244.w != uniforms.alphaRef1;
                    break;
                }
                case 6:
                {
                    _365 = true;
                    _366 = _244.w >= uniforms.alphaRef1;
                    break;
                }
                case 7:
                {
                    _365 = true;
                    _366 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_365)
            {
                break;
            }
            _365 = true;
            _366 = true;
            break;
        } while(false);
        bool _304;
        switch (uniforms.alphaLogic)
        {
            case 0:
            {
                if (_341)
                {
                    _304 = _366;
                }
                else
                {
                    _304 = false;
                }
                _302 = true;
                _303 = _304;
                break;
            }
            case 1:
            {
                if (_341)
                {
                    _304 = true;
                }
                else
                {
                    _304 = _366;
                }
                _302 = true;
                _303 = _304;
                break;
            }
            case 2:
            {
                _302 = true;
                _303 = _341 != _366;
                break;
            }
            case 3:
            {
                _302 = true;
                _303 = _341 == _366;
                break;
            }
            default:
            {
                break;
            }
        }
        if (_302)
        {
            break;
        }
        _302 = true;
        _303 = true;
        break;
    } while(false);
    if (!_303)
    {
        discard_fragment();
    }
    out.entryPointParam_fragmentMain = _244;
    return out;
}

)SHDR3";

#endif
