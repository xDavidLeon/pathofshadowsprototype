#include "model.h"
#include "d3ddefs.h"
#include "iostatus.h"
#include "texture_manager.h"
#include "world.h"
#include "component_model.h"
#include "data_saver.h"
#include "data_provider.h"
#include "mesh.h"
#include "cal3d/coretrack.h"
#include "component_animation.h"
#include <set>
#include "entity_manager.h"
#include "mesh_manager.h"

// Para el VB 
// MUST be coherent with mesh.cpp D3DVERTEXELEMENT9 skin_decl_elems
struct TDXCalVertex {
	D3DXVECTOR3 pos;		// position
	unsigned char weights[4];		// DWORD color => 
	unsigned char bone_ids[4];
	float       u,v;
	D3DXVECTOR3 normal;
	D3DXVECTOR4 tangent;
};

// -----------------------------
CalQuaternion toCal( const D3DXQUATERNION &q ) {
	return CalQuaternion( q.x, q.y, q.z, -q.w );
}

CalQuaternion toCal( const btQuaternion &q ) {
	return CalQuaternion( q.getX(), q.getY(), q.getZ(), -q.getW() );
}

CalVector toCal( const D3DXVECTOR3 &p ) {
	return CalVector( p.x, p.y, p.z );
}

CalVector toCal( const btVector3 &p ) {
	return CalVector( p.getX(), p.getY(), p.getZ() );
}

D3DXQUATERNION toDX( const CalQuaternion &q ) {
	return D3DXQUATERNION( q.x, q.y, q.z, -q.w );
}
D3DXVECTOR3 toDX( const CalVector &p ) {
	return D3DXVECTOR3( p.x, p.y, p.z );
}

D3DXVECTOR3 toDX( const btVector3 &p ) {
	return D3DXVECTOR3( p.getX(), p.getY(), p.getZ() );
}

btQuaternion toBullet( const CalQuaternion &q ) {
	return btQuaternion( q.x, q.y, q.z, -q.w );
}
btVector3 toBullet( const CalVector &p ) {
	return btVector3( p.x, p.y, p.z );
}

CalQuaternion buildQuat( const CalVector &axis, float angle ) {
	float cos_half_angle = cosf( angle * 0.5f );
	float sin_half_angle = sinf( angle * 0.5f );
	return CalQuaternion( axis.x * sin_half_angle
		, axis.y * sin_half_angle
		, axis.z * sin_half_angle
		,          cos_half_angle );
}

// ------------------------------------------------------------------
typedef std::map< int, int > MInt2Int;
// Keeps track of face_id using the same set of bone ids
struct TFaceGroup {

	typedef std::vector< int > VInts;
	MInt2Int remap_bones;     // cal bone_ids to new remapped id
	VInts    used_bones;      // cal bone_ids
	VInts    face_ids;

	bool isBoneInUse( int bone_id ) const {
		return remap_bones.find( bone_id ) != remap_bones.end();
	}

	bool addFaceIfPosible( CalCoreSubmesh *sm, int fid ) {
		const CalCoreSubmesh::Face &cal_face = sm->getVectorFace()[ fid ];

		// This will hold the unique set of cal bone id's used by this face
		typedef std::set< int > TBonesSet;
		TBonesSet unique_bones;

		// For each 3 vertexs of the cal face
		for( int k=0; k<3; ++k ) {
			TMesh::TIndex cal_index = cal_face.vertexId[ k ];
			const CalCoreSubmesh::Vertex &cal_vtx = sm->getVectorVertex()[ cal_index ];

			// collect the unique set of used bones of this face
			for( size_t nf = 0; nf<cal_vtx.vectorInfluence.size(); ++nf ) {
				const CalCoreSubmesh::Influence &influence = cal_vtx.vectorInfluence[ nf ];
				unique_bones.insert( influence.boneId );
			}
		}

		// Now that we have the unique bones check how many are new in this group
		int n_new_bones = 0;
		TBonesSet::const_iterator i = unique_bones.begin();
		while( i != unique_bones.end() ) {
			if( !isBoneInUse( *i ) )
				++n_new_bones;
			++i;
		} 

		// Can we add the whole face? or there will be too many bones?
		if( n_new_bones + remap_bones.size() > CCoreModel::max_bones_supported ) 
			return false;

		// Keep the face id
		face_ids.push_back( fid );

		// Keep the bone_id's and assign a new bone_id directly
		// De los bones que realmente se estan usando, hacer un remap
		// Si uso los bones: 5,6,10
		// Los vertices que usaban el bone 5 -> usaran el bone 0
		// Los vertices que usaban el bone 6 -> usaran el bone 1
		// Los vertices que usaban el bone 10 -> usaran el bone 2
		TBonesSet::const_iterator ub = unique_bones.begin();
		while( ub != unique_bones.end() ) {
			if( remap_bones.find( *ub ) == remap_bones.end() ) {
				used_bones.push_back( *ub );
				remap_bones[ *ub ] = used_bones.size()-1;
			}
			assert( remap_bones.size() <= CCoreModel::max_bones_supported );
			++ub;
		}

		return true;
	}

};


void CCoreModel::convertCalMeshToMCVMesh( int mesh_id, const std::string &out_filename ) {

	CMemoryDataSaver mds_vtxs;
	CMemoryDataSaver mds_indices;
	TMesh::VGroupsInfo dx_groups;

	int total_nindices = 0;
	int total_nvertexs = 0;

	CalCoreMesh *mesh = getCoreMesh( mesh_id );

	// get the number of submeshes
	// 1 submesh == group of triangles with common properties/texture
	int submeshCount = mesh->getCoreSubmeshCount( );

	// Check the real bones used
	for(int submeshId = 0; submeshId < submeshCount; submeshId++) {
		CalCoreSubmesh *sm = mesh->getCoreSubmesh( submeshId );
		size_t nvertexs = sm->getVertexCount( );
		int    nfaces = sm->getFaceCount( );

		// stats of this submesh
		typedef std::vector< TFaceGroup > VFaceGroups;
		VFaceGroups face_groups;


		//nfaces = 100;

		// Splits faces in groups of unique used bones
		for( int fid = 0; fid<nfaces; ++fid ) {

			// Can we add the face to the already existing groups?
			bool added_to_group = false;
			for( size_t g=0; g<face_groups.size(); ++g ) {
				if( face_groups[g].addFaceIfPosible( sm, fid ) ) {
					added_to_group = true;
					break;
				}
			}

			// No?, then create a new group
			if( !added_to_group ) {
				face_groups.resize( face_groups.size() + 1 );
				added_to_group = face_groups.back().addFaceIfPosible( sm, fid );
				assert( added_to_group );
			}
		} 

		//dbg( "We need %ld groups for cal submesh %d\n", face_groups.size(), submeshId );

		// Prepare to export data
		size_t ntexcoords = sm->getVectorVectorTextureCoordinate( ).size();
		assert( ntexcoords > 0 ); 

		typedef std::vector<CalCoreSubmesh::TextureCoordinate> TCalTexCoords;
		const TCalTexCoords &cal_texcoords = sm->getVectorVectorTextureCoordinate( )[ 0 ];

		// Tengo que tener una texcoord para cada vertice
		assert( cal_texcoords.size() == nvertexs );

		// For each group, process the faces 
		for( size_t group_idx=0; group_idx<face_groups.size(); ++group_idx ) {
			TFaceGroup &fg = face_groups[group_idx];

			int nvertex_per_ninfluences[ 4 ] = { 0,0,0,0};

			MInt2Int remap_vertex_ids;

			// For each face id in this group
			for( size_t idx=0; idx<fg.face_ids.size(); ++idx ) {
				int fid = fg.face_ids[ idx ];
				const CalCoreSubmesh::Face &cal_face = sm->getVectorFace()[ fid ];

				// For each vertex of the face
				for( int k=0; k<3; ++k ) {
					TMesh::TIndex cal_index = cal_face.vertexId[ k ];
					TMesh::TIndex dx_index;

					// Is the vertex already used in this group?
					MInt2Int::const_iterator it = remap_vertex_ids.find( cal_index );

					// No, then we need to create a dx vertex
					if( it == remap_vertex_ids.end() ) {
						const CalCoreSubmesh::Vertex &cal_vtx = sm->getVectorVertex()[ cal_index ];
						TDXCalVertex                  dx_vtx;
						memset( &dx_vtx, 0x00, sizeof( TDXCalVertex ) );
						dx_vtx.normal = toDX( cal_vtx.normal );
						dx_vtx.pos    = toDX( cal_vtx.position );
						dx_vtx.u      = cal_texcoords[ cal_index ].u;
						dx_vtx.v      = cal_texcoords[ cal_index ].v;

						// No mas de 4 influencias por vertice!!!
						assert( cal_vtx.vectorInfluence.size() <= 4 );
						assert( cal_vtx.vectorInfluence.size() > 0 );

						// Add influences
						int total_influence = 0;
						for( size_t nf = 0; nf<cal_vtx.vectorInfluence.size(); ++nf ) {
							const CalCoreSubmesh::Influence &influence = cal_vtx.vectorInfluence[ nf ];
							unsigned char w8 = (unsigned char) (influence.weight*255.0f);

							MInt2Int::const_iterator bit = fg.remap_bones.find( influence.boneId );
							assert( bit != fg.remap_bones.end() );
							assert( bit->second < (int) fg.used_bones.size() );
							dx_vtx.bone_ids[ nf ] = bit->second;
							dx_vtx.weights [ nf ] = w8;
							total_influence += w8;
						}

						// Check the total influence still sum 1 in the range 0..255
						if( total_influence != 255 ) {
							int err = 255 - total_influence;		// 1 
							assert( err < 5 );					// We should more error than 1 unit per bone
							dx_vtx.weights[ 0 ] += err;
						}

						// Get stats of how many vertexs we have with 1, 2 3 or 4 influences
						nvertex_per_ninfluences[ cal_vtx.vectorInfluence.size()-1 ]++;
						mds_vtxs.write( dx_vtx );

						assert( remap_vertex_ids.size() < 65536 ); 

						// Assign a number and save it maybe another face uses the same vertex
						dx_index = total_nvertexs + remap_vertex_ids.size();
						remap_vertex_ids[ cal_index ] = dx_index;

					} else {

						dx_index = it->second;
					}

					// Save the dx index
					mds_indices.write( dx_index );
				}

			}

			//dbg( "  Group %ld uses %ld faces and %ld unique vertexs\n", group_idx, fg.face_ids.size(), remap_vertex_ids.size() );

			// Generate a description of the group
			TMesh::TGroupInfo dx_sm;
			dx_sm.first_index = total_nindices;
			dx_sm.nindices    = fg.face_ids.size() * 3;

			// Saved used bones as part of the sm info
			dx_sm.bone_ids_used = fg.used_bones;		

			dx_groups.push_back( dx_sm );

			// Update global count of indices and vertexs
			total_nindices += dx_sm.nindices;
			total_nvertexs += remap_vertex_ids.size();
			assert( total_nvertexs < 65536 ); 
		}
	}

	// Create a THeader
	TMesh::THeader header;
	header.bytes_per_index = sizeof( CalIndex );
	header.bytes_per_vertex = sizeof( TDXCalVertex );
	header.nindices    = total_nindices;
	header.ngroups     = dx_groups.size();
	header.nfaces      = total_nindices / 3;
	header.nvertexs    = total_nvertexs;
	header.vertex_type = TMesh::TVTPositionNormalUVSkin;
	header.primitive_type = D3DPT_TRIANGLELIST;

	// Generar fichero salida
	CFileDataSaver fds( out_filename.c_str() );
	fds.write( header );

	for( size_t i=0; i<dx_groups.size(); ++i ) {
		const TMesh::TGroupInfo &g = dx_groups[ i ];
		fds.write( g.first_index );
		fds.write( g.nindices );
		fds.write( (int) g.bone_ids_used.size() );
		fds.writeBytes( &g.bone_ids_used[0], g.bone_ids_used.size() * sizeof( int ) );
	}

	fds.writeBytes( &mds_vtxs.buffer[0], mds_vtxs.buffer.size() );
	fds.writeBytes( &mds_indices.buffer[0], mds_indices.buffer.size() );
	fds.write( TMesh::THeader::valid_magic );

	//dbg( "Cal mesh of %ld vtxs and %ld submeshes converted. Using %ld bones from %ld\n"
	//	, total_nvertexs
	//	, dx_groups.size()
	//	, used_bones.size(), getCoreSkeleton()->getVectorCoreBone().size() );
	//for( int i=0; i<4; ++i ) 
	//	dbg( "  With %d influences: %ld vtxs\n", i+1, nvertex_per_ninfluences[ i ] );
}

// -----------------------------
bool CCoreModel::loadMeshFormat( const std::string &mesh_name ) {


	std::string base = "data/characters/" + getName() + "/";
	std::string mesh_filename = base + mesh_name + ".mesh";
	if( !CFileDataProvider::exists(mesh_filename.c_str()) ) {

		//carga cal3d
		mesh_id = loadCoreMesh( base + mesh_name + ".cmf" );
		assert( mesh_id != -1 );
		//convertir
		if(TMeshManager::get().exists(mesh_name) == false) 		convertCalMeshToMCVMesh( mesh_id, mesh_filename );
	}

	CFileDataProvider fdp( mesh_filename.c_str() );

	TMesh* mesh = new TMesh;
	bool is_ok = mesh->load( fdp );
	_objects.push_back(mesh);
	assert( is_ok );



	// Give ownership of the mesh to the mesh manager
	TMeshManager::get().registerMesh( mesh_name, mesh );

	return is_ok;
}



// -----------------------------
CCoreModel::CCoreModel(const char* name ) : CalCoreModel( name ), scale( 1.0f ){ }

void CCoreModel::load( const std::string &name ) {
	//setName
	setName( name.c_str() );
	//Configurar cal3d para que la malla gire 90g sobre el eje X
	CalLoader::setLoadingMode( LOADER_ROTATE_X_AXIS );

	std::string base = "data/characters/" + name + "/";
	// Cargar la definicion del modelo: skeleton, animaciones

	int id;
	bool is_ok = loadCoreSkeleton( base + name + ".csf" );
	assert( is_ok );

	if( name.compare("shadow") == 0 )
	{
		id = loadCoreAnimation( base + "reborn.caf", "reborn" );
		assert( id >= 0 );
		_animations["reborn"] = id;
		id = loadCoreAnimation( base + "walk.caf", "walk" );
		assert( id >= 0 );
		_animations["walk"] = id;
		id = loadCoreAnimation( base + "run.caf", "run" );
		assert( id >= 0 );
		_animations["run"] = id;
		id = loadCoreAnimation( base + "idle.caf", "idle" );
		assert( id >= 0 );
		_animations["idle"] = id;
		id = loadCoreAnimation( base + "idle_inactive.caf", "idle_inactive" );
		assert( id >= 0 );
		_animations["idle_inactive"] = id;
		id = loadCoreAnimation( base + "decoy.caf", "decoy" );
		assert( id >= 0 );
		_animations["decoy"] = id;
		id = loadCoreAnimation( base + "aim.caf", "aim" );
		assert( id >= 0 );
		_animations["aim"] = id;
		id = loadCoreAnimation( base + "aimming.caf", "aimming" );
		assert( id >= 0 );
		_animations["aimming"] = id;
		id = loadCoreAnimation( base + "creating_shadow.caf", "creating_shadow" );
		assert( id >= 0 );
		_animations["creating_shadow"] = id;
		id = loadCoreAnimation( base + "stop_shadow.caf", "stop_shadow" );
		assert( id >= 0 );
		_animations["stop_shadow"] = id;
		id = loadCoreAnimation( base + "teleport.caf", "teleport" );
		assert( id >= 0 );
		_animations["teleport"] = id;
		id = loadCoreAnimation( base + "stop_teleport.caf", "stop_teleport" );
		assert( id >= 0 );
		_animations["stop_teleport"] = id;
		id = loadCoreAnimation( base + "vision.caf", "vision" );
		assert( id >= 0 );
		_animations["vision"] = id;
		id = loadCoreAnimation( base + "visioning.caf", "visioning" );
		assert( id >= 0 );
		_animations["visioning"] = id;
		id = loadCoreAnimation( base + "stop_vision.caf", "stop_vision" );
		assert( id >= 0 );
		_animations["stop_vision"] = id;
		id = loadCoreAnimation( base + "blend.caf", "blend" );
		assert( id >= 0 );
		_animations["blend"] = id;
		id = loadCoreAnimation( base + "blending.caf", "blending" );
		assert( id >= 0 );
		_animations["blending"] = id;
		id = loadCoreAnimation( base + "stop_blend.caf", "stop_blend" );
		assert( id >= 0 );
		_animations["stop_blend"] = id;
		id = loadCoreAnimation( base + "turn_around_right.caf", "turn_around_right" );
		assert( id >= 0 );
		_animations["turn_around_right"] = id;
		id = loadCoreAnimation( base + "turn_around_left.caf", "turn_around_left" );
		assert( id >= 0 );
		_animations["turn_around_left"] = id;
		id = loadCoreAnimation( base + "turn_around_right_legs.caf", "turn_around_right_legs" );
		assert( id >= 0 );
		_animations["turn_around_right_legs"] = id;
		id = loadCoreAnimation( base + "turn_around_left_legs.caf", "turn_around_left_legs" );
		assert( id >= 0 );
		_animations["turn_around_left_legs"] = id;
		
		id = loadCoreAnimation( base + "kill.caf", "kill" );
		assert( id >= 0 );
		_animations["kill"] = id;
		id = loadCoreAnimation( base + "kill2.caf", "kill2" );
		assert( id >= 0 );
		_animations["kill2"] = id;
		id = loadCoreAnimation( base + "kill3.caf", "kill3" );
		assert( id >= 0 );
		_animations["kill3"] = id;
		id = loadCoreAnimation( base + "kill_shadow.caf", "kill_shadow" );
		assert( id >= 0 );
		_animations["kill_shadow"] = id;
		id = loadCoreAnimation( base + "kill_shadow2.caf", "kill_shadow2" );
		assert( id >= 0 );
		_animations["kill_shadow2"] = id;
		id = loadCoreAnimation( base + "kill_air.caf", "kill_air" );
		assert( id >= 0 );
		_animations["kill_air"] = id;
		id = loadCoreAnimation( base + "kill_blend.caf", "kill_blend" );
		assert( id >= 0 );
		_animations["kill_blend"] = id;
		id = loadCoreAnimation( base + "kill_panic.caf", "kill_panic" );
		assert( id >= 0 );
		_animations["kill_panic"] = id;
		
		id = loadCoreAnimation( base + "recharge.caf", "recharge" );
		assert( id >= 0 );
		_animations["recharge"] = id;

		id = loadCoreAnimation( base + "dying.caf", "dying" );
		assert( id >= 0 );
		_animations["dying"] = id;

		id = loadCoreAnimation( base + "idle_legs.caf", "idle_legs" );
		assert( id >= 0 );
		_animations["idle_legs"] = id;
		id = loadCoreAnimation( base + "walk_legs.caf", "walk_legs" );
		assert( id >= 0 );
		_animations["walk_legs"] = id;
		id = loadCoreAnimation( base + "falling.caf", "falling" );
		assert( id >= 0 );
		_animations["falling"] = id;
		id = loadCoreAnimation( base + "land.caf", "land" );
		assert( id >= 0 );
		_animations["land"] = id;
		id = loadCoreAnimation( base + "accelerate.caf", "accelerate" );
		assert( id >= 0 );
		_animations["accelerate"] = id;
		id = loadCoreAnimation( base + "brake.caf", "brake" );
		assert( id >= 0 );
		_animations["brake"] = id;

		id = loadCoreAnimation( base + "idle_kill.caf", "idle_kill" );
		assert( id >= 0 );
		_animations["idle_kill"] = id;
		id = loadCoreAnimation( base + "walk_kill.caf", "walk_kill" );
		assert( id >= 0 );
		_animations["walk_kill"] = id;

		id = loadCoreAnimation( base + "idle_cg.caf", "idle_cg" );
		assert( id >= 0 );
		_animations["idle_cg"] = id;
		id = loadCoreAnimation( base + "open.caf", "open" );
		assert( id >= 0 );
		_animations["open"] = id;


		
		id = loadCoreAnimation( base + "summon.caf", "summon" );
		assert( id >= 0 );
		_animations["summon"] = id;

		//************** RELLENAMOS LOS ANIMATION TRACKS
		readAnimationFile( base + "kill.xml",  "kill");
		readAnimationFile( base + "kill2.xml",  "kill2");
		readAnimationFile( base + "kill3.xml",  "kill3");
		readAnimationFile( base + "kill_air.xml",  "kill_air");
		readAnimationFile( base + "kill_panic.xml",  "kill_panic");
		readAnimationFile( base + "dying.xml",  "dying");
		readAnimationFile( base + "open.xml",  "open");


		//**************


		loadMeshFormat( "shadow_sword" );

	}
	else if( name.compare("xu") == 0 || name.compare("xu_farolillo") == 0 || name.compare("xu_shield") == 0 )
	{
		id = loadCoreAnimation( base + "walk.caf", "walk" );
		assert( id >= 0 );
		_animations["walk"] = id;
		id = loadCoreAnimation( base + "run.caf", "run" );
		assert( id >= 0 );
		_animations["run"] = id;
		id = loadCoreAnimation( base + "idle.caf", "idle" );
		assert( id >= 0 );
		_animations["idle"] = id;	
		id = loadCoreAnimation( base + "idle2.caf", "idle2" );
		assert( id >= 0 );
		_animations["idle2"] = id;	
		id = loadCoreAnimation( base + "idle_walk.caf", "idle_walk" );
		assert( id >= 0 );
		_animations["idle_walk"] = id;
		id = loadCoreAnimation( base + "idle_guard.caf", "idle_guard" );
		assert( id >= 0 );
		_animations["idle_guard"] = id;	
		id = loadCoreAnimation( base + "idle_caution.caf", "idle_caution" );
		assert( id >= 0 );
		_animations["idle_caution"] = id;
		id = loadCoreAnimation( base + "idle_fight.caf", "idle_fight" );
		assert( id >= 0 );
		_animations["idle_fight"] = id;	
		id = loadCoreAnimation( base + "caution.caf", "caution" );
		assert( id >= 0 );
		_animations["caution"] = id;
		
		id = loadCoreAnimation( base + "kill.caf", "kill" );
		assert( id >= 0 );
		_animations["kill"] = id;
		id = loadCoreAnimation( base + "kill2.caf", "kill2" );
		assert( id >= 0 );
		_animations["kill2"] = id;
		id = loadCoreAnimation( base + "kill3.caf", "kill3" );
		assert( id >= 0 );
		_animations["kill3"] = id;
		id = loadCoreAnimation( base + "kill_sword.caf", "kill_sword" );
		assert( id >= 0 );
		_animations["kill_sword"] = id;
		id = loadCoreAnimation( base + "kill2_sword.caf", "kill2_sword" );
		assert( id >= 0 );
		_animations["kill2_sword"] = id;
		id = loadCoreAnimation( base + "kill3_sword.caf", "kill3_sword" );
		assert( id >= 0 );
		_animations["kill3_sword"] = id;
		id = loadCoreAnimation( base + "kill_shadow.caf", "kill_shadow" );
		assert( id >= 0 );
		_animations["kill_shadow"] = id;
		id = loadCoreAnimation( base + "kill_shadow2.caf", "kill_shadow2" );
		assert( id >= 0 );
		_animations["kill_shadow2"] = id;
		id = loadCoreAnimation( base + "kill_shadow_sword.caf", "kill_shadow_sword" );
		assert( id >= 0 );
		_animations["kill_shadow_sword"] = id;
		id = loadCoreAnimation( base + "kill_shadow2_sword.caf", "kill_shadow2_sword" );
		assert( id >= 0 );
		_animations["kill_shadow2_sword"] = id;
		id = loadCoreAnimation( base + "kill_air.caf", "kill_air" );
		assert( id >= 0 );
		_animations["kill_air"] = id;
		id = loadCoreAnimation( base + "kill_air_sword.caf", "kill_air_sword" );
		assert( id >= 0 );
		_animations["kill_air_sword"] = id;
		id = loadCoreAnimation( base + "kill_blend.caf", "kill_blend" );
		assert( id >= 0 );
		_animations["kill_blend"] = id;
		id = loadCoreAnimation( base + "kill_blend_sword.caf", "kill_blend_sword" );
		assert( id >= 0 );
		_animations["kill_blend_sword"] = id;
		
		id = loadCoreAnimation( base + "unsheathe.caf", "unsheathe" );
		assert( id >= 0 );
		_animations["unsheathe"] = id;
		id = loadCoreAnimation( base + "sheathe.caf", "sheathe" );
		assert( id >= 0 );
		_animations["sheathe"] = id;
		id = loadCoreAnimation( base + "death2.caf", "death2" );
		assert( id >= 0 );
		_animations["death2"] = id;

		id = loadCoreAnimation( base + "turn_right.caf", "turn_right" );
		assert( id >= 0 );
		_animations["turn_right"] = id;
		id = loadCoreAnimation( base + "turn_left.caf", "turn_left" );
		assert( id >= 0 );
		_animations["turn_left"] = id;
		id = loadCoreAnimation( base + "turn_around_left.caf", "turn_around_left" );
		assert( id >= 0 );
		_animations["turn_around_left"] = id;
		id = loadCoreAnimation( base + "turn_around_right.caf", "turn_around_right" );
		assert( id >= 0 );
		_animations["turn_around_right"] = id;

		
		id = loadCoreAnimation( base + "fall.caf", "fall" );
		assert( id >= 0 );
		_animations["fall"] = id;
		id = loadCoreAnimation( base + "panic.caf", "panic" );
		assert( id >= 0 );
		_animations["panic"] = id;
		id = loadCoreAnimation( base + "panic_up.caf", "panic_up" );
		assert( id >= 0 );
		_animations["panic_up"] = id;
		id = loadCoreAnimation( base + "kill_panic.caf", "kill_panic" );
		assert( id >= 0 );
		_animations["kill_panic"] = id;

		id = loadCoreAnimation( base + "talk_right.caf", "talk_right" );
		assert( id >= 0 );
		_animations["talk_right"] = id;

		id = loadCoreAnimation( base + "talk_left.caf", "talk_left" );
		assert( id >= 0 );
		_animations["talk_left"] = id;
		

		readAnimationFile( base + "unsheathe.xml",  "unsheathe");
		//************** RELLENAMOS LOS ANIMATION TRACKS
		readAnimationFile( base + "kill.xml",  "kill");
		readAnimationFile( base + "kill2.xml",  "kill2");
		readAnimationFile( base + "kill3.xml",  "kill3");
		readAnimationFile( base + "kill_sword.xml",  "kill_sword");
		readAnimationFile( base + "kill2_sword.xml",  "kill2_sword");
		readAnimationFile( base + "kill3_sword.xml",  "kill3_sword");
		readAnimationFile( base + "kill_panic.xml",  "kill_panic");
		readAnimationFile( base + "panic_up.xml",  "panic_up");
		readAnimationFile( base + "fall.xml",  "fall");
		

		//**************

		loadMeshFormat( name + "_sword" );
		
		if( name.compare("one") != 0 ) loadMeshFormat( name + "_funda" );

	}
	else if( name.compare("shadow_blend") == 0 )
	{
		id = loadCoreAnimation( base + "emerge.caf", "emerge" );
		assert( id >= 0 );
		_animations["emerge"] = id;
		id = loadCoreAnimation( base + "blending_bufanda.caf", "blending_bufanda" );
		assert( id >= 0 );
		_animations["blending_bufanda"] = id;
	}
	else if( name.compare("goddess") == 0 )
	{
		id = loadCoreAnimation( base + "flying.caf", "flying" );
		assert( id >= 0 );
		_animations["flying"] = id;
		id = loadCoreAnimation( base + "give.caf", "give" );
		assert( id >= 0 );
		_animations["give"] = id;
		id = loadCoreAnimation( base + "appear.caf", "appear" );
		assert( id >= 0 );
		_animations["appear"] = id;
		id = loadCoreAnimation( base + "disappear.caf", "disappear" );
		assert( id >= 0 );
		_animations["disappear"] = id;
	}
	else if( name.compare("shadow_snake") == 0 )
	{
		id = loadCoreAnimation( base + "kill_shadow.caf", "kill_shadow" );
		assert( id >= 0 );
		_animations["kill_shadow"] = id;
		id = loadCoreAnimation( base + "kill_shadow2.caf", "kill_shadow2" );
		assert( id >= 0 );
		_animations["kill_shadow2"] = id;
	}
	else if( name.compare("shadow_stele") == 0 )
	{
		id = loadCoreAnimation( base + "stop_teleport.caf", "stop_teleport" );
		assert( id >= 0 );
		_animations["stop_teleport"] = id;

	}
	else if( name.compare("raven") == 0 )
	{
		id = loadCoreAnimation( base + "idle.caf", "idle" );
		assert( id >= 0 );
		_animations["idle"] = id;
		id = loadCoreAnimation( base + "idle_fly.caf", "idle_fly" );
		assert( id >= 0 );
		_animations["idle_fly"] = id;
		id = loadCoreAnimation( base + "fly.caf", "fly" );
		assert( id >= 0 );
		_animations["fly"] = id;
		id = loadCoreAnimation( base + "reborn.caf", "reborn" );
		assert( id >= 0 );
		_animations["reborn"] = id;

		readAnimationFile( base + "reborn.xml",  "reborn");

	}
	else if( name.compare("animesh_door") == 0 )
	{
		id = loadCoreAnimation( base + "open.caf", "open" );
		assert( id >= 0 );
		_animations["open"] = id;
	}
	else
	{
		id = loadCoreAnimation( base + name + ".caf", name );
		assert( id >= 0 );
		_animations[name] = id;
	}

	loadMeshFormat( name );

	//mesh_id = loadCoreMesh( base + name + ".cmf" );
	scale = 1.0f;		// read this from the xml!!
}

void CCoreModel::readAnimationFile( std::string path, std::string name )
{
	actual_track_animation_info =  new TTranslationAnimationInfo();
	actual_track_animation_info->name = name;

	bool is_ok = xmlParseFile(path);
	if( !is_ok ) fatalErrorWindow(std::string("No se ha encontrado el archivo " + std::string(path) + " o bien contiene errores").c_str());
}

void CCoreModel::onStartElement (const std::string &elem, MKeyValue &atts)
{
	if (elem == "at")
	{
		assert(atts.find("time") != atts.end());
		CalVector position;
		position.x = atof(atts["x"].c_str());
		position.y = atof(atts["z"].c_str());
		position.z = -atof(atts["y"].c_str());

		actual_track_animation_info->positions.push_back(position);
	}


}
void CCoreModel::onEndElement (const std::string &elem) {
	if( elem == "animation" ) 
	{
		_animations_track[actual_track_animation_info->name] = actual_track_animation_info;
	}
}
// ----------------------
CModel::CModel(CCoreModel* pCoreModel, Entity* entity) 
	: CalModel( pCoreModel ) 
	, scale( 1.0f )
{ 
	_entity = entity;
	scale = pCoreModel->getScale();
	CMixer* mixer = new CMixer(this);
	setAbstractMixer(mixer);

	prev_box_translation = CalVector(0,0,0);
	delta_box_translation = CalVector(0,0,0);
	box_active = false;
}

void CModel::renderSkelLines( ) const {
	// Reservar espacio 2 puntos 3d por cada bone del modelo
	typedef std::vector< D3DXVECTOR3 > VPoints;
	VPoints points;
	size_t nbones = getSkeleton()->getCoreSkeleton( )->getVectorCoreBone().size();
	points.resize( nbones * 2 );
	// Pedir las lineas como array de floats
	getSkeleton( )->getBoneLines( &points[ 0 ].x );
	// Pintar las lineas, cada 2 puntos es una linea
	for( size_t i=0; i<points.size(); i += 2 ) {
		drawLineD3X( points[ i ], points[ i+1 ], 0xffffff00 );
	}
}

//
//void CModel::renderMesh( ) {
//	TTexture texture = TTextureManager::get().getTexture("hardcoded/shadow_d");
//	g_App.GetDevice()->SetTexture( 0,texture );
//
//
//	// get the renderer of the model
//	CalRenderer *pCalRenderer = getRenderer();
//
//	// begin the rendering loop
//	if(!pCalRenderer->beginRendering()) return;
//
//	// get the number of meshes
//	int meshCount = pCalRenderer->getMeshCount();
//
//	// render all meshes of the model
//	for(int meshId = 0; meshId < meshCount; meshId++)
//	{
//		// get the number of submeshes
//		int submeshCount = pCalRenderer->getSubmeshCount(meshId);
//
//		// render all submeshes of the mesh
//		for(int submeshId = 0; submeshId < submeshCount; submeshId++)
//		{
//
//			// select mesh and submesh for further data access
//			if(pCalRenderer->selectMeshSubmesh(meshId, submeshId))
//			{
//				int nvertexs = pCalRenderer->getVertexCount();
//				int nfaces   = pCalRenderer->getFaceCount();
//
//				struct VERTEX  {
//					D3DXVECTOR3    pos;
//					D3DXVECTOR3	normal;
//					FLOAT tu,tv;
//				};
//
//				VERTEX *pVertices = new VERTEX[ nvertexs ];
//				int vertexCount = pCalRenderer->getVerticesNormalsAndTexCoords(&pVertices->pos.x);
//				assert( vertexCount == nvertexs );
//
//				int nindices = nfaces * 3;
//				CalIndex *meshFaces = new CalIndex[ nindices ];
//				int faceCount = pCalRenderer->getFaces( meshFaces );
//				assert( faceCount == nfaces );
//
//				g_App.GetDevice()->SetFVF( D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1 );
//				g_App.GetDevice()->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
//				//          g_pD3DDevice->SetTexture( 0, (LPDIRECT3DTEXTURE9)pCalRenderer->getMapUserData(0));
//
//				HRESULT hr = g_App.GetDevice()->DrawIndexedPrimitiveUP( 
//					D3DPT_TRIANGLELIST,
//					0,
//					vertexCount,
//					nfaces,
//					meshFaces,		// the index data
//					sizeof( CalIndex ) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32,
//					pVertices,		// los datos (floats) de cada vertice
//					sizeof( VERTEX )
//					);
//
//				delete [] meshFaces;
//				delete [] pVertices;
//			}
//		}
//	}
//
//	// end the rendering
//	pCalRenderer->endRendering();
//
//
//	g_App.GetDevice()->SetTexture( 0, NULL );
//}
//


CCoreModel* CModel::getCore() {
	// Llamando a la clase de Cal3d
	CalCoreModel *cal_core_model = getCoreModel();
	return static_cast<CCoreModel*>( cal_core_model );
}

void CModel::init()
{
	TransformComponent* transform = EntityManager::get().getComponent<TransformComponent>(_entity);

	if( box_active )
		transform->moveLocal(toBullet(delta_box_translation));
	else
		prev_box_translation.clear();

	CalVector loc = toCal( transform->getPosition() );
	ModelComponent * model = EntityManager::get().getComponent<ModelComponent>(_entity);
	if( model->getMesh() )	loc.y -= model->getMesh()->aabb.half.y;


	loc /= scale;

	CalQuaternion q = toCal(transform->transform->getRotation());

	//// Pasarlas al mixer 
	getMixer()->setWorldTransform( loc, q );
}

//(el only_position por defecto es false, y sirve para actualizar el la posicion en el Mixer aunque el animationComponent este disabled )
void CModel::update( float delta, bool only_position ) {

	//btTransform* e_trans = EntityManager::get().getTransformComponent(_entity)->transform;

	////Lo siguiente es temporal
	//btTransform rot = btTransform(*e_trans);
	//rot.setOrigin(btVector3(0,0,0)); //Me cargo la pos para que la matrix contenga solo orientacion
	//
	//D3DXQUATERNION quatDX;
	//D3DXQuaternionRotationMatrix(&quatDX, &convertBulletTransform(&rot));

	//	// Pruebas para ensenyar a Dani
	//	ModelComponent * model = EntityManager::get().getComponent<ModelComponent>(_entity);
	//	btVector3 trans = e_trans->getOrigin();
	//	if( model->getMesh() )	trans.setY( trans.getY() - model->getMesh()->aabb.half.getY() );
	//	CalVector loc = toCal( convertBulletVector3(&trans) );

	//// Sacar de e, la pos y la rotation
	//CalQuaternion q = toCal(quatDX);
	////CalVector loc = toCal( convertBulletVector3(&e_trans->getOrigin()) );
	//

	TransformComponent* transform = EntityManager::get().getComponent<TransformComponent>(_entity);

	if( box_active )
		transform->moveLocal(toBullet(delta_box_translation));
	else
		prev_box_translation.clear();

	CalVector loc = toCal( transform->getPosition() );
	ModelComponent * model = EntityManager::get().getComponent<ModelComponent>(_entity);
	if( model->getMesh() )	loc.y -= model->getMesh()->aabb.half.y;


	loc /= scale;

	CalQuaternion q = toCal(transform->transform->getRotation());

	//// Pasarlas al mixer 
	getMixer()->setWorldTransform( loc, q );


	if(EntityManager::get().getComponent<AnimationComponent>(_entity)->enabled)
		CalModel::update( delta );

	else if(only_position)
	{
		getMixer()->updateSkeleton();
	}

	/*getMixer()->getWorldTransform(loc, q);

	e_trans->setOrigin(toBullet(loc));
	e_trans->setRotation(toBullet(q));*/
}

void CModel::getMatrixOfBone( D3DXMATRIX *out, int bone_id ) {
	CalBone *bone = getSkeleton()->getBone( bone_id );
	assert( bone );
	CalVector trans = bone->getTranslationAbsolute( );
	const CalQuaternion &quat = bone->getRotationAbsolute( );

	trans *= scale;

	D3DXQUATERNION quat_dx( quat.x, quat.y, quat.z, -quat.w );
	D3DXMatrixRotationQuaternion( out, &quat_dx );
	out->_41 = trans.x;
	out->_42 = trans.y;
	out->_43 = trans.z;
}

void CModel::printMixer( ) const {

	// Mixer 
	// Pintar el nombre de todos los ciclos, sus weights, sus tiempos
	// y su state
	// Y lo mismo para las acciones
}

void CModel::renderBone( const std::string &bone_name ) {
	int bone_id = getCoreModel()->getBoneId( bone_name );
	if( bone_id == -1 )
		return;
	D3DXMATRIX mrot;
	getMatrixOfBone( &mrot, bone_id );
	g_App.GetDevice()->SetTransform( D3DTS_WORLD, &mrot );
	drawAxis( );
}

void CModel::renderSubGroupWithShader( int sg_id, size_t object ) {

	// subir las matrices
	struct TMatrix3x4 {
		D3DXVECTOR4 row1; 
		D3DXVECTOR4 row2; 
		D3DXVECTOR4 row3; 
	};
	const int floats_per_bone = 12;

	TMesh *mesh = getCore()->_objects.at(object);
	//TMesh *mesh = getCore()->mesh;
	const TMesh::TGroupInfo &g = mesh->getGroupInfo( sg_id );

	TMatrix3x4 bones[ CCoreModel::max_bones_supported ];
	TMatrix3x4 *out_matrices = bones;
	size_t nbones = g.bone_ids_used.size();
	assert( nbones <= CCoreModel::max_bones_supported );
	for( size_t i=0; i<nbones; ++i, ++out_matrices ) {
		int bone_id = g.bone_ids_used[ i ] ;
		CalBone *pBone = getSkeleton()->getBone( bone_id );
		const CalMatrix& rot   = pBone->getTransformMatrix();
		const CalVector& trans = pBone->getTranslationBoneSpace();

		out_matrices->row1.x = rot.dxdx;
		out_matrices->row1.y = rot.dxdy;
		out_matrices->row1.z = rot.dxdz;

		out_matrices->row2.x = rot.dydx;
		out_matrices->row2.y = rot.dydy;
		out_matrices->row2.z = rot.dydz;

		out_matrices->row3.x = rot.dzdx;
		out_matrices->row3.y = rot.dzdy;
		out_matrices->row3.z = rot.dzdz;

		out_matrices->row1.w = trans.x;
		out_matrices->row2.w = trans.y;
		out_matrices->row3.w = trans.z;
	}

	HRESULT hr = g_App.effect->SetFloatArray( "bones", &bones[0].row1.x, nbones * floats_per_bone );
	assert( hr == D3D_OK );

	// mandar a pintar la malla
	getCore()->_objects.at(object)->renderGroup( sg_id );
}

// ----------------------
CMixer::CMixer(CModel* pModel) 
	: CalMixer( pModel ) 
{ 

}

void CMixer::updateSkeleton()
{
	// get the skeleton we need to update
	CalSkeleton* pSkeleton = m_pModel->getSkeleton();
	if(pSkeleton == 0) return;

	// clear the skeleton state
	pSkeleton->clearState();

	// get the bone vector of the skeleton
	BoneList& vectorBone = pSkeleton->getVectorBone();

	// For each bone, reset the transform-related variables to the core (bind pose) bone position and orientation.
	BoneList::iterator curIter = vectorBone.begin();
	BoneList::iterator endIter = vectorBone.end();
	for( ; curIter != endIter; ++curIter)
	{
		(*curIter)->setCoreTransformStateVariables();
	}

	// The bone adjustments are "replace" so they have to go first, giving them
	// highest priority and full influence.  Subsequent animations affecting the same bones, 
	// including subsequent replace animations, will have their incluence attenuated appropriately.
	applyBoneAdjustments();

	((CModel*)getCalModel())->box_active = false;

	// loop through all animation actions
	CalAnimationAction* pAction = NULL;
	std::list<CalAnimationAction *>::iterator iteratorAnimationAction;
	for(iteratorAnimationAction = m_listAnimationAction.begin(); iteratorAnimationAction != m_listAnimationAction.end(); ++iteratorAnimationAction)
	{
		pAction = *iteratorAnimationAction;
		if (pAction->on())
		{
			// get the core animation instance
			CalCoreAnimation* pCoreAnimation = pAction->getCoreAnimation();

			// get the list of core tracks of above core animation
			std::list<CalCoreTrack *>& listCoreTrack = pCoreAnimation->getListCoreTrack();

			// loop through all core tracks of the core animation
			CalCoreTrack* pTrack = NULL;
			std::list<CalCoreTrack *>::iterator iteratorCoreTrack;
			for(iteratorCoreTrack = listCoreTrack.begin(); iteratorCoreTrack != listCoreTrack.end(); ++iteratorCoreTrack)
			{
				pTrack = *iteratorCoreTrack;

				// get the appropriate bone of the track
				CalBone* pBone = vectorBone[pTrack->getCoreBoneId()];

				// get the current translation and rotation
				CalVector translation;
				CalQuaternion rotation;
				pTrack->getState(pAction->getTime(), translation, rotation);

				// Replace and CrossFade both blend with the replace function.
				CalAnimation::CompositionFunction compFunc = pAction->getCompositionFunction();
				bool replace = compFunc != CalAnimation::CompositionFunctionAverage && compFunc != CalAnimation::CompositionFunctionNull;
				float scale = pAction->getScale();

				CModel* cmodel = (CModel*)getCalModel();
				CCoreModel* ccore_model  = cmodel->getCore();

				if (ccore_model->_animations_track.find(pCoreAnimation->getName().c_str()) != ccore_model->_animations_track.end())
				{

					cmodel->box_active = true;
					CCoreModel::TTranslationAnimationInfo* track_animation_info = ccore_model->_animations_track[pCoreAnimation->getName().c_str()];
					CalVector box_translation = track_animation_info->getAtTime(pAction->getTime());

					// MCV. Si es un root bone...
					if( pBone->getCoreBone()->getParentId() == -1 ) {

						//translation.y = 0; //PROVISIONAL SI NO SE HACE BIEN LA BOX
						//translation.x = 0;


						//dbg( "box %f %f %f\n", box_translation.x, box_translation.y, box_translation.z );

						//dbg( "before %f\n", /*translation.x, translation.y, */translation.z);

						translation -= box_translation;
						//translation = CalVector(0,0,0);

						//dbg( "after %f\n", /*translation.x, translation.y, */translation.z);

						//dbg( "translation %f %f %f box %f %f %f\n", translation.x, translation.y, translation.z, box_translation.x, box_translation.y, box_translation.z );


						// How much we have moved since the last frame
						cmodel->delta_box_translation = box_translation - cmodel->prev_box_translation;


						//dbg( "deltabox %f %f %f\n", cmodel->delta_box_translation.x, cmodel->delta_box_translation.y, cmodel->delta_box_translation.z);

						// Keep it Updated to the next frame
						cmodel->prev_box_translation = box_translation; 


						// translation.z = 0.0;
						//translation.x = 0.0;
						//translation.y = 0.0;
						//	  translation.x = 2.0;
						//		  rotation.clear();
						//float angle = ( 45.0f ) * 3.1415f / 180.0f;;
						//float cos_half_angle = cosf( angle * 0.5f );
						//float sin_half_angle = sinf( angle * 0.5f );
						//CalQuaternion xtra( 0, sin_half_angle, 0, cos_half_angle );
						//rotation *= xtra;
					}
				}

				bool absoluteTrans = pTrack->getTranslationRequired();
				pBone->blendState( pAction->getWeight(), translation, rotation, scale, replace, pAction->getRampValue(), absoluteTrans );
			}
		}
	}

	// lock the skeleton state
	pSkeleton->lockState();

	// loop through all animation cycles
	CalAnimationCycle* pAnimCycle = NULL;
	std::list<CalAnimationCycle *>::iterator iteratorAnimationCycle;
	for(iteratorAnimationCycle = m_listAnimationCycle.begin(); iteratorAnimationCycle != m_listAnimationCycle.end(); ++iteratorAnimationCycle)
	{
		pAnimCycle = *iteratorAnimationCycle;

		// get the core animation instance
		CalCoreAnimation* pCoreAnimation = pAnimCycle->getCoreAnimation();

		// calculate adjusted time
		float animationTime;
		if(pAnimCycle->getState() == CalAnimation::STATE_SYNC)
		{
			if(m_animationDuration == 0.0f)
			{
				animationTime = 0.0f;
			}
			else
			{
				animationTime = m_animationTime * pCoreAnimation->getDuration() / m_animationDuration;
			}
		}
		else
		{
			animationTime = pAnimCycle->getTime();
		}

		// get the list of core tracks of above core animation
		std::list<CalCoreTrack *>& listCoreTrack = pCoreAnimation->getListCoreTrack();

		// loop through all core tracks of the core animation
		CalCoreTrack* pTrack = NULL;
		std::list<CalCoreTrack *>::iterator iteratorCoreTrack;
		for(iteratorCoreTrack = listCoreTrack.begin(); iteratorCoreTrack != listCoreTrack.end(); ++iteratorCoreTrack)
		{
			pTrack = *iteratorCoreTrack;

			// get the appropriate bone of the track
			CalBone *pBone = vectorBone[pTrack->getCoreBoneId()];

			// get the current translation and rotation
			CalVector translation;
			CalQuaternion rotation;
			pTrack->getState(animationTime, translation, rotation);


			// blend the bone state with the new state
			bool absoluteTrans = pTrack->getTranslationRequired();
			pBone->blendState(pAnimCycle->getWeight(), translation, rotation, 1.0f, false, 1.0f, absoluteTrans);
		}
	}

	// lock the skeleton state
	pSkeleton->lockState();

	// MCV Tocar la transform del root bone para llevarlo directamente
	// al world coords
	CalBone* root_bone = pSkeleton->getBone( 0 );
	CalVector local_trans = root_bone->getTranslation();
	CalQuaternion local_rot = root_bone->getRotation();
	// Pasar el offset local de la animacion a espacio de mundo
	// Los 30cm locales, hay que rotarlos antes de sumarlos a la posicion
	// absoluta donde queremos llevar el modelo
	CalVector world_trans = local_trans;
	world_trans *= m_world_rotation;

	// Now, update the world coords
	root_bone->setRotation( m_world_rotation * local_rot );
	root_bone->setTranslation( world_trans + m_world_position );

	// let the skeleton calculate its final state
	pSkeleton->calculateState();
}

