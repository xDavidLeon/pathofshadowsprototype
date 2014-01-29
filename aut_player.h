#ifndef _AUTPLAYER_H
#define _AUTPLAYER_H

#include "automat.h"
#include "iostatus.h"
#include "component_shadow_actions.h"
#include "component_transform.h"
#include "component_player_controller.h"
#include "counter_clock.h"
#include "system_sound.h"
#include <deque>
class AnimationComponent;
class ParticleEffectComponent;

class AUTPlayer : public Automat
{
	CIOStatus* _ios;

	bool _enableIdleCG;

	public:

		enum VictimType {
			NORMAL,
			SHADOW,
			AERIAL,
			BLEND,
			PANIC
		};

		AUTPlayer(Entity* entity);
		~AUTPlayer();


		SoundSystem::SoundInfo * _sound_channel;
		SoundSystem::SoundInfo * _sound_channel2;
		SoundSystem::SoundInfo * _sound_channel_current;


		//************************ GUARRERIA SEXUAL ******************//
		Entity* _stele;
		Entity* _snake;
		//************************ GUARRERIA SEXUAL ******************//

		bool _lockedInPlace;
		PlayerControllerComponent* _playerContC;
		//float _decoy_noise;
		//float _run_noise;
		TransformComponent* _transformC;
		ShadowActionsComponent* _shadowAcComp;
		AnimationComponent* _animation_component;
		bool _telepGround;
		float _silentKillDistMh;
		float _panicKillDistMh;
		Entity* _silentKillVictim;
		VictimType _victimType;
		bool _frame_catched;

		std::deque<btVector3> _pathToEnd;

		float _inactiveTimeTrigger;
		float _inactiveAnimationTime;
		CounterClock _inactive;
		ParticleEffectComponent * _particleShadowHand;
		//head controller
		TransformComponent* _target;
		btVector3 _random_look_at;
		float _time_forget;
		CyclicCounterClock _retarget;
		void getNearEnemyTarget( float delta );

		void setVisibility( unsigned visibility );
		void getVisibilityString(std::string& str) const;

		void prepareForRebirth();

		void init();
		void reset();
		void render();
		void update( float delta );
		void updateAnimations( float delta );
		void updateTransitionAnimations( std::string state );
		void setSilentKill();
		void stopMovement();
		void setEnableIdleCG(bool enable){ _enableIdleCG = enable; }
		
		//head controller
		void headController(float delta);
		void computeRandomWpt(std::string& state);

		void checkBasicTransitions();
		void checkFightTransitions();
		void checkShadowTransitions();
		void changeState( std::string state );

	//cine
		void reborn(float delta);
		void idleCG(float delta);
		void doorCG(float delta);
		void crouchRecharge(float delta);
		void recharging(float delta);
		void riseRecharge(float delta);

	//"basic state"
		void idleBasic(float delta);
		void idleInactive(float delta);
		void idleKill(float delta);
		void walk(float delta);
		void walkKill(float delta);
		void run(float delta);
		void accelerate(float delta);
		void brake(float delta);
		void silentMurder(float delta);
		void decoy(float delta);			//senyuelo
		void enteringShadow(float delta);	//fundiendose
		void emerge(float delta);			//bufanda emerge de la sombra
		void emergeWall(float delta);		//bufanda emerge de la sombra
		void blended(float delta);			//fundido en sombra
		void blendedWall(float delta);		//fundido en sombra en pared
		void leavingShadow(float delta);	//des-fundiendose
		void leavingShadowWall(float delta);//des-fundiendose
		void crouchSVision(float delta);
		void specialVision(float delta);
		void riseSVision(float delta);
		void summonCrow(float delta);

	//"shadow state"
		void idleShadow(float delta);
		void movingShadow(float delta);
		void creatingShadow(float delta);
		void stopCreateShadow(float delta);
		//void telepReady(float delta);			//Listo para el teletrans.
		void accelerateTeleport(float delta);
		void teleporting(float delta);
		void brakeTeleport(float delta);

	//others
		void falling(float delta);
		void landing(float delta);
		void grounding(float delta);
		void dying(float delta);

	//misc. functions
		bool checkVictim();
		bool checkAerialVictim();
		bool checkBlendVictim();
		bool checkPanicVictim();

		std::string special_action;
		Entity* _lastVictim;
		void updateSteps();

};

#endif
