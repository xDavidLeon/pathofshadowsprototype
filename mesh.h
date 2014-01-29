#ifndef INC_MESH_H_
#define INC_MESH_H_

#include <vector>
#include "aabb.h"

class CDataProvider;
#include "d3ddefs.h"

class TMesh {
public:
	typedef unsigned short TIndex;

	typedef std::vector<int> VInts;

	struct THeader {
		static const unsigned valid_magic = 0x2233EE11;
		static const unsigned min_supported_version = 3;
		static const unsigned current_version = 4;

		unsigned         magic;
		unsigned         version;
		unsigned         nfaces;
		unsigned         nvertexs;
		unsigned         vertex_type;
		unsigned         bytes_per_vertex;
		unsigned         bytes_per_index;
		D3DPRIMITIVETYPE primitive_type;
		unsigned         nindices;
		unsigned         flags;
		unsigned         ngroups;
		unsigned         magic_end;

		THeader( ) 
		: magic( valid_magic )
		, version( current_version )
		, nfaces( 0 )
		, nvertexs( 0 )
		, vertex_type( 0 )
		, bytes_per_vertex( 0 )
		, bytes_per_index( 0 )
		, primitive_type( D3DPT_FORCE_DWORD )
		, nindices( 0 )
		, flags( 0 )
		, ngroups( 0 )
		, magic_end( valid_magic )
		{}

		bool isValid() const {
			return magic == valid_magic
				&& version >= min_supported_version
				&& magic_end == valid_magic;
		}
	};

	// Informacion de cada grupo
	struct TGroupInfo {
		int first_index;
		int nindices;
		VInts bone_ids_used;		// used by the skin meshes
	};
	typedef std::vector< TGroupInfo > VGroupsInfo;

	enum TVertexType {
		TVTPosition	     	 = 1000
	,	TVTPositionUV        = 1001
	,	TVTPositionNormal	 = 1099
	,   TVTPositionNormalUV  = 1002
	,   TVTPositionNormalUV2 = 1003
	,   TVTPositionNormalUVSkin = 1004
	,   TVTPositionNormalUVTangent = 1010
	,   TVTPositionNormalUV2Tangent = 1011
	,   TVTPositionNormalUV3Tangent = 1012
	//,	TVTPositionNormalUVSkinTangent = 1012
	};

	// Para evitar que hagan copias de mi obj
	// declaro el ctor copia como private y no lo implemento
	TMesh( const TMesh &m );

	THeader					header;
	VGroupsInfo             groups;
	DWORD                   fvf;
	LPDIRECT3DVERTEXDECLARATION9 vertex_decl;
	LPDIRECT3DVERTEXBUFFER9 vb;
	LPDIRECT3DINDEXBUFFER9  ib;
	TAABB					aabb;

	TMesh( );
	~TMesh( );
	void destroyVB( );
	bool load( CDataProvider &dp );
	void render( ) const;
	void renderGroup( int n ) const;
	size_t getGroupsCount( ) const { return groups.size(); }

	void renderDebugNormals( float sz, unsigned color ) const;

		const TGroupInfo &getGroupInfo( int group_id ) const {
		return groups[ group_id ];
	}
	
	static void destroyVertexDeclarations( );
};


#endif
