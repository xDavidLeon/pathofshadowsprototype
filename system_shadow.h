#ifndef M_SHADOW_MGR
#define M_SHADOW_MGR

#include "d3ddefs.h"

class Entity;
class ShadowSystem
{
	unsigned _activeShadows, _maxActiveShadows, _created_shadows;

public:
	static ShadowSystem & get()
	{
		static ShadowSystem singleton;
		return singleton;
	}

	Entity * createShadow(const D3DXVECTOR3 & pos, const D3DXVECTOR3 & normal);
	
	void update(float delta);

	bool checkPosInShadows(const D3DXVECTOR3& ipos, float offset=0.0f) const;
	bool checkPlayerInShadows() const;
	bool checkValidPosToTeleport(const D3DXVECTOR3& ipos) const;

	void destroyShadows();

	//static ShadowSystem* get() {
	//	static ShadowSystem* pm = new ShadowSystem();
	//	return pm;
	//}
private:
	ShadowSystem(void);
	~ShadowSystem(void);
};

#endif
