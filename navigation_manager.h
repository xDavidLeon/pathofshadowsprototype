#ifndef NAV_MANAGER
#define NAV_MANAGER

#include "xml_parser.h"
#include "dijkstra.h"

class NavigationManager : public CXMLParser
{
	NavigationManager(void);
	~NavigationManager(void);

	// XML parser
	int _currentId;

	void onStartElement (const std::string &elem, MKeyValue &atts);
	void onEndElement (const std::string &elem);
	void onData (const std::string &data);

public:
	static NavigationManager& get()
	{
		static NavigationManager nM = NavigationManager();
		return nM;
	}

	void releaseNavGraph(){ DijkstraGraph::get().releaseGraph(); }
};



#endif