#include "shared.h"

/**
 * Method for creating arbirary 2D cuts
 * Written by James Daly
 */
void CalcEquiCuts2D(node *root, int* dims)
{
	const int numSplits = 2;
	const int numChildren = 4;
	
	childlist.clear();
	
	node* current = new node;
	cp_node(root, current);
	
	unsigned long long splits[numSplits];
	
	for (int i = 0; i < numSplits; i++)
	{
		splits[i] = BestSplitPoint(current, dims[i]);
		root->cuts[i] = 2;
	}
	
	for (int i = 0; i < numSplits; i++)
	{
		for (int j = 0; j < numSplits; j++)
		{
			int index = i * numSplits + j;
			node* child = SpawnChild(current);
			
			if (i == 0) 
			{
				child->boundary.field[dims[0]].high = splits[0];
			}
			else
			{
				child->boundary.field[dims[0]].low = splits[0] + 1;
			}
			
			if (j == 0) 
			{
				child->boundary.field[dims[1]].high = splits[1];
			}
			else
			{
				child->boundary.field[dims[1]].low = splits[1] + 1;
			}
			
			for (int i = 0; i < MAXDIMENSIONS; i++)
			{
				if (child->boundary.field[i].low > child->boundary.field[i].high)
				{
					cout << "boundary problems!" << " " << i << " " << child->boundary.field[i].low << " " << child->boundary.field[i].high << endl;
				}
			}
			
			// TODO : set bounds
			for (list <pc_rule*>::iterator rule = current->classifier.begin();
				rule != current->classifier.end(); ++rule)
			{
				if (is_present(child->boundary, *rule))
				{
					child->classifier.push_back(*rule);
				}
			}
			childlist.push_back(child);
		}
	}
	
	ClearMem(current);
}

/**
 * Written for creating wider arbitrary splits
 */
void CalcMultiEquiCuts1D(node *root, int dim)
{
	childlist.clear();
	list<node*> clonelist = CalcEquiCuts1D(root, dim);
	
	for (list<node*>::iterator nodex = clonelist.begin();
			nodex != clonelist.end(); ++nodex)
	{
		if ((*nodex)->classifier.size() > bucketSize)
		{
			list<node*> children = CalcEquiCuts1D(*nodex, dim);
			while (!children.empty())
			{
				node* n = children.front();
				children.pop_front();
				n->depth--;
				childlist.push_back(n);
			}
			//ClearMem(*nodex);
		}
		else
		{
			childlist.push_back(*nodex);
		}
	}
	
	root->cuts[dim] = childlist.size();
}

/**
 * Method for creating arbitrary splits
 * Written by James Daly
 */
list<node*> CalcEquiCuts1D(node *root, int dim)
{
	// Whatever spmf actually stands for
	int spmf = int(floor(root->classifier.size() * spfac));
	
	list<node*> results;
	
	node* current = new node;
	cp_node(root, current);
	
	unsigned long long split = BestSplitPoint(current, dim);
	
	// Split
	node* lowerNode = SpawnChild(current);
	node* upperNode = SpawnChild(current);
	lowerNode->boundary.field[dim].high = split;
	upperNode->boundary.field[dim].low = split + 1;
	// TODO : initialize nodes
	for (list <pc_rule*>::iterator rule = current->classifier.begin();
			rule != current->classifier.end(); ++rule)
	{
		if ((*rule)->field[dim].low <= split)
		{
			lowerNode->classifier.push_back(*rule);
		}
		if ((*rule)->field[dim].high > split)
		{
			upperNode->classifier.push_back(*rule);
		}
	}
	
	if (lowerNode->classifier.size() == current->classifier.size())
	{
		cout << "size problems:" << split << endl;
		// for (list <pc_rule*>::iterator rule = lowerNode->classifier.begin();
				// rule != lowerNode->classifier.end(); ++rule)
		// {
			// cout << (*rule)->field[dim].low << " " << (*rule)->field[dim].high << endl;
		// }
		results.push_back(root);
	} else {
		results.push_back(lowerNode);
		results.push_back(upperNode);
		ClearMem(current);
	}
	
		
	return results;
}

/**
 * Helper method for creating child nodes
 * Memory is allocated by this method that the client is responsible for
 * Written by James Daly
 */
node* SpawnChild(node* parent)
{
	node* child = new node;
	child->depth = parent->depth + 1;
	child->boundary = parent->boundary;
	child->is_compressed = false;
	return child;
}

/**
 * Helper method for calculating the best split point
 * Written by James Daly
 */
unsigned long long BestSplitPoint(node * current, int dim)
{	
	vector<unsigned long long> ubounds;
	vector<int> endsByCount;
	vector<int> crossesCount;
	
	// Find all of the unique upper bounds
	for (list <pc_rule*>::iterator rule = current->classifier.begin();
			rule != current->classifier.end(); ++rule) 
	{
		unsigned long long upper = min((*rule)->field[dim].high, current->boundary.field[dim].high);
		bool contains = false;
		for (vector<unsigned long long>::iterator item = ubounds.begin();
				item != ubounds.end(); ++item)
		{
			if (*item == upper)
			{
				contains = true;
				break;
			}
		}
		if (!contains)
		{
			ubounds.push_back(upper);
			endsByCount.push_back(0);
			crossesCount.push_back(0);
		}
	}
	sort(ubounds.begin(), ubounds.end());
	
	// tally the number of rules that end at certain spots
	// or that cross each spot
	for (list <pc_rule*>::iterator rule = current->classifier.begin();
			rule != current->classifier.end(); ++rule)
	{
		for (int i = 0; i < ubounds.size(); i++)
		{
			if ((*rule)->field[dim].high <= ubounds[i])
			{
				endsByCount[i] = endsByCount[i] + 1;
				break;
			}
			else
			{
				if ((*rule)->field[dim].low < ubounds[i])
				{
					crossesCount[i] = crossesCount[i] + 1;
				}
			}
		}
	}
	
	// find the best split point
	// Try to get half the rules on either side of the split
	// and no crossings
	// Should probably weight crossings higher
	// so rules don't get replicated
	unsigned long long split = 0;
	int worstCost = numeric_limits<int>::max();
	int tally = 0;//-current->classifier.size() / 2;
	for (int i = 0; i < ubounds.size() - 1; i++)
	{
		tally += endsByCount[i];
		//int cost = (tally < 0) ? -tally : tally;
		//cost += crossesCount[i];// * 2;
		int cost = 2 * tally - current->classifier.size() + crossesCount[i];
		cost *= (cost < 0) ? -1 : 1;
		cost += crossesCount[i];
		//cout << cost << " " << tally << " " << crossesCount[i] << endl;
		if (cost < worstCost)
		{
			
			worstCost = cost;
			split = ubounds[i];
		}
	}
	
	//cout << worstCost << " " << split << endl;
	
	if (split >= ubounds.back())
	{
		cout << "Bad split: " << split << " " << worstCost << endl;
		for (int i = 0; i < ubounds.size(); i++)
		{
			cout << endsByCount[i] << " " << crossesCount[i] << endl;
		}
		exit(1);
	}
	return split;
}
