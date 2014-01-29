#pragma once

class Entity;
class CharacterControllerComponent;
class PlayerControllerSystem
{
public:
	static PlayerControllerSystem & get()
	{
		static PlayerControllerSystem singleton;
		return singleton;
	}

	bool update(float delta, bool orientateMesh=false, bool canRun=false, bool canMove=true);
	void setPlayer(Entity* p);
		
	float accelerationFactor;

	float getMaxSpeed() { return _maxSpeed; }
	float getCurrentSpeed() { return _currentSpeed; }
	float getDesiredSpeed() { return _desiredSpeed; }
	float getStdSpeed() { return _stdSpeed; }

private:
	PlayerControllerSystem(void);
	~PlayerControllerSystem(void);

	Entity*	_player;
	CharacterControllerComponent * _charController;

	float _stdSpeed;
	float _maxSpeed;
	float _rotationSpeed;
	float _currentSpeed;
	float _desiredSpeed;
	float _aimSpeed;
};

