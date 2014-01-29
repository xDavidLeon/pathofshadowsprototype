#ifndef COUNTER_CLOCK
#define COUNTER_CLOCK

#include <cassert>

class CounterClock //cron�metro
{
protected:
	float _count;
	float _target;

public:
	CounterClock() : _count(0.0f), _target(-1.0f){};
	
	void setTarget(float target)
	{
		_target = target;
		_count = 0.0f;
	}

	bool count(float delta) //devuelve true si ha llegado al target
	{
		assert(_target!=-1.0f);
		_count += delta;
		if(_count >= _target){  _target = -1.0f; return true;  }
		else return false;
	}

	bool hasTarget() const{ return _target != -1.0f; }
	float getCount() const { return _count; }
	void setCount(float count){ if(count > 0) _count = count; }
};


class CyclicCounterClock : public CounterClock //cron�metro c�clico: se le da un l�mite 1 vez y cuando llega se resetea
{
public:
	CyclicCounterClock(){};

	bool count(float delta) //devuelve true si ha llegado al target
	{
		assert(_target!=-1.0f);
		_count += delta;
		if(_count >= _target){  _count = 0.0f; return true;  }
		else return false;
	}
};

#endif