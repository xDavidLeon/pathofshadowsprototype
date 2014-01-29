#pragma once
#include "mesh.h"
#include "material.h"
#include "component.h"
#include "model.h"
#include <map>
#include <string>

class ModelComponent :
	public Component
{
public:
	ModelComponent(Entity* e);
	ModelComponent(Entity* e, TMesh* mesh,VMaterials* materials);
	~ModelComponent(void);

	TMaterial * getFirstMaterial(void);
	VMaterials*	getMaterials(void) const;
	TMesh*		getMesh(void);
	TMesh*		getMeshLow(void);
	void		setMesh(TMesh* m);
	void		setMeshLow(TMesh* m_l);
	void		setMaterials(VMaterials* m);
	void		addMaterial(TMaterial * new_mat);
	void		setOffset(btTransform * transform);
	btTransform*	getOffset(void);
	void		setCModel(CModel * m);
	CModel*		getCModel(void);
	void		setCurrentTexture(TTexture texture);
	void		setCurrentMaterialName(string name);
	void		setCurrentTextures(TTexture texture);
	void		setCurrentMaterialsName(string name);
	void		addAlpha(float d);
	void		deleteMaterial(const string& matName); //Para uso ultra especifico (apagar antorchas)


	std::map<std::string, bool> render_flags;
	D3DXVECTOR4 diffuseColor;


private:
	VMaterials*	_materials;
	TMesh		*_mesh, *_meshLow;
	btTransform	*	_offset;
	CModel*		_cmodel;
	std::map<std::string, CModel*> _cmodels;
};

