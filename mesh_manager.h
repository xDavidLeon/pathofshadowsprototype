#ifndef INC_MESH_MANAGER_H_
#define INC_MESH_MANAGER_H_

#include <map>
#include <string>
#include <cassert>
#include "globals.h"
#include "mesh.h"
#include "data_provider.h"

class TMeshManager {

public:
	TMesh* getMesh( const std::string &name, bool assertIsOk = true );

	static TMeshManager & get() {
		static TMeshManager singleton;
		return singleton;
	}	
	TMesh *exists( const std::string &name ) const;
	void registerMesh( const std::string &unique_name, TMesh *new_mesh );
	void releaseAll( );

private:
	TMeshManager();
	~TMeshManager();
	
	typedef std::map< std::string, TMesh* > MMeshesByName;
	MMeshesByName meshes_by_name;
};


#endif

