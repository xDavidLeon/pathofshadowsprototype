#ifndef _LOGIC_MANAGER
#define _LOGIC_MANAGER

#include "globals.h"
#include "SLB/include/SLB/SLB.hpp"

extern "C"
{
	#include "lua\lua.h"
	#include "lua\lualib.h"
	#include "lua\lauxlib.h"
}

class LogicManager
{
	LogicManager(void);
	~LogicManager(void);

	SLB::Manager* _SLBManager;
	SLB::Script* _script;

	void create();

public:
	static LogicManager &get()
	{
		static LogicManager lm = LogicManager();
		return lm;
	}

	bool runScript(const char* script);
};

#endif