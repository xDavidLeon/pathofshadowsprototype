#ifndef CAM_SYS
#define CAM_SYS

#include "world.h"
#include "entity.h"
#include "entity_manager.h"
#include "component_camera.h"
#include <vector>
#include <map>
#include "counter_clock.h"
#include "xml_parser.h"
#include "globals.h"
#include "system_physics.h"
#include <deque>
#include <string>
#include "texture_manager.h"


class CameraSystem : public CXMLParser
{
	Entity *_currentCamera, *_playerCamera, *_dbgCamera, *_cinCamera;

	CameraSystem(void);
	~CameraSystem(void);

	std::deque<int> _cameraQueue;

	TCamera& getCamera(Entity* entity);

	//Datos para cinemáticas
	std::map<int, std::deque<btVector3>> _cinePositions;
	std::map<int, std::deque<btVector3>> _cineLookAts;
	std::map<int, btVector3> _cineTimings; //v[0]:travelling time; v[1]:start_delay; v[2]:finish_delay
	std::map<int, std::pair<bool, bool>> _cineStartEndInterp; //first:interp from camera player; second:interp to camera player

	CounterClock _waitClock; //Crono para esperas
	float _t, _T, pos_factor, lookAt_factor;

	std::string _blackTextureName;
	TTexture _blackTexture, _loadingT;
	D3DXMATRIX _blackM, _loadingM;

	//Franjas negras
	bool _barsActivated, _barsLocked;
	D3DXMATRIX _upperBarT, _lowerBarT;
	float _upperBarY_a, _lowerBarY_a, _upperBarY_d, _lowerBarY_d;

	//Pantalla negra (para fundido a/desde negro)
	bool _blackScreenActivated;
	TTexture _blackScreenTexture;
	D3DXMATRIX _blackScreenM;
	float _blackScreenAlpha;

	//logo y pressEnter
	bool _logoActivated;
	TTexture _logoT;
	D3DXMATRIX _logoM;
	TTexture _pressEnterT;
	D3DXMATRIX _pressEnterM;

	//creditos
	bool _creditsActivated;
	TTexture _creditsT;
	D3DXMATRIX _creditsM, _creditsInitM;
	float _creditsY, _creditsMaxY, _creditsVel;

	//subtitulos
	std::string _languageFolder;
	std::map<string, TTexture> _subtTextures;
	string _currentSubt, _dyingSubt;
	float _currentSubtA, _dyingSubtA;
	D3DXMATRIX _subsM;
	CounterClock _subTimer;

	void initSubs();

	int _activeCine;
	bool _pauseCine, _pauseOnEnd;

	bool executeCineCamera(int cam_id);

	bool _lockCamera3rd;

public:
	static CameraSystem &get()
	{
		static CameraSystem cs = CameraSystem();
		return cs;
	}

	void addCinCameraPos(int cam_id, const btVector3& pos);
	void addCinCameraLookAt(int cam_id, const btVector3& lookAt);
	void addCinCameraTiming(int cam_id, const btVector3& timing, bool interpFromPlayer, bool interpToPlayer);

	void render(void);
	void update(float delta);

	void renderDbgCameraInfo();

	void setCinePaused(bool p){ _pauseCine=p; };
	void pauseOnEnd(){ _pauseOnEnd=true; };
	void setBarsLocked(bool l){ _barsLocked=l; };

	void setLockCamera3rd(bool l){ _lockCamera3rd = l; }
	bool isLockedCamera3rd(){ return _lockCamera3rd; }

	float getBSAlpha(){ return _blackScreenAlpha; }

	void lookAt(Entity* cam_entity, const btVector3& pos, const btVector3& lookAt);

	void setCurrentCamera(Entity* camera){ _currentCamera = camera; }
	Entity* getCurrentCameraEntity()		 { return _currentCamera; }
	TCamera& getCurrentCamera()			 { return getCamera(_currentCamera); }

	void setPlayerCamera(Entity* camera) { _playerCamera = camera; }
	Entity* getPlayerCameraEntity()		 { return _playerCamera; }
	TCamera& getPlayerCamera()			 { return getCamera(_playerCamera); }

	void setDbgCamera(Entity* camera)	 { _dbgCamera = camera; }
	Entity* getDbgCameraEntity()		 { return _dbgCamera; }
	TCamera& getDbgCamera()				 { return getCamera(_dbgCamera); }

	void setCinCamera(Entity* camera)	 { _cinCamera = camera; }
	Entity* getCinCameraEntity()		 { return _cinCamera; }

	bool currentIsDbg(){ return _currentCamera == _dbgCamera; }
	bool toggleDbgCamera(bool setAtCamPlayer);
	void toggleKillCamera();
	void changeKillCamera();
	void toggleDeathCamera();

	void addCamToQueue(int cam_id);
	void activateCineCamera(int cam_id);
	bool isCineActive();
	int getActiveCamId() const;

	void toggleBars(){ _barsActivated = !_barsActivated; }
	void renderBars();

	void toggleBlackScreen(){ _blackScreenActivated = !_blackScreenActivated; }
	void renderBlackScreen();
	void setBlackToLoadingScreen(bool loading);
	void renderLogo();
	void activateCredits();
	void renderCredits();
	void activateSubt(const string& subtName, float subTime = -1.0f);
	void deactivateCurrentSubt();
	void renderSubs();
	void setBlackScreen(bool black);
	void enableLogo(){ _logoActivated = true; }
	void disableLogo(){ _logoActivated = false; }

	void releaseCineCameras();

	struct CameraInfo 
	{
		Entity* cameraEntity;
		btVector3 position;
		btVector3 lookAt;
		unsigned frame;
	};

	struct CameraSequence
	{
		unsigned num_times_played;
		unsigned index;
		std::vector< CameraInfo* > cameraInfos;
		
		void advanceInSequence()
		{
			index++;
			if( index >= cameraInfos.size() )
				index = 0;
		}
	};

	struct KillCameraInfo 
	{
		std::string name;
		btVector3 shadow_translation;
		int type_shadow;
		int type_air;
		int type_blend;
		int type_panic;
		
		std::vector<CameraSequence*> cameraSequences;
	};

	CameraSequence* getBestCameraSequence(std::map<std::string, KillCameraInfo*> cameras);
	size_t getLessPlayedSequenceIndex(std::vector<CameraSequence*> camera_sequences);
	CameraSequence* getBestCameraSequence() { return _best_camera_sequence; };

	CameraInfo* getValidCameraInfo(btTransform* target_transform, CameraSequence* camera_sequence);
	Entity* getAndPlaceKillCamera( CameraSequence* camera_sequence );
	Entity* getAndPlaceDeathCamera( CameraSequence* camera_sequence );

	std::map<std::string, KillCameraInfo*> _killCameras;
	std::map<std::string, KillCameraInfo*> _deathCameras;
	CameraInfo* _valid_camera;
	CameraSequence*  _best_camera_sequence;

	void readKillCamerasFile();
	
	void onStartElement (const std::string &elem, MKeyValue &atts);
	void onEndElement (const std::string &elem);
	void onData (const std::string &data){};

	KillCameraInfo* actual_kill_camera_info;
	CameraSequence*  actual_camera_sequence;
	CameraInfo*  actual_camera_info;

	int _logoAlpha;
};

#endif
