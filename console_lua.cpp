#include "console_lua.h"

#include "iostatus.h"
#include "d3ddefs.h"
#include "logic_manager.h"
#include "system_camera.h"

ConsoleLua::ConsoleLua(void)
{
	_commands = std::vector<std::string>(5);
	_open = false;

	_kToggle = 220; // 'º'
	_kEnter = 13;
	_kBackspace = 8;

	_asciiMinValid = 32;
	_asciiMaxValid = 126;

	_x = 5;
	_y = g_App.GetHeight()*0.95;
	_y_jump = 20;
	_color = D3DCOLOR_ARGB( 255, 255, 255, 255 );

	_curr_command = 0;
}


ConsoleLua::~ConsoleLua(void)
{
}



void ConsoleLua::update()
{
	CIOStatus* ios = CIOStatus::instance();

	if(_open)
	{
		if(ios->becomesPressed_key(_kEnter))
		{
			_commands.pop_back();
			if(LogicManager::get().runScript(_commands.at(0).c_str()))
				_commands.insert(_commands.begin(), "OK");
			else 
				_commands.insert(_commands.begin(), "NOT OK");
		}
		else if(ios->becomesPressed_key(_kBackspace))
		{
			if(_commands.at(0).size()) _commands.at(0).pop_back();
		}
		else if(ios->becomesPressed_key(_kToggle))
		{
			_open = false;
			_commands.at(0) = "";

			if(CameraSystem::get().isCineActive())
				CameraSystem::get().setCinePaused(false);
		}
		else if(ios->becomesPressed(CIOStatus::TButton::KEYBOARD_ALT)){ _commands.at(0) = ""; }
		else if(_currentkey == 38 )
		{
			_curr_command += 1;
			if(_curr_command >= _commands.size()) _curr_command = _commands.size()-1;
			_commands.at(0) = _commands.at(_curr_command);
		}
		//else if(_currentkey == 40 ) //Por algun motivo el 40 es tanto flecha abajo como abrir parentesis  :D
		//{
		//	_curr_command -= 1;
		//	if(_curr_command < 1) _curr_command = 1;
		//	_commands.at(0) = _commands.at(_curr_command);
		//}
		else if(ios->becomesPressed(CIOStatus::TButton::KEYBOARD_ALT)){ _commands.at(0) = ""; }
		else
		{
			if(_currentkey>=_asciiMinValid && _currentkey<=_asciiMaxValid)
			{ 
				_commands.at(0).push_back(_currentkey); 
				_curr_command = 0;
			}
		}
		
	}
	else
	{
		if(ios->becomesPressed_key(_kToggle))
		{
			_open = true;

			if(CameraSystem::get().isCineActive())
				CameraSystem::get().setCinePaused(true);
		}
	}

	_currentkey = -1;
}

void ConsoleLua::render()
{
	if(!_open) return;

	int y = _y;
	printf2D( _x, y, _color, "> %s", _commands.at(0).c_str());

	for(unsigned i=1; i<_commands.size(); i++)
	{
		y-=_y_jump;
		printf2D( _x, y, _color, "%s", _commands.at(i).c_str());
	}
}

void ConsoleLua::setCurrentKey(int key)
{
	_currentkey = key;
}