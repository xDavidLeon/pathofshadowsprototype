#include "dijkstra.h"
#include "d3ddefs.h"
#include <set>
#include <deque>
#include "angular.h"
#include "world.h"
#include <algorithm>
#include "system_physics.h"

DijkstraGraph::DijkstraGraph(void)
{
	_finalNodeId = -1;
}


DijkstraGraph::~DijkstraGraph(void)
{
}

void DijkstraGraph::computePath(int from, int to, std::deque<btVector3>& out_path)
{
	assert(_nodes.find(from)!=_nodes.end());
	assert(_nodes.find(to)!=_nodes.end());

	//Iniciamos variables de los nodos destinadas a este calculo
	std::map<int, DijkstraNode*>::iterator node;
	for(node=_nodes.begin(); node!=_nodes.end(); node++)
	{
		node->second->_bestDistance = 999;
		node->second->_bestParent = -1;
	}

	std::deque<int> toVisit; //cola de prioridad
	std::set<int> visitted;  //visitados

	//agregar el origen a la cola, y poner que su distancia es 0
	_nodes.at(from)->_bestDistance = 0.0f;
	toVisit.push_back(from);

	while(!toVisit.empty())
	{
		int currentNode = toVisit.at(0); toVisit.pop_front();
		//Si el nodo ya se ha visitado pasamos de el
		if(visitted.find(currentNode) != visitted.end()) continue;
		//Se "marca" el nodo como visitado
		visitted.insert(currentNode);

		//Recorremos nodos adyacentes
		std::map<int, float>::iterator neighbour;
		for(neighbour=_nodes.at(currentNode)->getNeighbours().begin(); neighbour!=_nodes.at(currentNode)->getNeighbours().end(); neighbour++)
		{
			if(visitted.find(neighbour->first)!=visitted.end()) continue; //Si ya esta visitado, siguiente
			float distance = neighbour->second;
			//Si desde aqui se recorre menos distancia total hasta el vecino...
			if(_nodes.at(currentNode)->_bestDistance+distance < _nodes.at(neighbour->first)->_bestDistance)
			{
				_nodes.at(neighbour->first)->_bestDistance = _nodes.at(currentNode)->_bestDistance+distance; //Actualizamos distancia total hasta ese nodo
				_nodes.at(neighbour->first)->_bestParent = currentNode; //Actualizamos mejor camino desde ese nodo
				toVisit.push_back(neighbour->first); //Anyadimos el nodo vecino para visitar
			}
		}
	}

	//-->En este punto tenemos en cada nodo la distancia mas corta hasta el partiendo de "from"
	//Metemos el path en el vector que nos han pasado (out_path) y en _bestPath (debug)
	out_path.clear();

	int path_node = to;
	while(path_node != from)
	{
		out_path.push_front(_nodes.at(path_node)->getPos());
		//Actualizar path_node
		path_node = _nodes.at(path_node)->_bestParent;
	}

	out_path.push_front(_nodes.at(path_node)->getPos());
}

void DijkstraGraph::computePath(const btVector3& from, const btVector3& to, std::deque<btVector3>& out_path)
{
	//Se calcula el camino solo si no se puede ir directamente de from a to
	if(PhysicsSystem::get().checkCollision(from, to, PhysicsSystem::get().colMaskNavigation))
	{
		int fromNode = getAccessibleNode(from);
		int toNode = getAccessibleNode(to);

		if(fromNode == -1 || toNode == -1) return;

		computePath(fromNode, toNode, out_path);
	}

	//anyadimos la posicion destino a la cola de prioridad
	out_path.push_back(to);
}

//Obtiene la id del nodo accesible desde "point" mas cercano... o casi
int DijkstraGraph::getAccessibleNode(const btVector3& point)
{
	float near_pos = 25.0f; //La distancia maxima de un nodo a "point" que consideramos valida
	int candidate = -1; //Nodo que se devolvera si no se encuentra ninguno accesible
	std::vector<std::pair<int, float>> nearAccessibleNodes;
	int nearestNode = -1;
	float nearestDist = 999.0f;

	//Pillar nodos mas cercanos (manhattan) que near_pos
	std::map<int, DijkstraNode*>::iterator node;
	for(node=_nodes.begin(); node!=_nodes.end(); node++)
	{
		const btVector3& nodePos = node->second->getPos();
		float mh_dist = manhattanDist(nodePos, point);
		if(mh_dist <= near_pos) //Si nodo "cercano"
		{
			//Nos vamos guardando el nodo mas cercano por si acaso
			if(mh_dist < nearestDist)
			{
				nearestNode = node->first;
				nearestDist = mh_dist;
			}

			if(!PhysicsSystem::get().checkCollision(point, nodePos, PhysicsSystem::get().colMaskNavigation))
			{	//Si el nodo es accesible lo guardamos
				nearAccessibleNodes.push_back(std::pair<int, float>(node->first, mh_dist));
			}
		}
	}

	//En este punto tenemos un vector con los nodos mas cerca de "near_pos" unidades de "point", a los cuales se puede acceder
	if(!nearAccessibleNodes.size()) return nearestNode;

	nearestDist = 999.0f;

	//Buscamos, de entre los nodos accesibles, el mas cercano.
	for(unsigned i=0; i<nearAccessibleNodes.size(); i++)
	{
		if(nearAccessibleNodes.at(i).second < nearestDist)
		{
			nearestDist = nearAccessibleNodes.at(i).second;
			nearestNode = nearAccessibleNodes.at(i).first;
		}
	}
	
	return nearestNode;
}

const btVector3& DijkstraGraph::getNodePos(int id) const
{
	assert(_nodes.find(id) != _nodes.end());

	return _nodes.at(id)->getPos();
}

//Devuelve una posician accesible cercana ('dist' unidades) a un punto
//WARNING: si no se desea un armagedon, no poner una 'dist' mayor a 2 o 3
void DijkstraGraph::getNearPos(const btVector3& pos, float dist, btVector3& out_near_pos)
{
	//Posicion del nodo mas cercano accesible desde 'pos'
	const btVector3 nodePos = getNodePos(getAccessibleNode(pos));
	//Devolvemos punto a 'dist' de 'pos', con respecto a 'nodePos'
	btVector3 dir = nodePos-pos;  dir.normalize();
	out_near_pos = pos + dir*dist;
}

void DijkstraGraph::renderGraph(unsigned nodeColor, unsigned edgeColor)
{
	std::map<int, DijkstraNode*>::iterator iter;
	float v_offset, d_v_offset, v_max;
	v_offset = 0.0f;
	d_v_offset = 0.03f;
	v_max = 1.0f;

	//Recorrer nodos
	for(iter=_nodes.begin(); iter!=_nodes.end(); iter++)
	{
		const btVector3& nodePos= iter->second->getPos();
		drawLine_bt(nodePos, nodePos+btVector3(0,1,0), nodeColor); //el nodo iter1
		//Recorrer vecinos
		std::map<int, float>::iterator iter2;
		for(iter2=iter->second->getNeighbours().begin(); iter2!=iter->second->getNeighbours().end(); iter2++)
		{
			const btVector3& neighbourPos = _nodes.at(iter2->first)->getPos();
			drawLine_bt(nodePos + btVector3(0,v_offset,0), neighbourPos + btVector3(0,v_offset,0), edgeColor); //linea hacia vecino iter2
			v_offset += d_v_offset;
			if(v_offset > v_max || v_offset < 0.0f) d_v_offset *= -1;
		}
	}
}

//void DijkstraGraph::renderBestPath(unsigned color, float offset)
//{
//	if(!_bestPath.size()) return;
//
//	btVector3 up = btVector3(0,offset,0); //Desplazamos arriba un poco para que no se solape con el grafo
//
//	for(unsigned i=0; i<_bestPath.size()-1; i++)
//	{
//		drawLine_bt(_bestPath.at(i)->getPos()+up, _bestPath.at(i+1)->getPos()+up, color); 
//	}
//}

void DijkstraGraph::renderPath(std::deque<btVector3> path, unsigned color, float offset)
{
	if(!path.size()) return;
	btVector3 up = btVector3(0,offset,0); //Desplazamos arriba un poco para que no se solape con el grafo, si se pinta
	btVector3 node_up = btVector3(0,0.5f,0); //para poner donde estan los nodos
	node_up = up+node_up;

	for(unsigned i=0; i<path.size()-1; i++)
	{
		drawLine_bt(path.at(i)+up, path.at(i+1)+up, color); 
		drawLine_bt(path.at(i)+up, path.at(i)+node_up, color); 
	}
}
