#ifndef INC_TEXTURE_MANAGER_H_
#define INC_TEXTURE_MANAGER_H_

#include <map>
#include <string>
#include "d3ddefs.h"
#include "globals.h"

typedef LPDIRECT3DTEXTURE9 TTexture;

class TTextureManager {
public:

	TTexture getTexture( const std::string &name );
	TTexture getTextureResized( const std::string &name, const float width, const float height);

	static TTextureManager &get() {
		static TTextureManager tm;
		return tm;
	}

	void releaseAll( );

private:

	~TTextureManager();
	typedef std::map< std::string, TTexture* > MTexturesByName;
	MTexturesByName textures_by_name;
};


#endif

