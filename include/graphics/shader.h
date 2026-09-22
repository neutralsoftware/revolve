#ifndef REVOLVE_GRAPHICS_SHADER
#define REVOLVE_GRAPHICS_SHADER

inline constexpr const char *FULLSCREEN_SHADER = R"SHDR0(
#include <metal_stdlib>
#include <metal_math>
#include <metal_texture>
using namespace metal;

#line 6 "/Users/maxvdec/Coding/Projects/Revolve/shaders/fullscreen.slang"
struct VertexOut_0
{
    float4 position_0 [[position]];
    float2 uv_0 [[user(TEXCOORD)]];
};


#line 6
struct vertexInput_0
{
    float2 position_1 [[attribute(0)]];
    float2 uv_1 [[attribute(1)]];
};


#line 24
struct KernelContext_0
{
    texture2d<float, access::sample> xfbTexture_0;
    sampler xfbSampler_0;
};


#line 15
[[vertex]] VertexOut_0 vertexMain(vertexInput_0 _S1 [[stage_in]], texture2d<float, access::sample> xfbTexture_1 [[texture(0)]], sampler xfbSampler_1 [[sampler(0)]])
{

#line 15
    KernelContext_0 kernelContext_0;

#line 15
    (&kernelContext_0)->xfbTexture_0 = xfbTexture_1;

#line 15
    (&kernelContext_0)->xfbSampler_0 = xfbSampler_1;
    thread VertexOut_0 output_0;
    (&output_0)->position_0 = float4(_S1.position_1, 0.0, 1.0);
    (&output_0)->uv_0 = _S1.uv_1;
    return output_0;
}


#line 4325 "core.meta.slang"
struct pixelOutput_0
{
    float4 output_1 [[color(0)]];
};


#line 4325
struct pixelInput_0
{
    float2 uv_2 [[user(TEXCOORD)]];
};


#line 23 "/Users/maxvdec/Coding/Projects/Revolve/shaders/fullscreen.slang"
[[fragment]] pixelOutput_0 fragmentMain(pixelInput_0 _S2 [[stage_in]], float4 position_2 [[position]], texture2d<float, access::sample> xfbTexture_2 [[texture(0)]], sampler xfbSampler_2 [[sampler(0)]])
{

#line 23
    KernelContext_0 kernelContext_1;

#line 23
    (&kernelContext_1)->xfbTexture_0 = xfbTexture_2;

#line 23
    (&kernelContext_1)->xfbSampler_0 = xfbSampler_2;

#line 23
    pixelOutput_0 _S3 = { (((&kernelContext_1)->xfbTexture_0).sample((xfbSampler_2), (_S2.uv_2))) };
    return _S3;
}

)SHDR0";

inline constexpr const char *MINIMAL_SHADER = R"SHDR1(
#include <metal_stdlib>
#include <metal_math>
#include <metal_texture>
using namespace metal;

#line 7 "/Users/maxvdec/Coding/Projects/Revolve/shaders/minimal.slang"
struct VertexOut_0
{
    float4 position_0 [[position]];
    float4 color_0 [[user(COLOR)]];
};


#line 7
struct vertexInput_0
{
    float4 position_1 [[attribute(0)]];
    float4 color_1 [[attribute(1)]];
};

[[vertex]] VertexOut_0 vertexMain(vertexInput_0 _S1 [[stage_in]])
{

#line 14
    thread VertexOut_0 output_0;
    (&output_0)->position_0 = _S1.position_1;
    (&output_0)->color_0 = _S1.color_1;
    return output_0;
}


#line 17
struct pixelOutput_0
{
    float4 output_1 [[color(0)]];
};


#line 17
struct pixelInput_0
{
    float4 color_2 [[user(COLOR)]];
};


#line 21
[[fragment]] pixelOutput_0 fragmentMain(pixelInput_0 _S2 [[stage_in]], float4 position_2 [[position]])
{

#line 21
    pixelOutput_0 _S3 = { _S2.color_2 };

#line 21
    return _S3;
}

)SHDR1";

#endif
