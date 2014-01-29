#ifndef CONFIG_MGR
#define CONFIG_MGR

#include <string>
#include "xml_parser.h"

static const char* configFile = "data/config.xml";

class ConfigManager : public CXMLParser
{
	ConfigManager(void);
	~ConfigManager(void);

	void onStartElement (const std::string &elem, MKeyValue &atts);
	void onEndElement (const std::string &elem){}
	void onData (const std::string &data){}

public:
	static ConfigManager &get()
	{
		static ConfigManager cm;
		return cm;
	}

	std::string init_scene;
	int inv_x, inv_y;

	void readConfigFile();

};

#endif