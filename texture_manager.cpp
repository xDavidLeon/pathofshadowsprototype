#include "texture_manager.h"
#include <cassert>
#include "globals.h"
#include "d3ddefs.h"

TTextureManager::~TTextureManager()
{
	//releaseAll();
}

TTexture TTextureManager::getTexture( const std::string &name )
 {
	// Check if exists
	MTexturesByName::iterator it = textures_by_name.find( name );

	// found
	if( it != textures_by_name.end() ) {
		return *it->second;	
	}

	// Not found. try to load
	TTexture new_texture = NULL;

	std::string base_filename = "data/textures/" + name;

	static const char *exts[] = { ".tga", ".png", ".bmp", ".jpg",  ".dds",  NULL };
	const char **p = exts;
	while( *p ) {
		std::string filename = base_filename + std::string( *p );
		HRESULT hr = D3DXCreateTextureFromFile( g_App.GetDevice()
												, filename.c_str()
												, &new_texture
												);
		if( hr == D3D_OK )
			break;
		++p;
	}
	assert( new_texture != NULL );

	textures_by_name[ name ] = new TTexture(new_texture);
	return new_texture;
}

TTexture TTextureManager::getTextureResized( const std::string &name, float width, float height)
 {
	// Check if exists
	MTexturesByName::iterator it = textures_by_name.find( name );

	// found
	if( it != textures_by_name.end() ) {
		return *it->second;	
	}

	// Not found. try to load
	TTexture new_texture = NULL;

	std::string base_filename = "data/textures/" + name;

	static const char *exts[] = { ".tga", ".png", ".bmp", ".jpg",  ".dds",  NULL };
	const char **p = exts;
	while( *p ) {
		std::string filename = base_filename + std::string( *p );
		HRESULT hr = D3DXCreateTextureFromFileEx( g_App.GetDevice()
												, filename.c_str() 
												, width
												, height
												, 0,D3DPOOL_DEFAULT,D3DFMT_UNKNOWN,D3DPOOL_DEFAULT,D3DX_DEFAULT,D3DX_DEFAULT,NULL,NULL, NULL,&new_texture
												);

		if( hr == D3D_OK )
			break;
		++p;
	}
	assert( new_texture != NULL );

	textures_by_name[ name ] = new TTexture(new_texture);
	return new_texture;
}

void TTextureManager::releaseAll( ) {
	MTexturesByName::iterator it = textures_by_name.begin();
	while( it != textures_by_name.end() ) {
 		(*it->second)->Release();
		++it;
	}
	textures_by_name.clear( );
}
