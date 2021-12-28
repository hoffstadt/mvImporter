
#define M_PI 3.1415926535897932384626433832795

cbuffer MetaData : register(b0)
{
    int resolution;
    int width;
    int height;
};

RWStructuredBuffer<float4> BufferIn : register(u0);

RWStructuredBuffer<float4> FaceOut_0 : register(u1);
RWStructuredBuffer<float4> FaceOut_1 : register(u2);
RWStructuredBuffer<float4> FaceOut_2 : register(u3);
RWStructuredBuffer<float4> FaceOut_3 : register(u4);
RWStructuredBuffer<float4> FaceOut_4 : register(u5);
RWStructuredBuffer<float4> FaceOut_5 : register(u6);
 
float3 uvToXYZ(int face, float2 uv)
{
	if (face == 0) // right
        return float3(1.f, -uv.y, -uv.x);

	else if (face == 1) // left
        return float3(-1.f, -uv.y, uv.x);

	else if (face == 2) // top
        return float3(uv.x, 1.f, +uv.y);

	else if (face == 3) // bottom
        return float3(uv.x, -1.f, -uv.y);

	else if (face == 4) // front
        return float3(uv.x, -uv.y, 1.f);

	else //if(face == 5)
        return float3(-uv.x, -uv.y, -1.f);
}

float2 dirToUV(float3 dir)
{
    return float2(
            0.5f + 0.5f * atan2(dir.z, dir.x) / M_PI,
            1.f - acos(dir.y) / M_PI);
}

void writeFace(int pixel, int face, float3 colorIn)
{
    float4 color = float4(colorIn.rgb, 1.0f);

    if (face == 0)
        FaceOut_0[pixel] = color;
    else if (face == 1)
        FaceOut_1[pixel] = color;
    else if (face == 2)
        FaceOut_2[pixel] = color;
    else if (face == 3)
        FaceOut_3[pixel] = color;
    else if (face == 4)
        FaceOut_4[pixel] = color;
    else //if(face == 5)
        FaceOut_5[pixel] = color;
}

[numthreads(16, 16, 3)]
void main(uint3 groupID : SV_GroupID, uint3 threadID : SV_GroupThreadID)
{
 
    const float xcoord = groupID.x*16 + threadID.x;
    const float ycoord = groupID.y*16 + threadID.y;
    const int face = groupID.z * 3 + threadID.z;
    const float xinc = 1.0f / (float) resolution;
    const float yinc = 1.0f / (float) resolution;
    const float2 inUV = float2(xcoord * xinc, ycoord * yinc);
    const int currentPixel = xcoord + ycoord * resolution;
    
    float3 scan = uvToXYZ(face, inUV * 2.0 - 1.0);	
    float3 direction = normalize(scan);
    float2 src = dirToUV(direction);
   
    int columnindex = width - floor(src.x * (float) width);
    int rowindex = height - floor(src.y * (float) height);
        
    int srcpixelIndex = columnindex + rowindex * width;
        
    float3 color = float3(BufferIn[srcpixelIndex].r, BufferIn[srcpixelIndex].g, BufferIn[srcpixelIndex].b);
	
    //color = pow(color, float3(0.4545.xxx));
    writeFace(currentPixel, face, color);
}