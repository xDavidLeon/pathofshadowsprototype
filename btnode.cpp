#include <stdio.h>
#include "btnode.h"
#include "behaviour_tree.h"
#include <cassert>

btnode::btnode(const string& s)
{
	name=s;
}


bool btnode::isRoot()
{
	return (parent==NULL);
}


void btnode::setParent(btnode *p)
{
	parent=p;
}


void btnode::setRight(btnode *p)
{
	right=p;
}


void btnode::setType(int t)
{
	type=t;
}


const string& btnode::getName()
{
	return name;
}


void btnode::addChild(btnode *c)
{
	if (!children.empty()) // if this node already had children, connect the last one to this
		children.back()->setRight(c);  // new one so the new one is to the RIGHT of the last one
	children.push_back(c); // in any case, insert it
	c->right=NULL; // as we're adding from the right make sure right points to NULL
}


void btnode::recalc(BehaviourTree *tree)
{
	//dbg("recalcing node %s\n",name.c_str()); // activate this line to debug

	switch (type)
	{
		case ACTION:	
		{
			// run the controller of this node
			int res=tree->execAction(name);
			if(res == PUSHSTATE) return;
			// now, the next lines compute what's the NEXT node to use in the next frame...
			if (res==STAY) { tree->setCurrent(this); return; }// looping vs. on-shot actions
			// climb tree iteratively, look for the next unfinished sequence to complete
			btnode *candidate=this;
			while (candidate->parent!=NULL)
			{
				btnode *daddy=candidate->parent;
				if (daddy->type==SEQUENCE) // oh we were doing a sequence. make sure we finished it!!!
				{
					if (candidate->right!=NULL)
					{
						tree->setCurrent(candidate->right);
						break;
					}
					else candidate=daddy; // sequence was finished (there is nobody on right). Go up to daddy.
				}
				else candidate=daddy; // i'm not on a sequence, so keep moving up to the root of the BT
			}
			// if we've reached the root, means we can reset the traversal for next frame.
			if (candidate->parent==NULL) tree->setCurrent(NULL);
			break;
		}
		case RANDOM:
		{
			int r=rand()%children.size();
			children[r]->recalc(tree);
			break;
		}
		case PRIORITY:
		{
			for (unsigned int i=0;i<children.size();i++)
			{
				if (tree->testCondition(children[i]->getName())) 
				{
					children[i]->recalc(tree);
					break;
				}
			}
			break;
		}
		case SEQUENCE:
		{
			// begin the sequence...the inner node (action) will take care of the sequence
			// via the "setCurrent" mechanism
			//WARNING: if in pos 0 of children there's nothing, this will explode.
			assert(children[0]);
			children[0]->recalc(tree);	
			break;
		}
	}
}
