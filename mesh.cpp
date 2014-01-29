#include "mesh.h"
#include "data_provider.h"
#include "globals.h"
#include "d3ddefs.h"

// -------------------------------------------------------
// maps to TDXCalVertex
static D3DVERTEXELEMENT9 skin_decl_elems[] = {
    {0,  0, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,     0},
	{0, 12, D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT,  0},
	{0, 16, D3DDECLTYPE_UBYTE4,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},
	{0, 20, D3DDECLTYPE_FLOAT2,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,     0},
	{0, 28, D3DDECLTYPE_FLOAT3,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,       0},
	{0, 40, D3DDECLTYPE_FLOAT4,  D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,     1},
    D3DDECL_END()
};
static LPDIRECT3DVERTEXDECLARATION9 skin_decl;

// -------------------------------------------------------
void TMesh::destroyVertexDeclarations( ) {
	if( skin_decl ) 
		skin_decl->Release(), skin_decl = NULL;
}

TMesh::TMesh( ) 
: vb( NULL ), ib( NULL ), fvf( 0 ), vertex_decl( 0 ) 
{ 
}

TMesh::~TMesh()
{
	destroyVB();
}

void TMesh::destroyVB( ) {
	if( vb )
		vb->Release(), vb = NULL;
	if( ib )
		ib->Release(), ib = NULL;
}

bool TMesh::load( CDataProvider &dp ) {

	dp.read( header );

	if( !header.isValid() ) {
		dbg( "Header of mesh %s is invalid\n", dp.getName() );
		return false;
	}

	groups.resize( header.ngroups );
	for( size_t i=0; i<header.ngroups; ++i ) {
		TGroupInfo &g = groups[ i ];
		dp.read( g.first_index );
		dp.read( g.nindices );

		// Has bone id's?
		if( header.version > 3 ) {
			int nbone_ids;
			dp.read( nbone_ids );
			g.bone_ids_used.resize( nbone_ids );
			if( nbone_ids )
				dp.readBytes( &g.bone_ids_used[ 0 ], sizeof( int ) * nbone_ids );
		}
	}

	assert( vb == NULL );

	size_t total_bytes = header.nvertexs * header.bytes_per_vertex;
	if( header.vertex_type == TVTPosition ) {
		fvf = D3DFVF_XYZ;
		assert( header.bytes_per_vertex == ( 3 ) * 4 );
	} else if( header.vertex_type == TVTPositionUV ) {
		fvf = D3DFVF_XYZ|D3DFVF_TEX1;
		assert( header.bytes_per_vertex == ( 3 + 2 ) * 4 );
	} else if (header.vertex_type == TVTPositionNormal) {
		fvf = D3DFVF_XYZ|D3DFVF_NORMAL;
		assert( header.bytes_per_vertex == ( 3 + 3 ) * 4 );
	} else if( header.vertex_type == TVTPositionNormalUV ) {
		fvf = D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1;
		assert( header.bytes_per_vertex == ( 3 + 3 + 2 ) * 4 );
	} else if( header.vertex_type == TVTPositionNormalUV2 ) {
		fvf = D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2;
		assert( header.bytes_per_vertex == ( 3 + 3 + 2*2 ) * 4 );
	} else if( header.vertex_type == TVTPositionNormalUVTangent ) {
		fvf = D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2|D3DFVF_TEXCOORDSIZE4(1);
		assert( header.bytes_per_vertex == ( 3 + 3 + 2 + 4 ) * 4 );
	} else if( header.vertex_type == TVTPositionNormalUV2Tangent ) {
		fvf = D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX3|D3DFVF_TEXCOORDSIZE4(2);
		assert( header.bytes_per_vertex == ( 3 + 3 + 2*2 + 4 ) * 4 );
	} else if( header.vertex_type == TVTPositionNormalUV3Tangent ) {
		fvf = D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX4|D3DFVF_TEXCOORDSIZE4(3);
		assert( header.bytes_per_vertex == ( 3 + 3 + 2*3 + 4 ) * 4 );
	} else if( header.vertex_type == TVTPositionNormalUVSkin ) {
		if( skin_decl == NULL ) {
			HRESULT hr = g_App.GetDevice()->CreateVertexDeclaration( skin_decl_elems, &skin_decl );
			assert( hr == D3D_OK );
		}
		vertex_decl = skin_decl;
		assert( header.bytes_per_vertex == ( 3 + 3 + 2 + 4 ) * 4 + 8 );
	} else {
		dbg( "Invalid vertex type %d\n", header.vertex_type );
		return false;
	}

	// Create vb
	if( FAILED( g_App.GetDevice()->CreateVertexBuffer( 
			total_bytes
		,	0		// usage
		,	fvf		// depends on the header.vertex_type
		,   D3DPOOL_DEFAULT
		,   &vb
		,   NULL ) ) )
        return false;

	void* pVertices;
	if( FAILED( vb->Lock( 0, total_bytes, &pVertices, 0 ) ) )
		return false;
	dp.readBytes( pVertices, total_bytes );
	vb->Unlock();

	// Create ib if the header says the mesh is indexed.
	if( header.nindices > 0 ) {
		// only support 2 or 4 bytes per index
		assert( header.bytes_per_index == 2 || header.bytes_per_index == 4 );
		size_t total_bytes_idxs = header.nindices * header.bytes_per_index;
		if( FAILED( g_App.GetDevice()->CreateIndexBuffer( 
				total_bytes_idxs	// total bytes
			,   0					// usage
			,	( header.bytes_per_index == 2 ) ? D3DFMT_INDEX16 : D3DFMT_INDEX32
			,   D3DPOOL_DEFAULT		//
			,   &ib					// The pointer to reference the index buffer in dx
			,   NULL ) ) )
			return false;

		void* pIndices;
		if( FAILED( ib->Lock( 0, total_bytes_idxs, &pIndices, 0 ) ) )
			return false;
		dp.readBytes( pIndices, total_bytes_idxs );
		ib->Unlock();
	}

	// Check we have read the full file. Now there should appear the footer
	// which, currently, it�s only a 4 bytes with the magic
	unsigned magic_footer;
	dp.read( magic_footer );
	if( magic_footer != THeader::valid_magic ) 
		return false;
	return true;
}

void TMesh::render( ) const {

	assert( vb );

	// Indica de donde van a salir los vertices. Salen de mi vertex buffer
	// a razon de header.bytes_per_vertex, desde el principio del mismo
	HRESULT hr = g_App.GetDevice()->SetStreamSource( 0, vb, 0, header.bytes_per_vertex );
	assert( hr == D3D_OK );
	// Indica el tipo de informacion que viene con cada vertice
	if( vertex_decl )
		hr = g_App.GetDevice()->SetVertexDeclaration( vertex_decl );
	else
		hr = g_App.GetDevice()->SetFVF( fvf );
	assert( hr == D3D_OK );

	if( header.nindices == 0 ) {
		hr = g_App.GetDevice()->DrawPrimitive( header.primitive_type, 0, header.nfaces );
		assert( hr == D3D_OK );

	} else {
		assert( ib );

		// Indicar de donde van a salir los indices
		HRESULT hr = g_App.GetDevice()->SetIndices( ib );
		assert( hr == D3D_OK );

		// La llamada de pintado indexada
		hr = g_App.GetDevice()->DrawIndexedPrimitive( 
			header.primitive_type
		  , 0
		  , 0
		  , header.nvertexs
		  , 0
		  , header.nfaces
		  );
		assert( hr == D3D_OK );
	}
}

// -------------
void TMesh::renderGroup( int group_id ) const {
	g_App.effect->CommitChanges();

	const TGroupInfo &g = groups[ group_id ];
	assert( vb );

	// Indica de donde van a salir los vertices. Salen de mi vertex buffer
	// a razon de header.bytes_per_vertex, desde el principio del mismo
	HRESULT hr = g_App.GetDevice()->SetStreamSource( 0, vb, 0, header.bytes_per_vertex );
	assert( hr == D3D_OK );
	// Indica el tipo de informacion que viene con cada vertice
    if( vertex_decl )
		hr = g_App.GetDevice()->SetVertexDeclaration( vertex_decl );
	else
		hr = g_App.GetDevice()->SetFVF( fvf );
	assert( hr == D3D_OK );

	assert( g.nindices > 0 );

	// Indicar de donde van a salir los indices
	hr = g_App.GetDevice()->SetIndices( ib );
	assert( hr == D3D_OK );

	assert( g.nindices % 3 == 0 );

	// La llamada de pintado indexada
	hr = g_App.GetDevice()->DrawIndexedPrimitive( 
		header.primitive_type
	  , 0
	  , 0
	  , header.nvertexs
	  , g.first_index
	  , g.nindices / 3
	  );
	assert( hr == D3D_OK );
}

void TMesh::renderDebugNormals( float sz, unsigned color ) const {
	void* pVertices;
	if( FAILED( vb->Lock( 0, header.bytes_per_vertex * header.nvertexs, &pVertices, 0 ) ) )
		return;

	if( header.vertex_type != TVTPositionNormalUV 
	 && header.vertex_type != TVTPositionNormalUV2 
	 && header.vertex_type != TVTPositionNormalUVTangent 
	 )
		return;

	// El buffer empieza en la posicion
	const char *position = (const char *) pVertices;
	// Skip 3 floats for the position 
	const char *normal = position + 3*sizeof(float);

	unsigned color_tangent = D3DCOLOR_XRGB( 0, 255, 0 );
	unsigned color_binormal = D3DCOLOR_XRGB( 255, 0, 0 );
	const char *tangent = NULL;
	if( header.vertex_type == TVTPositionNormalUVTangent )
		tangent = position + (3+3)*sizeof(float) + 2*sizeof(float);		// start of tangent

	for( unsigned i=0; i<header.nvertexs; ++i ) {
		const D3DXVECTOR3 *pos = (const D3DXVECTOR3*) position;
		const D3DXVECTOR3 *n3d = (const D3DXVECTOR3*) normal;
		const D3DXVECTOR3 *t3d = (const D3DXVECTOR3*) tangent;

		drawLineD3X( (*pos), (*pos) + (*n3d) * sz, color );
		if( t3d ) {
			drawLineD3X( (*pos), (*pos) + (*t3d) * sz, color_tangent );
			D3DXVECTOR3 b3d;
			D3DXVec3Cross( &b3d, n3d, t3d );		// binormal = normal x tangent
			const float *tangent_as_floats = (const float*) tangent;
			b3d *= tangent_as_floats[3];		// * tangent.w
			drawLineD3X( (*pos), (*pos) + b3d * sz, color_binormal );
		}
		
		position += header.bytes_per_vertex;
		normal += header.bytes_per_vertex;
		tangent += header.bytes_per_vertex;
	}
	
	vb->Unlock();
}
