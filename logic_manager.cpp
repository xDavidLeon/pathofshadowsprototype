#include "logic_manager.h"
#include <stdlib.h>
#include <conio.h>
#include <stdio.h>
#include "lua_helper.h"

LogicManager::LogicManager(void)
{
	create();
}


LogicManager::~LogicManager(void)
{
}


//Se le enchufan al SLB las clases con los atributos y métodos que queremos que estén visibles.
void BootLuaSLB(SLB::Manager *m)
{
	SLB::Class< LuaHelper >("luaMgr",m) //Se define el nombre que tendrá en lua la clase publicada

		// a comment/documentation for the class [optional]
		.comment("lh=lua helper, calls super cool functions in the game's engine")

		.constructor()

		// a method/function/value...
		// lo mismo de siempre con los nombres
		.set("tdc", &LuaHelper::toggleDbgCamera)
		.set("setCamDist", &LuaHelper::setCameraDistance)
		.set("createEnemy", &LuaHelper::createEnemy)
		.set("destroyEnemy", &LuaHelper::destroyEnemy)
		.set("loadScene", &LuaHelper::loadScene)
		.set("t_ai", &LuaHelper::toggleAI)
		.set("addCamToQueue", &LuaHelper::addCamToQueue)
		.set("r_graph", &LuaHelper::renderGraph)
		.set("enableEntity", &LuaHelper::enableEntity)
		.set("disableEntity", &LuaHelper::disableEntity)
		.set("setPlayerLife", &LuaHelper::setPlayerLife)
		.set("updatePlayerRespawn", &LuaHelper::updatePlayerRespawn)
		.set("t_anim", &LuaHelper::toggleAnimation)
		.set("t_entity", &LuaHelper::toggleEntity)
		.set("r_automat", &LuaHelper::toggleRAutomat)
		.set("r_ai", &LuaHelper::toggleRAI)
		.set("t_triggers", &LuaHelper::toggleTriggers)
		.set("setPlayerPos", &LuaHelper::setPlayerPos)
		.set("execCineSeq", &LuaHelper::execCineSeq)
		.set("playGoddessVoice", &LuaHelper::playGoddessVoice)
		.set("disableTutorial", &LuaHelper::disableTutorial)
		.set("enableTutorial", &LuaHelper::enableTutorial)
		.set("enableSub", &LuaHelper::enableSubtitle)
		;
}

void LogicManager::create()
{
	//Ahora SLB es la capa entre lua y nosotros
	_SLBManager = new SLB::Manager();
	BootLuaSLB(_SLBManager);

	_script = new SLB::Script(_SLBManager); //Se crea una instancia de "ejecutor de script"

	_script->doFile("./data/lua/init.lua");
	_script->doFile("./data/lua/functions.lua"); //Se ejecuta un script--> Los comandos se ejecutan, las funciones se quedan cargadas en _script
	
}

bool LogicManager::runScript(const char* str)
{
	try
	{
		_script->doString(str);
		return true;
	}
	catch (...){ return false; }
}



//SLB::Class< lm >("LogicManager",m) //Se define el nombre que tendrá en lua la clase publicada

	//	// a comment/documentation for the class [optional]
	//	.comment("This is our wrapper of LogicManager class")

	//	// empty constructor, we can also wrapper constructors with arguments using .constructor<TypeArg1,TypeArg2,..>()
	//	// también está el .destructor()
	//	.constructor()

	//	// a method/function/value...
	//	// lo mismo de siempre con los nombres
	//	.set("LogicPrint", &lm::LogicPrint)
	//	.set("TeleportPlayer", &lm::TeleportPlayer)
	//	.set("GetPlayerLife", &lm::GetPlayerLife)
	//	.property("numagents", &lm::numagents)
	//	;
