#ifndef INC_MODEL_H_
#define INC_MODEL_H_

#include "cal3d/cal3d.h"
#include "xml_parser.h"
#include "texture_manager.h"
#include "entity.h"
#include "mesh.h"

typedef std::vector<CalBone *> BoneList;

// -----------------------------
CalQuaternion toCal( const D3DXQUATERNION &q );
CalQuaternion toCal( const btQuaternion &q );
	CalVector toCal( const D3DXVECTOR3 &p );
	CalVector toCal( const btVector3 &p );
D3DXQUATERNION toDX( const CalQuaternion &q );
	D3DXVECTOR3 toDX( const CalVector &p );
	D3DXVECTOR3 toDX( const btVector3 &p );
	btQuaternion toBullet( const CalQuaternion &q );
	btVector3 toBullet( const CalVector &p );
CalQuaternion buildQuat( const CalVector &axis, float angle );

// -----------------------------------------
class CCoreModel : public CalCoreModel, public CXMLParser  {
	float scale;
	// blend times de los ciclos
	// in_time y out_time de las acciones
	// las animaciones que son autolock

	void convertCalMeshToMCVMesh( int mesh_id, const std::string &out_filename );
	bool loadMeshFormat( const std::string &mesh_name );

public:
	static const int max_bones_supported = 64;

	std::map<std::string, int> _animations;
	int mesh_id;
	TTexture    model_diffuse;
	CCoreModel( const char* name );
	void load( const std::string &name );
	float getScale( ) const { return scale; }

	typedef std::vector< int > VInts;
	
	VInts       used_bones;		// bone_ids realmente usados
	std::vector<TMesh*> _objects;

	struct TTranslationAnimationInfo {
		std::vector< CalVector > positions;
		//CalCoreAnimation *core_anim;
		std::string name;
		CalVector getAtTime( float time ) {
			//buscar las dos claves e interpolar linealmente
			float frame = time * 30.0f;
			size_t int_frame = (size_t)frame;
			if(int_frame == positions.size()) --int_frame; // acciones bloqueadas
			CalVector position = positions[int_frame];
			CalVector next_position;
			if( int_frame < positions.size() - 1)
				next_position = positions[int_frame+1];
			else 
				next_position = positions[int_frame];
			float weight = frame - int_frame;
			position.blend(weight, next_position);
			return position;
		};
	};

	std::map<std::string, TTranslationAnimationInfo*> _animations_track;
	void readAnimationFile( std::string path, std::string name );

	void onStartElement (const std::string &elem, MKeyValue &atts);
	void onEndElement (const std::string &elem);
	void onData (const std::string &data){}

	TTranslationAnimationInfo* actual_track_animation_info;
};

// -----------------------------------------
class CModel : public CalModel {
public:
	float scale;
	Entity* _entity;
	CModel(CCoreModel* pCoreModel, Entity* entity);
	float getScale( ) const { return scale; }
	void renderSkelLines( ) const;
	void renderBone( const std::string &bone_name );
	CCoreModel* getCore();
	void init();
	void update( float delta, bool only_positon = false  );
	void getMatrixOfBone( D3DXMATRIX *out, int bone_id );
	void printMixer( ) const;

	void renderSubGroupWithShader( int sg_id, size_t index );

	CalVector prev_box_translation;
	CalVector delta_box_translation;
	bool box_active;

};

// -----------------------------------------
class CMixer : public CalMixer {
public:
	
	CMixer(CModel* pModel);
	void updateSkeleton();
};

#endif
