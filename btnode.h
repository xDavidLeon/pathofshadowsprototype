#ifndef _BTNODE_INC
#define _BTNODE_INC

#include <string>
#include <vector>

using namespace std;

class BehaviourTree;
class btnode;

#define RANDOM 0
#define SEQUENCE 1
#define PRIORITY 2
#define ACTION 3

#define STAY 0
#define LEAVE 1
#define PUSHSTATE 2

class btnode
{
	string name;
	int type;
	vector<btnode *>children;
	btnode *parent;
	btnode *right;

	public:
		btnode(const string&);
		bool isRoot();
		void setParent(btnode *);
		void setRight(btnode *);
		void addChild(btnode *);
		void setType(int );
		void recalc(BehaviourTree *);
		const string& getName();
};

#endif