#ifndef REVOLVE_GRAPHICS_SHADER
#define REVOLVE_GRAPHICS_SHADER

inline constexpr const char *SHADER = R"REVOLVE_SHADER(
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

)REVOLVE_SHADER";

#endif
