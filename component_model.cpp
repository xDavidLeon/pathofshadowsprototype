#include "component_model.h"

//ModelComponent::ModelComponent() : Component()
//{
//	_cmodel = NULL;
//	_mesh = NULL;
//	_materials = new VMaterials();
//	_offset = NULL;
//	_cmodel = NULL;
//	_cmodels.clear();
//}

ModelComponent::ModelComponent(Entity* e) : Component(e)
{
	_cmodel = NULL;
	_mesh = NULL;
	_meshLow = NULL;
	_materials = new VMaterials();
	_offset = NULL;
	diffuseColor = D3DXVECTOR4(1,1,1,1);
}

ModelComponent::ModelComponent(Entity* e, TMesh* mesh,VMaterials* materials) : Component(e)
{
	_cmodel = NULL;
	_mesh = mesh;
	_materials = materials;
	_offset = NULL;
	diffuseColor = D3DXVECTOR4(1,1,1,1);
}

ModelComponent::~ModelComponent(void)
{
	_mesh = NULL; //Destru�r la mesh depende del mesh manager
	if(_materials) delete _materials;
	delete _offset;
}

TMaterial * ModelComponent::getFirstMaterial(void)
{
	return _materials->at(0);
}

void ModelComponent::setMaterials(VMaterials* m)
{
	_materials = m;
}

TMesh* ModelComponent::getMesh()
{
	return _mesh;
}

TMesh* ModelComponent::getMeshLow()
{
	return _meshLow;
}

VMaterials* ModelComponent::getMaterials() const
{
	return _materials;
}

void ModelComponent::addMaterial(TMaterial * new_mat)
{
	_materials->push_back(new_mat);
}

void ModelComponent::setMesh(TMesh* m)
{
	_mesh = m;
}

void ModelComponent::setMeshLow(TMesh* m_l)
{
	_meshLow = m_l;
}

void ModelComponent::setOffset(btTransform * transform)
{
	_offset = transform;
}

btTransform* ModelComponent::getOffset(void)
{
	return _offset;
}

void ModelComponent::setCModel(CModel * m)
{
	_cmodel = m;
}
CModel* ModelComponent::getCModel(void)
{
	return _cmodel;
}

void ModelComponent::setCurrentTexture(TTexture texture)
{
	_materials->at(0)->main_texture = texture;
}

void ModelComponent::setCurrentMaterialName(string name)
{
	_materials->at(0)->name = name;

}

void ModelComponent::setCurrentTextures(TTexture texture)
{
	for(int i = 0; i < _materials->size(); i++)
	{
		_materials->at(i)->main_texture = texture;
	}
}

void ModelComponent::setCurrentMaterialsName(string name)
{
	for(int i = 0; i < _materials->size(); i++)
	{
		_materials->at(i)->name = name;
	}

}

void ModelComponent::deleteMaterial(const string& matName)
{
	//eliminar el material con ese nombre, si esta en el vector
	TTexture t = TTextureManager::get().getTexture(matName);
	if(!t) return;

	VMaterials::iterator iter;
	for(iter=_materials->begin(); iter!=_materials->end(); iter++)
	{
		if((*iter)->main_texture == t)
		{
			_materials->erase(iter);
			return;
		}
	}

}

void ModelComponent::addAlpha(float d)
{
	diffuseColor.w += d;
	if (diffuseColor.w > 1) diffuseColor.w = 1.0f;
	else if (diffuseColor.w < 0) diffuseColor.w = 0;
}
