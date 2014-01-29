// General vars
float4x4 View;
float4x4 Projection;
float4x4 ViewProjection;
float4x4 InvertViewProjection;
float4x4 InvertWVP;
float3 CameraPosition;
float3 CameraViewDirection;
float CameraNear;
float CameraFar;
float ScreenWidth;
float ScreenHeight;
float DistanceToTarget;
float4 VportXformInv;
float WorldTime;
float WiggleTime = 0;
float PlayerLife = 100;

// Object Specific
matrix World;
float RTWidth;
float RTHeight;
float4 DiffuseColor = float4(1,1,1,1);

// Textures
texture2D Diffuse;
texture2D Diffuse2;
texture2D Lightmap;
texture2D Mask;
texture2D Emissive;
texture2D Normal;
texture SkyBoxTexture;
texture2D Noise;
texture2D Stereo;

// RT Textures
texture2D ColorRT;
//texture2D EmissiveRT;
//texture2D LightmapRT;
texture2D NormalRT;
texture2D DepthRT;
texture2D LightRT;
texture2D FinalRT;
texture2D BlurRT;
texture2D BloomRT;

// Light parameters
float	AmbientReductionFactor = 1;
float3	LightAmbient = float3(0.5f,0.5f,0.5f);
float3	LightPosition   = float3(0,1,0);
float	LightRadius     = 1;
float3	LightColor      = float3(0.25f,0.25f,0.5f);
float	LightIntensity  = 1.0f;
float3	LightDir		= float3(-1,-1,-1);

float SpecularIntensity = 0.8f;
float SpecularPower     = 0.5;

// Image Effects
float Brightness = 0.1;
float Contrast = 1.2;
float3 Saturation = float3(0,0,0);

// Toon Shader
float3 LineColor = float3(0.0,0,0.0f);
float LineThickness = .005;
float Atten = 1;

// Forward Fog
float3 FogColor = float3(0.5f,0.5f,0.5f);
float FogDensity = 0.01;

// Deferred Fog
float FogStart = 1.0f;
float FogEnd = 35.0f;

// Vignetting
float VigForce = 1.0f;

// Blur
//float BlurSize = 0.001f;
float SimpleBlurSize = 1.0f;
float BlurSize = 1.0f;

// Bloom
float Threshold = 0.4;

// Controls the Intensity of the bloom texture
float BloomIntensity = 1.2;
 
// Controls the Intensity of the original scene texture
float OriginalIntensity = 1.0;
 
// Saturation amount on bloom
float BloomSaturation = 1.1;
 
// Saturation amount on original scene
float OriginalSaturation = 1.0;

// Depth of Field
//float Distance = 1.0f;
float Range = 40.0f;
//float Near = 0.5f;
//float Far = 20.0f;

// Samplers
sampler DiffuseSampler = sampler_state
{
	Texture = <Diffuse>;
	
	AddressU = Wrap;
	AddressV = Wrap;
	
	MagFilter = Linear;
	MinFilter = Linear;
	MipFilter = Linear;
};

sampler Diffuse2Sampler = sampler_state
{
	Texture = <Diffuse2>;
	
	AddressU = Wrap;
	AddressV = Wrap;
	
	MagFilter = Linear;
	MinFilter = Linear;
	MipFilter = Linear;
};

sampler NoiseSampler = sampler_state
{
	Texture = <Noise>;
	
	AddressU = Wrap;
	AddressV = Wrap;
	
	MagFilter = Linear;
	MinFilter = Linear;
	MipFilter = Linear;
};

sampler StereoSampler = sampler_state
{
	Texture = <Stereo>;
	
	AddressU = Wrap;
	AddressV = Wrap;
	
	MagFilter = NONE;
	MinFilter = NONE;
	MipFilter = NONE;
};

sampler LightmapSampler = sampler_state
{
	Texture = <Lightmap>;
	
	AddressU = Wrap;
	AddressV = Wrap;
	
	MagFilter = Linear;
	MinFilter = Linear;
	MipFilter = Linear;
};

sampler MaskSampler = sampler_state
{
	Texture = <Mask>;
	
	AddressU = Wrap;
	AddressV = Wrap;
	
	MagFilter = LINEAR;
	MinFilter = LINEAR;
	MipFilter = LINEAR;
};

sampler EmissiveSampler = sampler_state
{
	Texture = <Emissive>;
	
	AddressU = Wrap;
	AddressV = Wrap;
	
	MagFilter = LINEAR;
	MinFilter = LINEAR;
	MipFilter = LINEAR;
};

sampler NormalSampler = sampler_state
{
	Texture = <Normal>;
	
	AddressU = CLAMP;
	AddressV = CLAMP;
	
	MagFilter = POINT;
	MinFilter = POINT;
	MipFilter = POINT;
};

sampler ColorRTSampler = sampler_state
{
	Texture = <ColorRT>;
	
	AddressU = CLAMP;
	AddressV = CLAMP;
	
	MagFilter = Linear;
	MinFilter = Linear;
	MipFilter = Linear;
};

sampler BlurRTSampler = sampler_state
{
	Texture = <BlurRT>;
	
	AddressU = CLAMP;
	AddressV = CLAMP;
	
	MagFilter = Linear;
	MinFilter = Linear;
	MipFilter = Linear;
};

sampler BloomRTSampler = sampler_state
{
	Texture = <BloomRT>;
	
	AddressU = CLAMP;
	AddressV = CLAMP;
	
	MagFilter = Linear;
	MinFilter = Linear;
	MipFilter = Linear;
};

sampler NormalRTSampler = sampler_state
{
	Texture = <NormalRT>;

	MagFilter = POINT;
	MinFilter = POINT;
	MinFilter = POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

sampler DepthRTSampler = sampler_state
{
	Texture = <DepthRT>;

	MagFilter = POINT;
	MinFilter = POINT;
	Mipfilter = POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

sampler LightRTSampler = sampler_state
{
	Texture = <LightRT>;
	AddressU = CLAMP;
	AddressV = CLAMP;
	MagFilter = Linear;
	MinFilter = Linear;
	Mipfilter = Linear;
};

sampler FinalRTSampler = sampler_state
{
	Texture = <FinalRT>;
	AddressU = CLAMP;
	AddressV = CLAMP;
	MagFilter = Linear;
	MinFilter = Linear;
	Mipfilter = Linear;
};

samplerCUBE SkyBoxSampler = sampler_state
{
	texture = <SkyBoxTexture>;
	
	AddressU = MIRROR;
	AddressV = MIRROR;
	
	MagFilter = LINEAR;
	MinFilter = LINEAR;
	MipFilter = LINEAR;
};

// Auxiliar methods
uniform float4 bones[ 64*3 ];
float3x4 getSkinMatrix( float4 iBoneIds, float4 iWeights ) {
	float3x4 skin_mtx;			// 3 rows, 4 columns
	skin_mtx[0] = bones[iBoneIds.x*3+0] * iWeights.x
	            + bones[iBoneIds.y*3+0] * iWeights.y
	            + bones[iBoneIds.z*3+0] * iWeights.z
	            + bones[iBoneIds.w*3+0] * iWeights.w;
	skin_mtx[1] = bones[iBoneIds.x*3+1] * iWeights.x
	            + bones[iBoneIds.y*3+1] * iWeights.y
	            + bones[iBoneIds.z*3+1] * iWeights.z
	            + bones[iBoneIds.w*3+1] * iWeights.w;
	skin_mtx[2] = bones[iBoneIds.x*3+2] * iWeights.x
	            + bones[iBoneIds.y*3+2] * iWeights.y
	            + bones[iBoneIds.z*3+2] * iWeights.z
	            + bones[iBoneIds.w*3+2] * iWeights.w;
	return skin_mtx;
}

float3 AdjustSaturation(float3 color, float saturation)

{
    // We define gray as the same color we used in the grayscale shader
    float grey = dot(color, float3(0.3, 0.59, 0.11));
   
    return lerp(grey, color, saturation);
}

//Normal Decoding Function
float3 decode(float3 enc)
{
	return (2.0f * enc.xyz - 1.0f);
}

float rand(float2 co)
{
    return frac(sin(dot(co.xy ,float2(12.9898,78.233))) * 43758.5453);
}

int randInt(int start, int end)
{
    return int(frac(sin(dot(float2(start, end),float2(12.9898,78.233))) * 43758.5453));
}

struct VS_SIMPLE
{
	float4 vPos0		: POSITION0;
	float2 vTex0		: TEXCOORD0;
	float3 vNrm            : NORMAL;
};

struct VS_INPUT_BASIC
{
    float3  vPos            : POSITION0;
    float3  vNrm            : NORMAL;
    float2  vTex0           : TEXCOORD0;
    float4  vTan            : TEXCOORD1;
};

struct VS_INPUT_LIGHTMAP
{
    float3  vPos            : POSITION0;
    float3  vNrm            : NORMAL;
    float2  vTex0           : TEXCOORD0;
	float2  vTex1           : TEXCOORD1; // lightmap uv
    float4  vTan            : TEXCOORD2;
};

struct VS_INPUT_MIX
{
    float3  vPos            : POSITION0;
    float3  vNrm            : NORMAL;
    float2  vTex0           : TEXCOORD0;
	float2  vTex1           : TEXCOORD1; // mix uv
    float4  vTan            : TEXCOORD2;
};

struct VS_INPUT_MIX_LIGHTMAP
{
    float3  vPos            : POSITION0;
    float3  vNrm            : NORMAL;
    float2  vTex0           : TEXCOORD0;
	float2  vTex1           : TEXCOORD1; // mix uv
	float2  vTex2           : TEXCOORD2; // lmap uv
    float4  vTan            : TEXCOORD3;
};

struct VS_INPUT_SKIN
{
    float4  vPos            : POSITION0;
    float3  vNrm            : NORMAL;
    float2  vTex0           : TEXCOORD0;
    float4  vTan            : TEXCOORD1;
	float4	vWeights		: BLENDWEIGHT;
	float4	vBoneIds		: BLENDINDICES;
};

struct VS_OUTPUT_BASIC
{
	float4	vPos			: POSITION0;
    float2  vTex0           : TEXCOORD0;
	float2	vDepth			: TEXCOORD1;
	float3	vNrm			: TEXCOORD2;
	float3  vWorldPos		: TEXCOORD3;
	float4  vTan			: TEXCOORD4;
};

struct VS_OUTPUT_LIGHTMAP
{
	float4	vPos			: POSITION0;
    float2  vTex0           : TEXCOORD0;
	float2  vTex1           : TEXCOORD1;
	float2	vDepth			: TEXCOORD2;
	float3	vNrm			: TEXCOORD3;
	float3  vWorldPos		: TEXCOORD4;
	float4  vTan			: TEXCOORD5;
};

struct VS_OUTPUT_MIX
{
	float4	vPos			: POSITION0;
    float2  vTex0           : TEXCOORD0;
	float2  vTex1           : TEXCOORD1;
	float2	vDepth			: TEXCOORD2;
	float3	vNrm			: TEXCOORD3;
	float3  vWorldPos		: TEXCOORD4;
	float4  vTan			: TEXCOORD5;
};

struct VS_OUTPUT_MIX_LIGHTMAP
{
	float4	vPos			: POSITION0;
    float2  vTex0           : TEXCOORD0;
	float2  vTex1           : TEXCOORD1;
	float2  vTex2           : TEXCOORD2;
	float2	vDepth			: TEXCOORD3;
	float3	vNrm			: TEXCOORD4;
	float3  vWorldPos		: TEXCOORD5;
	float4  vTan			: TEXCOORD6;
};

struct VS_OUTPUT_SKYBOX
{
	float4 Position		: POSITION0;
	float3 Tex		: TEXCOORD0;
};

struct PS_SIMPLE
{
	float4 vPos0		: POSITION0;
	float2 vTex0		: TEXCOORD0;
};

PS_SIMPLE vsScreen(VS_SIMPLE input)
{
	PS_SIMPLE output;
	output.vPos0 = input.vPos0;
	output.vTex0 = input.vTex0;
	output.vTex0 -= float2(ScreenWidth/2.0f,ScreenHeight/2.0f);
	return output;
}

VS_OUTPUT_BASIC vsBasic(VS_INPUT_BASIC i)
{
	VS_OUTPUT_BASIC o;

	float4 pos = mul(float4(i.vPos, 1),World);
	o.vWorldPos = pos;

	o.vPos = mul(pos, ViewProjection);

	o.vTex0 = i.vTex0;	
	/*o.vTex1 = i.vTex1;*/

	o.vDepth.x = o.vPos.z;
	o.vDepth.y = o.vPos.w;

	o.vNrm = mul(i.vNrm, (float3x3) World);
	o.vNrm = normalize(o.vNrm);

	o.vTan.xyz = mul(i.vTan.xyz, (float3x3) World);
	o.vTan.w = 1;
	o.vTan = normalize(o.vTan);


	return o;
};

VS_OUTPUT_LIGHTMAP vsLightmap(VS_INPUT_LIGHTMAP i)
{
	VS_OUTPUT_LIGHTMAP o;

	float4 pos = mul(float4(i.vPos, 1),World);
	o.vWorldPos = pos;

	o.vPos = mul(pos, ViewProjection);

	o.vTex0 = i.vTex0;	
	o.vTex1 = i.vTex1;

	o.vDepth.x = o.vPos.z;
	o.vDepth.y = o.vPos.w;

	o.vNrm = mul(i.vNrm, (float3x3) World);
	o.vNrm = normalize(o.vNrm);

	o.vTan.xyz = mul(i.vTan.xyz, (float3x3) World);
	o.vTan.w = 1;
	o.vTan = normalize(o.vTan);

	return o;
};

VS_OUTPUT_MIX vsMix(VS_INPUT_MIX i)
{
	VS_OUTPUT_MIX o;

	float4 pos = mul(float4(i.vPos, 1),World);
	o.vWorldPos = pos;

	o.vPos = mul(pos, ViewProjection);

	o.vTex0 = i.vTex0;	
	o.vTex1 = i.vTex1;

	o.vDepth.x = o.vPos.z;
	o.vDepth.y = o.vPos.w;

	o.vNrm = mul(i.vNrm, (float3x3) World);
	o.vNrm = normalize(o.vNrm);

	o.vTan.xyz = mul(i.vTan.xyz, (float3x3) World);
	o.vTan.w = 1;
	o.vTan = normalize(o.vTan);

	return o;
};

VS_OUTPUT_MIX_LIGHTMAP vsMixLightmap(VS_INPUT_MIX_LIGHTMAP i)
{
	VS_OUTPUT_MIX_LIGHTMAP o;

	float4 pos = mul(float4(i.vPos, 1),World);
	o.vWorldPos = pos;

	o.vPos = mul(pos, ViewProjection);

	o.vTex0 = i.vTex0;	
	o.vTex1 = i.vTex1;
	o.vTex2 = i.vTex2;

	o.vDepth.x = o.vPos.z;
	o.vDepth.y = o.vPos.w;

	o.vNrm = mul(i.vNrm, (float3x3) World);
	o.vNrm = normalize(o.vNrm);

	o.vTan.xyz = mul(i.vTan.xyz, (float3x3) World);
	o.vTan.w = 1;
	o.vTan = normalize(o.vTan);

	return o;
};

VS_OUTPUT_BASIC vsWater(VS_INPUT_BASIC i)
{
	VS_OUTPUT_BASIC o;

	o.vNrm = mul(i.vNrm, (float3x3) World);
	o.vNrm = normalize(o.vNrm);

	float4 pos = mul(float4(i.vPos, 1),World);
	pos.y += sin(pos.x  + WorldTime/2)/4;
	o.vWorldPos = pos;
	o.vPos = mul(pos, ViewProjection);

	o.vTex0 = i.vTex0;	

	o.vDepth.x = o.vPos.z;
	o.vDepth.y = o.vPos.w;

	o.vTan.xyz = mul(i.vTan.xyz, (float3x3) World);
	o.vTan.w = 1;
	o.vTan = normalize(o.vTan);

	return o;
};

VS_OUTPUT_BASIC vsSkin( VS_INPUT_SKIN i)  
{
	VS_OUTPUT_BASIC o;

	float3x4 skin_mtx = getSkinMatrix( i.vBoneIds, i.vWeights );

	o.vPos = float4(mul(skin_mtx, i.vPos),1);
	o.vWorldPos = o.vPos;
	//o.vPos = float4(i.vPos,1);
	//o.vPos = mul (o.vPos, World);
	o.vPos = mul (o.vPos,ViewProjection); 

    o.vNrm = mul( (float3x3)skin_mtx, i.vNrm );
	//o.vNrm = mul(o.vNrm, (float3x3) World);
	o.vNrm = normalize(o.vNrm);

	o.vDepth.x = o.vPos.z;
	o.vDepth.y = o.vPos.w;

	o.vTan.xyz = mul(i.vTan.xyz, (float3x3) World);
	o.vTan.w = 1;
	o.vTan = normalize(o.vTan);

	o.vTex0 = i.vTex0;
	return o;
}

VS_OUTPUT_SKYBOX vsSkyBox(VS_SIMPLE input)
{
    VS_OUTPUT_SKYBOX output;
 
    float4 worldPosition = mul(input.vPos0, World);
    output.Position = mul(worldPosition, ViewProjection);
    float4 VertexPosition = mul(input.vPos0, World);
    output.Tex = VertexPosition.xyz - CameraPosition;
 
    return output;
}

struct PS_OUTPUT_BASIC
{
	float4	vColor		: COLOR0;
	float4	vNrm		: COLOR1;
	float4	vDepth		: COLOR2;
	float4	vLightmap	: COLOR3;
};

struct PS_INPUT_POINTLIGHT
{
	float4 vPos0		: POSITION0;
	float4 vScr0		: TEXCOORD0;
};

struct PS_INPUT_DECAL
{
	float4 vPos0		: POSITION0;
	float4 vScr0		: TEXCOORD0;
	float2 vTex0		: TEXCOORD1;
};

PS_OUTPUT_BASIC psBasic(VS_OUTPUT_BASIC i)
{
	PS_OUTPUT_BASIC o;

	// diffuse
	float4 diffuse = tex2D(DiffuseSampler, i.vTex0);
	if (diffuse.a < 0.75f) discard;
	diffuse.a = 1.0f;

	o.vColor = diffuse * DiffuseColor;

	// position
	o.vDepth = i.vDepth.x / i.vDepth.y;

	//// emissive
	float4 emissive = tex2D(EmissiveSampler, i.vTex0);

	// normal
	float3 biNormal = cross( i.vNrm, i.vTan.xyz ) * i.vTan.w;
	float3 bumpMap = tex2D(NormalSampler, i.vTex0).xyz;
	if ((bumpMap.x + bumpMap.y + bumpMap.z) <= 0) bumpMap = float3(0.5f,0.5f,1.0f);
	bumpMap = (bumpMap * 2.0f) - 1.0f;
	
	float3x3 TBN = float3x3(i.vTan.xyz, biNormal, i.vNrm );
	TBN = transpose( TBN );
	float3 bump_world_coords = mul( bumpMap, TBN );

	o.vNrm.xyz = i.vNrm * 0.5f + 0.5f;
	o.vNrm.w = 1.0;

	// normal
	o.vNrm.xyz = i.vNrm * 0.5f + 0.5f;
	o.vNrm.w = 1.0;
	//o.vLightmap = float4(0.25f,0.25f,0.25f,1);

	float3 ldir = -normalize(LightDir);
	float NdL = max(0,dot(i.vNrm.xyz,ldir));

	if (NdL <= 0.0f) NdL = 0.25f;
	else NdL = 0.75f;
	
	o.vLightmap = float4(LightAmbient * NdL,1);

	if (emissive.r > 0.25f)
	{
		o.vColor.rgb = emissive*2;
		o.vLightmap = float4(0.5f,0.5f,0.5f,0.99f);
	}
	return o;


};

PS_OUTPUT_BASIC psNoLight(VS_OUTPUT_BASIC i)
{
	PS_OUTPUT_BASIC o;

	// diffuse
	float4 diffuse = tex2D(DiffuseSampler, i.vTex0);
	if (diffuse.a < 0.75f) discard;
	diffuse.a = 1.0f;

	o.vColor = diffuse * DiffuseColor;

	// position
	o.vDepth = i.vDepth.x / i.vDepth.y;

	// emissive
	float4 emissive = tex2D(EmissiveSampler, i.vTex0);

	// normal
	float3 biNormal = cross( i.vNrm, i.vTan.xyz ) * i.vTan.w;
	float3 bumpMap = tex2D(NormalSampler, i.vTex0).xyz;
	if ((bumpMap.x + bumpMap.y + bumpMap.z) <= 0) bumpMap = float3(0.5f,0.5f,1.0f);
	bumpMap = (bumpMap * 2.0f) - 1.0f;

	//bumpMap = float3(0.5f,0.5f,1.0f);

	float3x3 TBN = float3x3(i.vTan.xyz, biNormal, i.vNrm );
	TBN = transpose( TBN );
	float3 bump_world_coords = mul( bumpMap, TBN );

	o.vNrm.xyz = bump_world_coords * 0.5f + 0.5f;
	o.vNrm.w = 1.0;

	float3 ldir = -normalize(LightDir);
	float NdL = max(0,dot(i.vNrm.xyz,ldir));

	if (NdL <= 0.0f) NdL = 0.25f;
	else NdL = 0.75f;
	
	o.vLightmap = float4((LightAmbient * NdL) / AmbientReductionFactor,1);

	if (emissive.r > 0.25f)
	{
		o.vColor.rgb = emissive*2;
		o.vLightmap = float4(0.5f,0.5f,0.5f,0.99f);
	}
	return o;
};

PS_OUTPUT_BASIC psCrow(VS_OUTPUT_BASIC i)
{
	PS_OUTPUT_BASIC o;

	// diffuse
	float2 vTexDif = i.vTex0;
	vTexDif.x += WorldTime/6;
	float4 diffuse = tex2D(DiffuseSampler, vTexDif);
	if (diffuse.a < 0.75f) discard;
	diffuse.a = 1.0f;

	o.vColor = diffuse * DiffuseColor;

	// position
	o.vDepth = i.vDepth.x / i.vDepth.y;

	// emissive
	float4 emissive = tex2D(EmissiveSampler, i.vTex0);

	// normal
	float3 biNormal = cross( i.vNrm, i.vTan.xyz ) * i.vTan.w;
	float3 bumpMap = tex2D(NormalSampler, i.vTex0).xyz;
	if ((bumpMap.x + bumpMap.y + bumpMap.z) <= 0) bumpMap = float3(0.5f,0.5f,1.0f);
	bumpMap = (bumpMap * 2.0f) - 1.0f;

	//bumpMap = float3(0.5f,0.5f,1.0f);

	float3x3 TBN = float3x3(i.vTan.xyz, biNormal, i.vNrm );
	TBN = transpose( TBN );
	float3 bump_world_coords = mul( bumpMap, TBN );

	o.vNrm.xyz = bump_world_coords * 0.5f + 0.5f;
	o.vNrm.w = 1.0;

	float3 ldir = -normalize(LightDir);
	float NdL = max(0,dot(i.vNrm.xyz,ldir));

	if (NdL <= 0.0f) NdL = 0.25f;
	else NdL = 0.75f;
	
	o.vLightmap = float4((LightAmbient * NdL) / AmbientReductionFactor,1);
	o.vLightmap = float4(0.5f,0.5f,0.5f,0.99f);
	if (emissive.r > 0.25f)
	{
		o.vColor.rgb = emissive*2;
		o.vLightmap = float4(0.5f,0.5f,0.5f,0.99f);
	}
	return o;
};

PS_OUTPUT_BASIC psSkinDirLight(VS_OUTPUT_BASIC i)
{
	PS_OUTPUT_BASIC o;

	// diffuse
	float4 diffuse = tex2D(DiffuseSampler, i.vTex0);
	if (diffuse.a < 0.75f) discard;
	diffuse.a = 1.0f;
	o.vColor = diffuse * DiffuseColor;

	// position
	o.vDepth = i.vDepth.x / i.vDepth.y;

	// emissive
	float4 emissive = tex2D(EmissiveSampler, i.vTex0);

	// normal
	float3 biNormal = cross( i.vNrm, i.vTan.xyz ) * i.vTan.w;
	float3 bumpMap = tex2D(NormalSampler, i.vTex0).xyz;
	if ((bumpMap.x + bumpMap.y + bumpMap.z) <= 0) bumpMap = float3(0.5f,0.5f,1.0f);
	bumpMap = (bumpMap * 2.0f) - 1.0f;

	//bumpMap = float3(0.5f,0.5f,1.0f);

	float3x3 TBN = float3x3(i.vTan.xyz, biNormal, i.vNrm );
	TBN = transpose( TBN );
	float3 bump_world_coords = mul( bumpMap, TBN );

	o.vNrm.xyz = bump_world_coords * 0.5f + 0.5f;
	o.vNrm.w = 1.0;

	float3 ldir = -normalize(LightDir);
	float NdL = max(0,dot(i.vNrm.xyz,ldir));

	if (NdL <= 0.0f) NdL = 0.25f;
	else NdL = 0.75f;
	
	o.vLightmap = float4((LightAmbient * NdL) / AmbientReductionFactor,1);

	if (emissive.r > 0.25f)
	{
		o.vColor.rgb = emissive*2;
		o.vLightmap = float4(0.5f,0.5f,0.5f,0.99f);
	}
	return o;
};

PS_OUTPUT_BASIC psShadow(VS_OUTPUT_BASIC i)
{
	PS_OUTPUT_BASIC o;

	// diffuse
	float4 diffuse = tex2D(DiffuseSampler, i.vTex0);
	if (diffuse.a < 0.75f) discard;
	diffuse.a = 1.0f;
	diffuse *= DiffuseColor;
	o.vColor.rgb =  diffuse.rgb;
	o.vColor.a = 1.0f;

	float limit = PlayerLife/100.0f;
	limit = limit*0.45f + 0.4f;
	
	float4 mask = tex2D(MaskSampler, i.vTex0);
	if (i.vTex0.y > limit) mask.rgb = float3(0,0,0);
	o.vColor.rgb += mask;

	// position
	o.vDepth = i.vDepth.x / i.vDepth.y;

	// emissive
	float4 emissive = tex2D(EmissiveSampler, i.vTex0);
	emissive += mask;

	// normal
	float3 biNormal = cross( i.vNrm, i.vTan.xyz ) * i.vTan.w;
	float3 bumpMap = tex2D(NormalSampler, i.vTex0).xyz;
	if ((bumpMap.x + bumpMap.y + bumpMap.z) <= 0) bumpMap = float3(0.5f,0.5f,1.0f);
	bumpMap = (bumpMap * 2.0f) - 1.0f;
	
	//bumpMap = float3(0.5f,0.5f,1.0f);

	float3x3 TBN = float3x3(i.vTan.xyz, biNormal, i.vNrm );
	TBN = transpose( TBN );
	float3 bump_world_coords = mul( bumpMap, TBN );

	o.vNrm.xyz = bump_world_coords * 0.5f + 0.5f;
	o.vNrm.w = 1.0;

	float3 ldir = -normalize(LightDir);
	float NdL = max(0,dot(i.vNrm.xyz,ldir));

	if (NdL <= 0.25f) NdL = 0.25f;
	else NdL = 0.75f;
	
	o.vLightmap = float4((LightAmbient/2) * NdL,1);
	
	if (emissive.r > 0.25f)
	{
		o.vColor.rgb = emissive*2;
		o.vLightmap = float4(0.5f,0.5f,0.5f,0.99f);
	}
	//o.vColor.rgb = emissive*2;
	//if (emissive.r < 0.5f) discard;
	return o;
};

PS_OUTPUT_BASIC psShadowScarf(VS_OUTPUT_BASIC i)
{
	PS_OUTPUT_BASIC o;

	float limit = PlayerLife/100.0f;
	limit = limit*0.45f + 0.4f;
	
	float4 mask = tex2D(MaskSampler, i.vTex0);
	if (i.vTex0.y > limit) mask.rgb = float3(0,0,0);
	o.vColor.rgb = float3(0,0,0);
	o.vColor.a = 1.0f;
	o.vColor *= DiffuseColor;

	// position
	o.vDepth = i.vDepth.x / i.vDepth.y;

	// emissive
	float4 emissive = tex2D(EmissiveSampler, i.vTex0);
	emissive += mask;

	// normal
	float3 biNormal = cross( i.vNrm, i.vTan.xyz ) * i.vTan.w;
	float3 bumpMap = tex2D(NormalSampler, i.vTex0).xyz;
	if ((bumpMap.x + bumpMap.y + bumpMap.z) <= 0) bumpMap = float3(0.5f,0.5f,1.0f);
	bumpMap = (bumpMap * 2.0f) - 1.0f;
	
	//bumpMap = float3(0.5f,0.5f,1.0f);

	float3x3 TBN = float3x3(i.vTan.xyz, biNormal, i.vNrm );
	TBN = transpose( TBN );
	float3 bump_world_coords = mul( bumpMap, TBN );

	o.vNrm.xyz = bump_world_coords * 0.5f + 0.5f;
	o.vNrm.w = 1.0;

	float3 ldir = -normalize(LightDir);
	float NdL = max(0,dot(i.vNrm.xyz,ldir));

	if (NdL <= 0.25f) NdL = 0.25f;
	else NdL = 0.75f;
	
	o.vLightmap = float4((LightAmbient/2) * NdL,1);
	
	if (emissive.r > 0.25f)
	{
		o.vColor.rgb = emissive*2;
		o.vLightmap = float4(0.5f,0.5f,0.5f,0.99f);
	}
	o.vColor.rgb = emissive*2;
	if (emissive.r < 0.025f) discard;
	return o;
};

PS_OUTPUT_BASIC psLightmap(VS_OUTPUT_LIGHTMAP i)
{
	PS_OUTPUT_BASIC o;

	// diffuse
	float4 diffuse = tex2D(DiffuseSampler, i.vTex0);
	if (diffuse.a < 0.75f) discard;
	diffuse.a = 1.0f;
	o.vColor = diffuse * DiffuseColor;
		
	// position
	o.vDepth = i.vDepth.x / i.vDepth.y;

	// emissive
	float4 emissive = tex2D(EmissiveSampler, i.vTex0);

	// lightmap
	float4 lmap = tex2D(LightmapSampler, i.vTex1);
	o.vLightmap.rgb = (lmap.rgb/2.0f)/AmbientReductionFactor;
	o.vLightmap.a = 1.0f;

	if (emissive.r > 0.25f)
	{
		o.vColor.rgb = emissive*2;
		o.vLightmap = float4(0.5f,0.5f,0.5f,0.99f);
	}

	// normal
	o.vNrm.xyz = i.vNrm.xyz * 0.5f + 0.5f;
	o.vNrm.w = 1.0;

	return o;
};

PS_OUTPUT_BASIC psMix(VS_OUTPUT_MIX i)
{
	PS_OUTPUT_BASIC o;

	// diffuse
	float4 diffuse = tex2D(DiffuseSampler, i.vTex0);
	if (diffuse.a < 0.75f) discard;
	diffuse.a = 1.0f;

	float4 diffuse2 = tex2D(Diffuse2Sampler, i.vTex0);
	float mask = length(tex2D(MaskSampler, i.vTex1).rgb);
	o.vColor.rgb = diffuse.rgb * (1-mask) + diffuse2.rgb * mask;
	o.vColor.a = 1.0f;
	o.vColor *= DiffuseColor;
	// position
	o.vDepth = i.vDepth.x / i.vDepth.y;

	// emissive
	float emissive = tex2D(EmissiveSampler, i.vTex0).r;

	// normal
	o.vNrm.xyz = i.vNrm.xyz * 0.5f + 0.5f;
	o.vNrm.w = 1.0;
	
	o.vLightmap = float4(0.25f,0.25f,0.25f,1.0f);
	if (emissive.r > 0.25f)
	{
		o.vColor.rgb = emissive*2;
		o.vLightmap = float4(0.5f,0.5f,0.5f,0.99f);
	}

	return o;
};

PS_OUTPUT_BASIC psMixLightmap(VS_OUTPUT_MIX_LIGHTMAP i)
{
	PS_OUTPUT_BASIC o;
	
	// diffuse
	float4 diffuse = tex2D(DiffuseSampler, i.vTex0);
	if (diffuse.a < 0.75f) discard;
	diffuse.a = 1.0f;

	float4 diffuse2 = tex2D(Diffuse2Sampler, i.vTex0);
	
	float mask = length(tex2D(MaskSampler, i.vTex1).rgb);

	o.vColor.rgb = diffuse.rgb * (1-mask) + diffuse2.rgb * mask;
	o.vColor.a = 1.0f;
	o.vColor *= DiffuseColor;

	// position
	o.vDepth = i.vDepth.x / i.vDepth.y;

	// emissive
	float emissive = tex2D(EmissiveSampler, i.vTex0).r;
	//o.vColor.a = emissive;

	// lightmap
	float4 lmap = tex2D(LightmapSampler, i.vTex2);
	o.vLightmap.rgb = (lmap.rgb/2.0f)/AmbientReductionFactor;
	o.vLightmap.a = 1.0f;

	if (emissive.r > 0.25f)
	{
		o.vColor.rgb = emissive*2;
		o.vLightmap = float4(0.5f,0.5f,0.5f,0.99f);
	}

	o.vNrm.xyz = i.vNrm.xyz * 0.5f + 0.5f;
	o.vNrm.w = 1.0;

	return o;
};

PS_OUTPUT_BASIC psWater(VS_OUTPUT_BASIC i)
{
	PS_OUTPUT_BASIC o;

	// diffuse
	i.vTex0.x = i.vTex0.x + WorldTime*0.06f;
	i.vTex0.y = i.vTex0.y - WorldTime*0.02f;
	float4 diffuse = tex2D(DiffuseSampler, i.vTex0);
	if (diffuse.a < 0.75f) discard;

	o.vColor = diffuse * DiffuseColor;
	o.vColor.a = 1.0f;

	// emissive
	float emissive = tex2D(EmissiveSampler, i.vTex0).r;

	// normal
	i.vTex0.y += (sin(WorldTime*3+10)/256)+(WorldTime/16);
	float3 biNormal = cross( i.vNrm, i.vTan.xyz ) * i.vTan.w;
	float3 bumpMap = tex2D(NormalSampler, i.vTex0).xyz;
	if ((bumpMap.x + bumpMap.y + bumpMap.z) <= 0) bumpMap = float3(0.5f,0.5f,1.0f);
	bumpMap = (bumpMap * 2.0f) - 1.0f;
	float3x3 TBN = float3x3(i.vTan.xyz, biNormal, i.vNrm );
		TBN = transpose( TBN );
	float3 bump_world_coords = mul( bumpMap, TBN );
	o.vNrm.xyz = bump_world_coords * 0.5f + 0.5f;
	o.vNrm.w = 1.0;
	
	// position
	o.vDepth = i.vDepth.x / i.vDepth.y;

	o.vLightmap = float4(0.5f,0.5f,0.5f,1);
	if (emissive.r > 0.25f)
	{
		o.vColor.rgb = emissive*2;
		o.vLightmap = float4(0.5f,0.5f,0.5f,0.99f);
	}

	return o;
};

PS_OUTPUT_BASIC psWaterStill(VS_OUTPUT_BASIC i)
{
	PS_OUTPUT_BASIC o;

	// diffuse
	i.vTex0.x += sin(WorldTime)*0.005f;
	i.vTex0.y += sin(WorldTime)*0.005f;

	float4 diffuse = tex2D(DiffuseSampler, i.vTex0);
	if (diffuse.a < 1) discard;
	diffuse.a = 1.0f;

	o.vColor = diffuse * DiffuseColor;

	// emissive
	float emissive = tex2D(EmissiveSampler, i.vTex0).r;
	o.vColor.a = emissive;

	// normal
	float3 biNormal = cross( i.vNrm, i.vTan.xyz ) * i.vTan.w;
	float3 bumpMap = tex2D(NormalSampler, i.vTex0).xyz;
	if ((bumpMap.x + bumpMap.y + bumpMap.z) <= 0) bumpMap = float3(0.5f,0.5f,1.0f);
	bumpMap = (bumpMap * 2.0f) - 1.0f;
	float3x3 TBN = float3x3(i.vTan.xyz, biNormal, i.vNrm );
		TBN = transpose( TBN );
	float3 bump_world_coords = mul( bumpMap, TBN );
	o.vNrm.xyz = bump_world_coords* 0.5f + 0.5f;
	o.vNrm.w = 1.0;
	
	// position
	o.vDepth = i.vDepth.x / i.vDepth.y;

	// Lightmap
	o.vLightmap = float4(0.5f,0.5f,0.5f,1);
	if (emissive.r > 0.25f)
	{
		o.vColor.rgb = emissive*2;
		o.vLightmap = float4(0.5f,0.5f,0.5f,0.99f);
	}
	return o;
};

float4 StereoToMonoClipSpace(float4 StereoClipPos)
{
	float4 MonoClipPos = StereoClipPos;
	float2 StereoParms = tex2D(StereoSampler, 0.0625).xy;
	MonoClipPos.x += StereoParms.x * (MonoClipPos.w - StereoParms.y); 
	return MonoClipPos;
}

float4 ScreenToClip(float2 ScreenPos, float EyeDepth) 
{
	float4 ClipPos = float4(ScreenPos.xy * VportXformInv.xy + VportXformInv.zw, 0,EyeDepth);
	// Move the coordinates to the appropriate distance
	// for the depth specified. 
	ClipPos.xy *= EyeDepth;
	// Screen and clip space are inverted in the Y direction
	// from each other.
	ClipPos.y = -ClipPos.y;
	return ClipPos;
}

float4 ScreenToWorld(float2 ScreenPos, float EyeDepth) {
	float4 StereoClipPos = ScreenToClip(ScreenPos, EyeDepth);
	float4 MonoClipPos = StereoToMonoClipSpace(StereoClipPos);
	return float4(mul(InvertWVP,MonoClipPos).xyz, 1.0f);
}

PS_INPUT_POINTLIGHT vsPointLight(VS_SIMPLE input)
{
    PS_INPUT_POINTLIGHT output;
    //processing geometry coordinates
    float4 worldPosition = mul(input.vPos0, World);
    output.vPos0 = mul(worldPosition, ViewProjection);
    output.vScr0 = output.vPos0;
    return output;
}

float4 psPointLight(PS_INPUT_POINTLIGHT i) : COLOR0
{
	
	//obtain screen position
	i.vScr0 = StereoToMonoClipSpace(i.vScr0);
	i.vScr0.xy /= i.vScr0.w;
	//
	//obtain textureCoordinates corresponding to the current pixel
	//the screen coordinates are in [-1,1]*[1,-1]
    //the texture coordinates need to be in [0,1]*[0,1]
	float2 texCoord = 0.5f * (float2(i.vScr0.x,-i.vScr0.y) + 1.0f);
	texCoord.x += ScreenWidth/2.0f;
	texCoord.y += ScreenHeight/2.0f;
	
	// Read Buffers
	float4 texColor = tex2D(ColorRTSampler, texCoord);
	float4 texNormal = tex2D(NormalRTSampler, texCoord);
	float4 texLight = tex2D (LightRTSampler, texCoord);
	float texDepth = tex2D(DepthRTSampler,texCoord).r;
	
	// normals are stored in texture space [0,1] -> convert them back to [-1,+1] range
	float3 normal = 2.0f * texNormal.xyz - 1.0f;

	//compute screen-space position
	float4 position = float4(1,1,1,1);
	position.xy = i.vScr0.xy;
    position.z = texDepth;
    position.w = 1.0f;
	//position = ScreenToWorld(position, 4.5f);

	//transform to world space
	position = mul(position, InvertViewProjection);
	position /= position.w;

    //surface-to-light vector
    float3 lightVector = LightPosition - position;
	
    float attenuation = saturate(1.0f - length(lightVector)/LightRadius);
	//float dist_intensity = round(attenuation *3.0f) / 3.0f;
	//dist_intensity += 0.4f;
	//if (dist_intensity < 0.25f) dist_intensity = 0.25f;
	//else if (dist_intensity < 0.5f) dist_intensity = 0.5f;
	//else dist_intensity = 1.0f;
    lightVector = normalize(lightVector); 
	normal = normalize(normal);
	float NdL = max(0,dot(normal,lightVector));

	float3 diffuseColor = float3(0.65f,0.475f,0.3f) * 3.0f * NdL;
	attenuation *= LightIntensity;
	if (attenuation > 0 && attenuation < 0.25f) attenuation = 0.25f;
	float4 c = float4(diffuseColor.rgb * LightIntensity,attenuation);
	c = texLight + c * attenuation;
	//c = float4(1,0,0,attenuation);
	//clamp(c,0.0f,1.0f);
    return c;
};

PS_INPUT_DECAL vsDecal(VS_SIMPLE input)
{
	PS_INPUT_DECAL output;
    float4 worldPosition = mul(input.vPos0, World);
    output.vPos0 = mul(worldPosition, ViewProjection);
	output.vScr0 = output.vPos0;
	output.vTex0 = float2(input.vPos0.x, input.vPos0.z);
	return output;
}

float4 psDecal(PS_INPUT_DECAL i) : COLOR0
{
	i.vScr0.xy /= i.vScr0.w;

	float2 texCoord = 0.5f * (float2(i.vScr0.x,-i.vScr0.y) + 1);

	texCoord.x += ScreenWidth/2.0f;
	texCoord.y += ScreenHeight/2.0f;
	// Read Buffers
	float4 texColor = tex2D(ColorRTSampler, texCoord);
	float4 texNormal = tex2D(NormalRTSampler, texCoord);
	float texDepth = tex2D(DepthRTSampler,texCoord).r;
	float4 texLightmap = tex2D(LightRTSampler, texCoord);
	
	// normals are stored in texture space [0,1] -> convert them back to [-1,+1] ranget
	float3 normal = 2.0f * texNormal.xyz - 1.0f;

	//compute screen-space position
	float4 position;
	position.xy = i.vScr0.xy;
    position.z = texDepth;
    position.w = 1.0f;

	//transform to world space
	position = mul(position, InvertViewProjection);
	position /= position.w;
	
	// Shadow effect
	position.z += sin((position.x*12*cos(WorldTime/124)) + sin(position.y*14*sin(WorldTime/152)) + sin(position.z*16*sin(WorldTime/124)) )/20;
	position.x += sin((position.z*12*cos(WorldTime/150)) + sin(position.y*16*sin(WorldTime/171)) + sin(position.x*14*sin(WorldTime/183)) )/20;

    //surface-to-light vector
    float3 lightVector = LightPosition - position.xyz;
	//float4 texShadow = tex2D(DiffuseSampler, float2(position.x + sin(WorldTime/2.75f)/4, position.z + sin(WorldTime/2.5f)/4)/4);
	//texShadow += tex2D(DiffuseSampler, float2(position.x - sin(WorldTime/2.5f)/4, position.z*1.25f- sin(WorldTime/2.75f)/4)/4);
	//texShadow += tex2D(DiffuseSampler, float2(viposition.z*1.4f - sin(WorldTime/2)/4, position.x*1.5f  + sin(WorldTime/2)/4)/4);
	//texShadow += tex2D(DiffuseSampler, float2(position.x*1.7f - sin(WorldTime/2)/4, position.z*1.5f  + sin(WorldTime/2)/4)/4);
	//texShadow += tex2D(DiffuseSampler, float2(position.z*1.2f - sin(WorldTime/2)/4, position.x*1.9f  + sin(WorldTime/2)/4)/4);

    //compute attenuation based on distance - linear attenuation
    float attenuation = saturate(1.0f - length(lightVector)/LightRadius); 
	if (attenuation <= 0) attenuation = 0;
    //normalize light vector
    lightVector = normalize(lightVector); 

	float4 retColor = texLightmap;
	if (texLightmap.a < 1.0f) discard;

	if (attenuation > 0)
	{
		//if (texShadow.a > 0.5f) retColor.rgb = float3(0.013f,0.022f,0.03f);
		//else retColor.rgb = float3(0.013f,0.022f,0.03f);
		//else discard;
		retColor.rgb = float3(0.0125f,0.025f,0.0325f);
		//retColor.a *= 0.8f + attenuation;
	}


	//if (attenuation > 0) retColor = attenuation *  float4(0.03f,0.03f,0.03f,1);
	return retColor;
    //return attenuation * LightIntensity * float4(0,0,0,1);
}

technique tech_decal
{
    pass P0
	{
		VertexShader = compile vs_3_0 vsDecal();
		PixelShader = compile ps_3_0 psDecal();
		CullMode = ccw;
		ZWRITEENABLE = TRUE;
		ZENABLE = FALSE;
		blendop = ADD;
		srcblend = srcalpha;
		destblend = invdestalpha;
	}
};

float4 psBlob(PS_INPUT_DECAL i) : COLOR0
{
	i.vScr0.xy /= i.vScr0.w;

	float2 texCoord = 0.5f * (float2(i.vScr0.x,-i.vScr0.y) + 1);

	texCoord.x += ScreenWidth/2.0f;
	texCoord.y += ScreenHeight/2.0f;
	// Read Buffers
	float4 texColor = tex2D(ColorRTSampler, texCoord);
	float4 texNormal = tex2D(NormalRTSampler, texCoord);
	float texDepth = tex2D(DepthRTSampler,texCoord).r;
	float4 texLightmap = tex2D(LightRTSampler, texCoord);
	
	// normals are stored in texture space [0,1] -> convert them back to [-1,+1] ranget
	float3 normal = 2.0f * texNormal.xyz - 1.0f;

	//compute screen-space position
	float4 position;
	position.xy = i.vScr0.xy;
    position.z = texDepth;
    position.w = 1.0f;

	//transform to world space
	position = mul(position, InvertViewProjection);
	position /= position.w;
	
	// Shadow effect
	position.z += sin((position.x*5*cos(WorldTime/174)) + sin(position.y*14*sin(WorldTime/152)) + sin(position.z*16*sin(WorldTime/154)) )/30;
	position.x += sin((position.z*7*cos(WorldTime/150)) + sin(position.y*16*sin(WorldTime/171)) + sin(position.x*14*sin(WorldTime/183)) )/30;

	//position.z += sin((position.x*12*cos(WorldTime/124)) + sin(position.y*14*sin(WorldTime/152)) + sin(position.z*16*sin(WorldTime/124)) )/20;
	//position.x += sin((position.z*12*cos(WorldTime/150)) + sin(position.y*16*sin(WorldTime/171)) + sin(position.x*14*sin(WorldTime/183)) )/20;

    //surface-to-light vector
    float3 lightVector = LightPosition - position.xyz;
	//float4 texShadow = tex2D(DiffuseSampler, float2(position.x + sin(WorldTime/2.75f)/4, position.z + sin(WorldTime/2.5f)/4)/4);
	//texShadow += tex2D(DiffuseSampler, float2(position.x - sin(WorldTime/2.5f)/4, position.z*1.25f- sin(WorldTime/2.75f)/4)/4);
	//texShadow += tex2D(DiffuseSampler, float2(position.z*1.4f - sin(WorldTime/2)/4, position.x*1.5f  + sin(WorldTime/2)/4)/4);
	//texShadow += tex2D(DiffuseSampler, float2(position.x*1.7f - sin(WorldTime/2)/4, position.z*1.5f  + sin(WorldTime/2)/4)/4);
	//texShadow += tex2D(DiffuseSampler, float2(position.z*1.2f - sin(WorldTime/2)/4, position.x*1.9f  + sin(WorldTime/2)/4)/4);

    //compute attenuation based on distance - linear attenuation
    float attenuation = saturate(1.0f - length(lightVector)/LightRadius); 
	//if (attenuation <= 0) attenuation = 0;
    //normalize light vector
	lightVector = normalize(lightVector); 
	normal = normalize(normal);
	float4 retColor = texLightmap;
	if (texLightmap.a < 1.0f) discard;

	if (attenuation > 0)
	{
		//if (texShadow.a > 0.5f) retColor.rgb = float3(0.013f,0.022f,0.03f);
		//else retColor.rgb = float3(0.013f,0.022f,0.03f);
		//else discard;
		retColor.rgb = LightColor.rgb;
		//retColor.
		//retColor.a *= 0.8f + attenuation;
	}


	//if (attenuation > 0) retColor = attenuation *  float4(0.03f,0.03f,0.03f,1);
	return retColor;
    //return attenuation * LightIntensity * float4(0,0,0,1);
}


technique tech_blob
{
    pass P0
	{
		VertexShader = compile vs_3_0 vsDecal();
		PixelShader = compile ps_3_0 psBlob();
		CullMode = ccw;
		ZWRITEENABLE = TRUE;
		ZENABLE = FALSE;
		blendop = ADD;
		srcblend = srcalpha;
		destblend = invdestalpha;
	}
};

float4 psSkyBox(VS_OUTPUT_SKYBOX input) : COLOR0
{
	float4 c = texCUBE(SkyBoxSampler, normalize(input.Tex));
	c.a = 1.0f;
	return c;
}

PS_OUTPUT_BASIC psSpecialVision(VS_OUTPUT_BASIC i)
{
	PS_OUTPUT_BASIC o;

	// diffuse
	float4 diffuse = tex2D(DiffuseSampler, i.vTex0);

	o.vColor = diffuse * float4(1.5f,0.5f,0.5f,1);

	// position
	o.vDepth = i.vDepth.x / i.vDepth.y;

	// emissive
	float emissive = tex2D(EmissiveSampler, i.vTex0).r;
	o.vColor.a = emissive;

	// normal
	o.vNrm.xyz = i.vNrm.xyz * 0.5f + 0.5f;
	o.vNrm.w = 1.0;

	// Lightmap
	o.vLightmap = float4(0.5f,0.5f,0.5f,1);
	if (emissive.r > 0.25f)
	{
		o.vColor.rgb = emissive*2;
		o.vLightmap = float4(0.5f,0.5f,0.5f,0.99f);
	}
	return o;
};


// Composed GBuffer + Lighting
float4 psComposed(PS_SIMPLE input) : COLOR0
{
	float4 color = tex2D(ColorRTSampler,input.vTex0);
	float4 light = tex2D(LightRTSampler,input.vTex0);
	float emissive = tex2D(NormalRTSampler,input.vTex0).a;

	if (color.a < 1) discard;

	//float3 diffuseLight = light.rgb + LightAmbient;

	float4 c = color;

	//if (light.a == 0) c.rgb += light.rgb * 2;
	if (light.a > 0) c.rgb *= light.rgb * 2;
	c.a = 1.0f;
	//c.a = 1.0f;
	//c.rgb += light.a;

	//else c.rgb += light.rgb;
	//c.a = 1.0f;
	//if (color.r != 0 && color.g != 0 && color.b != 0) c.a = 1.0f;
	//c.a = 1.0f;

	return c;
}

// Final Render
float4 psFinal(PS_SIMPLE input) : COLOR0
{
	float4 color = tex2D(FinalRTSampler,input.vTex0);
	//float4 ssao = tex2D(SSAORTSampler,input.vTex0);
	//ssao.rgb = pow(ssao.rgb,5);
	return color;
}

float4 psFog(PS_SIMPLE i) : COLOR0
{	
	float depthValue = tex2D(DepthRTSampler, i.vTex0).r;
	clip(-depthValue + 0.9999f);

	//compute screen-space position
	float4 position;
	position.x = i.vTex0.x * 2.0f - 1.0f;
	position.y = -(i.vTex0.y * 2.0f - 1.0f);
	position.z = depthValue;
	position.w = 1.0f;

	//transform to world space
	position = mul(position, InvertViewProjection);
	position /= position.w;
	float d = length(CameraPosition - position);

	float4 color = tex2D(FinalRTSampler,i.vTex0);
	float fogStart = FogStart;
	float fogEnd = FogEnd*3 + sin(WorldTime/4)*5 + sin(WorldTime/2+position.x)*2 + sin(WorldTime/1.5f+position.z)*2 + sin(WorldTime/1.6f+position.y)*2;
	float l2 = saturate((d-fogStart) / (fogEnd));
	float3 fEndColor = float3(0.20f,0.31f,0.375f);
	float3 fStartColor = float3(0.05f, 0.14f, 0.18f);
	float3 fColor = lerp(fStartColor, fEndColor, l2);
	//float3 fColor = float3(0.05f,0.14f,0.18f); // temp
	//fColor.rgb = lerp (fColor.rgb, float3(0.45f,0.5f,0.7f), l2);
	color.rgb = lerp(color.rgb, float3(0.05f, 0.14f, 0.18f), l2*1.1f);
	//fColor = float3(0.05f,0.14f,0.18f); // temp
	//color.rgb = lerp(color.rgb, fColor, l2);
	return color;

} 

float4 psBlurHor(PS_SIMPLE i) : COLOR0
{
	float alpha = tex2D(FinalRTSampler, i.vTex0).a;
	float4 sum = float4(0.0f,0.0f,0.0f,alpha);
 
   // blur in y (vertical)
   // take nine samples, with the distance blurSize between them
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x - 4.0*RTWidth*BlurSize, i.vTex0.y)) * 0.05 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x - 3.0*RTWidth*BlurSize, i.vTex0.y)) * 0.09 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x - 2.0*RTWidth*BlurSize, i.vTex0.y)) * 0.12 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x - RTWidth*BlurSize, i.vTex0.y)) * 0.15 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x, i.vTex0.y)) * 0.16 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x + RTWidth*BlurSize, i.vTex0.y)) * 0.15 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x + 2.0*RTWidth*BlurSize, i.vTex0.y)) * 0.12 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x + 3.0*RTWidth*BlurSize, i.vTex0.y)) * 0.09 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x + 4.0*RTWidth*BlurSize, i.vTex0.y)) * 0.05 * 1.0f/0.98f;
 
   return sum;
}
float4 psBlurVer(PS_SIMPLE i) : COLOR0
{	
	float alpha = tex2D(FinalRTSampler, i.vTex0).a;
	float4 sum = float4(0.0f,0.0f,0.0f,alpha);
 
   // blur in y (vertical)
   // take nine samples, with the distance blurSize between them
  sum += tex2D(FinalRTSampler, float2(i.vTex0.x, i.vTex0.y - 4.0*RTHeight*BlurSize)) * 0.05 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x, i.vTex0.y - 3.0*RTHeight*BlurSize)) * 0.09 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x, i.vTex0.y - 2.0*RTHeight*BlurSize)) * 0.12 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x, i.vTex0.y - RTHeight*BlurSize)) * 0.15 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x, i.vTex0.y)) * 0.16 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x, i.vTex0.y + RTHeight*BlurSize)) * 0.15 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x, i.vTex0.y + 2.0*RTHeight*BlurSize)) * 0.12 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x, i.vTex0.y + 3.0*RTHeight*BlurSize)) * 0.09 * 1.0f/0.98f;
   sum += tex2D(FinalRTSampler, float2(i.vTex0.x, i.vTex0.y + 4.0*RTHeight*BlurSize)) * 0.05 * 1.0f/0.98f;
 
   return sum;
}

float4 psOutline(PS_SIMPLE i) : COLOR0
{
	float2 d = float2(ScreenWidth,ScreenHeight);
	i.vTex0.x += d.x;
	i.vTex0.y += d.y;

	// Center
	float2 tcenter = i.vTex0;
	float depthValue = tex2D(DepthRTSampler, tcenter).r;
	clip(-0 + 0.9999f);

	float4 positionCenter;
	positionCenter.x = tcenter.x * 2.0f - 1.0f;
	positionCenter.y = -(tcenter.y * 2.0f - 1.0f);
	positionCenter.z = depthValue;
	positionCenter.w = 1.0f;

	//transform to world space
	positionCenter = mul(positionCenter, InvertViewProjection);
	positionCenter /= positionCenter.w;
	float distCenter = length(CameraPosition - positionCenter);

	// Up
	float2 tup = i.vTex0+d;
	depthValue = tex2D(DepthRTSampler, tup).r;
	clip(-0 + 0.9999f);

	float4 positionUp;
	positionUp.x = tup.x * 2.0f - 1.0f;
	positionUp.y = -(tup.y * 2.0f - 1.0f);
	positionUp.z = depthValue;
	positionUp.w = 1.0f;

	//transform to world space
	positionUp = mul(positionUp, InvertViewProjection);
	positionUp /= positionUp.w;
	float distUp = length(CameraPosition - positionUp);

	// Down
	float2 tdown = i.vTex0-d;
	depthValue = tex2D(DepthRTSampler, tdown).r;
	clip(-0 + 0.9999f);

	float4 positionDown;
	positionDown.x = tdown.x * 2.0f - 1.0f;
	positionDown.y = -(tdown.y * 2.0f - 1.0f);
	positionDown.z = depthValue;
	positionDown.w = 1.0f;

	//transform to world space
	positionDown = mul(positionDown, InvertViewProjection);
	positionDown /= positionDown.w;
	float distDown = length(CameraPosition - positionDown);

	// Left
	float2 tleft = i.vTex0+ float2( d.x, -d.y );
	depthValue = tex2D(DepthRTSampler, tleft).r;
	clip(-0 + 0.9999f);

	float4 positionLeft;
	positionLeft.x = tleft.x * 2.0f - 1.0f;
	positionLeft.y = -(tleft.y * 2.0f - 1.0f);
	positionLeft.z = depthValue;
	positionLeft.w = 1.0f;

	//transform to world space
	positionLeft = mul(positionLeft, InvertViewProjection);
	positionLeft /= positionLeft.w;
	float distLeft = length(CameraPosition - positionLeft);

	// Right
	float2 tright = i.vTex0- float2( d.x, -d.y );
	depthValue = tex2D(DepthRTSampler, tright).r;
	clip(-0 + 0.9999f);

	float4 positionRight;
	positionRight.x = tright.x * 2.0f - 1.0f;
	positionRight.y = -(tright.y * 2.0f - 1.0f);
	positionRight.z = depthValue;
	positionRight.w = 1.0f;

	//transform to world space
	positionRight = mul(positionRight, InvertViewProjection);
	positionRight /= positionRight.w;
	float distRight = length(CameraPosition - positionRight);

	float4 color = tex2D(FinalRTSampler, i.vTex0);

	float delta_z = (abs( distDown - distUp ) + abs( distRight - distLeft ) );

	bool draw_outline = false;

	if (delta_z > (0.1f + distCenter/10) ) draw_outline = true;

	if (draw_outline == false) discard;

	//float4 outline_color = float4(LineColor.rgb, 1.0f);

	return float4(LineColor.rgb, 1.0f);
}

// Vignetting
float4 psVignetting(PS_SIMPLE input) : COLOR0
{
	float4 color = tex2D(FinalRTSampler,input.vTex0);
	//float2 dist = input.vTex0 - 0.5f;
	float dist = distance(input.vTex0, float2(0.5f,0.5f));

	color.rgb *= smoothstep(1.0f, 0.0f, pow(dist,VigForce));
	return color;
}

// Bloom
float4 psBloom(PS_SIMPLE input) : COLOR0
{
	float4 color = tex2D(FinalRTSampler,input.vTex0);
	color.rgb = saturate((color.rgb -Threshold) / (1 - Threshold));
	return color;
}

float4 psBloomComposed(PS_SIMPLE input) : COLOR0
{
	// Get our bloom pixel from bloom texture
	float4 bloomColor = tex2D(BloomRTSampler, input.vTex0);
 
	// Get our original pixel from ColorMap
	float4 originalColor = tex2D(FinalRTSampler, input.vTex0);
	float4 originalColor2 = tex2D(FinalRTSampler, input.vTex0);
	// Adjust color saturation and intensity based on the input variables to the shader
	bloomColor.rgb = AdjustSaturation(bloomColor.rgb, BloomSaturation) * BloomIntensity;
	originalColor.rgb = AdjustSaturation(originalColor.rgb, OriginalSaturation) * OriginalIntensity;
   
	// make the originalColor darker in very bright areas, avoiding these areas look burned-out
	originalColor.rgb *= (1 - saturate(bloomColor.rgb));
	// Combine the two images.
	return float4(originalColor.rgb + bloomColor.rgb,originalColor.a);
}

// Image Effects
float4 psImgEffects(PS_SIMPLE input) : COLOR0
{
	float4 color = tex2D(FinalRTSampler,input.vTex0);

	float3 LuminanceWeights = float3(0.299,0.587,0.114);
	float luminance = dot(color,LuminanceWeights);

	float3 sat = float3(1.0f,1.0f,1.2f);
	float3 saturate = lerp(luminance,color.rgb,sat);

	color.rgb = saturate;

	////float4 grayscale = dot(color, float3(0.3, 0.59, 0.11));

	//// Apply contrast
	//color.rgb = ((color.rgb - 0.5f) * max(Contrast, 0)) + 0.5f;

	//// Apply brightness
	//color.rgb += Brightness;

	return color;
}

float4 psWiggle(PS_SIMPLE input) : COLOR0
{
	float2 tex = input.vTex0;

	float d = distance(0.5f, tex.x);
	float d2 = distance(0.5f, tex.y);
	float diffTime = WorldTime-WiggleTime;

	tex.x += sin(sin(diffTime)*d *10)*0.01f;
	tex.y += sin(sin(diffTime)*d2 *15)*0.01f;

	float4 color = tex2D(FinalRTSampler,tex);
	color.rgb *= 0.45f;
	//float4 special = dot(color, float3(0.0f, 0.69, 0.61));
	////float3 LuminanceWeights = float3(0.099,0.587,0.514);
	////float luminance = dot(color,LuminanceWeights);

	////float3 sat = float3(1.75f,0.0f,0.40f);
	////float3 saturate = lerp(luminance,color.rgb,sat);

	////color.rgb = saturate;
	////color.g = grayscale.g;
	////color.b = grayscale.b;
	//special.r = color.r;
	//special.g *= color.g;
	//special.b *= color.b;

	return color;
}

float4 psSimpleBlur(PS_SIMPLE input) : COLOR0
{
	float2 Tex = input.vTex0;

	float4 color;

	color  = tex2D( FinalRTSampler, float2(Tex.x+RTWidth*SimpleBlurSize*4, Tex.y+RTHeight*SimpleBlurSize*4));
    color += tex2D( FinalRTSampler, float2(Tex.x-RTWidth*SimpleBlurSize*4, Tex.y-RTHeight*SimpleBlurSize*4));
    color += tex2D( FinalRTSampler, float2(Tex.x+RTWidth*SimpleBlurSize*4, Tex.y-RTHeight*SimpleBlurSize*4));
    color += tex2D( FinalRTSampler, float2(Tex.x-RTWidth*SimpleBlurSize*4, Tex.y+RTHeight*SimpleBlurSize*4));
    // We need to devide the color with the amount of times we added
    // a color to it, in this case 4, to get the avg. color
    color = color / 4.0f; 

	return color;
}

float4 psDoF(PS_SIMPLE input) : COLOR0
{
	float2 Tex = input.vTex0;
	float4 color = tex2D(FinalRTSampler, Tex);
	float4 blur = tex2D(BlurRTSampler, Tex);
	float depth = tex2D(DepthRTSampler, Tex).r;
	depth = depth;

	float Far = CameraFar / (CameraFar-CameraNear);

	// Calculate the distance from the selected distance and range on our DoF effect, set from the application
	float fSceneZ = ( -CameraNear * Far ) / ( depth -Far);
	float blurFactor = saturate(abs(fSceneZ-DistanceToTarget)/Range);
 
	// Based on how far the texel is from "distance" in Distance, stored in blurFactor, mix the scene
	return float4(lerp(color.rgb,blur.rgb,blurFactor),color.a);
}

technique tech_basic
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsBasic();
		PixelShader = compile ps_3_0 psBasic();	
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;
		alphablendenable = false;
	}
};

technique tech_lightmap
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsLightmap();
		PixelShader = compile ps_3_0 psLightmap();	
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;
		alphablendenable = false;
	}
};

technique tech_mix
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsMix();
		PixelShader = compile ps_3_0 psMix();	
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;
		alphablendenable = false;
	}
};

technique tech_mix_lightmap
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsMixLightmap();
		PixelShader = compile ps_3_0 psMixLightmap();	
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;
		alphablendenable = false;
	}
};

technique tech_water
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsWater();
		PixelShader = compile ps_3_0 psWater();
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;
		alphablendenable = false;
	}
};

technique tech_water_still
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsBasic();
		PixelShader = compile ps_3_0 psWaterStill();
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;
		alphablendenable = false;
	}
};

technique tech_skin
{
    pass P0
    {          
        VertexShader = compile vs_3_0 vsSkin( );
        PixelShader  = compile ps_3_0 psNoLight( );
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;
		alphablendenable = false;
    }
};

technique tech_crow
{
    pass P0
    {          
        VertexShader = compile vs_3_0 vsSkin( );
        PixelShader  = compile ps_3_0 psCrow( );
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;
		alphablendenable = false;
    }
};


technique tech_no_light
{
    pass P0
    {          
        VertexShader = compile vs_3_0 vsBasic( );
        PixelShader  = compile ps_3_0 psNoLight( );
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;	
		alphablendenable = false;
    }
};

technique tech_skin_dirLight
{
    pass P0
    {          
        VertexShader = compile vs_3_0 vsSkin( );
        PixelShader  = compile ps_3_0 psSkinDirLight( );
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;	
		alphablendenable = false;
    }
};

technique tech_shadow
{
    pass P0
    {          
        VertexShader = compile vs_3_0 vsSkin( );
        PixelShader  = compile ps_3_0 psShadow( );
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;	
		alphablendenable = false;
    }
};

technique tech_shadow_scarf
{
    pass P0
    {          
        VertexShader = compile vs_3_0 vsSkin( );
        PixelShader  = compile ps_3_0 psShadowScarf( );
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;	
		alphablendenable = false;
    }
};

technique tech_pointlights
{
    pass P0
	{
		VertexShader = compile vs_3_0 vsPointLight();
		PixelShader = compile ps_3_0 psPointLight();	
		CullMode = ccw;
		ZWRITEENABLE = TRUE;
		ZENABLE = FALSE;
		//destblend = ONE;	
		blendop = ADD;
	}
};



technique tech_composed
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsScreen();
		PixelShader = compile ps_3_0 psComposed();
		CullMode = ccw;
		ZWRITEENABLE = TRUE;
		ZENABLE = FALSE;
		alphablendenable = false;
	}
}

technique tech_simple_blur
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsScreen();
		PixelShader = compile ps_3_0 psSimpleBlur();
		CullMode = ccw;
		ZWRITEENABLE = TRUE;
		ZENABLE = FALSE;
		alphablendenable = false;
	}

}

technique tech_blur_hor
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsScreen();
		PixelShader = compile ps_3_0 psBlurHor();
		CullMode = ccw;
		ZWRITEENABLE = TRUE;
		ZENABLE = FALSE;
		alphablendenable = false;
	}
}

technique tech_blur_ver
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsScreen();
		PixelShader = compile ps_3_0 psBlurVer();
		CullMode = ccw;
		ZWRITEENABLE = TRUE;
		ZENABLE = FALSE;
		alphablendenable = false;
	}
}

technique tech_dof
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsScreen();
		PixelShader = compile ps_3_0 psDoF();
		CullMode = ccw;
		ZWRITEENABLE = TRUE;
		ZENABLE = FALSE;
		alphablendenable = false;
	}
}

technique tech_outline
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsScreen();
		PixelShader = compile ps_3_0 psOutline();
		CullMode = ccw;
		Zenable = false;
		ZWriteEnable = false;
		ZFunc = less;
		//StencilEnable = false;
		//blendop = Add;
		srcblend = srcalpha;
		destblend = invdestalpha;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;	
		//ZWRITEENABLE = TRUE;
		//ZENABLE = FALSE;
	}
};

technique tech_vignetting
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsScreen();
		PixelShader = compile ps_3_0 psVignetting();
		CullMode = ccw;
		ZWRITEENABLE = TRUE;
		ZENABLE = FALSE;
		alphablendenable = false;

	}
}

technique tech_bloom
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsScreen();
		PixelShader = compile ps_3_0 psBloom();
		CullMode = ccw;
		ZWRITEENABLE = TRUE;
		ZENABLE = FALSE;
		alphablendenable = false;
		blendop = ADD;
		alphablendenable = false;

	}
}
technique tech_bloom_composed
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsScreen();
		PixelShader = compile ps_3_0 psBloomComposed();
		CullMode = ccw;
		ZWRITEENABLE = TRUE;
		ZENABLE = FALSE;
		alphablendenable = false;
		blendop = ADD;
		alphablendenable = false;

	}
}


technique tech_image_effects
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsScreen();
		PixelShader = compile ps_3_0 psImgEffects();
		CullMode = ccw;
		ZWRITEENABLE = TRUE;
		ZENABLE = FALSE;
		alphablendenable = false;

	}

};

technique tech_skybox
{
    pass P0
	{
		VertexShader = compile vs_3_0 vsSkyBox();
        PixelShader = compile ps_3_0 psSkyBox();
		CullMode = ccw;
		//Alphablnd=ADD;
		blendop = ADD;
		AlphaBlendEnable = true;
		destblend = destALPHA;
		srcblend = invdestAlpha;
		ZEnable           = false;
		ZWriteEnable      = true;
	}
};

technique tech_fog
{
    pass P0
    {	
		VertexShader = compile vs_3_0 vsScreen();
        PixelShader = compile ps_3_0 psFog();
		CullMode = ccw;
		ZEnable           = false;
		blendop = ADD;
		destblend = invsrcALPHA;
		srcblend = srcAlpha;
		alphablendenable = false;

    }
};

technique tech_final
{
	pass P0
	{
		VertexShader = compile vs_3_0 vsScreen();
		PixelShader = compile ps_3_0 psFinal();
		//ZEnable           = true;
		AlphaBlendEnable	= FALSE;
		CullMode = ccw;
		ZWRITEENABLE = TRUE;
		Zenable = false;

	}

};



technique tech_wiggle
{
	pass P0
	{
		PixelShader  = compile ps_3_0 psWiggle( );
		CullMode = ccw;
	}
}

// FORWARD RENDERING 
void vs_fwd_water_patio( in  float4 iPos : POSITION0
			  , in float2 iTex0	: TEXCOORD0
			  , in  float4 iTan		: TEXCOORD1
			  , in float3 iNormal : NORMAL
			  , out float4 oPos : POSITION
			  , out float2 oTex0	: TEXCOORD0
			  , out float3 oNormal : TEXCOORD1
			  , out float3 oPos2 : TEXCOORD2
			  , out float4 oTan		: TEXCOORD3
			  )
{

	oPos = mul(iPos,World);
	oPos.y += sin(oPos.x  + WorldTime/2)/7;

	float4 normal = normalize(mul(iNormal,World));
	oPos2 = oPos.xyz;

	oPos = mul( oPos, ViewProjection );

	oNormal = normal;
	oTex0 = iTex0;
	oTan.xyz = mul(iTan.xyz, (float3x3) World);
	oTan.w = 1;
}

// FORWARD RENDERING 
void vs_fwd_water_cementerio( in  float4 iPos : POSITION0
			  , in float2 iTex0	: TEXCOORD0
			  , in  float4 iTan		: TEXCOORD1
			  , in float3 iNormal : NORMAL
			  , out float4 oPos : POSITION
			  , out float2 oTex0	: TEXCOORD0
			  , out float3 oNormal : TEXCOORD1
			  , out float3 oPos2 : TEXCOORD2
			  , out float4 oTan		: TEXCOORD3
			  )
{

	oPos = mul(iPos,World);
	oPos.y += sin(oPos.x  + WorldTime/2)/4;

	float4 normal = normalize(mul(iNormal,World));
	oPos2 = oPos.xyz;

	oPos = mul( oPos, ViewProjection );

	oNormal = normal;
	oTex0 = iTex0;
	oTan.xyz = mul(iTan.xyz, (float3x3) World);
	oTan.w = 1;
}

float4 ps_fwd_water_cementerio(in float2 iTex0 : TEXCOORD0, in float3 iNormal : TEXCOORD1, in float3 iPos : TEXCOORD2) : COLOR0
{
	// Diffuse
	iTex0.x = iTex0.x - WorldTime*0.04f;
	//iTex0.y = iTex0.y + WorldTime*0.06f;
	float4 color = tex2D(DiffuseSampler, iTex0);

	// emissive
	float4 emissive = tex2D(EmissiveSampler, iTex0);
	
	if (emissive.r > 0.25f)
	{
		color.rgb = emissive.rgb;
		color.a = 1.0f;
	}

	return color;
}

void vs_fwd_water_font( in  float4 iPos : POSITION0
			  , in float2 iTex0	: TEXCOORD0
			  , in  float4 iTan		: TEXCOORD1
			  , in float3 iNormal : NORMAL
			  , out float4 oPos : POSITION
			  , out float2 oTex0	: TEXCOORD0
			  , out float3 oNormal : TEXCOORD1
			  , out float3 oPos2 : TEXCOORD2
			  , out float4 oTan		: TEXCOORD3
			  )
{

	oPos = mul(iPos,World);
	//oPos.y += sin(oPos.x  + WorldTime/2)/30;

	float4 normal = normalize(mul(iNormal,World));
	oPos2 = oPos.xyz;

	oPos = mul( oPos, ViewProjection );

	oNormal = normal;
	oTex0 = iTex0;
	oTan.xyz = mul(iTan.xyz, (float3x3) World);
	oTan.w = 1;
}

float4 ps_fwd_water_font(in float2 iTex0 : TEXCOORD0, in float3 iNormal : TEXCOORD1, in float3 iPos : TEXCOORD2) : COLOR0
{
	// Diffuse
	//iTex0.x = iTex0.x - sin(WorldTime*0.1f)/10;
	//iTex0.y = iTex0.y + WorldTime*0.02f;
	float4 color = tex2D(DiffuseSampler, iTex0);

	// emissive
	float4 emissive = tex2D(EmissiveSampler, iTex0);
	
	if (emissive.r > 0.25f)
	{
		color.rgb = emissive.rgb;
		color.a = 1.0f;
	}

	return color;
}

void vs_fwd_water_font_vertical( in  float4 iPos : POSITION0
			  , in float2 iTex0	: TEXCOORD0
			  , in  float4 iTan		: TEXCOORD1
			  , in float3 iNormal : NORMAL
			  , out float4 oPos : POSITION
			  , out float2 oTex0	: TEXCOORD0
			  , out float3 oNormal : TEXCOORD1
			  , out float3 oPos2 : TEXCOORD2
			  , out float4 oTan		: TEXCOORD3
			  )
{

	oPos = mul(iPos,World);
	//oPos.y += sin(oPos.x  + WorldTime/2)/30;

	float4 normal = normalize(mul(iNormal,World));
	oPos2 = oPos.xyz;

	oPos = mul( oPos, ViewProjection );

	oNormal = normal;
	oTex0 = iTex0;
	oTan.xyz = mul(iTan.xyz, (float3x3) World);
	oTan.w = 1;
}

float4 ps_fwd_water_font_vertical(in float2 iTex0 : TEXCOORD0, in float3 iNormal : TEXCOORD1, in float3 iPos : TEXCOORD2) : COLOR0
{
	// Diffuse
	//iTex0.x = iTex0.x - sin(WorldTime*0.1f)/10;
	iTex0.y = iTex0.y + WorldTime*0.02f;
	float4 color = tex2D(DiffuseSampler, iTex0);

	// emissive
	float4 emissive = tex2D(EmissiveSampler, iTex0);
	
	if (emissive.r > 0.25f)
	{
		color.rgb = emissive.rgb;
		color.a = 1.0f;
	}

	return color;
}

void vs_fwd_water_aldea( in  float4 iPos : POSITION0
			  , in float2 iTex0	: TEXCOORD0
			  , in  float4 iTan		: TEXCOORD1
			  , in float3 iNormal : NORMAL
			  , out float4 oPos : POSITION
			  , out float2 oTex0	: TEXCOORD0
			  , out float3 oNormal : TEXCOORD1
			  , out float3 oPos2 : TEXCOORD2
			  , out float4 oTan		: TEXCOORD3
			  )
{

	oPos = mul(iPos,World);
	oPos.y += sin(oPos.x  + WorldTime/2)/4;

	float4 normal = normalize(mul(iNormal,World));
	oPos2 = oPos.xyz;

	oPos = mul( oPos, ViewProjection );

	oNormal = normal;
	oTex0 = iTex0;
	oTan.xyz = mul(iTan.xyz, (float3x3) World);
	oTan.w = 1;
}

float4 ps_fwd_water_aldea(in float2 iTex0 : TEXCOORD0, in float3 iNormal : TEXCOORD1, in float3 iPos : TEXCOORD2) : COLOR0
{
	// Diffuse
	iTex0.x = iTex0.x + WorldTime*0.04f;
	iTex0.y = iTex0.y - WorldTime*0.01f;
	float4 color = tex2D(DiffuseSampler, iTex0);

	// emissive
	float4 emissive = tex2D(EmissiveSampler, iTex0);
	
	if (emissive.r > 0.25f)
	{
		color.rgb = emissive.rgb;
		color.a = 1.0f;
	}

	return color;
}

void vs_fwd_water_still( in  float4 iPos : POSITION0
			  , in float2 iTex0	: TEXCOORD0
			  , in  float4 iTan		: TEXCOORD1
			  , in float3 iNormal : NORMAL
			  , out float4 oPos : POSITION
			  , out float2 oTex0	: TEXCOORD0
			  , out float3 oNormal : TEXCOORD1
			  , out float3 oPos2 : TEXCOORD2
			  , out float4 oTan		: TEXCOORD3
			  )
{

	oPos = mul(iPos,World);
	oPos.y += sin(oPos.x  + WorldTime/2)/10;

	float4 normal = normalize(mul(iNormal,World));
	oPos2 = oPos.xyz;

	oPos = mul( oPos, ViewProjection );

	oNormal = normal;
	oTex0 = iTex0;
	oTan.xyz = mul(iTan.xyz, (float3x3) World);
	oTan.w = 1;
}

float4 ps_fwd_water_still(in float2 iTex0 : TEXCOORD0, in float3 iNormal : TEXCOORD1, in float3 iPos : TEXCOORD2) : COLOR0
{
	// Diffuse
	iTex0.x = iTex0.x - sin(WorldTime*0.04f)/10;
	//iTex0.y = iTex0.y + WorldTime*0.06f;
	float4 color = tex2D(DiffuseSampler, iTex0);

	// emissive
	float4 emissive = tex2D(EmissiveSampler, iTex0);
	
	if (emissive.r > 0.25f)
	{
		color.rgb = emissive.rgb;
		color.a = 1.0f;
	}

	return color;
}

float4 ps_fwd_water_patio(in float2 iTex0 : TEXCOORD0, in float3 iNormal : TEXCOORD1, in float3 iPos : TEXCOORD2) : COLOR0
{
	// Diffuse
	//iTex0.x = iTex0.x - WorldTime*0.06f;
	iTex0.y = iTex0.y - WorldTime*0.06f;
	float4 color = tex2D(DiffuseSampler, iTex0);

	// emissive
	float4 emissive = tex2D(EmissiveSampler, iTex0);
	
	if (emissive.r > 0.25f)
	{
		color.rgb = emissive.rgb;
		color.a = 1.0f;
	}

	return color;
}

void vs_fwd_basic( in  float4 iPos : POSITION0
			  , in float2 iTex0	: TEXCOORD0
			  , in  float4 iTan		: TEXCOORD1
			  , in float3 iNormal : NORMAL
			  , out float4 oPos : POSITION
			  , out float2 oTex0	: TEXCOORD0
			  , out float3 oNormal : TEXCOORD1
			  , out float3 oPos2 : TEXCOORD2
			  , out float4 oTan		: TEXCOORD3
			  )
{
	float4 normal = normalize(mul(iNormal,World));
	oPos = mul(iPos, World);
	oPos2 = oPos.xyz;

	oPos = mul( oPos, ViewProjection );

	oNormal = normal;
	oTex0 = iTex0;
	oTan.xyz = mul(iTan.xyz, (float3x3) World);
	oTan.w = 1;
}

void vs_fwd_lightmap( in  float4 iPos : POSITION
			  , in float2 iTex0	: TEXCOORD0
			  , in  float2 iTex1	: TEXCOORD1 // Lightmap uvs
			  , in  float4 iTan		: TEXCOORD2
			  , in float3 iNormal : NORMAL
			  , out float4 oPos : POSITION
			  , out float2 oTex0	: TEXCOORD0
			  , out float3 oNormal : TEXCOORD1
			  , out float3 oPos2 : TEXCOORD2
			  , out float2 oTex1 : TEXCOORD3
			  , out float4 oTan	: TEXCOORD4
			  )
{
	float4 normal = normalize(mul(iNormal,World));
	oPos = mul(iPos, World);
	oPos2 = oPos.xyz;

	oPos = mul( oPos, ViewProjection );

	oNormal = normal;
	oTex0 = iTex0;
	oTex1 = iTex1;
	oTan.xyz = mul(iTan.xyz, (float3x3) World);
	oTan.w = 1;
}

void vs_fwd_mix( in  float4 iPos : POSITION
			  , in float2 iTex0	: TEXCOORD0
			  , in  float2 iTex1	: TEXCOORD1 // Lightmap uvs
			  , in  float4 iTan		: TEXCOORD2
			  , in float3 iNormal : NORMAL
			  , out float4 oPos : POSITION
			  , out float2 oTex0	: TEXCOORD0
			  , out float3 oNormal : TEXCOORD1
			  , out float3 oPos2 : TEXCOORD2
			  , out float2 oTex1 : TEXCOORD3
			  , out float4 oTan	: TEXCOORD4
			  )
{
	float4 normal = normalize(mul(iNormal,World));
	oPos = mul(iPos, World);
	oPos2 = oPos.xyz;

	oPos = mul( oPos, ViewProjection );

	oNormal = normal;
	oTex0 = iTex0;
	oTex1 = iTex1;
	oTan.xyz = mul(iTan.xyz, (float3x3) World);
	oTan.w = 1;
}

void vs_fwd_mixlightmap( in  float4 iPos : POSITION
			  , in float2 iTex0	: TEXCOORD0
			  , in  float2 iTex1	: TEXCOORD1 // Mask uvs
			  , in  float2 iTex2	: TEXCOORD2 // lmap uvs
			  , in  float4 iTan		: TEXCOORD3
			  , in float3 iNormal : NORMAL
			  , out float4 oPos : POSITION
			  , out float2 oTex0	: TEXCOORD0
			  , out float3 oNormal : TEXCOORD1
			  , out float3 oPos2 : TEXCOORD2
			  , out float2 oTex1 : TEXCOORD3
			  , out float2 oTex2 : TEXCOORD4
			  , out float4 oTan	: TEXCOORD5
			  )
{
	float4 normal = normalize(mul(iNormal,World));
	oPos = mul(iPos, World);
	oPos2 = oPos.xyz;

	oPos = mul( oPos, ViewProjection );

	oNormal = normal;
	oTex0 = iTex0;
	oTex1 = iTex1;
	oTex2 = iTex2;
	oTan.xyz = mul(iTan.xyz, (float3x3) World);
	oTan.w = 1;
}

void vs_fwd_skin( in  float4 iPos    : POSITION
				  , in  float3 iNormal : NORMAL
				  , in  float2 iTex0   : TEXCOORD0
				  , in  float4 iTan		: TEXCOORD1
				  , in  float4 iWeights : BLENDWEIGHT
				  , in  float4 iBoneIds : BLENDINDICES
				  , out float4 oPos    : POSITION
				  , out float2 oTex0   : TEXCOORD0
				  , out float3 oNormal : TEXCOORD1	
				  , out float3 oPos2   : TEXCOORD2
				  , out float4 oTan		: TEXCOORD3
				  )
{
	float3x4 skin_mtx = getSkinMatrix( iBoneIds, iWeights );

	oPos = float4(mul( skin_mtx, iPos ), 1);
	oPos2 = oPos;
	oPos = mul(oPos, ViewProjection);
    oNormal = mul( (float3x3)skin_mtx, iNormal );
    oPos2 = oPos2;

	oTex0 = iTex0;
	oTan.xyz = mul(iTan.xyz, (float3x3) World);
	oTan.w = 1;
}

void vs_fwd_skin_inflated( in  float4 iPos    : POSITION
				  , in  float3 iNormal : NORMAL
				  , in  float2 iTex0   : TEXCOORD0
				  , in  float4 iTan		: TEXCOORD1
				  , in  float4 iWeights : BLENDWEIGHT
				  , in  float4 iBoneIds : BLENDINDICES
				  , out float4 oPos    : POSITION
				  , out float2 oTex0   : TEXCOORD0
				  , out float3 oNormal : TEXCOORD1	
				  , out float3 oPos2   : TEXCOORD2
				  , out float4 oTan		: TEXCOORD3
				  )
{
	float3x4 skin_mtx = getSkinMatrix( iBoneIds, iWeights );
    oNormal = mul( (float3x3)skin_mtx, iNormal );

	oPos = float4(mul( skin_mtx, iPos ), 1);
	oPos2 = oPos;
	oPos.xyz += mul(LineThickness,oNormal);
	oPos = mul(oPos, ViewProjection);
    oPos2 = oPos2;

	oTex0 = iTex0;
	oTan.xyz = mul(iTan.xyz, (float3x3) World);
	oTan.w = 1;
}

float4 ps_fwd_basic(in float2 iTex0 : TEXCOORD0, in float3 iNormal : TEXCOORD1, in float3 iPos : TEXCOORD2) : COLOR0
{
	// Diffuse
	float4 color = tex2D(DiffuseSampler, iTex0);
	color *= DiffuseColor;
	//if (color.a < 0.001f) discard;
	// emissive
	float4 emissive = tex2D(EmissiveSampler, iTex0);

	if (emissive.r > 0.25f)
	{
		color.rgb = emissive*2;
	}

	return color;
}

float4 ps_fwd_clouds(in float2 iTex0 : TEXCOORD0, in float3 iNormal : TEXCOORD1, in float3 iPos : TEXCOORD2) : COLOR0
{
	// Diffuse
	iTex0.x += WorldTime * 0.0025f;
	//iTex0.y += sin(WorldTime)/500;
	float4 color = tex2D(DiffuseSampler, iTex0);
	color *= DiffuseColor;
	//if (color.a < 0.001f) discard;
	// emissive
	float4 emissive = tex2D(EmissiveSampler, iTex0);

	if (emissive.r > 0.25f)
	{
		color.rgb = emissive*2;
	}

	return color;
}

float4 ps_fwd_shadow_transparent(in float2 iTex0 : TEXCOORD0, in float3 iNormal : TEXCOORD1, in float3 iPos : TEXCOORD2) : COLOR0
{
	// Diffuse
	float4 color = tex2D(DiffuseSampler, iTex0);
	color *= DiffuseColor;
	float limit = PlayerLife/100.0f;
	limit = limit*0.45f + 0.4f;
	float4 mask = tex2D(MaskSampler, iTex0);
	if (iTex0.y > limit) mask.rgb = float3(0,0,0);
	color += mask;

	// noise
	//float2 noiseTex = iTex0;
	//noiseTex.x += sin(WorldTime/6);
	//noiseTex.y += sin(WorldTime/6);
	//float4 noise = tex2D(NoiseSampler, noiseTex);
	//noise += DiffuseColor;
	//noise = saturate(noise);
	//color =  color * saturate(noise*DiffuseColor);
	color.a = 0.75f;

	// emissive
	float4 emissive = tex2D(EmissiveSampler, iTex0);
	emissive += mask;

	// normal

	float3 ldir = -normalize(LightDir);
	float NdL = max(0,dot(iNormal,ldir));

	if (NdL <= 0.0f) NdL = 0.25f;
	else NdL = 0.75f;
	
	color.rgb = color.rgb * float3(LightAmbient/AmbientReductionFactor * NdL);
	if (emissive.r > 0.25f)
	{
		color.rgb = emissive*2;
	}

	return color;
}

float4 ps_fwd_special_vision(in float2 iTex0 : TEXCOORD0, in float3 iNormal : TEXCOORD1, in float3 iPos : TEXCOORD2) : COLOR0
{
	// Diffuse
	float4 color = tex2D(DiffuseSampler, iTex0);
	float d = length(CameraPosition - iPos);
	color = float4(2,2,2,1.0f - d/50.0f);

	return color;
}

float4 ps_fwd_lightmap(in float2 iTex0 : TEXCOORD0, in float3 iNormal : TEXCOORD1, in float3 iPos : TEXCOORD2, in float2 iTex1 : TEXCOORD3) : COLOR0
{
	iNormal = normalize( iNormal );

	// Diffuse
	float4 color = tex2D(DiffuseSampler, iTex0);
		color *= DiffuseColor;
	if (color.a < 0.75) discard;

	// Lightmap
	float3 lmap = tex2D(LightmapSampler, iTex1).rgb;
	color.rgb *= lmap/AmbientReductionFactor;
	
	float4 emissive = tex2D(EmissiveSampler, iTex0);

	if (emissive.r > 0.25f)
	{
		color.rgb = emissive;
	}

	return color;
}

float4 ps_fwd_mix(in float2 iTex0 : TEXCOORD0, in float3 iNormal : TEXCOORD1, in float3 iPos : TEXCOORD2, in float2 iTex1 : TEXCOORD3) : COLOR0
{
	iNormal = normalize( iNormal );

	// Diffuse
	float4 color = tex2D(DiffuseSampler, iTex0);
		color *= DiffuseColor;
	if (color.a < 0.75f) discard;	
	float4 diffuse2 = tex2D(Diffuse2Sampler, iTex0);
	float mask = length(tex2D(MaskSampler, iTex1).rgb);
	color.rgb = color.rgb * (1-mask) + diffuse2.rgb * mask;
	
	float4 emissive = tex2D(EmissiveSampler, iTex0);

	if (emissive.r > 0.25f)
	{
		color.rgb = emissive;
	}

	return color;
}

float4 ps_fwd_mixlightmap(in float2 iTex0 : TEXCOORD0, in float3 iNormal : TEXCOORD1, in float3 iPos : TEXCOORD2, in float2 iTex1 : TEXCOORD3, in float2 iTex2 : TEXCOORD4) : COLOR0
{
	iNormal = normalize( iNormal );

	// Diffuse
	float4 color = tex2D(DiffuseSampler, iTex0);
		color *= DiffuseColor;
	if (color.a < 0.75f) discard;
	float4 diffuse2 = tex2D(Diffuse2Sampler, iTex0);
	float mask = length(tex2D(MaskSampler, iTex1).rgb);
	color.rgb = color.rgb * (1-mask) + diffuse2.rgb * mask;

	// Lightmap
	float3 lmap = tex2D(LightmapSampler, iTex2).rgb;
	color.rgb *= lmap/AmbientReductionFactor;
	
	float4 emissive = tex2D(EmissiveSampler, iTex0);

	if (emissive.r > 0.25f)
	{
		color.rgb = emissive;
	}

	return color;
}

technique tech_fwd_basic
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_fwd_basic( );
		PixelShader  = compile ps_3_0 ps_fwd_basic( );

		CULLMODE = none;
		//ZWRITEENABLE = false;
		ZENABLE = TRUE;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = srcalpha;
		//DESTBLEND = invsrc;
		blendop = ADD;
		blendopalpha = ADD;
	}

}

technique tech_fwd_clouds
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_fwd_basic( );
		PixelShader  = compile ps_3_0 ps_fwd_clouds( );

		CULLMODE = none;
		//ZWRITEENABLE = false;
		ZENABLE = TRUE;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = srcalpha;
		//DESTBLEND = invsrc;
		blendop = ADD;
		blendopalpha = ADD;
	}

}

technique tech_fwd_basic_forced
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_fwd_basic( );
		PixelShader  = compile ps_3_0 ps_fwd_basic( );

		CULLMODE = none;
		ZWRITEENABLE = false;
		ZENABLE = TRUE;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = srcalpha;
		DESTBLEND = ONE;
		blendop = ADD;
		blendopalpha = ADD;
	}

}

technique tech_fwd_lightmap
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_fwd_lightmap( );
		PixelShader  = compile ps_3_0 ps_fwd_lightmap( );
		CULLMODE = none;
		//ZWRITEENABLE = true;
		//ZFUNC = LESSEQUAL;
		//ALPHATESTENABLE = TRUE;
		//ALPHAFUNC = ALWAYS;
		//ALPHAREF = 128;
		ZENABLE = TRUE;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = srcalpha;
		//DESTBLEND = invsrcalpha;
		blendop = ADD;
		blendopalpha = ADD;
	}
}

technique tech_fwd_mix
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_fwd_mix( );
		PixelShader  = compile ps_3_0 ps_fwd_mix( );
		CULLMODE = none;
		//ZWRITEENABLE = true;
		ZENABLE = TRUE;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = srcalpha;
		//DESTBLEND = ONE;
		blendop = ADD;
		blendopalpha = ADD;
	}
}

technique tech_fwd_mixlightmap
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_fwd_mixlightmap( );
		PixelShader  = compile ps_3_0 ps_fwd_mixlightmap( );
		CULLMODE = none;
		//ZWRITEENABLE = true;
		ZENABLE = TRUE;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = srcalpha;
		//DESTBLEND = ONE;
		blendop = ADD;
		blendopalpha = ADD;
	}
}

technique tech_fwd_skin
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_fwd_skin( );
		PixelShader  = compile ps_3_0 ps_fwd_basic( );
		CULLMODE = cw;
		ZWRITEENABLE = false;
		ZENABLE = TRUE;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = srcalpha;
		DESTBLEND = INVSRCALPHA;
		blendop = ADD;
		blendopalpha = ADD;
	}
}

technique tech_fwd_shadow_transparent
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_fwd_skin( );
		PixelShader  = compile ps_3_0 ps_fwd_shadow_transparent( );
		CULLMODE = cw;
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = srcalpha;
		DESTBLEND = INVSRCALPHA;
		blendop = ADD;
		blendopalpha = ADD;
	}
}

technique tech_fwd_special_vision
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_fwd_skin();
		PixelShader = compile ps_3_0 ps_fwd_special_vision();
		Zenable = false;
		ZWriteEnable = true;
		//ZFunc = less;
		//StencilEnable = false;
		blendop = ADD;
		srcblend = srcalpha;
		destblend = INVSRCALPHA;
		AlphaBlendEnable = true;
		//AlphaTestEnable = true;	
		CullMode = cw;
	}

};

technique tech_fwd_water_patio
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_fwd_water_patio( );
		PixelShader  = compile ps_3_0 ps_fwd_water_patio( );
		CULLMODE = none;
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = srcalpha;
		DESTBLEND = INVSRCALPHA;
		blendop = ADD;
		blendopalpha = ADD;
	}
}

technique tech_fwd_water_font
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_fwd_water_font( );
		PixelShader  = compile ps_3_0 ps_fwd_water_font( );
		CULLMODE = none;
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = srcalpha;
		DESTBLEND = INVSRCALPHA;
		blendop = ADD;
		blendopalpha = ADD;
	}
}

technique tech_fwd_water_font_vertical
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_fwd_water_font_vertical( );
		PixelShader  = compile ps_3_0 ps_fwd_water_font_vertical( );
		CULLMODE = none;
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = srcalpha;
		DESTBLEND = INVSRCALPHA;
		blendop = ADD;
		blendopalpha = ADD;
	}
}

technique tech_fwd_water_cementerio
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_fwd_water_cementerio( );
		PixelShader  = compile ps_3_0 ps_fwd_water_cementerio( );
		CULLMODE = none;
		ZWRITEENABLE = true;
		ZENABLE = TRUE;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = srcalpha;
		DESTBLEND = INVSRCALPHA;
		blendop = ADD;
		blendopalpha = ADD;
	}
}

technique tech_fwd_water_aldea
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_fwd_water_aldea( );
		PixelShader  = compile ps_3_0 ps_fwd_water_aldea( );
		CULLMODE = none;
		ZWRITEENABLE = TRUE;
		ZENABLE = TRUE;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = srcalpha;
		DESTBLEND = INVSRCALPHA;
		blendop = ADD;
		blendopalpha = ADD;
	}
}

uniform float3 CameraLeft;
uniform float3 CameraUp;
uniform float  ParticleNumFrames = 1.0f;

uniform float angle = 3.14f/2.0f;

//--------------------------------------------------------------------------------------
void vsParticles( in  float3 iLocalPos   : POSITION0
                 , in  float2 iTex0       : TEXCOORD0
                 
                 , in  float3 iWorldPos   : POSITION1
                 , in  float4 iColor      : COLOR1
                 , in  float3  iSize       : TEXCOORD1
				 , in  float  iFrame       : TEXCOORD2
				 , in  float3  iAngle       : TEXCOORD3
                 
                 , out float4 oPos        : POSITION
                 , out float2 oTex0       : TEXCOORD0
                 , out float4 oColor      : TEXCOORD1
				 , out float4 oPos2		: TEXCOORD2
				 , out float4 oPos3		: TEXCOORD3
                 ) {

	iLocalPos.x = (iLocalPos.x - 0.5f) * cos(iAngle.x) + 0.5f;
	iLocalPos.y = (iLocalPos.y - 0.5f) * cos(iAngle.y) + 0.5f; 

	//ROTATION
	float c = cos(iAngle.z);
	float s = sin(iAngle.z);
	float4 rotationMatrix = float4(c, -s, s, c);
	rotationMatrix *= 0.5;
	rotationMatrix += 0.5;
	float4 rotation = rotationMatrix * 2 - 1;
	iLocalPos.xy -= 0.5;
	iLocalPos.xy = mul( iLocalPos.xy, float2x2(rotation));
	iLocalPos.xy *= sqrt(2);
	iLocalPos.xy += 0.5;

	float3 world_pos = ( CameraUp  * ( iLocalPos.y - 0.5 )
	                   - CameraLeft * ( iLocalPos.x - 0.5 ) );

	world_pos *= iSize;

    oPos = mul( float4(world_pos,1), World );
	
	oPos += float4(iWorldPos,0);

	oPos3 = oPos;
    oPos = mul(oPos, ViewProjection);
	oPos2 = oPos;

	if (ParticleNumFrames > 1)
	{
		int frame = (iFrame*5)%ParticleNumFrames;
		oTex0.x = (iTex0.x + frame)/ParticleNumFrames;
	}
	else 
	oTex0 = iTex0;
    oColor = iColor;
}

void vsParticlesNoWorld( in  float3 iLocalPos   : POSITION0
                 , in  float2 iTex0       : TEXCOORD0
                 
                 , in  float3 iWorldPos   : POSITION1
                 , in  float4 iColor      : COLOR1
                 , in  float3  iSize       : TEXCOORD1
				 , in  float  iFrame       : TEXCOORD2
				 , in  float3  iAngle       : TEXCOORD3
                 
                 , out float4 oPos        : POSITION
                 , out float2 oTex0       : TEXCOORD0
                 , out float4 oColor      : TEXCOORD1
				 , out float4 oPos2		: TEXCOORD2
				 , out float4 oPos3		: TEXCOORD3
                 ) {

	iLocalPos.x = (iLocalPos.x - 0.5f) * cos(iAngle.x);
	iLocalPos.y = (iLocalPos.y - 0.5f) * cos(iAngle.y); 

	//ROTATION
	float c = cos(iAngle.z);
	float s = sin(iAngle.z);
	float4 rotationMatrix = float4(c, -s, s, c);
	rotationMatrix *= 0.5;
	rotationMatrix += 0.5;
	float4 rotation = rotationMatrix * 2 - 1;
	//iLocalPos.xy -= 0.5;
	iLocalPos.xy = mul( iLocalPos.xy, float2x2(rotation));
	iLocalPos.xy *= sqrt(2);
	iLocalPos.xy += 0.5;

	float3 world_pos = ( CameraUp  * ( iLocalPos.y - 0.5 )
	                   - CameraLeft * ( iLocalPos.x - 0.5 ) );

	world_pos *= iSize;
	
    oPos = float4(world_pos,1);
	oPos += float4(iWorldPos,0);
	oPos3 = oPos;
	
    oPos = mul(oPos, ViewProjection);
	oPos2 = oPos;
	if (ParticleNumFrames > 1)
	{
		int frame = (iFrame*5)%ParticleNumFrames;
		oTex0.x = (iTex0.x + frame)/ParticleNumFrames;
	}
	else oTex0 = iTex0;
    oColor = iColor;
}

//--------------------------------------------------------------------------------------
float4 psParticles( in  float2 iTex0   : TEXCOORD0
                   , in  float4 iColor  : TEXCOORD1
				   , in float4 iPos	: TEXCOORD2
				   , in float4 iPos2 : TEXCOORD3
                   ) : COLOR0
{

	float4 diffuse = tex2D( DiffuseSampler, iTex0);
	diffuse.rgb = iColor.rgb;
	diffuse.a *= iColor.a;

	float d = length(CameraPosition - iPos2);
	if ( d < 1.0f ) diffuse.a *= d/1.0f;

	return diffuse;
}

float4 psParticlesSoft( in  float2 iTex0   : TEXCOORD0
                   , in  float4 iColor  : TEXCOORD1
				   , in float4 iPos	: TEXCOORD2
				   , in float4 iPos2 : TEXCOORD3
                   ) : COLOR0
{

	float4 diffuse = tex2D( DiffuseSampler, iTex0);
	diffuse.rgb = iColor.rgb;
	diffuse.a *= iColor.a;

	float d = length(CameraPosition - iPos2);
	if ( d < 1.0f ) diffuse.a *= d/1.0f;

	// Test depth
	iPos.xy /= iPos.w;
	float2 texCoord = 0.5f * (float2(iPos.x,-iPos.y) + 1.0f);
	texCoord.x += ScreenWidth/2.0f;
	texCoord.y += ScreenHeight/2.0f;
	float texDepth = tex2D(DepthRTSampler,texCoord).r;

	float z = iPos.z / iPos.w;

	if ( z > texDepth) discard;
	float fade = saturate((texDepth-z)*100);

	diffuse.a *= fade;
	//diffuse.rgb = float3(fade, fade, fade);
	//diffuse.a = 1.0f;

	return diffuse;
}

//--------------------------------------------------------------------------------------
technique tech_particles
{   
	pass P0
    {      
        VertexShader = compile vs_3_0 vsParticles( );
        PixelShader  = compile ps_3_0 psParticles( );

		CULLMODE = none;
		ZWRITEENABLE = FALSE;
		ZENABLE = TRUE;
		//ALPHATESTENABLE = TRUE;
		//ALPHAREF = 80;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = SRCALPHA;
		//DESTBLEND = ONE;
		blendop = ADD;
		//blendopalpha = ADD;
    }
}

technique tech_particles_soft
{   
	pass P0
    {      
        VertexShader = compile vs_3_0 vsParticles( );
        PixelShader  = compile ps_3_0 psParticlesSoft( );

		CULLMODE = none;
		ZWRITEENABLE = FALSE;
		ZENABLE = TRUE;
		//ALPHATESTENABLE = TRUE;
		//ALPHAREF = 80;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = SRCALPHA;
		//DESTBLEND = ONE;
		blendop = ADD;
		//blendopalpha = ADD;
    }
}

technique tech_particles_no_world
{   
	pass P0
    {      
        VertexShader = compile vs_3_0 vsParticlesNoWorld( );
        PixelShader  = compile ps_3_0 psParticles( );

		CULLMODE = none;
		ZWRITEENABLE = FALSE;
		ZENABLE = TRUE;
		//ALPHATESTENABLE = TRUE;
		//ALPHAREF = 80;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = SRCALPHA;
		//DESTBLEND = ONE;
		blendop = ADD;
		//blendopalpha = ADD;
    }
}

technique tech_particles_no_world_soft
{   
	pass P0
    {      
        VertexShader = compile vs_3_0 vsParticlesNoWorld( );
        PixelShader  = compile ps_3_0 psParticlesSoft( );

		CULLMODE = none;
		ZWRITEENABLE = FALSE;
		ZENABLE = TRUE;
		//ALPHATESTENABLE = TRUE;
		//ALPHAREF = 80;
		ALPHABLENDENABLE = TRUE;
		SRCBLEND = SRCALPHA;
		//DESTBLEND = ONE;
		blendop = ADD;
		blendopalpha = ADD;
    }
}
