#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef struct
{
    vector_float2 position;
    vector_float2 textureCoordinate;
} MetalVertex;

typedef struct
{
    // The [[position]] attribute qualifier of this member indicates this value is
    // the clip space position of the vertex when this structure is returned from
    // the vertex shader
    float4 position [[position]];

    // Since this member does not have a special attribute qualifier, the rasterizer
    // will interpolate its value with values of other vertices making up the triangle
    // and pass that interpolated value to the fragment shader for each fragment in
    // that triangle.
    float2 textureCoordinate;

} RasterizerData;

// Vertex Function
vertex RasterizerData
vertexShader(uint vertexID [[ vertex_id ]], constant MetalVertex *vertexArray [[ buffer(0) ]], constant vector_uint2 *viewportSizePointer  [[ buffer(1) ]])
{

    RasterizerData out;

    float2 pixelSpacePosition = vertexArray[vertexID].position.xy;

    float2 viewportSize = float2(*viewportSizePointer);

    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.xy = pixelSpacePosition / (viewportSize / 2.0);
    out.textureCoordinate = vertexArray[vertexID].textureCoordinate;

    return out;
}

fragment float4
fragmentShader(RasterizerData in [[stage_in]], texture2d<half> colorTexture [[ texture(0) ]], constant int *videoModePointer [[ buffer(1) ]])
{
	int videoMode = int(*videoModePointer);
	
	if ( videoMode == 0)
	{
		constexpr sampler textureSampler (mag_filter::nearest, min_filter::nearest);
		const half4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
		return float4(colorSample);
	}
	else
	{
		constexpr sampler textureSampler (mag_filter::linear, min_filter::linear);
		const half4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
		return float4(colorSample);
	}
}
