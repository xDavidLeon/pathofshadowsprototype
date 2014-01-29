#pragma once
#include "texture_manager.h"
#include <vector>
#include <algorithm>
#include "camera.h"
#define NO_STEREO_D3D10

#include "nvapi.h"
#include "nvstereo.h"
class Entity;
class Component;
class World;
class ModelComponent;
class ShadowComponent;
class TransformComponent;
class LightComponent;


class RendererSystem
{
public:
	static RendererSystem & get()
	{
		static RendererSystem cs;
		return cs;
	}

	void update(float delta);
	void render(void);
	bool loadShaders( void );

	void takeScreenshot(bool rt);
	void setFog(float start, float end);
	void drawSprite(D3DXVECTOR3 pivot, D3DXVECTOR3 position, LPDIRECT3DTEXTURE9 texture, D3DCOLOR color);
	void drawSprite(D3DXVECTOR2 spriteCentre, D3DXVECTOR2 position, LPDIRECT3DTEXTURE9 texture, D3DXVECTOR2 scale, D3DCOLOR color  );
	void drawSprite(LPDIRECT3DTEXTURE9 texture, const D3DXMATRIX &sM, D3DCOLOR color);

private:
	RendererSystem(void);
	~RendererSystem(void);

	World * w;

	HRESULT hr;
	float fogStart, fogEnd;
	TCamera * currentCamera;

	// Render Target Buffers - 32 bits each buffer
	IDirect3DTexture9* _rt_color; // 4 channels: [R, G, B, Emissive flag]
	//IDirect3DTexture9* _rt_lightmap; // 4 channels: [R, G, B, FREE]
	IDirect3DTexture9* _rt_normal; // 4 channels: [Normal X, Y, Z, FREE]
	IDirect3DTexture9* _rt_depth; // 1 channel: [Depth]

	IDirect3DTexture9* _rt_lights; // 4 channels: [R,G,B,A]
	IDirect3DTexture9* _rt_composed; // 4 channels: [R,G,B,A]
	IDirect3DTexture9* _rt_composed_alt; // 4 channels: [R,G,B,A] 
	IDirect3DTexture9* _rt_low; // 4 channels: [R,G,B,A] 
	IDirect3DTexture9* _rt_blur; // 4 channels: [R,G,B,A] 
	IDirect3DTexture9* _rt_bloom; // 4 channels: [R,G,B,A] 

	LPDIRECT3DCUBETEXTURE9 _texture_skyBox;
	TTexture _texture_noise;
	//TTexture _texture_hp_back;
	//TTexture _texture_hp_front;
	float _hp_width;
	float _hp_height;

	LPD3DXMESH _mesh_skyBox;
	LPD3DXMESH _mesh_sphere;
	LPD3DXSPRITE _sprite;
	IDirect3DTexture9* _texture_shadow_creation;

	std::string _curr_tech;

	nv::stereo::ParamTextureManagerD3D9* gStereoTexMgr;
	IDirect3DTexture9* gStereoParamTexture;
	HRESULT CreateStereoParamTextureAndView();

	void releaseRenderTargets(void);
	void setRenderTarget(int index, IDirect3DTexture9 * rt);

	void clearTexture(IDirect3DTexture9* pd3dTexture,D3DCOLOR xColor);
	void resolveRT(void);
	void clearBuffers(void);

	void loadContent(void);
	bool loadShader(LPD3DXEFFECT & effect, const char * filename);
	void releaseShaders(void);

	void swapBuffers(IDirect3DTexture9* b1, IDirect3DTexture9* b2);
	void copyBuffers(IDirect3DTexture9* b1, IDirect3DTexture9* b2);

	void setCommonShaderParams(void);
	void renderPostFX(void);
	void renderSkyBox(void);
	void renderEntity(Entity* entity);
	void renderModel(ModelComponent * model, bool detail_low = false);
	void renderScene(void);
	void renderBuildPass(void);
	//void renderDebug(void);
	void renderLights(void);
	void renderComposed(void);
	void renderOutline(void);
	//void renderDirectionalLight(TransformComponent * t, LightComponent * l);
	void renderPointLight(TransformComponent * t, LightComponent * l);
	void renderShadowOrb(TransformComponent* t, ShadowComponent * l);
	void renderBlob(TransformComponent* t, LightComponent * l);

	//void renderSpotLight(TransformComponent * t, LightComponent * l);

	void renderVignetting(void);

	void renderImageEffects(float contrast, float brightness, D3DXVECTOR3 saturation);
	void renderFog(void);
	void renderPreOutline(void);
	void renderFinal(void);
	void renderForward(void);
	void renderUI(void);
	void applyRenderFlags(ModelComponent* model);
	void restoreRenderFlags(ModelComponent* model);
	void renderForwardPass(void);
	void renderFXWiggle(void);
	void renderFXDoF(void);
	void renderFXBloom(void);

	void renderParticleEffects(void);
	void renderSkinnedMesh(ModelComponent* model);
};

