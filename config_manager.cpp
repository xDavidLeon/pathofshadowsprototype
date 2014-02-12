#include "config_manager.h"
#include "globals.h"
#include <Windows.h>
#include <cassert>
#include "main.h"

ConfigManager::ConfigManager(void)
{
}


ConfigManager::~ConfigManager(void)
{
}

void ConfigManager::readConfigFile()
{
	bool is_ok = xmlParseFile(configFile);
	if( !is_ok ) fatalErrorWindow(std::string("No se ha encontrado el archivo " + std::string(configFile) + " o bien contiene errores").c_str());
}

void ConfigManager::onStartElement (const std::string &elem, MKeyValue &atts)
{
	if (elem == "init_scene")
	{
		assert(atts.find("file") != atts.end());
		init_scene = atts["file"];
	}
	else if(elem == "invert_x")
	{
		assert(atts.find("value") != atts.end());
		if(atts["value"] == "yes") inv_x = -1;
		else inv_x = 1;
	}
	else if(elem == "invert_y")
	{
		assert(atts.find("value") != atts.end());
		if(atts["value"] == "yes") inv_y = -1;
		else inv_y = 1;
	}
}

void ConfigManager::readConfig()
{
	switch(g_App.GetLevel())
	{
		case 0: init_scene = "cementerio"; break;
		case 1: init_scene = "aldea"; break;
		case 2: init_scene = "patio"; break;
	}

	if(g_App.GetXinv() == 0) inv_x = 1;
	else inv_x = -1;

	if(g_App.GetYinv() == 0) inv_y = 1;
	else inv_y = -1;
}