
struct Vertex
{
    float3 position;
    float3 color;
};

RaytracingAccelerationStructure gRtScene : register(t0);
RWTexture2D<float4> gOutput : register(u0);
StructuredBuffer<Vertex> gVertices : register(t1);


struct RayPayload
{
    float3 color;
};

[shader("raygeneration")]
void rayGen()
{
    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();

    float2 crd = float2(launchIndex.xy);
    float2 dims = float2(launchDim.xy);

    float2 d = ((crd / dims) * 2.f - 1.f);
    float aspectRatio = dims.x / dims.y;

    RayDesc ray;
    ray.Origin = float3(0, 0, -2);
    ray.Direction = normalize(float3(d.x * aspectRatio, -d.y, 1));
    ray.TMin = 0;
    ray.TMax = 100000;

    RayPayload payload;
    TraceRay(gRtScene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);

    gOutput[launchIndex.xy] = float4(payload.color, 1);
}

[shader("miss")]
void miss(inout RayPayload payload)
{
    payload.color = float3(0.1, 0.2, 0.4);
}

[shader("closesthit")]
void chs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    uint triIndex = PrimitiveIndex();
    
    uint v0Idx = triIndex * 3 + 0;
    uint v1Idx = triIndex * 3 + 1;
    uint v2Idx = triIndex * 3 + 2;

    Vertex v0 = gVertices[v0Idx];
    Vertex v1 = gVertices[v1Idx];
    Vertex v2 = gVertices[v2Idx];

    float u = attribs.barycentrics.x;
    float v = attribs.barycentrics.y;
    float w = 1.0 - u - v;

    float3 color = v0.color * w + v1.color * u + v2.color * v;

    payload.color = color;  // ? Color interpolado
}