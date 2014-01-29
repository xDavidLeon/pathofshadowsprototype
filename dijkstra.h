#ifndef DIJKSTRA
#define DIJKSTRA

#include <map>
#include <vector>
#include "btBulletDynamicsCommon.h"
#include <cassert>
#include <deque>

class DijkstraNode
{
	//"constantes"
	btVector3 _pos;
	std::map< int, float > _neighbours;

public:
	DijkstraNode(const btVector3& pos) : _pos(pos), _bestParent(-1), _bestDistance(-1){};
	~DijkstraNode(){ _neighbours.clear(); }

	//modificables por la funci�n computePath
	int _bestParent;
	float _bestDistance;

	void addNeighbour(int neigh_id, float distance)
	{
		assert(_neighbours.find(neigh_id) == _neighbours.end()); //No deber�a estar ya
		_neighbours.insert(std::pair<int,float>(neigh_id, distance));
	}

	const btVector3& getPos() const{ return _pos; }
	std::map< int, float >& getNeighbours(){ return _neighbours; }
};


class DijkstraGraph
{
	std::map< int, DijkstraNode* > _nodes;
	int _finalNodeId;

	DijkstraGraph(void);
	~DijkstraGraph(void);

public:
	static DijkstraGraph& get() //de momento es singleton: habr� s�lo 1
	{
		static DijkstraGraph dg = DijkstraGraph();
		return dg;
	}

	void addNode(int id, DijkstraNode* node, bool isFinal)
	{
		assert(_nodes.find(id) == _nodes.end()); //No deber�a estar ya
		_nodes.insert(std::pair<int, DijkstraNode*>(id,node));
		if(isFinal) _finalNodeId = id;
	}

	const btVector3& getFinalNodePos()
	{
		assert(_finalNodeId != -1);
		return _nodes.at(_finalNodeId)->getPos();
	}

	bool navMeshAvailable()
	{
		return _finalNodeId >= 0;
	}

	void releaseGraph()
	{
		std::map< int, DijkstraNode* >::iterator node;
		for(node=_nodes.begin(); node!=_nodes.end(); node++)
		{
			delete node->second; //Nos petamos el nodo
		}

		_nodes.clear(); //No petamos el resto.
	}

	std::map< int, DijkstraNode* >& getNodes(){ return _nodes; }

	void computePath(int from, int to, std::deque<btVector3>& out_path);
	void computePath(const btVector3& from, const btVector3& to, std::deque<btVector3>& out_path);
	int getAccessibleNode(const btVector3& point);
	const btVector3& getNodePos(int id) const;
	void getNearPos(const btVector3& pos, float dist, btVector3& out_near_pos);

	void renderGraph(unsigned nodeColor, unsigned edgeColor);
	//void renderBestPath(unsigned color, float offset=0.0f);
	void renderPath(std::deque<btVector3> path, unsigned color, float offset);

};

#endif
