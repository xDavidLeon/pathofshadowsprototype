#ifndef SOUND_SYSTEM_H
#define SOUND_SYSTEM_H

#include "fmod.h"
#include "fmod_errors.h"
#include "assert.h"
#include <vector>
#include <map>
#include <string>
#include <btBulletCollisionCommon.h>
#include "d3ddefs.h"

class TransformComponent;
class SoundSystem {
public:

	struct ChannelInfo {
		FMOD_CHANNEL* channel;
		float volume;
		float desired_volume;
		float time;
		float delay;
	};

	struct SoundInfo {
		std::string id;
		std::string channel_name;
		FMOD_CHANNEL * channel;
		FMOD_SOUND * sound;
		float volume;
		float desired_volume;
		float time;
		float delay;
		bool isPlaying;
		float grow_factor;
		bool restarting;
		std::string filename;
		std::string tag;
		std::string tag2;

		SoundInfo()
		{
			id = "";
			channel_name = "";
			channel = NULL;
			sound = NULL;
			volume = 1.0f;
			desired_volume = 1.0f;
			time = 0;
			delay = 0;
			isPlaying = false;
			restarting = false;
			grow_factor = 0.25f;
			filename = "";
			tag = "";
			tag2 = "";
		}

		SoundInfo(FMOD_CHANNEL * c, FMOD_SOUND * s)
		{
			id = "";
			channel_name = "";
			channel = c;
			sound = s;
			volume = 1.0f;
			desired_volume = 1.0f;
			time = 0;
			delay = 0;
			isPlaying = false;
			restarting = false;
			grow_factor = 0.25f;
			filename = "";
			tag = "";
			tag2 = "";
		}
	};

	static SoundSystem & get()
	{
		static SoundSystem singleton;
		return singleton;
	}

	void init();
	void updateListener(const btVector3 & position, btVector3 & velocity, btVector3 & forward, btVector3 & up);
	void update(float delta);
	void render();

	SoundInfo * getSoundInfo( std::string channelname );

	SoundInfo * playSFX(std::string id, std::string filename, std::string channelname, float start_volume, float desired_volume, bool loop);
	SoundInfo * playStream(std::string id, std::string filename, std::string channelname, float start_volume, float desired_volume, bool loop);
	SoundInfo * playSFX3D(std::string id, std::string filename, std::string channelname, const btVector3 & position, const btVector3 & velocity, bool loop, float start_volume, float desired_volume, float decay_start_distance = -1.0f, float decay_end_distance = 10000.0f);
	void stopSound(std::string id, bool restart = false, bool force = false);
	void stopAllSounds(bool force);
	void release(void);
	FMOD_SYSTEM * getFMODSystem() { return _fmod_system;};
	float getCPUUsage();
	void addColliderSound( const btCollisionObject* collider, std::string sound_name ) { _collider_sounds[collider] = sound_name; };
	std::string getGroundSound(TransformComponent * t);
	void set3DPosition(SoundInfo * sound, const btVector3 & position);
	void set3DPosition(std::string id_sound, const btVector3 & position);

private:
	SoundSystem(void);
	~SoundSystem(void);

	char *_current_sound; //current sound to play

	//FMOD system stuff
	FMOD_RESULT _result;
	FMOD_SYSTEM *_fmod_system;
	unsigned int _fmod_version;
	int	_num_drivers;
	FMOD_SPEAKERMODE _fmod_speaker_mode;
	std::map<const btCollisionObject*, std::string> _collider_sounds;
	float _distance_factor;

	std::map<std::string, FMOD_SOUND*> _sounds;
	std::map<std::string, SoundInfo*> _my_sounds;

	FMOD_CHANNELGROUP *_channels_music;
	FMOD_CHANNELGROUP *_channels_sfx;
	std::map<std::string, FMOD_CHANNEL*> _my_channels;

	SoundSystem::SoundInfo * loadMusicStream(std::string id, std::string filename, std::string channel_name, bool is3D);
	SoundSystem::SoundInfo * loadSFXSound(std::string id, std::string filename, std::string channel_name, bool is3D);

	FMOD_VECTOR _listener_pos;

	void FMODErrorCheck(FMOD_RESULT result);

};

#endif
