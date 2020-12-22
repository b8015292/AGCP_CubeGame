//***************************************************************************************
// Default.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Default shader, currently supports lighting.
//***************************************************************************************

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 2
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 1
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

// Include structures and functions for lighting.
#include "LightingUtil.hlsl"

// Constant data that varies per frame.

Texture2D    textureMap : register(t0);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
};

cbuffer cbMaterial : register(b1)
{
	float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float  gRoughness;
	float4x4 gMatTransform;
	float4x4 gMatTransformTop;
	float4x4 gMatTransformBottom;
};

// Constant data that varies per material.
cbuffer cbPass : register(b2)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
};
 
struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC    : TEXCOORD;
    int Side       : SIDE;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

    // Transform to world space.
    float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);

    vout.TexC = vin.TexC;

    //Determine the side of the current face. 1 - top, -1 - bottom, 0 - other
    if (vin.NormalL.y == 1) {
        vout.Side = 1;
    }
    else if (vin.NormalL.y == -1) {
        vout.Side = -1;
    }
    else {
        vout.Side = 0;
    }

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    //Uses different texture positions for different faces
    float2 mulPos;
    if(pin.Side == 1)
        mulPos = mul(float4(pin.TexC, 1, 1), gMatTransformTop);
    else if (pin.Side == -1)
        mulPos = mul(float4(pin.TexC, 1, 1), gMatTransformBottom);
    else 
        mulPos = mul(float4(pin.TexC, 1, 1), gMatTransform);

    float4 col = textureMap.Sample(gsamPointClamp, mulPos);

    // Interpolating normal can unnormalize it, so renormalize it.
    pin.NormalW = normalize(pin.NormalW);

    // Vector from point being lit to eye. 
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

	// Indirect lighting.
    float4 ambient = gAmbientLight*col;

    //const float shininess = 1.0f - gRoughness;ee
    const float shininess = 0.0f;
    Material mat = { col, gFresnelR0, shininess };
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW, 
        pin.NormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // Common convention to take alpha from diffuse material.
    //litColor.a = gDiffuseAlbedo.a;
    //litColor.a = 0.0f;

    return litColor;
}


//*************************************************************************
//  GUI
//*************************************************************************


////User interface shader (doesn't take lighting in account)
//float4 UIPS(VertexOut pin) : SV_Target
//{
//    return textureMap.Sample(gsamPointClamp, pin.TexC);
//}


struct TwoDInput{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};


TwoDInput TwoDVS(float4 position : POSITION, float2 uv : TEXCOORD)
{
    TwoDInput result;
    
    result.position = mul(position, gWorld);
    //result.position = position;
    result.uv = uv;

    return result;
}

float4 TwoDPS(TwoDInput input) : SV_TARGET
{
    return textureMap.Sample(gsamPointWrap, input.uv);
}


//*************************************************************************
//  Sky
//*************************************************************************

TextureCube gCubeMap : register(t0);

struct SkyVertexOut {
    float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};

SkyVertexOut SkyVS(float3 pos : POSITION)
{
    SkyVertexOut vout;

	// Use local vertex position as cubemap lookup vector.
    vout.PosL = pos;
	
	// Transform to world space.
    float4 posW = mul(float4(pos, 1.0f), gWorld);

	// Always center sky about camera.
    posW.xyz += gEyePosW;

	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
    vout.PosH = mul(posW, gViewProj).xyww;
	
    return vout;
}

float4 SkyPS(SkyVertexOut pin) : SV_Target
{
    return gCubeMap.Sample(gsamLinearWrap, pin.PosL);
}