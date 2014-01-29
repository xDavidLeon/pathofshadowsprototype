#include "bullet_debug_drawer.h"
#include <stdio.h> //printf debugging
#include "d3ddefs.h"

BulletDebugDrawer::BulletDebugDrawer(void)
:m_debugMode(0)
{
	//hdc=CreateCompatibleDC(NULL);
	//D3DXCreateFont( g_App.GetDevice()
	//	          , 20
	//			  , 10
	//			  , FW_NORMAL
	//			  , 1
	//			  , FALSE
	//			  , DEFAULT_CHARSET
	//			  , OUT_DEFAULT_PRECIS
	//			  , DEFAULT_QUALITY
	//			  , DEFAULT_PITCH|FF_DONTCARE
	//			  , "Arial"
	//			  , &font
	//			  );
}


BulletDebugDrawer::~BulletDebugDrawer(void)
{
}

void    BulletDebugDrawer::drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
{
	drawLineD3X(D3DXVECTOR3(from),D3DXVECTOR3(to),D3DCOLOR_XRGB((int)color.getX()*255,(int)color.getY()*255,(int)color.getZ()*255));
}

void    BulletDebugDrawer::setDebugMode(int debugMode)
{
   m_debugMode = debugMode;
}

void    BulletDebugDrawer::draw3dText(const btVector3& location,const char* textString)
{
	//SelectObject(hdc, font);
	//D3DXCreateText(g_App.GetDevice(),hdc,textString,0.001f,0.4f,&mesh,NULL,NULL);
	//g_App.GetDevice()->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	//btTransform t;
	//t.setOrigin(location);
	//D3DXMATRIX m;
	//convertBulletTransform(&t,m);
	//g_App.GetDevice()->SetTransform(D3DTS_WORLD,&m);
	//mesh->DrawSubset(0);

}

void    BulletDebugDrawer::reportErrorWarning(const char* warningString)
{
   printf(warningString);
}

void    BulletDebugDrawer::drawContactPoint(const btVector3& pointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
{
      //btVector3 to=pointOnB+normalOnB*distance;
      //const btVector3&from = pointOnB;
      //glColor4f(color.getX(), color.getY(), color.getZ(), 1.0f);   
      
      //GLDebugDrawer::drawLine(from, to, color);
      
      //glRasterPos3f(from.x(),  from.y(),  from.z());
      //char buf[12];
      //sprintf(buf," %d",lifeTime);
      //BMF_DrawString(BMF_GetFont(BMF_kHelvetica10),buf);
}
