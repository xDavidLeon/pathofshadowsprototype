#ifndef GL_DEBUG_DRAWER_H
#define GL_DEBUG_DRAWER_H
#include <LinearMath\btIDebugDraw.h>

class BulletDebugDrawer :
	public btIDebugDraw
{
	int m_debugMode;
public:
	BulletDebugDrawer(void);
	~BulletDebugDrawer(void);

	virtual void   drawLine(const btVector3& from,const btVector3& to,const btVector3& color);
      
    virtual void   drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);
      
    virtual void   reportErrorWarning(const char* warningString);
      
    virtual void   draw3dText(const btVector3& location,const char* textString);
      
    virtual void   setDebugMode(int debugMode);
      
    virtual int    getDebugMode() const { return m_debugMode;}

private:
	//HDC hdc;
	//LPD3DXFONT font;
	//LPD3DXMESH mesh;
};

#endif
