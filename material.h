#ifndef INC_MATERIAL_H_
#define INC_MATERIAL_H_

#include <vector>
#include "d3ddefs.h"
#include "texture_manager.h"

// 
struct TMaterial {
	// Sort materials by name, then by main texture

	std::string name;
	TTexture main_texture;
	TTexture diffuse;
	TTexture diffuse2;
	TTexture diffuse3;
	TTexture mask;
	TTexture lightmap;
	TTexture specular;
	TTexture emissive;
	TTexture normalmap;
	TTexture bumpmap;
	// Shader
	// params
	TMaterial( ) 
	: name("tech_basic")
	, main_texture (NULL)
	, diffuse ( NULL )
    , diffuse2 ( NULL )
	, diffuse3 ( NULL )
    , mask ( NULL )
    , lightmap( NULL )
	, specular( NULL )
	, emissive(NULL)
	, bumpmap (NULL)
	, normalmap (NULL)
	{ }
};

typedef std::vector< TMaterial* > VMaterials;



#endif
