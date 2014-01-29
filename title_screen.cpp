#include "title_screen.h"
#include "d3ddefs.h"
#include "game_state_manager.h"
#include "iostatus.h"
#include "system_renderer.h"

TitleScreen::TitleScreen(void)
{
	_total_time = 0;
	_alpha_multiplier = 0;
	_start_pressed = false;
	//_start_pressed2 = true;
	//_controls_screen = false;

}


TitleScreen::~TitleScreen(void)
{
	delete _texture_title_wall;
	/*delete _texture_title_player;
	delete _texture_title_logo;
	delete _texture_title_press_start;
	delete _texture_title_dragon;*/
	//delete _texture_controls;

}

void TitleScreen::init(void)
{
	_texture_title_wall = TTextureManager::get().getTextureResized("hardcoded/logo", g_App.GetWidth(), g_App.GetHeight());
	//_texture_title_player = TTextureManager::get().getTextureResized("hardcoded/title_player", g_App.GetWidth(), g_App.GetHeight());
	//_texture_title_logo = TTextureManager::get().getTextureResized("hardcoded/title_logo", g_App.GetWidth(), g_App.GetHeight());
	//_texture_title_press_start = TTextureManager::get().getTextureResized("hardcoded/title_press_start", g_App.GetWidth(), g_App.GetHeight());

	float width = g_App.GetWidth()/8;
	float height = width;
	//_texture_title_dragon = TTextureManager::get().getTextureResized("hardcoded/title_dragon", width, height);

	//_texture_controls = TTextureManager::get().getTextureResized("hardcoded/controls", g_App.GetWidth(), g_App.GetHeight());

}

void TitleScreen::update(float delta)
{
	_total_time += delta;

	if (_start_pressed == false)
	{
		_alpha_multiplier += delta/3.0f;
		if (_alpha_multiplier >= 1) _alpha_multiplier = 1;
	}
	else
	{
		_alpha_multiplier -= delta/6.0f;
		if (_alpha_multiplier <= 0) 
		{
			_alpha_multiplier = 0;
			launchGame();
		}
	}

	if (CIOStatus::instance()->isPressed(CIOStatus::START) || CIOStatus::instance()->isPressed(CIOStatus::CREATE_SHADOW)) 
	{
		//if (_start_pressed) _start_pressed2 = true;
		_start_pressed = true;
	}
}

void TitleScreen::render(void)
{
	g_App.GetDevice()->SetTransform( D3DTS_WORLD, &d3dxidentity );

	g_App.GetDevice()->SetRenderState (D3DRS_ALPHABLENDENABLE, TRUE);
	g_App.GetDevice()->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	g_App.GetDevice()->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
	g_App.GetDevice()->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);

	//if (_controls_screen)
	//{
	//	drawSprite(D3DXVECTOR3(0,0,0),D3DXVECTOR3(0,0,0),_texture_controls,D3DCOLOR_RGBA(255,255,255,255));
	//	return;
	//}

	RendererSystem::get().drawSprite(D3DXVECTOR3(0,0,0),D3DXVECTOR3(0,0,0),_texture_title_wall,D3DCOLOR_RGBA(255,255,255,(int)(255*_alpha_multiplier)));
	//RendererSystem::get().drawSprite(D3DXVECTOR3(sin(_total_time)*15,0,0),D3DXVECTOR3(0,0,0),_texture_title_player,D3DCOLOR_RGBA(255,255,255,(int)(255*_alpha_multiplier)));
	//RendererSystem::get().drawSprite(D3DXVECTOR3(0,0,0),D3DXVECTOR3(0,0,0),_texture_title_logo,D3DCOLOR_RGBA(255,255,255,(int)(255*_alpha_multiplier)));
	//RendererSystem::get().drawSprite(D3DXVECTOR3(0,0,0),D3DXVECTOR3(0,0,0),_texture_title_press_start,D3DCOLOR_RGBA(255,255,255,(int)(((128*sin(_total_time*5)) + 128)*_alpha_multiplier)));

	//LPD3DXSPRITE sprite=NULL;

	//HRESULT hr = D3DXCreateSprite(g_App.GetDevice(),&sprite);
	//assert (hr == D3D_OK);
	//sprite->Begin(D3DXSPRITE_ALPHABLEND);
	//D3DXVECTOR2 spriteCentre=D3DXVECTOR2(g_App.GetWidth()/16,g_App.GetWidth()/16);
	//D3DXVECTOR2 trans=D3DXVECTOR2(g_App.GetWidth() - g_App.GetWidth()/7,g_App.GetHeight() - g_App.GetWidth()/7.0f);
	//float rotation=-_total_time/2;
	//D3DXMATRIX mat;
	//D3DXMatrixTransformation2D(&mat,NULL,0.0,NULL,&spriteCentre,rotation,&trans);
	//sprite->SetTransform(&mat);
	//RendererSystem::get().drawSprite(
	//sprite->Draw(_texture_title_dragon,NULL, NULL,NULL,D3DCOLOR_RGBA(255,255,255,(int)(255*_alpha_multiplier)));
	//sprite->End();
	//sprite->Release();

}

void TitleScreen::onKeyDown(WPARAM key)
{
	//if (_start_pressed) _start_pressed2 = true;
	_start_pressed = true;
}

void TitleScreen::launchGame()
{
	//if (_start_pressed) _controls_screen = true;
	SoundSystem::get().stopSound("title_track",false,true);
	if (_start_pressed) GameStateManager::get().setGameState(GameStateManager::WORLD);
}
