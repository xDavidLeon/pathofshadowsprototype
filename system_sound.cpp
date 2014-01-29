#include "system_sound.h"
#include "globals.h"
#include "world.h"
#include <iostream>

SoundSystem::SoundSystem(void)
{
	_fmod_system = NULL;
	_distance_factor = 1.0f;
	init();
}

SoundSystem::~SoundSystem(void)
{
	release();
}

void SoundSystem::release(void)
{
	// Release
	//std::map<std::string, FMOD_SOUND*>::iterator iter;
	//for (iter = _sounds.begin(); iter != _sounds.end(); ++iter) FMOD_Sound_Release(iter->second);

	FMOD_ChannelGroup_Release(_channels_music);
	FMOD_ChannelGroup_Release(_channels_sfx);
	FMOD_System_Release(_fmod_system);
	_my_sounds.clear();
	_my_channels.clear();
	_sounds.clear();
	//FMOD_ChannelGroup_Release(_channels_steps);
}

void SoundSystem::init()
{
	_result = FMOD_System_Create(&_fmod_system);
	FMODErrorCheck(_result);

	// Check version
	_result = FMOD_System_GetVersion(_fmod_system,&_fmod_version);
	FMODErrorCheck(_result);
 
	if (_fmod_version < FMOD_VERSION)
	{
		std::cout << "Error! You are using an old version of FMOD " << _fmod_version << ". This program requires " << FMOD_VERSION << std::endl;  
		assert(_fmod_version < FMOD_VERSION); 
	}

	// Get number of sound cards 
	_result = FMOD_System_GetNumDrivers(_fmod_system,&_num_drivers);
	FMODErrorCheck(_result);

	FMOD_CAPS caps = 0;
	// No sound cards (disable sound)
	if (_num_drivers == 0)
	{
		_result = FMOD_System_SetOutput(_fmod_system, FMOD_OUTPUTTYPE_NOSOUND);
		FMODErrorCheck(_result);
	}
	// At least one sound card
	else
	{
		// Get the capabilities of the default (0) sound card
		_result = FMOD_System_GetDriverCaps(_fmod_system,0, &caps, 0, &_fmod_speaker_mode);
		FMODErrorCheck(_result);
 
		// Set the speaker mode to match that in Control Panel
		_result = FMOD_System_SetSpeakerMode(_fmod_system,_fmod_speaker_mode);
		FMODErrorCheck(_result);
	}

	// Increase buffer size if user has Acceleration slider set to off
	if (caps & FMOD_CAPS_HARDWARE_EMULATED)
	{
		_result = FMOD_System_SetDSPBufferSize(_fmod_system,1024, 10);
		FMODErrorCheck(_result);
	}

	// Get name of driver
	char name[256];
	_result = FMOD_System_GetDriverInfo(_fmod_system,0, name, 256, 0);
    FMODErrorCheck(_result);
 
    // SigmaTel sound devices crackle for some reason if the format is PCM 16-bit.
    // PCM floating point output seems to solve it.
    if (strstr(name, "SigmaTel"))
    {
		_result = FMOD_System_SetSoftwareFormat(_fmod_system, 48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0, 0, FMOD_DSP_RESAMPLER_LINEAR);
        FMODErrorCheck(_result);
    }

	//Initializes the system with channels
	_result = FMOD_System_Init(_fmod_system, 128, FMOD_INIT_3D_RIGHTHANDED, 0);
	// If the selected speaker mode isn't supported by this sound card, switch it back to stereo
	if (_result == FMOD_ERR_OUTPUT_CREATEBUFFER)
	{
		_result = FMOD_System_SetSpeakerMode(_fmod_system, FMOD_SPEAKERMODE_STEREO);
		FMODErrorCheck(_result);
 
		_result = FMOD_System_Init(_fmod_system, 128, FMOD_INIT_NORMAL, 0);
	}
	else  FMOD_System_Set3DSettings(_fmod_system,1.0f, _distance_factor, 2.0f);

	FMOD_System_Set3DNumListeners(_fmod_system,1);

	FMODErrorCheck(FMOD_System_CreateChannelGroup(_fmod_system, "music", &_channels_music));
	FMODErrorCheck(FMOD_System_CreateChannelGroup(_fmod_system, "sfx", &_channels_sfx));
	//FMODErrorCheck(FMOD_System_CreateChannelGroup(_fmod_system, "steps", &_channels_steps));

	FMOD_ChannelGroup_SetVolume(_channels_music, 1.0f);
	FMOD_ChannelGroup_SetVolume(_channels_sfx, 1.0f);
	//FMOD_ChannelGroup_SetVolume(_channels_steps, 1.0f);

}

void SoundSystem::update(float delta)
{
	std::map<std::string, SoundInfo*>::iterator iter;
	for (iter = _my_sounds.begin(); iter != _my_sounds.end(); ++iter) 
	{
		SoundInfo* info = iter->second;
		//if (info->isPlaying == false) continue;
		if (info->restarting)
		{
			FMOD_Channel_SetPaused(info->channel, false);
			info->volume -= delta * info->grow_factor;
			if (info->volume <= 0.0f) 
			{
				info->restarting = false;
			}
		}
		else if (info->desired_volume > info->volume)
		{
			FMOD_Channel_SetPaused(info->channel, false);
			info->volume += delta * info->grow_factor;
			if (info->volume > info->desired_volume) info->volume = info->desired_volume;
			FMOD_Channel_SetVolume(info->channel, info->volume);
		}
		else if (info->desired_volume < info->volume )
		{
			FMOD_Channel_SetPaused(info->channel, false);
			info->volume -= delta * info->grow_factor;
			if (info->volume < info->desired_volume) 
			{
				info->volume = info->desired_volume;
			}
			FMOD_Channel_SetVolume(info->channel, info->volume);
		}
		else if (info->desired_volume <= 0.0f && info->volume <= 0.0f)
		{
			FMOD_Channel_SetPaused(info->channel, true);
			info->isPlaying = false;
		}
	}

	FMOD_System_Update(_fmod_system);
}

void SoundSystem::render()
{
}

void SoundSystem::updateListener(const btVector3 & position, btVector3 & velocity, btVector3 & forward, btVector3 & up)
{
	FMOD_VECTOR vpos, vvel, vfor, vup;

	vpos.x = position.getX();
	vpos.y = position.getY();
	vpos.z = position.getZ();
	vvel.x = velocity.getX();
	vvel.y = velocity.getY();
	vvel.z = velocity.getZ();
	vfor.x = forward.getX();
	vfor.y = forward.getY();
	vfor.z = forward.getZ();
	vup.x = up.getX();
	vup.y = up.getY();
	vup.z = up.getZ();

	FMOD_System_Set3DListenerAttributes(_fmod_system,0,&vpos,&vvel,&vfor,&vup);
}

SoundSystem::SoundInfo * SoundSystem::loadMusicStream(std::string id, std::string filename, std::string channel_name, bool is3D)
{
	// Create the stream
	FMOD_SOUND *stream;
	if (_sounds.find(filename) == _sounds.end()) // not found
	{
		_result = FMOD_System_CreateStream(_fmod_system, filename.c_str(), FMOD_DEFAULT, 0, &stream);
		FMODErrorCheck(_result);
		if (is3D) FMOD_Sound_SetMode(stream, FMOD_DEFAULT | FMOD_3D);
		_sounds[filename] = stream;
	}
	// we already have it, just return it
	else stream = _sounds[filename];

	FMOD_CHANNEL *channel;

	if (_my_channels.find(channel_name) != _my_channels.end())
	{
		channel = _my_channels[channel_name];
	}

	FMODErrorCheck(FMOD_System_PlaySound(_fmod_system, FMOD_CHANNEL_FREE, stream, true, &channel));
	FMOD_Channel_SetChannelGroup(channel, _channels_music);
	FMOD_Channel_SetMode(channel,FMOD_LOOP_NORMAL);
	FMOD_Channel_SetLoopCount(channel,-1);
	_my_channels[channel_name] = channel;

	SoundInfo* mySound;
	if (_my_sounds.find(id) == _my_sounds.end())
	{
		mySound = new SoundInfo(channel, stream);
		mySound->id = id;
		mySound->filename = filename;
		mySound->channel_name = channel_name;
		_my_sounds[id] = mySound;
	}
	else 
	{
		mySound = _my_sounds[id];
		mySound->channel = channel;
		mySound->sound = stream;
		mySound->filename = filename;
		mySound->channel_name = channel_name;
	}
	return mySound;
}

SoundSystem::SoundInfo * SoundSystem::loadSFXSound(std::string id, std::string filename, std::string channel_name, bool is3D)
{
	// load the sfx into our sounds map
	FMOD_SOUND *sound;
	if (_sounds.find(filename) == _sounds.end()) // not found
	{
		FMODErrorCheck(FMOD_System_CreateSound(_fmod_system,filename.c_str(), FMOD_DEFAULT, 0, &sound));
		if (is3D) FMOD_Sound_SetMode(sound, FMOD_DEFAULT | FMOD_3D);
		_sounds[filename] = sound;
	}
	// we already have it, just return it
	else sound = _sounds[filename];

	// Create a channel for it?
	FMOD_CHANNEL *channel = NULL;
	if (channel_name != "" && _my_channels.find(channel_name) != _my_channels.end()) 
	{
		channel = _my_channels[channel_name];
		FMOD_Channel_Stop(channel);
	}
	SoundInfo* mySound;
	if (_my_sounds.find(id) == _my_sounds.end())
	{
		mySound = new SoundInfo(channel, sound);
		mySound->filename = filename;
		mySound->id = id;
		mySound->channel_name = channel_name;
		_my_sounds[id] = mySound;
	}
	else 
	{
		mySound = _my_sounds[id];
		FMOD_Channel_Stop(mySound->channel);
		mySound->channel = channel;
		mySound->sound = sound;
		mySound->filename = filename;
		mySound->channel_name = channel_name;
	}
	return mySound;
}

SoundSystem::SoundInfo * SoundSystem::playStream(std::string id, std::string filename, std::string channel_name, float start_volume, float desired_volume, bool loop)
{
	SoundInfo * info = loadMusicStream(id, filename, channel_name, false);
	info->volume = start_volume;
	info->desired_volume = desired_volume;
	info->isPlaying = true;
	FMOD_Channel_SetPaused(info->channel, false);
	if (loop) FMOD_Channel_SetMode(info->channel,FMOD_LOOP_NORMAL);
	else FMOD_Channel_SetMode(info->channel,FMOD_LOOP_OFF);
	FMOD_Channel_SetVolume(info->channel, start_volume);

	return info;
}

SoundSystem::SoundInfo * SoundSystem::getSoundInfo( std::string id )
{
	SoundInfo* mySound = NULL;
	if (_my_sounds.find(id) == _my_sounds.end())
	{
		return NULL;
	}
	else mySound = _my_sounds[id];
	return mySound;
}

void SoundSystem::stopSound(std::string id, bool restart, bool force)
{
	SoundInfo * info = getSoundInfo(id);
	if (info == NULL) return;
	info->desired_volume = 0.0f;
	if (restart) info->restarting = true;
	if (force) FMOD_Channel_Stop(info->channel);
}

float SoundSystem::getCPUUsage()
{
	float total = 0;
	FMOD_System_GetCPUUsage(_fmod_system,NULL,NULL,NULL,NULL,&total);
	return total;
}

SoundSystem::SoundInfo * SoundSystem::playSFX (std::string id,std::string filename, std::string channelname, float start_volume, float desired_volume, bool loop) 
{
	SoundInfo * info = loadSFXSound(id, filename, channelname, false);
	_result = FMOD_System_PlaySound(_fmod_system,FMOD_CHANNEL_FREE, info->sound, false, &info->channel);
	FMODErrorCheck(_result);
	info->volume = start_volume;
	info->desired_volume = desired_volume;
	info->isPlaying = true;
	FMOD_Channel_SetVolume(info->channel, start_volume);
	if (loop) FMOD_Channel_SetMode(info->channel, FMOD_LOOP_NORMAL);
	else FMOD_Channel_SetMode(info->channel, FMOD_LOOP_OFF);

	return info;
}

SoundSystem::SoundInfo * SoundSystem::playSFX3D (std::string id, std::string filename, std::string channelname, const btVector3 & position, const btVector3 & velocity, bool loop, float start_volume, float desired_volume, float decay_start_distance, float decay_end_distance)
{
	FMOD_VECTOR pos = { position.getX(),position.getY(),position.getZ()};
	FMOD_VECTOR vel = { velocity.getX()/(1.0f/60.0f),velocity.getY()/(1.0f/60.0f),velocity.getZ()/(1.0f/60.0f)};

	SoundInfo * info = loadSFXSound(id, filename, channelname, true);
	info->volume = start_volume;
	info->desired_volume = desired_volume;
	info->isPlaying = true;
	if (info->channel != NULL) FMOD_Channel_Stop(info->channel);
	_result = FMOD_System_PlaySound(_fmod_system,FMOD_CHANNEL_FREE, info->sound, false, &info->channel);
	FMODErrorCheck(_result);
	FMOD_Channel_SetVolume(info->channel, start_volume);
		
	if (loop) 
	{
		_result = FMOD_Channel_SetMode(info->channel,FMOD_LOOP_NORMAL | FMOD_3D);
		FMODErrorCheck(_result);
		FMOD_Channel_SetLoopCount(info->channel,-1);
	}
	else FMOD_Channel_SetMode(info->channel,FMOD_LOOP_OFF | FMOD_3D);
	_result = FMOD_Channel_Set3DAttributes(info->channel,&pos,&vel);
	FMODErrorCheck(_result);
	if( decay_start_distance > 0)
		FMOD_Channel_Set3DMinMaxDistance(info->channel,decay_start_distance, decay_end_distance);
	return info;
}

//
//bool SoundSystem::blendStepsVolume (float volume, float weight)
//{
//	//std::string ground = getGroundSound();
//	//if (ground == "") return false;
//	////dbg("ground %s\n", ground.c_str());
//
//	////cutrez maxima, no quiero matarme con el sistema de sonido este. Bajamos el volumen de todos a 0 y ponemos a weigth el que toque
//
//	//blendChannelVolume( "grass_walk", 0.0f, 1.0f);
//	//blendChannelVolume( "wood_walk", 0.0f, 1.0f);
//	//blendChannelVolume( "stone_walk", 0.0f, 1.0f);
//	//blendChannelVolume( "outside_walk", 0.0f, 1.0f);
//	//blendChannelVolume( "gravel_walk", 0.0f, 1.0f);
//	//blendChannelVolume( "grass_run", 0.0f, 1.0f);
//	//blendChannelVolume( "wood_run", 0.0f, 1.0f);
//	//blendChannelVolume( "stone_run", 0.0f, 1.0f);
//	//blendChannelVolume( "outside_run", 0.0f, 1.0f);
//	//blendChannelVolume( "gravel_run", 0.0f, 1.0f);
//	//
//	//if(volume < 0.4f)
//	//{
//	//	//dbg("volume %f\n", volume);
//	//	blendChannelVolume( ground + "_walk", volume, weight);
//	//}
//	//else
//	//{
//	//	//dbg("volume %f\n", volume);
//	//	blendChannelVolume( ground + "_run", volume, weight);
//	//}
//
//	return true;
//}

#include "component_transform.h"
#include "entity_manager.h"
#include "system_physics.h"
std::string SoundSystem::getGroundSound(TransformComponent * t)
{
	//TransformComponent* _transformC = EntityManager::get().getComponent<TransformComponent>(World::instance()->getPlayer());

	//lanzar un ray al suelo y ver si la distacia es la minima para aterrizar
	btVector3 btRayFrom = t->getPosition();
	btVector3 btRayTo = btRayFrom;
	btRayTo.setY(-1000.0f);

	//Creamos callback para obtener resultado de la colisi�n
	btCollisionWorld::ClosestRayResultCallback rayCallback(btRayFrom,btRayTo);
	//Test de colisi�n
	PhysicsSystem::get().getCollisionWorld()->rayTest(btRayFrom, btRayTo, rayCallback);
	//Si colisiona hacemos cositas molonas
	if (rayCallback.hasHit())
	{
		btVector3 hitPosition = rayCallback.m_hitPointWorld;
		const btCollisionObject* object = rayCallback.m_collisionObject;
		if (_collider_sounds.find(object) != _collider_sounds.end()) return _collider_sounds[object];
		else return "";
	}

	return "NULL";
}

void SoundSystem::FMODErrorCheck(FMOD_RESULT result)
{
	if (result != FMOD_OK)
    {
        std::cout << "FMOD error! (" << result << ") " << FMOD_ErrorString(result) << std::endl;
        assert(result != FMOD_OK);
    }
}

void SoundSystem::stopAllSounds(bool force)
{
	std::map<std::string, SoundInfo*>::iterator iter;
	for (iter = _my_sounds.begin(); iter != _my_sounds.end(); ++iter) 
	{
		SoundInfo* info = iter->second;
		info->desired_volume = 0.0f;
		if (force) {
			info->volume = 0.0f;
			FMOD_Channel_SetVolume(info->channel, info->volume);
			FMOD_Channel_SetPaused(info->channel, true);
			info->isPlaying = false;
		}
	}
}

void SoundSystem::set3DPosition(SoundInfo * sound, const btVector3  &  position)
{
	FMOD_VECTOR pos = { position.getX(),position.getY(),position.getZ()};
	FMOD_VECTOR zero = { 0,0,0};
	FMOD_Channel_Set3DAttributes(sound->channel,&pos,&zero);
}

void SoundSystem::set3DPosition(std::string id_sound, const btVector3  &  position)
{
	SoundInfo * sound = getSoundInfo(id_sound);
	if (sound == NULL) return;
	return set3DPosition(sound,position);
}
