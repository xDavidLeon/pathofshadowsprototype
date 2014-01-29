#include "navigation_manager.h"

NavigationManager::NavigationManager(void)
{
	_currentId = -1;
}

NavigationManager::~NavigationManager(void)
{
}

void NavigationManager::onStartElement (const std::string &elem, MKeyValue &atts)
{
	if (elem == "node") 
	{
		assert(atts.find("id")    != atts.end());
		assert(atts.find("pos_x") != atts.end());
		assert(atts.find("pos_y") != atts.end());
		assert(atts.find("pos_z") != atts.end());

		int id = atoi(atts["id"].c_str());
		float x = atof(atts["pos_x"].c_str());
		float y = atof(atts["pos_y"].c_str());
		float z = atof(atts["pos_z"].c_str());

		//El nodo es el de fin de nivel?
		bool isFinal = false;
		if(atts.find("final") != atts.end()) isFinal = true;

		DijkstraGraph::get().addNode(id, new DijkstraNode(btVector3(x,y,z)), isFinal);
	}
	else if(elem == "edges")
	{
		assert(atts.find("id") != atts.end());
		assert(_currentId == -1);

		_currentId = atoi(atts["id"].c_str());
	}
	else if(elem == "edge")
	{
		assert(atts.find("id")		 != atts.end());
		assert(atts.find("distance") != atts.end());
		assert(_currentId != -1);

		int id = atoi(atts["id"].c_str());
		float dist = atof(atts["distance"].c_str());
		DijkstraGraph::get().getNodes().at(_currentId)->addNeighbour(id, dist);
	}
}

void NavigationManager::onEndElement (const std::string &elem)
{
	if(elem == "edges") _currentId = -1;
}

void NavigationManager::onData (const std::string &data)
{

}