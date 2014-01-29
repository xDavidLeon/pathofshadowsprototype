#ifndef CONSOLE_LUA
#define CONSOLE_LUA

#include <vector>
#include <string>
#include <set>

class ConsoleLua
{
	std::vector<std::string> _commands;
	int _curr_command;
	bool _open;
	int _currentkey;
	int _kToggle, _kEnter, _kBackspace;
	int _asciiMinValid, _asciiMaxValid;

	int _x, _y, _y_jump ;
	unsigned _color;

	ConsoleLua(void);
	~ConsoleLua(void);

public:
	static ConsoleLua &get()
	{
		static ConsoleLua cl = ConsoleLua();
		return cl;
	}

	void update();
	void render();

	void setCurrentKey(int key);

	bool isOpen(){return _open;}
};

#endif