
struct VertexOutput
{
    float4 sv_position : SV_Position;
};

static float2 positions[3] = float2[](
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5),
);


[shader("vertex")]
VertexOutput VertexMain(uint vertexIndex : SV_VertexID)
{
    VertexOutput output;
    output.sv_position = float4(positions[vertexIndex], 0.0, 1.0);

    return output;
}


[shader("fragment")]
float4 FragmentMain() : SV_Target
{
    return float4(1.0, 0.0, 0.0, 1.0);
}

