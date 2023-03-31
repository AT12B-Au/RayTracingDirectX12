#include "RayTracingHeader.hlsli"

[shader("closesthit")] void PointLightHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.color = float3(1.0f,1.0f,1.0f);
}