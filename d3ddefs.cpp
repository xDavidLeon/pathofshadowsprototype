#include "d3ddefs.h"
#include "globals.h"
#include "texture_manager.h"
#include "mesh_manager.h"

struct PlaneVertex {
	float x, y, z, w;
	float u, v;
};


void drawScreenPlane(const float screenWidth, const float screenHeight)
{
	static PlaneVertex axPlaneVertices[] =
	{
		{ 0,		 0,		  .5f, 1, 0 + .5f / screenWidth,	0 + .5f / screenHeight },
		{ screenWidth, 0,		  .5f, 1, 1 + .5f / screenWidth,	0 + .5f / screenHeight },
		{ screenWidth, screenHeight,    .5f, 1, 1 + .5f / screenWidth,	1 + .5f / screenHeight },
		{ 0,		 screenHeight, .5f, 1, 0 + .5f / screenWidth,	1 + .5f / screenHeight }
	};

	g_App.GetDevice()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
	g_App.GetDevice()->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, axPlaneVertices, sizeof(PlaneVertex));
}

void prepareMatrixFromRULP( D3DXMATRIX &matOutput, D3DXVECTOR3 *R, D3DXVECTOR3 *U, D3DXVECTOR3 *L, D3DXVECTOR3 *P )
{
    matOutput._11 = R->x;matOutput._12 = R->y;matOutput._13 = R->z;matOutput._14 = 0.f;
    matOutput._21 = U->x;matOutput._22 = U->y;matOutput._23 = U->z;matOutput._24 = 0.f;
    matOutput._31 = L->x;matOutput._32 = L->y;matOutput._33 = L->z;matOutput._34 = 0.f;
    matOutput._41 = P->x;matOutput._42 = P->y;matOutput._43 = P->z;matOutput._44 = 1.f;
}

void convertBulletTransform( const btTransform *bulletTransformMatrix, D3DXMATRIX& m_out)
{
   btVector3 R = bulletTransformMatrix->getBasis().getColumn(0);
   btVector3 U = bulletTransformMatrix->getBasis().getColumn(1);
   btVector3 L = bulletTransformMatrix->getBasis().getColumn(2);
   btVector3 P = bulletTransformMatrix->getOrigin();

   D3DXVECTOR3 vR, vU, vL, vP;
   vR.x = R.x();vR.y = R.y();vR.z = R.z();
   vU.x = U.x();vU.y = U.y();vU.z = U.z();
   vL.x = L.x();vL.y = L.y();vL.z = L.z();
   vP.x = P.x();vP.y = P.y();vP.z = P.z();

   prepareMatrixFromRULP( m_out, &vR, &vU, &vL, &vP );
}

void convertD3DXMatrix( const D3DXMATRIX *d3dMatrix, btTransform& m_out )
{
   m_out.setIdentity();
   btVector3 R,U,L,P;
   R.setX( d3dMatrix->_11 ); R.setY( d3dMatrix->_12 ); R.setZ( d3dMatrix->_13 );
   U.setX( d3dMatrix->_21 ); U.setY( d3dMatrix->_22 ); U.setZ( d3dMatrix->_23 );
   L.setX( d3dMatrix->_31 ); L.setY( d3dMatrix->_32 ); L.setZ( d3dMatrix->_33 );
   P.setX( d3dMatrix->_41 ); P.setY( d3dMatrix->_42 ); P.setZ( d3dMatrix->_43 );

   m_out.getBasis().setValue( R.x(), U.x(), L.x(), 
                                    R.y(), U.y(), L.y(), 
                                    R.z(), U.z(), L.z() );
   m_out.setOrigin( P );
}

void convertBulletVector3( const btVector3 *bulletVector3, D3DXVECTOR3& v_out )
{
	v_out = D3DXVECTOR3(bulletVector3->getX(),bulletVector3->getY(),bulletVector3->getZ());
}

void convertDXVector3( const D3DXVECTOR3& DXVector3, btVector3& v_out )
{
	v_out = btVector3(DXVector3.x, DXVector3.y, DXVector3.z);
}

void drawLineD3X( const D3DXVECTOR3 &src
			 , const D3DXVECTOR3 &dst
			 , unsigned color ) {
	CUSTOMVERTEX v[2];
	v[0].loc = src;
	v[0].color = color;
	v[1].loc = dst;
	v[1].color = color;
    g_App.GetDevice()->SetFVF( D3DFVF_CUSTOMVERTEX );
	g_App.GetDevice()->DrawPrimitiveUP( D3DPT_LINELIST
		                         , 1
								 , v
								 , sizeof(CUSTOMVERTEX)
								 );
}

void drawLine_bt(const btVector3& src, const btVector3& dst, unsigned color)
{
	drawLineD3X(D3DXVECTOR3(src.getX(), src.getY(), src.getZ()), D3DXVECTOR3(dst.getX(), dst.getY(), dst.getZ()), color);
}

void drawSphereD3X(const float radius ) {
	//	D3DXMATERIAL material;
	//material.MatD3D.Ambient.a = 0.0f;
 //   material.MatD3D.Ambient.r = 1.0f;
 //   material.MatD3D.Ambient.g = 0.0f;
 //   material.MatD3D.Ambient.b = 0.0f;
	LPD3DXMESH pMesh;
	D3DXCreateSphere(g_App.GetDevice(), radius, 20, 20, &pMesh, NULL);
	//g_App.GetDevice()->SetMaterial(&material.MatD3D);
	pMesh->DrawSubset(0);
	pMesh->Release();
}

void drawCubeAxis(const D3DXVECTOR3 & center, const D3DXVECTOR3 & half)
{
	drawLineD3X(D3DXVECTOR3(center.x - half.x, center.y,center.z),D3DXVECTOR3(center.x + half.x, center.y,center.z),D3DXCOLOR(0,255,255,255));
	drawLineD3X(D3DXVECTOR3(center.x, center.y - half.y,center.z),D3DXVECTOR3(center.x, center.y + half.y,center.z),D3DXCOLOR(0,255,255,255));
	drawLineD3X(D3DXVECTOR3(center.x, center.y,center.z-half.z),D3DXVECTOR3(center.x, center.y,center.z+half.z),D3DXCOLOR(0,255,255,255));
}

void printf2D ( float x, float y, unsigned color, const char *fmt, ... ) {
	char buf[ 1024 ];
	va_list ap;
	va_start( ap, fmt );
	int n = _vsnprintf_s( buf, sizeof( buf )-1, fmt, ap );
	assert( n < sizeof( buf ) );

	RECT rect;
	rect.left = (LONG)x;
	rect.right = (LONG)(x + 1000);
	rect.top = (LONG)y;
	rect.bottom = (LONG)(y + 1000);
	//drawRectangle(rect); //pintar rectangulo en espacio de pantallaaaaaa!!!!!!!!!!!
	
	INT nchars = g_App.GetDebugFont()->DrawText( NULL
		, buf
		, -1
		, &rect
		, DT_LEFT
		, color
		);
	assert( nchars >= 0 );

}

void printf2DBig ( float x, float y, unsigned color, const char *fmt, ... ) {
	char buf[ 1024 ];
	va_list ap;
	va_start( ap, fmt );
	int n = _vsnprintf_s( buf, sizeof( buf )-1, fmt, ap );
	assert( n < sizeof( buf ) );

	RECT rect;
	rect.left = (LONG)x;
	rect.right = (LONG)(x + 1000);
	rect.top = (LONG)y;
	rect.bottom = (LONG)(y + 1000);
	//drawRectangle(rect); //pintar rectangulo en espacio de pantallaaaaaa!!!!!!!!!!!
	
	INT nchars = g_App.GetDebugFontBig()->DrawText( NULL
		, buf
		, -1
		, &rect
		, DT_LEFT
		, color
		);
	assert( nchars >= 0 );

}

void drawRectangle(RECT rect)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;

	D3DXVECTOR3 tl, tr, bl, br;
	tl = D3DXVECTOR3(rect.left, rect.top, 0.0f);
	tr = D3DXVECTOR3(rect.right, rect.top, 0.0f);
	bl = D3DXVECTOR3(rect.left, rect.bottom, 0.0f);
	br = D3DXVECTOR3(rect.right, rect.bottom, 0.0f);

	DWORD black = D3DCOLOR_ARGB(255, 0,0,0);

	CUSTOMVERTEX sq_Vertices[] =
    {
        { tl, black, },
        { tr, black, },
        { br, black, },
        { bl, black, },
    };

    // Create the vertex buffer.
    if( FAILED( g_App.GetDevice()->CreateVertexBuffer( 4 * sizeof( CUSTOMVERTEX ),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
    {
        int kk = 2+2;
    }

    // Fill the vertex buffer.
    VOID* pVertices = NULL;
    if( FAILED( g_pVB->Lock( 0, sizeof( sq_Vertices ), ( void** )&pVertices, 0 ) ) )
        int kk = 2+2;
    memcpy( pVertices, sq_Vertices, sizeof( sq_Vertices ) );
    g_pVB->Unlock();

	g_App.GetDevice()->SetStreamSource( 0, g_pVB, 0, sizeof( CUSTOMVERTEX ) );
    g_App.GetDevice()->SetFVF( D3DFVF_CUSTOMVERTEX );
    g_App.GetDevice()->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 1 );
}



void drawAxis( ) {
	drawLineD3X( D3DXVECTOR3( 0,0,0 ), D3DXVECTOR3( 10,0,0 ), D3DCOLOR_XRGB( 255, 0, 0 ) );
	drawLineD3X( D3DXVECTOR3( 0,0,0 ), D3DXVECTOR3( 0,20,0 ), D3DCOLOR_XRGB( 0, 255, 0 ) );
	drawLineD3X( D3DXVECTOR3( 0,0,0 ), D3DXVECTOR3( 0,0,10 ), D3DCOLOR_XRGB( 0, 0, 255 ) );
}

void drawWaypoint(const float x, const float y, const float z ) {
	D3DXVECTOR3 waypoint(x,y,z);
	drawLineD3X( waypoint + D3DXVECTOR3( -1,0,-1 ), waypoint + D3DXVECTOR3( 1,0,1 ), D3DCOLOR_XRGB( 255, 255, 255 ) );
	drawLineD3X( waypoint + D3DXVECTOR3( -1,0,1 ), waypoint + D3DXVECTOR3( 1,0,-1 ), D3DCOLOR_XRGB( 255, 255, 255 ) );
}

void drawHeadWaypoint(const float x, const float y, const float z ) {
	D3DXVECTOR3 waypoint(x,y,z);
	drawLineD3X( waypoint + D3DXVECTOR3( -1,0,-1 ), waypoint + D3DXVECTOR3( 1,0,1 ), D3DCOLOR_XRGB( 255, 255, 0 ) );
	drawLineD3X( waypoint + D3DXVECTOR3( -1,0,1 ), waypoint + D3DXVECTOR3( 1,0,-1 ), D3DCOLOR_XRGB( 255, 255, 0 ) );
}

void drawGridXZ( int nblocks, int nsubsamples ) {
	int nsamples = nblocks * nsubsamples;
	unsigned color1 = D3DCOLOR_XRGB( 80, 80, 80 );
	unsigned color2 = D3DCOLOR_XRGB( 160, 160, 160 );
	for( int i=-nsamples; i<=nsamples; ++i ) {
		unsigned color = ( i % nsubsamples ) == 0 ? color2 : color1;
		drawLineD3X( D3DXVECTOR3( (FLOAT)i,0,(FLOAT)nsamples )
			    , D3DXVECTOR3( (FLOAT)i,0,(FLOAT)-nsamples )
				, color );
		drawLineD3X( D3DXVECTOR3( (FLOAT)nsamples,0,(FLOAT)i )
			    , D3DXVECTOR3( (FLOAT)-nsamples,0,(FLOAT)i )
				, color );
	}
}

void drawConeXZ_dx( const D3DXVECTOR3 &center, const D3DXVECTOR3 &front
			   , float half_fov
			   , unsigned color
			   , float zmax) {

	D3DXVECTOR3 left;
	D3DXVECTOR3 aux_up(0,1,0);
	D3DXVec3Cross( &left, &aux_up, &front );
	D3DXVec3Normalize( &left, &left );

	D3DXVECTOR3 ortho = tanf( half_fov ) * left;
	D3DXVECTOR3 d0 = front + ortho;
	D3DXVECTOR3 d1 = front - ortho;
	drawLineD3X( center, center + d0 * zmax, color );
	drawLineD3X( center, center + d1 * zmax, color );
	drawLineD3X( center, center + front * zmax, color );
}

void drawConeXZ_bt(const btVector3& center, const btVector3& front, float half_fov, unsigned color, float zmax)
{
	D3DXVECTOR3 c, f;
	c.x = center.getX(); c.y = center.getY(); c.z = center.getZ();
	f.x = -front.getX(); f.y = front.getY(); f.z = front.getZ();

	drawConeXZ_dx(c, f, half_fov, color, zmax);
}

// draws a cone in the current front direction 
void drawLocalConeXZ( float half_fov
			   , unsigned color
			   , float zmax) {
	D3DXVECTOR3 zero(0,0,0);
	D3DXVECTOR3 d0( tanf( half_fov ),0,1);
	D3DXVECTOR3 d1( -tanf( half_fov ),0,1);
	drawLineD3X( zero, zero + d0 * zmax, color );
	drawLineD3X( zero, zero + d1 * zmax, color );
}




//Pinta un quad (mesh) en espacio de pantalla con la textura que sea
//x e y est�n normalizados de -1 a 1. Son la pos. del CENTRO del quad con respecto al centro de la pantalla
//la quadM es por si se le quiere dar rotacion/escala
void renderTextureScreenSpace(const char* texture_name, float x, float y, const D3DXMATRIX& quadM)
{
	//Pintar en 2D Juan style
	//Guardamos matrices de dx
	D3DMATRIX dxView, dxProj;
	g_App.GetDevice()->GetTransform(D3DTS_VIEW, &dxView);
	g_App.GetDevice()->GetTransform(D3DTS_PROJECTION, &dxProj);

	//Creamos matrix ortogonal y la metemos en la view de dx
	D3DXMATRIX ortho;
	D3DXMatrixOrthoLH(&ortho, g_App.GetWidth()/2, g_App.GetHeight()/2, -1, 1);
	g_App.GetDevice()->SetTransform( D3DTS_VIEW, &ortho );

	//Metemos (x, y) aqui, asi valdra con usar valores normalizados de -1 a 1
	D3DXMATRIX desp; D3DXMatrixTranslation(&desp, x, y, 0.0f);
	g_App.GetDevice()->SetTransform( D3DTS_PROJECTION, &desp );

	//Le enchufamos la world de la textura
	g_App.GetDevice()->SetTransform( D3DTS_WORLD, &quadM );

	//Desactivamos culling
	g_App.GetDevice()->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);

	//Activamos canal alpha
	g_App.GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    g_App.GetDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    g_App.GetDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	
	g_App.GetDevice()->SetTexture( 0, TTextureManager::get().getTexture(texture_name));

	TMeshManager::get().getMesh("quadmesh")->render();
	
	g_App.GetDevice()->SetTexture( 0, NULL );
	g_App.GetDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );

	//Dejamos las matrices de dx como antes
	g_App.GetDevice()->SetTransform( D3DTS_VIEW, &dxView );
	g_App.GetDevice()->SetTransform( D3DTS_PROJECTION, &dxProj );
}

#include <time.h>
// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M-%S", &tstruct);

    return buf;
}

typedef void (*TRenderCB)();
void renderWithTechniqueCB( const char *tech_name, TRenderCB callback ) {
	HRESULT hr;

	hr = g_App.effect->SetTechnique( tech_name );
	if( hr != D3D_OK ) dbg( "Techname %s not found in effects\n", tech_name );
	assert( hr == D3D_OK );

	UINT npasses;
	hr = g_App.effect->Begin( &npasses, 0 );
	assert( hr == D3D_OK );

	// Para todos los pases de la technique
	for( UINT pass = 0; pass < npasses; ++pass ) {
		hr = g_App.effect->BeginPass( pass );
		assert( hr == D3D_OK );

		(*callback)();

		hr = g_App.effect->EndPass();
		assert( hr == D3D_OK );
	}

	g_App.effect->End();
}

float randomFloat(float a, float b)
{
	return ((b-a)*((float)rand()/RAND_MAX))+a;
}

void d3dxvec3Absolute(D3DXVECTOR3 & v)
{
	v.x = fabs(v.x);
	v.y = fabs(v.y);
	v.z = fabs(v.z);
}

std::string intToString(int x)
{
	return std::to_string(static_cast<long long>(x));
}

D3DXVECTOR3 lerp(const D3DXVECTOR3 start, const D3DXVECTOR3 end, float percent)
{
	return (start + percent*(end - start));
}

D3DXVECTOR3 slerp(const D3DXVECTOR3 start, const D3DXVECTOR3 end, float percent)
{
	float dot = D3DXVec3Dot(&start, &end);    
	clamp(dot, -1.0f, 1.0f);
	float theta = acos(dot)*percent;
	D3DXVECTOR3 RelativeVec = end - start*dot;
	D3DXVec3Normalize(&RelativeVec, &RelativeVec); 
	return ((start*cos(theta)) + (RelativeVec*sin(theta)));
}

D3DXVECTOR3 nlerp(const D3DXVECTOR3 start, const D3DXVECTOR3 end, float percent)
{
	D3DXVECTOR3 v;
	v = lerp(start,end,percent);
	D3DXVec3Normalize(&v,&v);
	return v;
}

D3DXVECTOR3 randomBetweenPoints(const D3DXVECTOR3 start, const D3DXVECTOR3 end)
{
	return D3DXVECTOR3(randomFloat(start.x,end.x),randomFloat(start.y,end.y),randomFloat(start.z,end.z));
}
