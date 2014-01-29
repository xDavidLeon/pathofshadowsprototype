#ifndef INC_SCENE_MANAGER_H_
#define INC_SCENE_MANAGER_H_

#include <map>
#include <string>
#include <cassert>
#include "globals.h"
#include "data_provider.h"
#include "xml_parser.h"
#include "material.h"

class Entity;
class ModelComponent;
class LightComponent;

class TSceneManager : public CXMLParser {

	typedef std::map< std::string, Entity* > MObjsByName;
	MObjsByName objs_by_name;

	// XML parser
	Entity *curr_obj;		// Used while loading scene
	std::string curr_element;		// Used while loading scene
	ModelComponent *curr_model;
	TMaterial *curr_material;
	LightComponent* curr_light;

	void onStartElement (const std::string &elem, MKeyValue &atts);
	void onEndElement (const std::string &elem);
	void onData (const std::string &data);

	float getFloat(const std::string & float_str);
	void getMatrix(const std::string & matrix, D3DXMATRIX &target);

public:
	std::string _subfileName;

	void render( );
	void update(float delta);
	TSceneManager( );
	static TSceneManager* &get() {
		static TSceneManager* sm = new TSceneManager();
		return sm;
	}

	Entity *getByName( const char *name );
	void releaseAll( );

	//Dejo aqui la dir light para poder acceder a su info, solo para debugar
	btVector3 dirLight;
};


#endif

