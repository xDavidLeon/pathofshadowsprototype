#ifndef VISION_INTERFACE
#define VISION_INTERFACE

#include "texture_manager.h"
#include <vector>
#include <string>
#include "entity.h"

class VisionInterface
{
	TTexture _compassTexture, _arrowTexture;
	D3DXMATRIX _compassM, _centerScreenM, _arrowCenterTextureM;
	float _compassRadius;
	bool _viewedByEnemy, _enemyAlert, _enemySearching;
	int _compassAlpha, _arrowAlpha;
	int _currentR, _currentG, _currentB;
	int _normalColor, _alertR, _alertG, _alertB;

	VisionInterface(void);
	~VisionInterface(void);

public:
	static VisionInterface& get()
	{
		static VisionInterface vi;
		return vi;
	}

	void render();

	bool thereIsAlert(){ return _enemyAlert; }
	void setCompassAlpha(float alpha){ if(alpha>=0.0f && alpha<=1.0f) _compassAlpha = alpha; };
};

#endif