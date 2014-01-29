#ifndef d3ddefs_h
#define d3ddefs_h

#include"main.h"
#include <btBulletDynamicsCommon.h>
#include <cassert>
#include <string>

//constants
#define D3DFVF_CUSTOMVERTEX	(D3DFVF_XYZ | D3DFVF_DIFFUSE)
#define FVF_FLAGS			D3DFVF_XYZ | D3DFVF_TEX1
//#define FVF_VERTEX D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX3 | D3DFVF_TEXCOORDSIZE4(1) | D3DFVF_TEXCOORDSIZE3(2);
#define SAFE_DELETE( p ) { if( p ) { delete ( p ); ( p ) = NULL; } } 
#define SAFE_DELETE_ARRAY( p ) { if( p ) { delete[] ( p ); ( p ) = NULL; } } 
#define SAFE_RELEASE( p ) { if( p ) { ( p )->Release(); ( p ) = NULL; } }

const D3DXMATRIX d3dxidentity(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);

struct CUSTOMVERTEX
{
    D3DXVECTOR3 loc;      // The untransformed, 3D position for the vertex
    DWORD       color;        // The vertex color
};


//struct Vertex
//{
//	D3DXVECTOR3 p; // Vertex Position
//	D3DXVECTOR3 n; // Vertex Normal
//	float u, v; // Texture Coord
//	D3DXVECTOR4 t; // Tangent
//	D3DXVECTOR3 bn; // Binormal
//};
const std::string currentDateTime();
void prepareMatrixFromRULP( D3DXMATRIX &matOutput, D3DXVECTOR3 *R, D3DXVECTOR3 *U, D3DXVECTOR3 *L, D3DXVECTOR3 *P );
void convertBulletTransform( const btTransform *bulletTransformMatrix, D3DXMATRIX& m_out);
void convertD3DXMatrix( const D3DXMATRIX *d3dMatrix, btTransform& m_out );
void convertBulletVector3( const btVector3 *bulletVector3, D3DXVECTOR3& v_out );
void convertDXVector3( const D3DXVECTOR3& DXVector3, btVector3& v_out );
void drawScreenPlane( const float screenWidth, const float screenHeight);
void drawLineD3X( const D3DXVECTOR3 &src, const D3DXVECTOR3 &dst, unsigned color );
void drawLine_bt(const btVector3& src, const btVector3& dst, unsigned color);
void drawCubeAxis(const D3DXVECTOR3 & center, const D3DXVECTOR3 & half);
void drawSphereD3X(const float radius );
void printf2D ( float x, float y, unsigned color, const char *fmt, ... );
void printf2DBig ( float x, float y, unsigned color, const char *fmt, ... );

void drawRectangle(RECT rect);
void drawAxis( );
void drawWaypoint(const float x, const float y, const float z );
void drawHeadWaypoint(const float x, const float y, const float z );
void drawGridXZ( int nblocks, int nsubsamples );
void drawConeXZ_dx( const D3DXVECTOR3 &center, const D3DXVECTOR3 &front
			   , float half_fov
			   , unsigned color
			   , float zmax);
void drawLocalConeXZ( float half_fov
			   , unsigned color
			   , float zmax);


void renderTextureScreenSpace(const char* texture_name, float x, float y, const D3DXMATRIX& quadM);


void drawConeXZ_bt(const btVector3& center, const btVector3& front, float half_fov, unsigned color, float zmax);

typedef void (*TRenderCB)();
void renderWithTechniqueCB( const char *tech_name, TRenderCB callback );

float randomFloat(float a, float b);
std::string intToString(int x);
void d3dxvec3Absolute(D3DXVECTOR3 & v);
D3DXVECTOR3 lerp(const D3DXVECTOR3 start, const D3DXVECTOR3 end, float percent);
D3DXVECTOR3 slerp(const D3DXVECTOR3 start, const D3DXVECTOR3 end, float percent);
D3DXVECTOR3 nlerp(const D3DXVECTOR3 start, const D3DXVECTOR3 end, float percent);
D3DXVECTOR3 randomBetweenPoints(const D3DXVECTOR3 start, const D3DXVECTOR3 end);


#endif
