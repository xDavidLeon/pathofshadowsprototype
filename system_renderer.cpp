#include "system_renderer.h"
#include "entity_manager.h"
#include "component_model.h"
#include "component_transform.h"
#include "component_rigidbody.h"
#include "d3ddefs.h"
#include "world.h"
#include "component_charcontroller.h"
#include "component_light.h"
#include "component_automat.h"
#include "component_shadow.h"
#include "dijkstra.h"
#include "system_camera.h"
#include "component_particle_effect.h"
#include "data_saver.h"
#include "component_player_controller.h"
#include "component_shadow_actions.h"
#include "system_debug.h"
#include "system_bt.h"

extern void buildMeshQuadXY( CDataSaver &ds );

RendererSystem::RendererSystem(void)
{
	w = World::instance();
	loadContent();
}

RendererSystem::~RendererSystem()
{
	releaseRenderTargets();
	releaseShaders();
	//_texture_hp_back->Release();
	_texture_skyBox->Release();
	_texture_shadow_creation->Release();
	_sprite->Release();
	_mesh_sphere->Release();
	_mesh_skyBox->Release();
	gStereoParamTexture->Release();
	delete gStereoTexMgr;
	NvAPI_Unload();
	ParticleEffectComponent::destroyStatic();
}

void RendererSystem::loadContent(void)
{
	NvAPI_Initialize();
	gStereoTexMgr = NULL;
	gStereoTexMgr = new nv::stereo::ParamTextureManagerD3D9;
	gStereoTexMgr->Init(g_App.GetDevice());
	CreateStereoParamTextureAndView();
	CFileDataSaver fds( "data/meshes/particle.mesh" );
	buildMeshQuadXY( fds ); 

	D3DXCreateBox(g_App.GetDevice(),10,10,10,&_mesh_skyBox,NULL);
	D3DXCreateSphere(g_App.GetDevice(), 5, 10, 10, &_mesh_sphere, NULL);
	D3DXCreateCubeTextureFromFile(g_App.GetDevice(), "data/textures/hardcoded/skybox.dds", &_texture_skyBox);
	D3DXCreateSprite(g_App.GetDevice(),&_sprite);

	int backBufferWidth =  g_App.GetParameters().BackBufferWidth;
	int backBufferHeight =  g_App.GetParameters().BackBufferHeight;

	g_App.GetDevice()->CreateTexture(backBufferWidth,backBufferHeight,1,D3DUSAGE_RENDERTARGET, 
		D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&_rt_color,NULL);
	g_App.GetDevice()->CreateTexture(backBufferWidth,backBufferHeight,1,D3DUSAGE_RENDERTARGET, 
		D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&_rt_normal,NULL);
	g_App.GetDevice()->CreateTexture(backBufferWidth,backBufferHeight,1,D3DUSAGE_RENDERTARGET, 
		D3DFMT_R32F,D3DPOOL_DEFAULT,&_rt_depth,NULL);
	g_App.GetDevice()->CreateTexture(backBufferWidth,backBufferHeight,1,D3DUSAGE_RENDERTARGET, 
		D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&_rt_lights,NULL);
	g_App.GetDevice()->CreateTexture(backBufferWidth,backBufferHeight,1,D3DUSAGE_RENDERTARGET, 
		D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&_rt_composed,NULL);
	g_App.GetDevice()->CreateTexture(backBufferWidth,backBufferHeight,1,D3DUSAGE_RENDERTARGET, 
		D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&_rt_composed_alt,NULL);
	g_App.GetDevice()->CreateTexture(backBufferWidth,backBufferHeight,1,D3DUSAGE_RENDERTARGET, 
		D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&_rt_blur,NULL);
	g_App.GetDevice()->CreateTexture(backBufferWidth,backBufferHeight,1,D3DUSAGE_RENDERTARGET, 
		D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&_rt_low,NULL);
	g_App.GetDevice()->CreateTexture(backBufferWidth,backBufferHeight,1,D3DUSAGE_RENDERTARGET, 
		D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&_rt_bloom,NULL);

	// UI
	float aspect = g_App.GetWidth() / g_App.GetHeight();
	_hp_width = 256.0f * aspect / 2.0f;
	_hp_height = 1024.0f * aspect / 2.0f;
	//_texture_hp_back = TTextureManager::get().getTextureResized("hardcoded/hp_empty",_hp_width, _hp_height);
	//_texture_hp_front = TTextureManager::get().getTextureResized("hardcoded/hp_full", _hp_width, _hp_height);
	_texture_noise = TTextureManager::get().getTexture("hardcoded/noise");
	_texture_shadow_creation = TTextureManager::get().getTexture("hardcoded/shadow_creation");

	// Shaders
	loadShaders();

	fogStart = 1.0f;
	fogEnd = 35.0f;
}

void RendererSystem::update(float delta)
{
	currentCamera = &CameraSystem::get().getCurrentCamera();

	std::map<Entity*,Component*>* entities = EntityManager::get().getAllEntitiesPosessingComponent<ParticleEffectComponent>();
	if (entities)
	{
		std::map<Entity*,Component*>::iterator iter;

		for (iter = entities->begin(); iter != entities->end(); ++iter)
		{
			Entity* entity = iter->first;
			if (entity == NULL) continue;
			if (entity->enabled == false) continue;

			ParticleEffectComponent * particles = EntityManager::get().getComponent<ParticleEffectComponent>(entity);
			if (particles->enabled == false) continue;


			particles->update(delta);
			if (particles->checkForDeletion()) return;
		}
	}


}

void RendererSystem::swapBuffers(IDirect3DTexture9* b1, IDirect3DTexture9* b2)
{
	IDirect3DTexture9* aux = b1;
	b1 = b2;
	b2 = aux;
}

void RendererSystem::copyBuffers(IDirect3DTexture9* b1, IDirect3DTexture9* b2)
{
	IDirect3DSurface9* pd3dSurfaceA;
	b1->GetSurfaceLevel(0,&pd3dSurfaceA);

	IDirect3DSurface9* pd3dSurfaceB;
	b2->GetSurfaceLevel(0,&pd3dSurfaceB);

	g_App.GetDevice()->StretchRect( pd3dSurfaceA, NULL, pd3dSurfaceB, NULL, D3DTEXF_LINEAR);
}

void RendererSystem::setRenderTarget(int index, IDirect3DTexture9 * rt)
{
	IDirect3DSurface9* pd3dSurface;
	rt->GetSurfaceLevel(0,&pd3dSurface);
	g_App.GetDevice()->SetRenderTarget(index,pd3dSurface);
	if (pd3dSurface) pd3dSurface->Release();
}

void RendererSystem::resolveRT(void)
{
	// disable render targets
	IDirect3DSurface9* pd3dBackBufferSurface;
	g_App.GetDevice()->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pd3dBackBufferSurface);
	g_App.GetDevice()->SetRenderTarget(0,pd3dBackBufferSurface);
	pd3dBackBufferSurface->Release();

	g_App.GetDevice()->SetRenderTarget(1, NULL);
	g_App.GetDevice()->SetRenderTarget(2, NULL);
	g_App.GetDevice()->SetRenderTarget(3, NULL);
}

void RendererSystem::clearTexture(IDirect3DTexture9* pd3dTexture,D3DCOLOR xColor)
{
	IDirect3DSurface9* pd3dSurface;
	pd3dTexture->GetSurfaceLevel(0,&pd3dSurface);
	g_App.GetDevice()->ColorFill(pd3dSurface,NULL,xColor);
	if (pd3dSurface) pd3dSurface->Release();
}

void RendererSystem::clearBuffers(void)
{
	clearTexture(_rt_color,0x00000000);
	clearTexture(_rt_normal,0xff7f7f7f);
	clearTexture(_rt_depth,0xffffffff);
	clearTexture(_rt_lights,0x00000000);
	clearTexture(_rt_composed,0x00000000);
	clearTexture(_rt_composed_alt,0x00000000);
	clearTexture(_rt_blur,0x00000000);
	clearTexture(_rt_bloom,0x00000000);
	clearTexture(_rt_low,0x00000000);
}

void RendererSystem::releaseRenderTargets(void)
{
	SAFE_RELEASE(_rt_color);
	SAFE_RELEASE(_rt_normal);
	SAFE_RELEASE(_rt_depth);
	SAFE_RELEASE(_rt_lights);
	SAFE_RELEASE(_rt_composed);
	SAFE_RELEASE(_rt_composed_alt);
	SAFE_RELEASE(_rt_blur);
	SAFE_RELEASE(_rt_bloom);
	SAFE_RELEASE(_rt_low);
}

bool RendererSystem::loadShaders( void )
{
	g_App.LoadShader( "data/shaders/shaders.fx" );
	return true;
}

bool RendererSystem::loadShader( LPD3DXEFFECT & effect, const char *filename ) {
	if ( effect ) effect->Release();

	// Mientras no consigamos compilar sin errores...
	while( true ) {
		LPD3DXBUFFER err_buffer;
		hr = D3DXCreateEffectFromFile( 
			g_App.GetDevice()
			, filename
			, NULL			// no macros
			, NULL			// no includes
			, 0				// no flags
			, NULL			// no parameters shared
			, &effect		// result received here
			, &err_buffer			
			);
		if( hr == D3D_OK )
		{
			hr = effect->ValidateTechnique(effect->GetTechnique(0));
			return true;
		}
		const char *buffer_text = (const char *) err_buffer->GetBufferPointer();
		dbg( "Error compiling FX %s: %s\n", filename, buffer_text );
		MessageBox( NULL, buffer_text, "Error compiling FX", MB_OK );
		err_buffer->Release();
	}
	return true;
}

void RendererSystem::releaseShaders(void)
{
	g_App.DestroyShaders();
}

void RendererSystem::render(void)
{
	clearBuffers();
	setCommonShaderParams();

	switch (DebugSystem::get().draw_mode)
	{
	case DebugSystem::DRAW_MODE::WIREFRAME:
		g_App.GetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		renderForward();
		//renderParticleEffects();
		g_App.GetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		break;
	case DebugSystem::DRAW_MODE::FORWARD:
		//setRenderTarget(0, _rt_composed);
		renderForward();
		//renderParticleEffects();
		//if (DebugSystem::get().is_postfx_active) 
		//{
			//renderSkyBox();
		//	renderFXBloom();
		//	renderImageEffects(1.2f, 0.1f, D3DXVECTOR3(0,0,0));
		//	if (w->_specialVision) renderFXWiggle();
		//	renderVignetting();
		//}
		//renderFinal();
		renderUI();
		break;
	case DebugSystem::DRAW_MODE::DEFERRED:
		renderBuildPass();
		if (DebugSystem::get().is_lights_active) renderLights();
		renderComposed();

		if (DebugSystem::get().is_postfx_active) 
		{
			renderFog();
			renderFXDoF();
			renderSkyBox();
			renderPreOutline();
			renderOutline();
			renderForwardPass();
			renderParticleEffects();
			renderFXBloom();
			renderImageEffects(1.2f, 0.1f, D3DXVECTOR3(0,0,0));
			if (w->_specialVision) renderFXWiggle();
			renderVignetting();
		}

		renderFinal();

		renderUI();
		break;
	case DebugSystem::DRAW_MODE::RT:
		renderBuildPass();
		drawSprite(D3DXVECTOR2(g_App.GetWidth()/2,g_App.GetHeight()/2),D3DXVECTOR2((g_App.GetWidth()/3) * 0,0), _rt_color, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
		drawSprite(D3DXVECTOR2(g_App.GetWidth()/2,g_App.GetHeight()/2),D3DXVECTOR2((g_App.GetWidth()/3) * 1,0), _rt_normal, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
		drawSprite(D3DXVECTOR2(g_App.GetWidth()/2,g_App.GetHeight()/2),D3DXVECTOR2((g_App.GetWidth()/3) * 2,0), _rt_depth, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
		if (DebugSystem::get().is_lights_active) 
		{
			renderLights();
			drawSprite(D3DXVECTOR2(g_App.GetWidth()/2,g_App.GetHeight()/2),D3DXVECTOR2((g_App.GetWidth()/3) * 0,(g_App.GetHeight()/3) * 1), _rt_lights, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
		}

		renderComposed();
		drawSprite(D3DXVECTOR2(g_App.GetWidth()/2,g_App.GetHeight()/2),D3DXVECTOR2((g_App.GetWidth()/3) * 1,(g_App.GetHeight()/3) * 1), _rt_composed, D3DXVECTOR2(1/3.0f,1/3.0f), D3DCOLOR_RGBA(255,255,255,255));
		
		if (DebugSystem::get().is_postfx_active) 
		{
			renderFog();
			renderFXDoF();
			renderSkyBox();
			renderPreOutline();
			renderOutline();
			renderForwardPass();
			renderParticleEffects();
			renderFXBloom();

			renderImageEffects(1.2f, 0.1f, D3DXVECTOR3(0,0,0));
			if (w->_specialVision) renderFXWiggle();
			renderVignetting();

		}

		drawSprite(D3DXVECTOR2(g_App.GetWidth()/2,g_App.GetHeight()/2),D3DXVECTOR2((g_App.GetWidth()/3) * 1,(g_App.GetHeight()/3) * 2), gStereoParamTexture, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
		break;
	default:
		break;
	}

	 
}

void RendererSystem::renderPostFX(void)
{
	renderFog();
	renderFXDoF();
	renderFXBloom();
	renderOutline();

	renderImageEffects(1.2f, 0.1f, D3DXVECTOR3(0,0,0));
	if (w->_specialVision) renderFXWiggle();
	renderVignetting();
}

void RendererSystem::renderBuildPass(void)
{
	resolveRT();

	setRenderTarget(0,_rt_color);
	setRenderTarget(1,_rt_normal);
	setRenderTarget(2,_rt_depth);
	setRenderTarget(3,_rt_lights);

	renderScene();
	if (gStereoTexMgr->IsStereoActive()) gStereoTexMgr->UpdateStereoTexture(g_App.GetDevice(), gStereoParamTexture, false);
	resolveRT();
}

void RendererSystem::renderScene(void)
{
	g_App.GetDevice()->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);

	std::map<Entity*,Component*>* entities = EntityManager::get().getAllEntitiesPosessingComponent<ModelComponent>();
	std::map<Entity*,Component*>::iterator iter;
	for (iter = entities->begin(); iter != entities->end(); ++iter)
	{
		renderEntity(iter->first);
	}
}

void RendererSystem::renderPreOutline(void)
{
	g_App.GetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	setRenderTarget(0, _rt_composed);

	// Special particles
	if (DebugSystem::get().is_particles_active) 
	{
		std::map<Entity*,Component*>* entities = EntityManager::get().getAllEntitiesPosessingComponent<ParticleEffectComponent>();
		if (entities != NULL) 
		{
			std::map<Entity*,Component*>::iterator iter;

			for (iter = entities->begin(); iter != entities->end(); ++iter)
			{
				Entity* entity = iter->first;
				if (entity->enabled == false) continue;

				ParticleEffectComponent * particles = EntityManager::get().getComponent<ParticleEffectComponent>(entity);
				if (particles->enabled == false) continue;
				if (particles->ignore_outline) continue;
				TransformComponent * tComponent = EntityManager::get().getComponent<TransformComponent>(entity);
				if (particles->aabb_radius > 0) 
				{
					if (currentCamera->frustum.isInside(particles->aabb) == TFrustum::OUT_OF_FRUSTUM) continue;
					if (currentCamera->getPosition().distance(tComponent->getPosition()) > 19.5f) continue;
				}

				if (_curr_tech != particles->tech_name)
				{
					_curr_tech = particles->tech_name;
					hr = g_App.effect->SetTechnique(_curr_tech.c_str());
					assert(hr == D3D_OK);
				}

				if (particles->additive) g_App.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
				else g_App.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

				g_App.effect->SetTexture("Diffuse",particles->texture);
				g_App.effect->SetInt("ParticleNumFrames", particles->num_frames);

				btTransform * world = tComponent->getWorldTransform();
				D3DXMATRIX d3dxMatrix;
				convertBulletTransform( world, d3dxMatrix);
				g_App.effect->SetMatrix("World",&d3dxMatrix);
				g_App.effect->CommitChanges();

				int iNumPasses;
				g_App.effect->Begin((UINT*)&iNumPasses,0);
				for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
				{
					g_App.effect->CommitChanges();
					g_App.effect->BeginPass(iPassIdx);
					particles->render();

					g_App.effect->EndPass();
				}
				g_App.effect->End();
			}
		}
	}
	
	// Special models
	std::map<Entity*,Component*>* entities = EntityManager::get().getAllEntitiesPosessingComponent<ModelComponent>();
	std::map<Entity*,Component*>::iterator iter;

	string tech_name;

	for (iter = entities->begin(); iter != entities->end(); ++iter)
	{
		Entity* entity = iter->first;
		if (entity->enabled == false) continue;

		ModelComponent * model = EntityManager::get().getComponent<ModelComponent>(entity);
		if (model->enabled == false) continue;
		if (model->render_flags["forward"] != true || model->render_flags["before_outline"] != true) continue;
		TransformComponent * tComponent = EntityManager::get().getComponent<TransformComponent>(entity);

		btTransform * world = tComponent->getWorldTransform();
		D3DXMATRIX d3dxMatrix;
		convertBulletTransform( world, d3dxMatrix);
		TAABB aabb = model->getMesh()->aabb;
		aabb = aabb.getRotatedBy(d3dxMatrix);

		if (currentCamera->frustum.isInside(aabb) == TFrustum::OUT_OF_FRUSTUM) continue;

		// Size + dist culling
		float dist = world->getOrigin().distance(currentCamera->getPosition());
		//float deg = atan(aabb.getRadius()/dist);
		//if (deg < (currentCamera->getFov()/50.0f)) return;

		g_App.effect->SetMatrix( "World", &d3dxMatrix );
		g_App.effect->SetFloatArray( "DiffuseColor", model->diffuseColor , 4);

		// Para todos los grupos de la mesh
		for( size_t i=0; i<model->getMesh()->getGroupsCount(); ++i ) {
			// Aplicar materiales

			if( i < model->getMaterials()->size() )
			{
				//tech_name = model->getMaterials()->at(i)->name;
				const TMaterial * m = model->getMaterials()->at( i );
				tech_name = m->name;
				//if (m->name == "tech_lightmap") tech_name = "tech_fwd_lightmap";
				g_App.effect->SetTexture( "Diffuse", m->diffuse );
				g_App.effect->SetTexture( "Diffuse2", m->diffuse2 );
				g_App.effect->SetTexture( "Lightmap", m->lightmap );
				g_App.effect->SetTexture( "Mask", m->mask );
				g_App.effect->SetTexture( "Normal", m->normalmap );
				g_App.effect->SetTexture( "Emissive", m->emissive );
			}
			if (model->render_flags["additive"] == true)
			{
				tech_name = "tech_fwd_basic_forced";
			}
			else
			{
				if (tech_name == "tech_basic") tech_name = "tech_fwd_basic";
				else if (tech_name == "tech_lightmap") tech_name = "tech_fwd_lightmap";
				else if (tech_name == "tech_mix_lightmap") tech_name = "tech_fwd_mixlightmap";
				else if (tech_name == "tech_mix") tech_name = "tech_fwd_mix";
			}
			//else if (tech_name == "tech_mix_lightmap") tech_name = "tech_fwd_basic";

			if (_curr_tech != tech_name)
			{
				_curr_tech = tech_name;
				hr = g_App.effect->SetTechnique(_curr_tech.c_str());
				assert(hr == D3D_OK);
			}

			applyRenderFlags(model);
			g_App.effect->CommitChanges();
			UINT npasses;
			g_App.effect->Begin(&npasses,0);
			for (UINT pass = 0; pass < npasses; ++pass) {
				g_App.effect->BeginPass(pass);
				model->getMesh()->renderGroup( i );
				g_App.effect->EndPass();
			}
			g_App.effect->End();
			restoreRenderFlags(model);
			DebugSystem::get().painted_objects++;
		}
	}

	g_App.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	resolveRT();
}

void RendererSystem::renderForwardPass(void)
{
	g_App.GetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	setRenderTarget(0, _rt_composed);

	//g_App.GetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	std::map<Entity*,Component*>* entities = EntityManager::get().getAllEntitiesPosessingComponent<ModelComponent>();
	std::map<Entity*,Component*>::iterator iter;

	string tech_name;

	for (iter = entities->begin(); iter != entities->end(); ++iter)
	{
		Entity* entity = iter->first;
		if (entity->enabled == false) continue;

		ModelComponent * model = EntityManager::get().getComponent<ModelComponent>(entity);
		if (model->enabled == false) continue;
		TransformComponent * tComponent = EntityManager::get().getComponent<TransformComponent>(entity);

		btTransform * world = tComponent->getWorldTransform();
		D3DXMATRIX d3dxMatrix;
		convertBulletTransform( world, d3dxMatrix);
		TAABB aabb = model->getMesh()->aabb;
		aabb = aabb.getRotatedBy(d3dxMatrix);

		if (currentCamera->frustum.isInside(aabb) == TFrustum::OUT_OF_FRUSTUM) continue;

		// Size + dist culling
		float dist = world->getOrigin().distance(currentCamera->getPosition());
		//float deg = atan(aabb.getRadius()/dist);
		//if (deg < (currentCamera->getFov()/50.0f)) return;


		g_App.effect->SetMatrix( "World", &d3dxMatrix );
		g_App.effect->SetFloatArray( "DiffuseColor", model->diffuseColor , 4);

		if (w->_specialVision && model->getCModel() != NULL && entity->type == "ENEMY") 
		{
			tech_name = "tech_fwd_special_vision";
			for( size_t object=0; object<model->getCModel()->getCore()->_objects.size(); ++object ) {
				// Para todos los grupos de la mesh
				for( size_t i=0; i<model->getCModel()->getCore()->_objects[object]->getGroupsCount(); ++i ) {
					// Aplicar materiales
					if( i < model->getMaterials()->size() )
					{
						TMaterial * m = model->getMaterials()->at( i );
						if (m->main_texture != NULL)
						{
							g_App.effect->SetTexture( "Diffuse", m->diffuse );
							g_App.effect->SetTexture( "Normal", m->normalmap );
							g_App.effect->SetTexture( "Emissive", m->emissive );
							g_App.effect->SetTexture( "Mask", m->mask );
						}
					}
				
					if (_curr_tech != tech_name)
					{
						_curr_tech = tech_name;
						hr = g_App.effect->SetTechnique(_curr_tech.c_str());
						assert(hr == D3D_OK);
					}

					applyRenderFlags(model);
					g_App.effect->CommitChanges();
					UINT npasses;
					hr = g_App.effect->Begin(&npasses,0);
					assert(hr == D3D_OK);
					for (UINT pass = 0; pass < npasses; ++pass) {
						g_App.effect->BeginPass(pass);
						model->getCModel()->renderSubGroupWithShader( i, object );
						g_App.effect->EndPass();
					}
					g_App.effect->End();
					restoreRenderFlags(model);
				}
			}
			DebugSystem::get().painted_objects++;

		}
		else if (model->getCModel() != NULL && entity->type == "PLAYER" && dist < 1.0f) 
		{
			tech_name = "tech_fwd_shadow_transparent";
			for( size_t object=0; object<model->getCModel()->getCore()->_objects.size(); ++object ) {
				// Para todos los grupos de la mesh
				for( size_t i=0; i<model->getCModel()->getCore()->_objects[object]->getGroupsCount(); ++i ) {
					// Aplicar materiales
					if( i < model->getMaterials()->size() )
					{
						TMaterial * m = model->getMaterials()->at( i );
						if (m->main_texture != NULL)
						{
							g_App.effect->SetTexture( "Diffuse", m->diffuse );
							g_App.effect->SetTexture( "Normal", m->normalmap );
							g_App.effect->SetTexture( "Emissive", m->emissive );
							g_App.effect->SetTexture( "Mask", m->mask );
						}
					}
				
					if (_curr_tech != tech_name)
					{
						_curr_tech = tech_name;
						hr = g_App.effect->SetTechnique(_curr_tech.c_str());
						assert(hr == D3D_OK);
					}
					applyRenderFlags(model);
					g_App.effect->CommitChanges();
					UINT npasses;
					hr = g_App.effect->Begin(&npasses,0);
					assert(hr == D3D_OK);
					for (UINT pass = 0; pass < npasses; ++pass) {
						g_App.effect->BeginPass(pass);
						model->getCModel()->renderSubGroupWithShader( i, object );
						g_App.effect->EndPass();
					}
					g_App.effect->End();
					restoreRenderFlags(model);
				}
			}
			DebugSystem::get().painted_objects++;

		}
		else if (model->getCModel() != NULL && model->render_flags["forward"] == true)
		{
			tech_name = "tech_fwd_skin";
			for( size_t object=0; object<model->getCModel()->getCore()->_objects.size(); ++object ) {
				// Para todos los grupos de la mesh
				for( size_t i=0; i<model->getCModel()->getCore()->_objects[object]->getGroupsCount(); ++i ) {
					// Aplicar materiales
					if( i < model->getMaterials()->size() )
					{
						TMaterial * m = model->getMaterials()->at( i );
						if (m->main_texture != NULL)
						{
							g_App.effect->SetTexture( "Diffuse", m->diffuse );
							g_App.effect->SetTexture( "Normal", m->normalmap );
							g_App.effect->SetTexture( "Emissive", m->emissive );
							g_App.effect->SetTexture( "Mask", m->mask );
						}
					}
				
					if (_curr_tech != tech_name)
					{
						_curr_tech = tech_name;
						hr = g_App.effect->SetTechnique(_curr_tech.c_str());
						assert(hr == D3D_OK);
					}
					applyRenderFlags(model);

					g_App.effect->CommitChanges();
					UINT npasses;
					hr = g_App.effect->Begin(&npasses,0);
					assert(hr == D3D_OK);
					for (UINT pass = 0; pass < npasses; ++pass) {
						g_App.effect->BeginPass(pass);
						model->getCModel()->renderSubGroupWithShader( i, object );
						g_App.effect->EndPass();
					}
					g_App.effect->End();
					restoreRenderFlags(model);
				}
			}
			DebugSystem::get().painted_objects++;

		}
		else if (model->render_flags["forward"] == true)
		{
			// Para todos los grupos de la mesh
			for( size_t i=0; i<model->getMesh()->getGroupsCount(); ++i ) {
				// Aplicar materiales

				if( i < model->getMaterials()->size() )
				{
					//tech_name = model->getMaterials()->at(i)->name;
					const TMaterial * m = model->getMaterials()->at( i );
					tech_name = m->name;
					//if (m->name == "tech_lightmap") tech_name = "tech_fwd_lightmap";
					g_App.effect->SetTexture( "Diffuse", m->diffuse );
					g_App.effect->SetTexture( "Diffuse2", m->diffuse2 );
					g_App.effect->SetTexture( "Lightmap", m->lightmap );
					g_App.effect->SetTexture( "Mask", m->mask );
					g_App.effect->SetTexture( "Normal", m->normalmap );
					g_App.effect->SetTexture( "Emissive", m->emissive );
				}
				if (model->render_flags["additive"] == true)
				{
					tech_name = "tech_fwd_basic_forced";
				}
				else
				{
					if (tech_name == "tech_basic") tech_name = "tech_fwd_basic";
					else if (tech_name == "tech_lightmap") tech_name = "tech_fwd_lightmap";
					else if (tech_name == "tech_mix_lightmap") tech_name = "tech_fwd_mixlightmap";
					else if (tech_name == "tech_mix") tech_name = "tech_fwd_mix";
				}
				//else if (tech_name == "tech_mix_lightmap") tech_name = "tech_fwd_basic";

				if (_curr_tech != tech_name)
				{
					_curr_tech = tech_name;
					hr = g_App.effect->SetTechnique(_curr_tech.c_str());
					assert(hr == D3D_OK);
				}

				applyRenderFlags(model);
				g_App.effect->CommitChanges();
				UINT npasses;
				g_App.effect->Begin(&npasses,0);
				for (UINT pass = 0; pass < npasses; ++pass) {
					g_App.effect->BeginPass(pass);
					model->getMesh()->renderGroup( i );
					g_App.effect->EndPass();
				}
				g_App.effect->End();
				restoreRenderFlags(model);
			}
			DebugSystem::get().painted_objects++;

		}
		
	}

	resolveRT();
}

void RendererSystem::renderEntity(Entity* entity)
{
	if (entity->enabled == false) return;

	ModelComponent * model = EntityManager::get().getComponent<ModelComponent>(entity);
	if (model->enabled == false) return;

	if (model->getCModel() != NULL && model->entity->type == "ENEMY" && w->_specialVision) return;
	if (model->render_flags["forward"] == true) return;

	TransformComponent * tComponent = EntityManager::get().getComponent<TransformComponent>(entity);

	btTransform * world = tComponent->getWorldTransform();

	D3DXMATRIX d3dxMatrix;
	convertBulletTransform( world, d3dxMatrix);

	TAABB aabb = model->getMesh()->aabb;
	aabb = aabb.getRotatedBy(d3dxMatrix);

	// Frustum culling
	if (currentCamera->frustum.isInside(aabb) == TFrustum::OUT_OF_FRUSTUM) return;

	// Size + dist culling
	float r = aabb.getRadius();
	float dist = world->getOrigin().distance(currentCamera->getPosition());
	float deg = atan(r/dist);
	float fov = currentCamera->getFov();
	if (deg < (fov/50.0f)) return;

	if (model->getCModel() != NULL && entity->type == "PLAYER" && dist < 1.0f) return;

	bool detail_low = false;
	if (deg < (fov/8.0f)) detail_low = true;

	g_App.effect->SetMatrix( "World", &d3dxMatrix );

	applyRenderFlags(model);

	if (model->getCModel() != NULL) renderSkinnedMesh(model);
	else renderModel(model, detail_low);

	restoreRenderFlags(model);
	
	g_App.GetDevice()->SetTexture(0,NULL);
	DebugSystem::get().painted_objects++;
}

void RendererSystem::applyRenderFlags(ModelComponent* model)
{
	if (model->render_flags["no_cull"] == true) g_App.GetDevice()->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	if (model->render_flags["additive"] == true) 
	{
		g_App.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		g_App.GetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	}
	else 
	{
		g_App.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		g_App.GetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	}
}

void RendererSystem::restoreRenderFlags(ModelComponent* model)
{
	if (model->render_flags["no_cull"] == true) g_App.GetDevice()->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);

	g_App.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	g_App.GetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

void RendererSystem::renderSkinnedMesh(ModelComponent* model)
{
	string tech_name = model->getMaterials()->at(0)->name;
	g_App.effect->SetFloatArray( "DiffuseColor", model->diffuseColor , 4);
	for( size_t object=0; object<model->getCModel()->getCore()->_objects.size(); ++object ) {
		// Para todos los grupos de la mesh
		for( size_t i=0; i<model->getCModel()->getCore()->_objects[object]->getGroupsCount(); ++i ) {

			// Aplicar materiales
			if( i < model->getMaterials()->size() )
			{
				TMaterial * m = model->getMaterials()->at( i );
				tech_name = m->name;

				if (m->main_texture != NULL)
				{
					g_App.effect->SetTexture( "Diffuse", m->main_texture );
					g_App.effect->SetTexture( "Mask", m->mask );
					g_App.effect->SetTexture( "Normal", m->normalmap );
					g_App.effect->SetTexture( "Emissive", m->emissive );
					assert(hr == D3D_OK);
				}
			}
				
			if (_curr_tech != tech_name)
			{
				_curr_tech = tech_name;
				hr = g_App.effect->SetTechnique(_curr_tech.c_str());
				assert(hr == D3D_OK);
			}
			g_App.effect->CommitChanges();
			UINT npasses;
			hr = g_App.effect->Begin(&npasses,0);
			assert(hr == D3D_OK);
			for (UINT pass = 0; pass < npasses; ++pass) {
				g_App.effect->BeginPass(pass);
				model->getCModel()->renderSubGroupWithShader( i, object );
				g_App.effect->EndPass();
			}
			g_App.effect->End();

		}
	}
	g_App.GetDevice()->SetTexture(0,NULL);
	g_App.GetDevice()->SetTransform( D3DTS_WORLD, &d3dxidentity );
}

void RendererSystem::renderModel(ModelComponent* model, bool detail_low)
{
	TMesh* mesh = model->getMesh();
	if (model->getMeshLow() != NULL && detail_low) mesh = model->getMeshLow();

	// Para todos los grupos de la mesh
	string tech_name = model->getMaterials()->at(0)->name;
	g_App.effect->SetFloatArray( "DiffuseColor", model->diffuseColor , 4);
	for( size_t i=0; i<mesh->getGroupsCount(); ++i ) {
		// Aplicar materiales

		if( i < model->getMaterials()->size() )
		{
			const TMaterial * m = model->getMaterials()->at( i );
			tech_name = m->name;

			g_App.effect->SetTexture( "Diffuse", m->diffuse );
			g_App.effect->SetTexture( "Diffuse2", m->diffuse2 );
			g_App.effect->SetTexture( "Lightmap", m->lightmap );
			g_App.effect->SetTexture( "Mask", m->mask );
			g_App.effect->SetTexture( "Normal", m->normalmap );
			g_App.effect->SetTexture( "Emissive", m->emissive );
		}

		if (_curr_tech != tech_name)
		{
			_curr_tech = tech_name;
			hr = g_App.effect->SetTechnique(_curr_tech.c_str());
			assert(hr == D3D_OK);
		}
		g_App.effect->CommitChanges();
		UINT npasses;
		g_App.effect->Begin(&npasses,0);
		for (UINT pass = 0; pass < npasses; ++pass) {
			g_App.effect->BeginPass(pass);
			mesh->renderGroup( i );
			g_App.effect->EndPass();
		}
		g_App.effect->End();
	}
	g_App.GetDevice()->SetTexture(0,NULL);
	g_App.GetDevice()->SetTransform( D3DTS_WORLD, &d3dxidentity );
}

void RendererSystem::renderForward(void)
{
	//setRenderTarget(0, _rt_composed);

	g_App.GetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	g_App.GetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);
	g_App.GetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	g_App.GetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	g_App.GetDevice()->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
	g_App.GetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	g_App.GetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	g_App.GetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_App.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	std::map<Entity*,Component*>* entities = EntityManager::get().getAllEntitiesPosessingComponent<ModelComponent>();
	std::map<Entity*,Component*>::iterator iter;

	for (iter = entities->begin(); iter != entities->end(); ++iter)
	{
		Entity* entity = iter->first;
		if (entity->enabled == false) continue;

		ModelComponent * model = EntityManager::get().getComponent<ModelComponent>(entity);
		if (model->enabled == false) continue;

		TransformComponent * tComponent = EntityManager::get().getComponent<TransformComponent>(entity);

		btTransform * world = tComponent->getWorldTransform();
		D3DXMATRIX d3dxMatrix;
		convertBulletTransform( world, d3dxMatrix);
		TAABB aabb = model->getMesh()->aabb;
		aabb = aabb.getRotatedBy(d3dxMatrix);
		if (currentCamera->frustum.isInside(aabb) == TFrustum::OUT_OF_FRUSTUM) continue;
		
		// Inside Frustum, render
		applyRenderFlags(model);

		string tech_name = "tech_fwd_basic";
		
		g_App.effect->SetMatrix( "World", &d3dxMatrix );
		//if (_curr_tech != tech_name)
		//{
		//	_curr_tech = tech_name;
		//	hr = g_App.effect->SetTechnique(_curr_tech.c_str());
		//	assert(hr == D3D_OK);
		//}

		if (model->getCModel() != NULL) 
		{
			//if (model->entity->type == "ENEMY" && w->_specialVision) return;
			for( size_t object=0; object<model->getCModel()->getCore()->_objects.size(); ++object ) {
				// Para todos los grupos de la mesh
				for( size_t i=0; i<model->getCModel()->getCore()->_objects[object]->getGroupsCount(); ++i ) {
					string tech_name = "tech_fwd_skin";

					// Aplicar materiales
					if( i < model->getMaterials()->size() )
					{
						TMaterial * m = model->getMaterials()->at( i );
						//tech_name = m->name;

						if (m->main_texture != NULL)
						{
							hr = g_App.effect->SetTexture( "Diffuse", m->main_texture );
							assert(hr == D3D_OK);
							hr = g_App.effect->SetTexture( "Normal", m->normalmap );
							assert(hr == D3D_OK);
						}
					}
					if (_curr_tech != tech_name)
					{
						_curr_tech = tech_name;
						hr = g_App.effect->SetTechnique(_curr_tech.c_str());
						assert(hr == D3D_OK);
					}
					g_App.effect->CommitChanges();
					UINT npasses;
					g_App.effect->Begin(&npasses,0);
					for (UINT pass = 0; pass < npasses; ++pass) {
						g_App.effect->BeginPass(pass);
						model->getCModel()->renderSubGroupWithShader( i, object );
						g_App.effect->EndPass();
					}
					g_App.effect->End();
				}
			}
		}

		else
		{
			// Para todos los grupos de la mesh
			for( size_t i=0; i<model->getMesh()->getGroupsCount(); ++i ) {
				// Aplicar materiales

				string tech_name = "tech_fwd_basic";
				if( i < model->getMaterials()->size() )
				{
					//tech_name = model->getMaterials()->at(i)->name;
					const TMaterial * m = model->getMaterials()->at( i );
					if (m->name == "tech_lightmap") tech_name = "tech_fwd_lightmap";

					hr = g_App.effect->SetTexture( "Diffuse", m->diffuse );
					assert(hr == D3D_OK);
					hr = g_App.effect->SetTexture( "Diffuse2", m->diffuse2 );
					assert(hr == D3D_OK);
					hr = g_App.effect->SetTexture( "Lightmap", m->lightmap );
					assert(hr == D3D_OK);
					hr = g_App.effect->SetTexture( "Mask", m->mask );
					assert(hr == D3D_OK);
					hr = g_App.effect->SetTexture( "Normal", m->normalmap );
					assert(hr == D3D_OK);
					hr = g_App.effect->SetTexture( "Emissive", m->emissive );
					assert(hr == D3D_OK);
				}
				if (_curr_tech != tech_name)
				{
					_curr_tech = tech_name;
					hr = g_App.effect->SetTechnique(_curr_tech.c_str());
					assert(hr == D3D_OK);
				}
				g_App.effect->CommitChanges();
				UINT npasses;
				g_App.effect->Begin(&npasses,0);
				for (UINT pass = 0; pass < npasses; ++pass) {
					g_App.effect->BeginPass(pass);
					model->getMesh()->renderGroup( i );
					g_App.effect->EndPass();
				}
				g_App.effect->End();
			}
		}

		restoreRenderFlags(model);
		g_App.GetDevice()->SetTexture(0,NULL);
		DebugSystem::get().painted_objects++;
	}

	g_App.GetDevice()->SetTransform( D3DTS_WORLD, &d3dxidentity );
}

void RendererSystem::renderFinal(void)
{
	//resolveRT();

	g_App.effect->SetTexture("FinalRT",_rt_composed);

	if (_curr_tech != "tech_final")
	{
		_curr_tech = "tech_final";
		g_App.effect->SetTechnique(_curr_tech.c_str());
	}
	g_App.effect->CommitChanges();
	int iNumPasses;
	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetWidth(), g_App.GetHeight());
		g_App.effect->EndPass();
	}
	g_App.effect->End();
	//resolveRT();
}

void RendererSystem::renderVignetting(void)
{
	setRenderTarget(0, _rt_composed);

	g_App.effect->SetTexture("FinalRT",_rt_composed);

	if (_curr_tech != "tech_vignetting")
	{
		_curr_tech = "tech_vignetting";
		g_App.effect->SetTechnique(_curr_tech.c_str());
	}
	g_App.effect->CommitChanges();
	int iNumPasses;
	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->CommitChanges();
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetWidth(), g_App.GetHeight());
		g_App.effect->EndPass();
	}
	g_App.effect->End();
	resolveRT();
}

void RendererSystem::renderFXDoF(void)
{
	setRenderTarget(0, _rt_low);

	g_App.effect->SetFloat( "RTWidth", 1.0f / g_App.GetParameters().BackBufferWidth );
	g_App.effect->SetFloat( "RTHeight", 1.0f / g_App.GetParameters().BackBufferHeight );
	g_App.effect->SetTexture("FinalRT",_rt_composed);

	if (_curr_tech != "tech_blur_hor")
	{
		_curr_tech = "tech_blur_hor";
		g_App.effect->SetTechnique(_curr_tech.c_str());
	}
	//g_App.effect->SetFloat("BlurSize",0.001f);
	g_App.effect->CommitChanges();

	int iNumPasses;
	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetParameters().BackBufferWidth, g_App.GetParameters().BackBufferHeight);
		g_App.effect->EndPass();
	}
	g_App.effect->End();

	if (_curr_tech != "tech_blur_ver")
	{
		_curr_tech = "tech_blur_ver";
		g_App.effect->SetTechnique(_curr_tech.c_str());
	}

	setRenderTarget(0, _rt_blur);
	g_App.effect->SetTexture("FinalRT",_rt_low);
	g_App.effect->CommitChanges();

	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetParameters().BackBufferWidth, g_App.GetParameters().BackBufferHeight);
		g_App.effect->EndPass();
	}
	g_App.effect->End();

	setRenderTarget(0, _rt_composed);

	g_App.effect->SetTexture("FinalRT",_rt_composed);
	g_App.effect->SetTexture("BlurRT",_rt_blur);

	if (_curr_tech != "tech_dof")
	{
		_curr_tech = "tech_dof";
		g_App.effect->SetTechnique(_curr_tech.c_str());
	}

	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->CommitChanges();
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetWidth(), g_App.GetHeight());
		g_App.effect->EndPass();
	}
	g_App.effect->End();

	resolveRT();

	if(DebugSystem::get().draw_mode == DebugSystem::DRAW_MODE::RT)
	{
		drawSprite(D3DXVECTOR2(g_App.GetWidth()/2,g_App.GetHeight()/2),D3DXVECTOR2((g_App.GetWidth()/3) * 2,g_App.GetHeight()/3) * 1, _rt_composed, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
	}
}

void RendererSystem::renderFXBloom(void)
{
	setRenderTarget(0, _rt_bloom);

	g_App.effect->SetTexture("FinalRT",_rt_composed);

	if (_curr_tech != "tech_bloom")
	{
		_curr_tech = "tech_bloom";
		hr = g_App.effect->SetTechnique(_curr_tech.c_str());
		assert(hr == D3D_OK);
	}
	int iNumPasses;
	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->CommitChanges();
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetWidth(), g_App.GetHeight());
		g_App.effect->EndPass();
	}
	g_App.effect->End();
	
	setRenderTarget(0, _rt_blur);
	g_App.effect->SetTexture("FinalRT",_rt_bloom);

	if (_curr_tech != "tech_blur_hor")
	{
		_curr_tech = "tech_blur_hor";
		hr = g_App.effect->SetTechnique(_curr_tech.c_str());
		assert(hr == D3D_OK);
	}
	//g_App.effect->SetFloat("BlurSize",0.002f);
	//g_App.effect->SetFloat("SimpleBlurSize",0.001f);

	g_App.effect->CommitChanges();

	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetWidth(), g_App.GetHeight());
		g_App.effect->EndPass();
	}
	g_App.effect->End();

	if (_curr_tech != "tech_blur_ver")
	{
		_curr_tech = "tech_blur_ver";
		hr = g_App.effect->SetTechnique(_curr_tech.c_str());
		assert(hr == D3D_OK);
	}
	setRenderTarget(0, _rt_composed_alt);
	g_App.effect->SetTexture("FinalRT",_rt_blur);
	g_App.effect->CommitChanges();

	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetWidth(), g_App.GetHeight());
		g_App.effect->EndPass();
	}
	g_App.effect->End();

	if (_curr_tech != "tech_simple_blur")
	{
		_curr_tech = "tech_simple_blur";
		hr = g_App.effect->SetTechnique(_curr_tech.c_str());
		assert(hr == D3D_OK);
	}
	setRenderTarget(0, _rt_bloom);
	g_App.effect->SetTexture("FinalRT",_rt_composed_alt);
	g_App.effect->CommitChanges();

	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetWidth(), g_App.GetHeight());
		g_App.effect->EndPass();
	}
	g_App.effect->End();

	setRenderTarget(0, _rt_composed);

	g_App.effect->SetTexture("FinalRT",_rt_composed);
	g_App.effect->SetTexture("BloomRT",_rt_bloom);

	if (_curr_tech != "tech_bloom_composed")
	{
		_curr_tech = "tech_bloom_composed";
		hr = g_App.effect->SetTechnique(_curr_tech.c_str());
		assert(hr == D3D_OK);
	}
	g_App.effect->CommitChanges();

	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetWidth(), g_App.GetHeight());
		g_App.effect->EndPass();
	}
	g_App.effect->End();

	resolveRT();

	if(DebugSystem::get().draw_mode == DebugSystem::DRAW_MODE::RT)
	{
		drawSprite(D3DXVECTOR2(g_App.GetWidth()/2,g_App.GetHeight()/2),D3DXVECTOR2((g_App.GetWidth()/3) * 2,(g_App.GetHeight()/3) * 2), _rt_composed, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
	}
}

void RendererSystem::renderOutline(void)
{
	setRenderTarget(0, _rt_composed);
	g_App.effect->SetTexture("FinalRT",_rt_composed);

	if (_curr_tech != "tech_outline")
	{
		_curr_tech = "tech_outline";
		hr = g_App.effect->SetTechnique(_curr_tech.c_str());
		assert(hr == D3D_OK);
	}
	int iNumPasses;
	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->CommitChanges();
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetWidth(), g_App.GetHeight());
		g_App.effect->EndPass();
	}
	g_App.effect->End();
	resolveRT();

	if(DebugSystem::get().draw_mode == DebugSystem::DRAW_MODE::RT)
	{
		drawSprite(D3DXVECTOR2(g_App.GetWidth()/2,g_App.GetHeight()/2),D3DXVECTOR2((g_App.GetWidth()/3) * 0,(g_App.GetHeight()/3) * 2), _rt_composed, D3DXVECTOR2(1/3.0f,1/3.0f), D3DCOLOR_RGBA(255,255,255,255));
	}
}

void RendererSystem::renderImageEffects(float contrast, float brightness, D3DXVECTOR3 saturation)
{
	setRenderTarget(0, _rt_composed);

	g_App.effect->SetTexture("FinalRT",_rt_composed);
	//g_App.effect->SetFloat("Contrast", contrast);
	//g_App.effect->SetFloat("Brightness", brightness);
	//g_App.effect->SetFloatArray("Saturation", saturation,3 );

	if (_curr_tech != "tech_image_effects")
	{
		_curr_tech = "tech_image_effects";
		hr = g_App.effect->SetTechnique(_curr_tech.c_str());
		assert(hr == D3D_OK);
	}
	int iNumPasses;
	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->CommitChanges();
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetWidth(), g_App.GetHeight());
		g_App.effect->EndPass();
	}
	g_App.effect->End();
	resolveRT();
}

void RendererSystem::renderFXWiggle(void)
{
	copyBuffers(_rt_composed,_rt_composed_alt);
	g_App.effect->SetTexture("FinalRT",_rt_composed_alt);
	if (_curr_tech != "tech_wiggle")
	{
		_curr_tech = "tech_wiggle";
		hr = g_App.effect->SetTechnique(_curr_tech.c_str());
		assert(hr == D3D_OK);
	}
	setRenderTarget(0, _rt_composed);
	g_App.effect->CommitChanges();

	int iNumPasses;
	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetWidth(), g_App.GetHeight());
		g_App.effect->EndPass();
	}
	g_App.effect->End();
	resolveRT();
}

void RendererSystem::renderSkyBox(void)
{
	setRenderTarget(0, _rt_composed);

	D3DXMATRIX matrix;
	D3DXMatrixIdentity(&matrix);
	D3DXMATRIX mScale;
	D3DXMatrixScaling(&mScale,1, 1, 1);
	D3DXMATRIX mTranslation;
	D3DXVECTOR3 camPos = currentCamera->getPosition();
	D3DXMatrixTranslation(&mTranslation,camPos.x, camPos.y, camPos.z);
	D3DXMatrixMultiply(&matrix, &mScale, &mTranslation);

	g_App.effect->SetTexture("SkyBoxTexture",_texture_skyBox);
	g_App.effect->SetMatrix("World",&matrix);
	//g_App.effect->SetTexture("FinalRT",_rt_composed);
	if (_curr_tech != "tech_skybox")
	{
		_curr_tech = "tech_skybox";
		hr = g_App.effect->SetTechnique(_curr_tech.c_str());
		assert(hr == D3D_OK);
	}
	int iNumPasses;
	g_App.effect->CommitChanges();
	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->BeginPass(iPassIdx);
		_mesh_skyBox->DrawSubset(0);
		g_App.effect->EndPass();
	}
	g_App.effect->End();
	resolveRT();
}

//void RendererSystem::renderRTs(void)
//{
//	//g_App.GetDevice()->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0 );
//
//	float width = g_App.GetWidth();
//	float height = g_App.GetHeight();
//
//	// 1024 / 3 = 341
//
//	//float low_scale_x = width/512.0f;
//	//float low_scale_y = height/512.0f;
//
//	// 3 Rows - 3 columns
//	// Row 1
//
//
//	// Row 2
//	drawSprite(D3DXVECTOR2(width/2,height/2),D3DXVECTOR2((width/3) * 0,(height/3) * 1), _rt_lights, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
//	
//		
//	// Row 3
//	drawSprite(D3DXVECTOR2(width/2,height/2),D3DXVECTOR2((width/3) * 1,(height/3) * 2), gStereoParamTexture, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
//	drawSprite(D3DXVECTOR2(width/2,height/2),D3DXVECTOR2((width/3) * 2,(height/3) * 2), _rt_composed, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
//}

//void RendererSystem::renderDebug(void)
//{
//	#if defined ALMUVA_DBG
//		//DijkstraGraph::get().renderGraph(D3DCOLOR_ARGB(255, 255,255,0), D3DCOLOR_ARGB(255, 0,255,0));
//		EntityManager::get().getComponent<AutomatComponent>(World::instance()->getPlayer())->render(); //Para debug, pintar el estado del automata del player
//		BTSystem::get().render(); //Debug enemigos
//	#endif
//	if(w->isDebugModeOn() == false) return;
//	if(w->_DEBUG_OPTIONS.RENDER_GRAPH) DijkstraGraph::get().renderGraph(D3DCOLOR_ARGB(255, 255,255,0), D3DCOLOR_ARGB(255, 0,255,0));
//
//	// Render the different targets for debug purposes
//	if (w->_DEBUG_OPTIONS.DRAW_RT)
//	{
//		g_App.GetDevice()->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0 );
//
//		float width = g_App.GetWidth();
//		float height = g_App.GetHeight();
//
//		// 1024 / 3 = 341
//
//		//float low_scale_x = width/512.0f;
//		//float low_scale_y = height/512.0f;
//
//		// 3 Rows - 3 columns
//		// Row 1
//		drawSprite(D3DXVECTOR2(width/2,height/2),D3DXVECTOR2((width/3) * 0,0), _rt_color, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
//		drawSprite(D3DXVECTOR2(width/2,height/2),D3DXVECTOR2((width/3) * 1,0), _rt_normal, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
//		drawSprite(D3DXVECTOR2(width/2,height/2),D3DXVECTOR2((width/3) * 2,0), _rt_depth, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
//
//		// Row 2
//		drawSprite(D3DXVECTOR2(width/2,height/2),D3DXVECTOR2((width/3) * 0,(height/3) * 1), _rt_lights, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
//		drawSprite(D3DXVECTOR2(width/2,height/2),D3DXVECTOR2((width/3) * 1,(height/3) * 1), _rt_blur, D3DXVECTOR2(1/3.0f,1/3.0f), D3DCOLOR_RGBA(255,255,255,255));
//		drawSprite(D3DXVECTOR2(width/2,height/2),D3DXVECTOR2((width/3) * 2,(height/3) * 1), _rt_bloom, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
//		
//		// Row 3
//		drawSprite(D3DXVECTOR2(width/2,height/2),D3DXVECTOR2((width/3) * 0,(height/3) * 2), _rt_low, D3DXVECTOR2(1/3.0f,1/3.0f), D3DCOLOR_RGBA(255,255,255,255));
//		drawSprite(D3DXVECTOR2(width/2,height/2),D3DXVECTOR2((width/3) * 1,(height/3) * 2), _rt_composed_alt, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
//		drawSprite(D3DXVECTOR2(width/2,height/2),D3DXVECTOR2((width/3) * 2,(height/3) * 2), _rt_composed, D3DXVECTOR2(1.0f/3.0f,1.0f/3.0f), D3DCOLOR_RGBA(255,255,255,255));
//	}
//}



void RendererSystem::renderLights(void)
{
	setRenderTarget(0, _rt_lights);

	std::map<Entity*,Component*>* shadows = EntityManager::get().getAllEntitiesPosessingComponent<ShadowComponent>();
	if (shadows != NULL)
	{
		std::map<Entity*,Component*>::iterator iter2;
		for (iter2 = shadows->begin(); iter2 != shadows->end(); ++iter2)
		{
			Entity* shadow = iter2->first;
			ShadowComponent* l = EntityManager::get().getComponent<ShadowComponent>(shadow);
			if (l->enabled == false) continue;
			TransformComponent* t = EntityManager::get().getComponent<TransformComponent>(shadow);
			renderShadowOrb(t,l);
		}
	}

	std::map<Entity*,Component*>* blobs = EntityManager::get().getAllEntitiesPosessingComponent<LightComponent>();
	if (blobs != NULL)
	{
		std::map<Entity*,Component*>::iterator iter;
		for (iter = blobs->begin(); iter != blobs->end(); ++iter)
		{
			Entity* light = iter->first;
			LightComponent* l = EntityManager::get().getComponent<LightComponent>(light);
			if (l->enabled == false) continue;
			TransformComponent* t = EntityManager::get().getComponent<TransformComponent>(light);

			switch (l->light_type)
			{
			case LIGHT_DIRECTIONAL:
				//g_App.effect->SetFloatArray("LightDir",l->light_direction,3);
				//renderDirectionalLight(t,l);
				break;
			case LIGHT_POINT:
				//renderPointLight(t,l);
				break;
			case LIGHT_SPOT:
				//renderSpotLight(t,l);
				break;
			case LIGHT_BLOB:
				renderBlob(t,l);
				break;
			}

		}
	}

	std::map<Entity*,Component*>* lights = EntityManager::get().getAllEntitiesPosessingComponent<LightComponent>();
	if (lights != NULL)
	{
		std::map<Entity*,Component*>::iterator iter;
		for (iter = lights->begin(); iter != lights->end(); ++iter)
		{
			Entity* light = iter->first;
			LightComponent* l = EntityManager::get().getComponent<LightComponent>(light);
			if (l->enabled == false) continue;
			TransformComponent* t = EntityManager::get().getComponent<TransformComponent>(light);

			switch (l->light_type)
			{
			case LIGHT_DIRECTIONAL:
				g_App.effect->SetFloatArray("LightDir",l->light_direction,3);
				//renderDirectionalLight(t,l);
				break;
			case LIGHT_POINT:
				renderPointLight(t,l);
				break;
			case LIGHT_SPOT:
				//renderSpotLight(t,l);
				break;
			case LIGHT_BLOB:
				//renderBlob(t,l);
				break;
			}

		}
	}

	resolveRT();
}

void RendererSystem::renderShadowOrb(TransformComponent* t, ShadowComponent * l)
{
	g_App.GetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MIN);
	D3DXVECTOR3 pos = t->getWorldTransform()->getOrigin();

	if (currentCamera->frustum.isInside(TAABB(pos,D3DXVECTOR3(l->radius,l->radius,l->radius))) == TFrustum::OUT_OF_FRUSTUM) 
	{
		g_App.GetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		return;
	}
	
	D3DXMATRIX sphereMatrix;
	D3DXMatrixIdentity(&sphereMatrix);
	D3DXMATRIX sphereMatrixScale;
	D3DXMatrixScaling(&sphereMatrixScale,l->radius, l->radius, l->radius);
	D3DXMATRIX sphereMatrixTranslation;
	D3DXMatrixTranslation(&sphereMatrixTranslation,pos.x, pos.y, pos.z);
	sphereMatrix = sphereMatrixScale * sphereMatrixTranslation;

	g_App.effect->SetMatrix("World",&sphereMatrix);
	g_App.effect->SetFloatArray("LightPosition",pos,3);
	g_App.effect->SetFloat("LightRadius",l->radius);
	g_App.effect->SetTexture("Diffuse",_texture_shadow_creation);

	if (_curr_tech != "tech_decal")
	{
		_curr_tech = "tech_decal";
		hr = g_App.effect->SetTechnique(_curr_tech.c_str());
		assert(hr == D3D_OK);
	}

	int iNumPasses;
	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->CommitChanges();
		g_App.effect->BeginPass(iPassIdx);
		_mesh_sphere->DrawSubset(0);
		g_App.effect->EndPass();
	}
	g_App.effect->End();
	
	g_App.GetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
}

void RendererSystem::renderBlob(TransformComponent* t, LightComponent * l)
{
	g_App.GetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MIN);
	D3DXVECTOR3 pos = t->getWorldTransform()->getOrigin();

	if (currentCamera->frustum.isInside(TAABB(pos,D3DXVECTOR3(l->getRadius(),l->getRadius(),l->getRadius()))) == TFrustum::OUT_OF_FRUSTUM) 
	{
		g_App.GetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		return;
	}
	
	D3DXMATRIX sphereMatrix;
	D3DXMatrixIdentity(&sphereMatrix);
	D3DXMATRIX sphereMatrixScale;
	D3DXMatrixScaling(&sphereMatrixScale,l->getRadius(), l->getRadius()/4, l->getRadius());
	D3DXMATRIX sphereMatrixTranslation;
	D3DXMatrixTranslation(&sphereMatrixTranslation,pos.x, pos.y, pos.z);
	sphereMatrix = sphereMatrixScale * sphereMatrixTranslation;

	g_App.effect->SetMatrix("World",&sphereMatrix);
	g_App.effect->SetFloatArray("LightColor",l->light_color,3);
	g_App.effect->SetFloatArray("LightPosition",pos,3);
	g_App.effect->SetFloat("LightRadius",l->getRadius());
	g_App.effect->SetTexture("Diffuse",_texture_shadow_creation);

	if (_curr_tech != "tech_blob")
	{
		_curr_tech = "tech_blob";
		hr = g_App.effect->SetTechnique(_curr_tech.c_str());
		assert(hr == D3D_OK);
	}

	int iNumPasses;
	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->CommitChanges();
		g_App.effect->BeginPass(iPassIdx);
		_mesh_sphere->DrawSubset(0);
		g_App.effect->EndPass();
	}
	g_App.effect->End();
	
	g_App.GetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
}

//
//void RendererSystem::renderSpotLight(TransformComponent* t, LightComponent * l)
//{
//	g_App.GetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MIN);
//
//	D3DXMATRIX sphereMatrix;
//	D3DXMatrixIdentity(&sphereMatrix);
//	D3DXMATRIX sphereMatrixScale;
//	D3DXMatrixScaling(&sphereMatrixScale,l->getRadius(), l->getRadius(), l->getRadius());
//	D3DXMATRIX sphereMatrixTranslation;
//	D3DXVECTOR3 pos = t->getPosition();
//	D3DXMatrixTranslation(&sphereMatrixTranslation,pos.x, pos.y, pos.z);
//	sphereMatrix = sphereMatrixScale * sphereMatrixTranslation;
//
//	if (currentCamera->frustum.isInside(TAABB(pos,D3DXVECTOR3(l->getRadius(),l->getRadius(),l->getRadius()))) == TFrustum::OUT_OF_FRUSTUM) 
//	{
//		g_App.GetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
//		return;
//	}
//
//	g_App.effect->SetMatrix("World",&sphereMatrix);
//	g_App.effect->SetFloatArray("LightPosition",pos,3);
//	g_App.effect->SetFloat("LightRadius",l->getRadius());
//
//	g_App.effect->SetTechnique("tech_decal");
//
//	LPD3DXMESH pMesh;
//	D3DXCreateSphere(g_App.GetDevice(), l->getRadius(), 20, 20, &pMesh, NULL);
//	int iNumPasses;
//	g_App.effect->Begin((UINT*)&iNumPasses,0);
//	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
//	{
//		g_App.effect->CommitChanges();
//		g_App.effect->BeginPass(iPassIdx);
//		pMesh->DrawSubset(0);
//		g_App.effect->EndPass();
//	}
//	g_App.effect->End();
//	pMesh->Release();
//	
//	g_App.GetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
//
//}

//void RendererSystem::renderDirectionalLight(TransformComponent* t, LightComponent * l)
//{
//	HRESULT hr;
//
//	g_App.effect->SetFloatArray("LightDir",l->light_direction,3);
//	g_App.effect->SetFloatArray("LightColor",l->light_color,3);
//	g_App.effect->SetFloat("LightIntensity",l->light_intensity);
//
//	if (_curr_tech != "tech_lights")
//	{
//		_curr_tech = "tech_lights";
//		hr = g_App.effect->SetTechnique(_curr_tech.c_str());
//		assert(hr == D3D_OK);
//	}
//
//	int iNumPasses;
//	g_App.effect->Begin((UINT*)&iNumPasses,0);
//	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
//	{
//		g_App.effect->CommitChanges();
//		g_App.effect->BeginPass(iPassIdx);
//		drawScreenPlane(g_App.GetWidth(), g_App.GetHeight());
//		g_App.effect->EndPass();
//	}
//	g_App.effect->End();
//}

void RendererSystem::renderPointLight(TransformComponent* t, LightComponent * l)
{
	D3DXMATRIX sphereMatrix;
	D3DXMatrixIdentity(&sphereMatrix);
	D3DXMATRIX sphereMatrixScale;
	D3DXMatrixScaling(&sphereMatrixScale,l->getRadius(), l->getRadius(), l->getRadius());
	D3DXMATRIX sphereMatrixTranslation;
	D3DXVECTOR3 pos = t->getWorldTransform()->getOrigin();
	D3DXMatrixTranslation(&sphereMatrixTranslation,pos.x, pos.y, pos.z);
	sphereMatrix = sphereMatrixScale * sphereMatrixTranslation;

	if (currentCamera->frustum.isInside(TAABB(pos,D3DXVECTOR3(l->getRadius(),l->getRadius(),l->getRadius()))) == TFrustum::OUT_OF_FRUSTUM) return;

	g_App.effect->SetMatrix("World",&sphereMatrix);
	D3DXMATRIX wvp;
	D3DXMATRIX invwvp;
	D3DXMatrixMultiply( &wvp, &sphereMatrix, &currentCamera->getViewProjection() );
	D3DXMatrixInverse(&invwvp,NULL,&wvp);
	hr = g_App.effect->SetMatrix("InvertWVP",&invwvp);
	assert( hr == D3D_OK );
	//float random = (float)rand()/(float)RAND_MAX;
	float new_radius = l->getRadius();
	float new_intensity = l->light_intensity;
	if (l->isTorch)
	{
		float s = sin((w->world_time+t->entity->eid)*5);
		new_radius = l->getRadius() + s*0.03f ;
		new_intensity = l->light_intensity - (1-s)*0.1f;
	}

	g_App.effect->SetFloatArray("LightPosition",t->getPosition(),3);
	g_App.effect->SetFloatArray("LightColor",l->light_color,3);
	g_App.effect->SetFloat("LightRadius",new_radius);
	g_App.effect->SetFloat("LightIntensity",new_intensity);

	if (_curr_tech != "tech_pointlights")
	{
		_curr_tech = "tech_pointlights";
		g_App.effect->SetTechnique(_curr_tech.c_str());
	}

	//calculate the distance between the camera and light center
	//D3DXVECTOR3 dist = D3DXVECTOR3(w->getCurrentCamera()->getPosition()) - position;
	//float cameraToCenter = D3DXVec3Length(&dist);
	//if we are inside the light volume, draw the sphere's inside face
	//if (cameraToCenter < radius_test + w->getCurrentCamera()->getZNear())
	//	g_App.GetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	//else
	//	g_App.GetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	int iNumPasses;
	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->CommitChanges();
		g_App.effect->BeginPass(iPassIdx);
		_mesh_sphere->DrawSubset(0);
		g_App.effect->EndPass();
	}
	g_App.effect->End();
}

void RendererSystem::renderComposed(void)
{
	setRenderTarget(0, _rt_composed);

	/*hr = g_App.effect->SetFloatArray("LightAmbient",D3DXVECTOR3(0,0,0),3);
	assert(hr == D3D_OK);*/
	hr = g_App.effect->SetTexture("ColorRT",_rt_color);
	assert(hr == D3D_OK);
	hr = g_App.effect->SetTexture("LightRT",_rt_lights);
	assert(hr == D3D_OK);
	hr = g_App.effect->SetTexture("NormalRT",_rt_normal);
	assert(hr == D3D_OK);
	if (_curr_tech != "tech_composed")
	{
		_curr_tech = "tech_composed";
		hr = g_App.effect->SetTechnique(_curr_tech.c_str());
		assert(hr == D3D_OK);
	}
	int iNumPasses;
	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->CommitChanges();
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetWidth(), g_App.GetHeight());
		g_App.effect->EndPass();
	}
	g_App.effect->End();

	resolveRT();
}

void RendererSystem::renderParticleEffects()
{
	if (DebugSystem::get().is_particles_active == false) return;
	setRenderTarget(0, _rt_composed);
	std::map<Entity*,Component*>* entities = EntityManager::get().getAllEntitiesPosessingComponent<ParticleEffectComponent>();
	if (entities == NULL) return;
	std::map<Entity*,Component*>::iterator iter;

	for (iter = entities->begin(); iter != entities->end(); ++iter)
	{
		Entity* entity = iter->first;
		if (entity->enabled == false) continue;

		ParticleEffectComponent * particles = EntityManager::get().getComponent<ParticleEffectComponent>(entity);
		if (particles->enabled == false) continue;
		if (particles->ignore_outline == false) continue;
		TransformComponent * tComponent = EntityManager::get().getComponent<TransformComponent>(entity);
		if (particles->aabb_radius > 0) 
		{
			if (currentCamera->frustum.isInside(particles->aabb) == TFrustum::OUT_OF_FRUSTUM) continue;
			if (currentCamera->getPosition().distance(tComponent->getPosition()) > 19.5f) continue;
		}

		if (_curr_tech != particles->tech_name)
		{
			_curr_tech = particles->tech_name;
			hr = g_App.effect->SetTechnique(_curr_tech.c_str());
			assert(hr == D3D_OK);
		}

		if (particles->additive) g_App.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		else g_App.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		g_App.effect->SetTexture("Diffuse",particles->texture);
		g_App.effect->SetInt("ParticleNumFrames", particles->num_frames);

		btTransform * world = tComponent->getWorldTransform();
		D3DXMATRIX d3dxMatrix;
		convertBulletTransform( world, d3dxMatrix);
		g_App.effect->SetMatrix("World",&d3dxMatrix);
		g_App.effect->CommitChanges();

		int iNumPasses;
		g_App.effect->Begin((UINT*)&iNumPasses,0);
		for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
		{
			g_App.effect->CommitChanges();
			g_App.effect->BeginPass(iPassIdx);
			particles->render();

			g_App.effect->EndPass();
		}
		g_App.effect->End();

	}
	g_App.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	resolveRT();
}

void RendererSystem::setFog(float start, float end)
{
	fogStart = start;
	fogEnd = end;
}

void RendererSystem::renderFog(void)
{
	setRenderTarget(0, _rt_composed);

	g_App.effect->SetTexture("FinalRT",_rt_composed);
	g_App.effect->SetFloat("FogDensity",0.04f);
	g_App.effect->SetFloat("FogStart",fogStart);
	g_App.effect->SetFloat("FogEnd",fogEnd);
	g_App.effect->SetFloatArray("FogColor",D3DXVECTOR3(0.1f,0.2f,0.25f),4);
	//g_App.effect->SetTexture("DepthRT",_rt_composed);
	if (_curr_tech != "tech_fog")
	{
		_curr_tech = "tech_fog";
		hr = g_App.effect->SetTechnique(_curr_tech.c_str());
		assert(hr == D3D_OK);
	}
	int iNumPasses;
	g_App.effect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_App.effect->CommitChanges();
		g_App.effect->BeginPass(iPassIdx);
		drawScreenPlane(g_App.GetWidth(), g_App.GetHeight());
		g_App.effect->EndPass();
	}
	g_App.effect->End();

	resolveRT();
}

void RendererSystem::renderUI(void)
{
	//Entity* player = w->getPlayer();
	//PlayerControllerComponent* p = EntityManager::get().getComponent<PlayerControllerComponent>(player);
	////drawSprite(D3DXVECTOR3(0,0,0),D3DXVECTOR3(g_App.GetWidth()*0.025f,g_App.GetHeight()*0.025f,0),_texture_hp_back,D3DCOLOR_RGBA(255,255,255,255));
	////drawSprite(D3DXVECTOR3(0,0,0),D3DXVECTOR3(g_App.GetWidth()*0.025f,g_App.GetHeight()*0.025f,0),_texture_hp_front,D3DXVECTOR4(0,0,_hp_width,_hp_height*(player_life/100)),D3DCOLOR_RGBA(255,255,255,255));
	//
	//DWORD color = D3DCOLOR_ARGB( 255, 255, 255, 200 );

	////if(EntityManager::get().getComponent<TransformComponent>(player)->getPosition().getZ() > 130)
	////{
	////	printf2DBig( g_App.GetWidth()*0.35f, g_App.GetHeight()*0.35f, color, "OBJECTIVE COMPLETE");
	////}
	//if(p->_life <= 0)
	//{
	//	printf2DBig( g_App.GetWidth()*0.35f, g_App.GetHeight()*0.35f, color, "GAME OVER");
	//}
}

void RendererSystem::takeScreenshot(bool rt)
{
	std::string thetime = currentDateTime();
	std::string filename = std::string("data/screenshots/") + thetime;
	std::string ext = ".png";

	std::string finalfilename = filename + ext;
	D3DXSaveTextureToFile(finalfilename.c_str() ,D3DXIFF_PNG,_rt_composed,NULL);

	if (rt)
	{
		finalfilename = filename + std::string("_c").c_str() + ext;
		D3DXSaveTextureToFile(finalfilename.c_str() ,D3DXIFF_PNG,_rt_color,NULL);
		finalfilename = filename + std::string("_n").c_str() + ext;
		D3DXSaveTextureToFile(finalfilename.c_str() ,D3DXIFF_PNG,_rt_normal,NULL);
		finalfilename = filename + std::string("_d").c_str() + ext;
		D3DXSaveTextureToFile(finalfilename.c_str() ,D3DXIFF_PNG,_rt_depth,NULL);
		finalfilename = filename + std::string("_l").c_str() + ext;
		D3DXSaveTextureToFile(finalfilename.c_str() ,D3DXIFF_PNG,_rt_lights,NULL);
	}
}

void RendererSystem::setCommonShaderParams(void)
{
	g_App.GetDevice()->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0 );
	g_App.GetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	g_App.GetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	g_App.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	g_App.GetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_App.GetDevice()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	g_App.GetDevice()->SetRenderState(D3DRS_ZENABLE, TRUE);
	g_App.GetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	g_App.GetDevice()->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);

	_curr_tech = "";
	currentCamera = &CameraSystem::get().getCurrentCamera();
	hr = g_App.effect->SetFloat( "ScreenWidth", 1.0f / (float) g_App.GetParameters().BackBufferWidth );
	assert( hr == D3D_OK );
	hr = g_App.effect->SetFloat( "ScreenHeight", 1.0f / (float) g_App.GetParameters().BackBufferHeight );
	assert( hr == D3D_OK );
	hr = g_App.effect->SetFloat( "WorldTime", w->world_time );
	assert( hr == D3D_OK );
	hr = g_App.effect->SetMatrix( "ViewProjection", &currentCamera->getViewProjection() );
	assert( hr == D3D_OK );
	hr = g_App.effect->SetMatrix( "Projection", &currentCamera->getProjection() );
	assert( hr == D3D_OK );
	hr = g_App.effect->SetFloat( "CameraNear", currentCamera->getZNear() );
	assert( hr == D3D_OK );
	hr = g_App.effect->SetFloat( "CameraFar", currentCamera->getZFar() );
	assert( hr == D3D_OK );
	hr = g_App.effect->SetFloat( "DistanceToTarget", currentCamera->getDistanceToTarget() );
	assert( hr == D3D_OK );
	D3DXVECTOR4 vxinv = D3DXVECTOR4();
	vxinv.x = 1.0f / ((float) g_App.GetParameters().BackBufferWidth / 2.0f);
	vxinv.y = 1.0f / ((float) g_App.GetParameters().BackBufferHeight / 2.0f);
	vxinv.z = -1;
	vxinv.w = -1;
	hr = g_App.effect->SetFloatArray( "VportXformInv", vxinv, 4 );
	assert( hr == D3D_OK );
	
	D3DXMATRIX invVP;
	D3DXMatrixInverse(&invVP,NULL,&currentCamera->getViewProjection());
	g_App.effect->SetMatrix("View",&currentCamera->getView());
	hr = g_App.effect->SetMatrix("InvertViewProjection",&invVP);
	assert( hr == D3D_OK );

	hr = g_App.effect->SetFloatArray("CameraPosition",currentCamera->getPosition(),3);
	assert( hr == D3D_OK );
	btVector3 camLeft;
	currentCamera->getLeft(camLeft);
	hr = g_App.effect->SetFloatArray("CameraLeft",camLeft,3);
	assert( hr == D3D_OK );
	btVector3 camUp;
	currentCamera->getUp(camUp);
	hr = g_App.effect->SetFloatArray("CameraUp",camUp,3);
	assert( hr == D3D_OK );

	btVector3 cam_front;
	currentCamera->getFront(cam_front);
	hr = g_App.effect->SetFloatArray("CameraViewDirection",cam_front,3);
	assert( hr == D3D_OK );
	hr = g_App.effect->SetTexture("ColorRT",_rt_color);
	assert( hr == D3D_OK );
	hr = g_App.effect->SetTexture("NormalRT",_rt_normal);
	assert( hr == D3D_OK );
	hr = g_App.effect->SetTexture("DepthRT",_rt_depth);
	assert( hr == D3D_OK );
	hr = g_App.effect->SetTexture("LightRT",_rt_lights);
	assert( hr == D3D_OK );
	hr = g_App.effect->SetTexture("Noise",_texture_noise);
	assert( hr == D3D_OK );
	Entity* player = w->getPlayer();
	if (player != NULL) 
	{
		PlayerControllerComponent* p = EntityManager::get().getComponent<PlayerControllerComponent>(player);
		hr = g_App.effect->SetFloat("PlayerLife",p->_life);
		assert( hr == D3D_OK );
	}
	if (gStereoTexMgr->IsStereoActive()) g_App.effect->SetTexture("Stereo",gStereoParamTexture);
	if (w->_specialVision) 
	{
		//g_App.effect->SetFloat("VigForce",0.5f);
		g_App.effect->SetFloat( "BloomIntensity", 3.0f );
	}
	else  
	{
		//g_App.effect->SetFloat("VigForce",1.0f);
		g_App.effect->SetFloat( "BloomIntensity", 1.2f );
	}

}

void RendererSystem::drawSprite(D3DXVECTOR3 pivot, D3DXVECTOR3 position, LPDIRECT3DTEXTURE9 texture, D3DCOLOR color)
{
	_sprite->Begin(D3DXSPRITE_ALPHABLEND);
	_sprite->Draw(texture,NULL, &pivot, &position,color);
	_sprite->End();
}

void RendererSystem::drawSprite(D3DXVECTOR2 spriteCentre, D3DXVECTOR2 position, LPDIRECT3DTEXTURE9 texture, D3DXVECTOR2 scale, D3DCOLOR color )
{
	_sprite->Begin(D3DXSPRITE_ALPHABLEND);

	D3DXMATRIX mat;
	float rotation=0;
	D3DXMatrixTransformation2D(&mat,NULL,0.0,&scale,&spriteCentre,rotation,&position);
	_sprite->SetTransform(&mat);

	_sprite->Draw(texture,NULL, NULL, NULL,color);
	_sprite->End();
}

void RendererSystem::drawSprite(LPDIRECT3DTEXTURE9 texture, const D3DXMATRIX &sM, D3DCOLOR color)
{
	g_App.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	g_App.GetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	_sprite->Begin(D3DXSPRITE_ALPHABLEND);
	_sprite->SetTransform(&sM);
	_sprite->Draw(texture,NULL, NULL, NULL,color);
	_sprite->End();

	g_App.GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	g_App.GetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}

HRESULT RendererSystem::CreateStereoParamTextureAndView()
{
    // This function creates a texture that is suitable to be stereoized by the driver.
    // Note that the parameters primarily come from nvstereo.h
    using nv::stereo::ParamTextureManagerD3D9;

    HRESULT hr = D3D_OK;
	hr = D3DXCreateTexture(g_App.GetDevice(), ParamTextureManagerD3D9::Parms::StereoTexWidth, ParamTextureManagerD3D9::Parms::StereoTexHeight,1, D3DUSAGE_DYNAMIC,ParamTextureManagerD3D9::Parms::StereoTexFormat, D3DPOOL_DEFAULT, &gStereoParamTexture);

    return hr;
}
