#ifndef ANIMATION_COMPONENT_H
#define ANIMATION_COMPONENT_H

#include "model.h"
#include "component.h"
#include "bone_adjust.h"
#include "ik_bone.h"
#include "component_transform.h"
#include "aut_player.h"


class AnimationComponent : public Component
{
	CCoreModel* _core_model;
	CModel    * _model;
	TBoneChainAdjust chain_adj;
	TIKBone    ik_arm;
	string		_name;

	btVector3 _last_front;
	// para debug, se puede eliminar posteriormente
	float _angle;

	// Datos para las animaciones de asesinato
	struct AnimationInfo {
		std::string name;
		unsigned num_times_played;
	};

	std::vector<AnimationInfo*> _animation_infos;
	std::vector<AnimationInfo*> _animation_infos_shadow;
	std::vector<AnimationInfo*> _animation_infos_air;
	std::vector<AnimationInfo*> _animation_infos_blend;
	std::vector<AnimationInfo*> _animation_infos_panic;

	std::string _silent_kill_animation;

	std::map<Entity*, std::string> _attached_entities;
		
public:
	AnimationComponent(Entity* entity, std::string name);
	~AnimationComponent(void);

	void init();
	void update(float delta, bool only_position = false);
	void updateAttachedEntities(float delta);
	void disableArm();
	void render(); 
	void renderDebug(size_t index);

	std::string& getName() { return _name; };
	std::string getEnemyType();

	CModel* getModel() const;
	bool blendCycle(const std::string name, float weight, float delay);
	bool clearCycle(const std::string name, float delay);
	bool clearCycles(float delay);
	bool executeAction(const std::string name, float delayIn, float delayOut, float weightTarget = 1.0f, bool autoLock=false);
	bool removeAction(const std::string name, float blendOut);
	bool removeActions(float blendOut);
	bool actionOn(const std::string name);
	bool actionBlocked(const std::string name);
	bool setTimeFactor(float factor);
	float getDuration(const std::string name);
	int getNumFrames(const std::string name);
	float getTime(const std::string name);
	bool isTimeInsideInterval(const std::string name, float first_time_factor, float second_time_factor);
	bool isFrameNumber(const std::string name, int frame);
	void setMixerWorldTransform(TransformComponent* tranform_component);

	bool lookAt(const btVector3 &position, float delta);
	bool lookTowards(const btVector3 &direction, float delta);
	void blendAimIn( float delay );
	void blendAimOut( float delay );
	void blendLookAtIn( float delay );
	void blendLookAtOut( float delay );
	void blockAim();
	void unblockAim();
	void turn(float angle);
	void turnPlayer(float angle);

	void movePlayerToKill(const std::string silent_kill_animation, TransformComponent* tranform_component);
	void moveSnakeToKill(TransformComponent* tranform_component);
	std::string getSilentKillAnimationName() { return _silent_kill_animation; };
	void setSilentKillAnimationName(const std::string silent_kill_animation) { _silent_kill_animation = silent_kill_animation; };
	AUTPlayer::VictimType chooseSilentKillAnimation(const btVector3 &player_pos, const btVector3 &enemy_pos);

	void getKillAnimations();
	size_t getLessPlayedAnimationIndex(std::vector<AnimationInfo*> _animation_infos);

	void attachEntityToBone( Entity* entity, std::string bone );
};

#endif
