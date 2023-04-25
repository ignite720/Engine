#include "../Include/Common.hlsli"

float4 main(VS_OUT i) : SV_TARGET
{
    float4 finalColour = defaultTexture.Sample(defaultSampler, i.uv);
    return finalColour * clamp(sin(timeSinceStartup * 2.0), 0.33, 1.0);
}