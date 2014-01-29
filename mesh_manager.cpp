#include "mesh_manager.h"

// Devuelvo un mesh que no me puede tocar
TMesh *TMeshManager::exists( const std::string &name ) const {
	// Check if exists
	MMeshesByName::const_iterator it = meshes_by_name.find( name );

	// found
	if( it != meshes_by_name.end() ) {
		return it->second;	
	}
	return NULL;
}

void TMeshManager::registerMesh( const std::string &unique_name, TMesh *new_mesh ) {
	if( !exists( unique_name ) ) meshes_by_name[ unique_name ] = new_mesh;
	
}

TMeshManager::TMeshManager()
{
	meshes_by_name.clear();
}

TMeshManager::~TMeshManager()
{
	releaseAll();
}

TMesh* TMeshManager::getMesh( const std::string &name, bool assertIsOk  ) {
	TMesh *old_mesh = exists( name );
	if( old_mesh ) 
		return old_mesh;	

	// Not found. try to load
	std::string filename = "data/meshes/" + name + std::string( ".mesh" );

	if(!CFileDataProvider::exists(filename.c_str()))
	{
		if(assertIsOk) assert( fatal( "Mesh does not exist %s\n", filename.c_str() ) );
		return NULL;
	}

	CFileDataProvider fdp( filename.c_str() );
	TMesh *new_mesh = new TMesh;
	bool is_ok = new_mesh->load( fdp );
	if(is_ok)
	{
		registerMesh( name, new_mesh );
	}
	else
	{
		if(assertIsOk) assert( is_ok || fatal( "Can't load mesh %s\n", filename.c_str() ) );
		new_mesh = NULL;
	}

	return new_mesh;
}

void TMeshManager::releaseAll( ) {
	MMeshesByName::iterator it = meshes_by_name.begin();
	while( it != meshes_by_name.end() ) {
		delete it->second;
		++it;
	}
	meshes_by_name.clear( );
}
