#include "forcemanager.h"
#include <algorithm>

vector<Force *> ForceManager::forces;

bool ForceManager::AddForce(Force * force)
{	
	if (std::find(ForceManager::forces.begin(), forces.end(), force) == forces.end())
	{
		forces.push_back(force);
		return true;
	}
	
	return false;	
}

Force * ForceManager::getForceByName(const string & name)
{
	for (int i = 0; i < forces.size(); i++)
	{
		if (forces[i] ->getName() == name)
		{
			return forces[i];
		}
	}
	return NULL;
}	

Force * ForceManager::RemoveForce(const string & name)
{
	Force * force = NULL;

	vector<Force *>::iterator it = std::find(ForceManager::forces.begin(), forces.end(), force);

	if (it != forces.end())
	{		
		force = *it;
		forces.erase(it);
	}

	return force;
}
