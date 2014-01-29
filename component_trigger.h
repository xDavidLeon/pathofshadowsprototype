#ifndef TRIGGER_COMP
#define TRIGGER_COMP

#include "component.h"
#include "entity.h"

enum triggerType {
	ONENTER,
	ONEXIT,
	ONFIRSTENTER,
	ONFIRSTEXIT,
	SCENELOADER
};

class TriggerComponent : public Component
{
	float _radiusSq;
	triggerType _type;
	bool _playerInNow, _playerInBefore;
	std::string _command;

public:
	TriggerComponent(Entity* e);
	~TriggerComponent(void);

	void setRadius(float radius){_radiusSq=radius*radius;}
	float getRadiusSq()const{return _radiusSq;}
	void setType(const std::string &type_str);
	void setType(triggerType type){ _type = type; }
	void setCommand(const std::string &command_str){ _command = command_str; }

	void update(float delta);
};

#endif