#ifndef _SHADOW_ACTIONS_COMP
#define _SHADOW_ACTIONS_COMP

#include "component.h"
#include "btBulletDynamicsCommon.h"
#include <vector>
#include <d3dx9.h>
#include "component_shadow.h"
#include "component_transform.h"
#include "component_light.h"

enum playerVisibility
{
	BLENDED,		//fusionado en sombra
	ONSHADOW,		//En sombra, ya sea magica o estatica
	VISIBLE,		//iluminado por la luna/luz "ambiente"
	ILLUMINATED,		//iluminado por una luz artificial
	TELEPORTING
};


class ShadowActionsComponent : public Component
{
	playerVisibility _playerVisibility;

	TransformComponent* _transC;

	float _maxColDist;
	btVector3 _lastCol;
	btVector3 _blobPos;

	btVector3 _lastNormal;
	btVector3 _previousCol;
	btVector3 _blendPos;
	bool _lastColValid;
	D3DXMATRIX _mCrosshair;
	ShadowComponent* _lastCreatedShadow;
	float _capsuleOffsetV, _capsuleOffsetH;
	btVector3 _exitBlendPos; //posicion a la que se aparecera despues del blend
	bool _telepMode;
	bool _canTeleport;

	void updateRayCollision();
	bool _shadow_explosion;

	Entity * _shadow_preview;
	LightComponent * _blob_preview;
	TransformComponent * _blob_transform;

public:
	ShadowActionsComponent(Entity* e);
	~ShadowActionsComponent(void);

	void init();

	void enableAiming();
	void disableAiming();

	btVector3 getExitPos() { return _exitBlendPos; };
	btVector3 getColPos() { return _lastCol; };
	btVector3 getNormal() { return _lastNormal; };

	void setBlendPos(const btVector3& bpos){ _blendPos = bpos; }

	void setLockOn(bool lockon);

	void setTeleportMode(bool activate);
	void update(float delta, bool creatingShadow=false);
	void updateTeleportMode();
	bool checkIfCanExitBlend();
	bool teleportToTarget();
	void updateVisibility(float delta);
	void updateVisibilityBlended(float delta);
	unsigned getVisibility() const { return _playerVisibility; }
	void setVisibility( playerVisibility visibility) { _playerVisibility = visibility; }
	void render();
	void renderDebug();
	void renderPlayerVisibility(float x, float y, unsigned color);
	void renderCapsuleCols(unsigned color);

	void shadowCreation();
	void stopCreatingShadow();
	void prepareToBlend();
	void blend();
	void exitBlend();

	bool canCreateShadow() const;
	bool canGrowShadow() const;
	bool canTeleport() const;
	bool canBlend() const;
	bool canRecharge() const;
	bool canUseSV() const;

	bool isHidden() const;
	bool isGrounded() const;
};

#endif
