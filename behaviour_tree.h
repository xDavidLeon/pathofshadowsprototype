#ifndef _BT_INC
#define _BT_INC

#include <string>
#include <map>
#include "btnode.h"
#include "component_animation.h"

using namespace std;

typedef int (BehaviourTree::*btaction)(); 
typedef bool (BehaviourTree::*btcondition)(); 

// Implementation of the behavior tree uses the BTnode so both work as a system tree implemented as a map of btnodes for easy traversal
// behaviours are a map to member function pointers, to be defined on a derived class. 
// BT is thus designed as a pure abstract class, so no instances or modifications to bt / btnode are needed...

class Entity;

class BehaviourTree
{
	// the nodes
	map<string,btnode *>tree;
	// the C++ functions that implement node actions, hence, the behaviours
	map<string,btaction> actions;
	// the C++ functions that implement conditions
	map<string,btcondition> conditions;

	protected:
		// moved to private as really the derived classes do not need to see this
		btnode *createNode(const string&);
		btnode *findNode(const string&);

		btnode *root;
		btnode *current;

		Entity* _entity;

		AnimationComponent* _animation_component;
		const string* _previousAction;
		string _action;

		//head controller
		TransformComponent* _target;
		btVector3 _random_look_at;
		float _time_forget;
		
	public:
		BehaviourTree(Entity* entity);
		// use a derived create to declare BT nodes for your specific BTs
		virtual void create(const string&){};
		// use this two calls to declare the root and the children. 
		// use NULL when you don't want a btcondition or btaction (inner nodes)
		btnode *createRoot(const string& rootName,int rootType,btcondition, btaction);
		btnode *addChild(const string& nodeParent,const string& nodeName,int nodeType,btcondition, btaction);
		
		// internals used by btnode and other bt calls
		void addAction(const string&,btaction);
		int execAction(const string&);
		void addCondition(const string&,btcondition);
		bool testCondition(const string&);
		void setCurrent(btnode *);

		const string* getCurrentAction() { return &_action; }; 
		const string* getPreviousAction() { return _previousAction; };

		// call this once per frame to compute the AI. No need to derive this one, 
		// as the behaviours are derived via btactions and the tree is declared on create
		void recalc(float delta);

		//for debug
		virtual void render(){};

		//This function sets *current to NULL, so in the next recalc the bt will be evaluated from the root.
		void reset(){ current = NULL; };

		virtual void init()=0; //Para que la implementen los hijos

		void updateAnimation(const string& action, const string& previousAction);

		//head controller
		void headController();
		void computeRandomWpt(const std::string& state);

		void pushState( const std::string& state)
		{
			assert(findNode(state));
			current = findNode(state);
		};

		string killAnimationName; //fix de una guarreria de edu
};


#endif