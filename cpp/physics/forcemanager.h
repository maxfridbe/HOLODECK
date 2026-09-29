#ifndef FORCEMANAGER_H
#define FORCEMANAGER_H

#include <vector>
#include "PNode.h" //temporary will use force.h

using namespace std;

class ForceManager
{
	public:		
		static Force * getForceByName(const string & name);
		static bool AddForce(Force * force);
		static Force * RemoveForce(const string & name);
		static const vector<Force *> & getForces();
	private:
		static vector<Force *> forces;
};

inline const vector<Force *> & ForceManager::getForces()
{
	return forces;
}

#endif
