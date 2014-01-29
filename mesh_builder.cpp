#include "mesh.h"
#include "data_saver.h"
#include "data_provider.h"

struct TVertexTextured {
  D3DXVECTOR3 loc;
  D3DXVECTOR2 uv;
  void from( const D3DXVECTOR3 &aloc, D3DXVECTOR2 &auv ) {
    loc = aloc;
    uv = auv;
  }
};

void buildMeshQuadXY( CDataSaver &ds ) {

  TMesh::THeader header;
  header.nfaces = 2;
  header.vertex_type = TMesh::TVTPositionUV;
  header.bytes_per_index = 2;
  header.bytes_per_vertex = sizeof( TVertexTextured );
  header.ngroups = 1;
  header.nvertexs = 4;
  header.nindices = 3 * header.nfaces;
  header.primitive_type = D3DPT_TRIANGLELIST;

  TMesh::TGroupInfo group;
  group.first_index = 0;
  group.nindices = header.nindices;

  ds.write( header );
  ds.write( group.first_index );
  ds.write( group.nindices );
  ds.write( 0 );

  TVertexTextured v[4];
  v[0].from( D3DXVECTOR3( 0,1,0 ), D3DXVECTOR2( 0,0 ) );
  v[1].from( D3DXVECTOR3( 1,1,0 ), D3DXVECTOR2( 1,0 ) );
  v[2].from( D3DXVECTOR3( 1,0,0 ), D3DXVECTOR2( 1,1 ) );
  v[3].from( D3DXVECTOR3( 0,0,0 ), D3DXVECTOR2( 0,1 ) );
  ds.writeBytes( v, header.nvertexs * header.bytes_per_vertex );

  TMesh::TIndex idxs[6] = { 0, 1, 2, 0, 2, 3 };
  ds.writeBytes( idxs, header.nindices * header.bytes_per_index );

  ds.write( TMesh::THeader::valid_magic );

}
