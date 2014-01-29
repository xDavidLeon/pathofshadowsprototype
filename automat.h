#ifndef AUTOMAT_H
#define AUTOMAT_H

// ai controllers using maps to function pointers for easy access and scalability. 
#include "entity.h"

// we put this here so you can assert from all controllers without including everywhere
#include <assert.h>	
#include <string>
#include <map>

using namespace std;

// states are a map to member function pointers, to 
// be defined on a derived class. 
class Automat;
class Entity;

typedef void (Automat::*statehandler)(float delta); 

class Automat
{
	protected:
		Entity* _entity;
		string _state;
		// the states, as maps to functions
		map<string,statehandler> _statemap;

	public:
		Automat(Entity* entity);

		/*unsigned color;
		float speed;
		float rotation_speed;*/
		float _clock;

		void changeState(const std::string& newstate);	// state we wish to go to
		virtual void init(){};							// resets the controller
		virtual void reset(){};
		virtual void update(float delta);				// recompute behaviour
		void addState(string,statehandler);
		const string& getState() const;

		virtual void render(){};

		Entity* getEntity();
		void setEntity(Entity* entity3D);
};

#endif