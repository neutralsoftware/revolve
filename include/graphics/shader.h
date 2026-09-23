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
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 input_position [[attribute(0)]];
    float4 input_color [[attribute(1)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = in.input_position;
    out.entryPointParam_vertexMain_color = in.input_color;
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
};

struct main0_out
{
    float4 entryPointParam_fragmentMain [[color(0)]];
};

struct main0_in
{
    float4 input_color [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant Uniforms_std140& uniforms [[buffer(0)]])
{
    bool _203 = false;
    bool _178 = false;
    bool _140 = false;
    main0_out out = {};
    _140 = false;
    bool _141;
    do
    {
        _178 = false;
        bool _179;
        do
        {
            switch (uniforms.alphaComp0)
            {
                case 0:
                {
                    _178 = true;
                    _179 = false;
                    break;
                }
                case 1:
                {
                    _178 = true;
                    _179 = in.input_color.w < uniforms.alphaRef0;
                    break;
                }
                case 2:
                {
                    _178 = true;
                    _179 = in.input_color.w == uniforms.alphaRef0;
                    break;
                }
                case 3:
                {
                    _178 = true;
                    _179 = in.input_color.w <= uniforms.alphaRef0;
                    break;
                }
                case 4:
                {
                    _178 = true;
                    _179 = in.input_color.w > uniforms.alphaRef0;
                    break;
                }
                case 5:
                {
                    _178 = true;
                    _179 = in.input_color.w != uniforms.alphaRef0;
                    break;
                }
                case 6:
                {
                    _178 = true;
                    _179 = in.input_color.w >= uniforms.alphaRef0;
                    break;
                }
                case 7:
                {
                    _178 = true;
                    _179 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_178)
            {
                break;
            }
            _178 = true;
            _179 = true;
            break;
        } while(false);
        _203 = false;
        bool _204;
        do
        {
            switch (uniforms.alphaComp1)
            {
                case 0:
                {
                    _203 = true;
                    _204 = false;
                    break;
                }
                case 1:
                {
                    _203 = true;
                    _204 = in.input_color.w < uniforms.alphaRef1;
                    break;
                }
                case 2:
                {
                    _203 = true;
                    _204 = in.input_color.w == uniforms.alphaRef1;
                    break;
                }
                case 3:
                {
                    _203 = true;
                    _204 = in.input_color.w <= uniforms.alphaRef1;
                    break;
                }
                case 4:
                {
                    _203 = true;
                    _204 = in.input_color.w > uniforms.alphaRef1;
                    break;
                }
                case 5:
                {
                    _203 = true;
                    _204 = in.input_color.w != uniforms.alphaRef1;
                    break;
                }
                case 6:
                {
                    _203 = true;
                    _204 = in.input_color.w >= uniforms.alphaRef1;
                    break;
                }
                case 7:
                {
                    _203 = true;
                    _204 = true;
                    break;
                }
                default:
                {
                    break;
                }
            }
            if (_203)
            {
                break;
            }
            _203 = true;
            _204 = true;
            break;
        } while(false);
        bool _142;
        switch (uniforms.alphaLogic)
        {
            case 0:
            {
                if (_179)
                {
                    _142 = _204;
                }
                else
                {
                    _142 = false;
                }
                _140 = true;
                _141 = _142;
                break;
            }
            case 1:
            {
                if (_179)
                {
                    _142 = true;
                }
                else
                {
                    _142 = _204;
                }
                _140 = true;
                _141 = _142;
                break;
            }
            case 2:
            {
                _140 = true;
                _141 = _179 != _204;
                break;
            }
            case 3:
            {
                _140 = true;
                _141 = _179 == _204;
                break;
            }
            default:
            {
                break;
            }
        }
        if (_140)
        {
            break;
        }
        _140 = true;
        _141 = true;
        break;
    } while(false);
    if (!_141)
    {
        discard_fragment();
    }
    out.entryPointParam_fragmentMain = in.input_color;
    return out;
}

)SHDR3";

#endif
