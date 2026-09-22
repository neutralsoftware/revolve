
struct VertexIn {
    float4 position : POSITION;
    float4 color : COLOR0;
};

struct VertexOut {
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

[shader("vertex")]
VertexOut vertexMain(VertexIn input) {
    VertexOut output;
    output.position = input.position;
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOut input) : SV_TARGET { return input.color; }
