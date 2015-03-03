
#include "compressedcuts.h"
#include "shared.h"
#include <sys/time.h>

using namespace std;

int MAXDIMENSIONS = 5;
int numReps = 1;

node::node()
{
	cuts.resize(MAXDIMENSIONS);
	
	depth = 0;
	problematic = 0;
	node_has_rule = 0;

	for (int i = 0; i < MAXDIMENSIONS; i++)
	{
		cuts[i] = 0;
	}
	
	// this is used only if this node is a result 
	// of cutting in 2D
	Row = 0;
	Column = 0;
	Index = 0;

	is_compressed = 0;
	
	count = 0;
}

node::~node()
{
	//printf("node deleted\n");
	for (list<node*>::iterator iter = children.begin();
			iter != children.end();
			iter++)
	{
		delete (*iter);
	}
	classifier.clear();
	children.clear();
}

void calc_dimensions_to_cut(node *curr_node,int *select_dim)
{
  int unique_elements[MAXDIMENSIONS];
  double average = 0;
  //int average = 0;
  range check;
  for (int i = 0;i < MAXDIMENSIONS;++i)
  {
    list <range> rangelist;
    rangelist.clear();
    for (list<pc_rule*>::iterator rule = curr_node->classifier.begin();
        rule != curr_node->classifier.end();++rule)
    {
      int found = 0;
      if ((*rule)->field[i].low > curr_node->boundary.field[i].low) {
        check.low = (*rule)->field[i].low;
      } else {
        check.low = curr_node->boundary.field[i].low;
      }
      if ((*rule)->field[i].high < curr_node->boundary.field[i].high) {
        check.high = (*rule)->field[i].high;
      } else {
        check.high = curr_node->boundary.field[i].high;
      }
      for (list <range>::iterator range = rangelist.begin();
          range != rangelist.end();++range)
      {
        if (check.low == (*range).low && check.high == (*range).high)
        {
          found = 1;
          break;
        }
      }
      if (!found) 
        rangelist.push_back(check);
    }
    unique_elements[i] = rangelist.size();
      //printf("unique_elements[%d] = %d\n",i,unique_elements[i]);

  }
  
  int dims_cnt = 0;
  for (int i = 0;i < MAXDIMENSIONS;++i)
  {
    if (curr_node->boundary.field[i].high > curr_node->boundary.field[i].low)
    {
      average += unique_elements[i];
      dims_cnt++;
    }
  }
  average = average / dims_cnt;

  int max = -1;
  for (int i = 0;i < MAXDIMENSIONS;++i)
  {
    if (curr_node->boundary.field[i].high > curr_node->boundary.field[i].low)
      if (unique_elements[i] > max)
        max = unique_elements[i];
  }

  // Daly: made it so only dimensions strictly greater than the average get picked
  // Such a dimension must exist unless all are equal
  // So we detect such a case
  // This encourages slightly higher draws
  bool areEqual = true;
  for (int i = 0;i < MAXDIMENSIONS;++i)
  {
    select_dim[i] = 0;
	if (unique_elements[i] != average && curr_node->boundary.field[i].high > curr_node->boundary.field[i].low)
		areEqual = false;
  }
  
  if (areEqual)
  {
	cout << "all are equal" << endl;
	for (int i = 0; i < MAXDIMENSIONS; i++)
	{
		if (curr_node->boundary.field[i].high > curr_node->boundary.field[i].low)
		{
			cout << i << " " << unique_elements[i] << endl;
			select_dim[i] = 1;
			return;	
		}
	}
  }

  int dim_count = 0;
  for (int i = 0;i < MAXDIMENSIONS;++i)
  {
    if (curr_node->boundary.field[i].high > curr_node->boundary.field[i].low)
    {
      if (hypercuts)
      {
        if (unique_elements[i] > average) // Daly: changed to strictly greater
        {
          select_dim[i] = 1;
          dim_count++;
          // don't cut on more than 2 dimensions
          if (dim_count == 2)
            break;
        }
      }
      else 
      {
        if (unique_elements[i] == max)
        {
          select_dim[i] = 1;
          break;
        }
      }
    }
  }

}

void calc_num_cuts_1D(node *root,int dim)
{
  node *curr_node;

  int nump = 0;

  int spmf = int(floor(root->classifier.size() * spfac));
  int sm = 0;

  int prev_depth = -1;

  int offset[MAXDIMENSIONS];

  int Index;

  if (!childlist.empty())
  {
    printf("Error: Unread Child nodes!\n");
    exit(1);
  }

  node* top = new node;
  cp_node(root,top);

  childlist.push_back(top);

  while (!childlist.empty())
  {
    curr_node = childlist.front();

    if (prev_depth != curr_node->depth)
    {
      if (sm < spmf)
      {
        nump++;
        sm = 1 << nump;
        prev_depth = curr_node->depth;
        Index = 0;
      }
      else 
        break;
    }

    for (int k = 0;k < 2;k++)
    {
      curr_node->cuts[dim] = 2;

      node* child = new node;
      child->depth = curr_node->depth + 1;

      for (int i = 0;i < MAXDIMENSIONS;i++)
        if (i == dim)
          offset[i] = k;
        else
          offset[i] = 0;

      child->boundary = get_bound(curr_node,offset);
      child->children.clear();

      for (int i=0;i < MAXDIMENSIONS;i++)
        child->cuts[i] = 1;

      for (list <pc_rule*>::iterator rule = curr_node->classifier.begin();rule != curr_node->classifier.end();
          ++rule) 
      {
        if (is_present(child->boundary,(*rule)) == true)
        {
          child->classifier.push_back(*rule);
        }
      }

      child->Index = Index++;

      child->is_compressed = false;

      sm += child->classifier.size();
      if (child->boundary.field[0].low == child->boundary.field[0].high &&
          child->boundary.field[1].low == child->boundary.field[1].high &&
          child->boundary.field[2].low == child->boundary.field[2].high &&
          child->boundary.field[3].low == child->boundary.field[3].high &&
          child->boundary.field[4].low == child->boundary.field[4].high )
        if (child->classifier.size() > 1)
        {
          printf("Error: Box 1X1X1X1X1 cannot contain more than 1 rule!\n");
          exit(1);
        }

      childlist.push_back(child);

    }

    childlist.pop_front();

    ClearMem(curr_node);

  }

  root->cuts[dim] = 1 << nump;
}

void calc_num_cuts_2D(node *root,int *dim)
{
  root->Row = 0;
  root->Column = 0;

  node *curr_node;

  int nump[2];
  for (int i=0;i<2;i++)
    nump[i] = 0;

  int spmf = int(floor(root->classifier.size() * spfac));
  int sm = 0;

  int prev_depth = -1;

  unsigned short chosen = 1;

  int offset[MAXDIMENSIONS];

  if (!childlist.empty())
  {
    printf("Error: Unread Child nodes!\n");
    exit(1);
  }

  node* top = new node;
  cp_node(root,top);

  childlist.push_back(top);

  while (!childlist.empty())
  {
    curr_node = childlist.front();

    if (prev_depth != curr_node->depth)
    {
      if (sm < spmf)
      {
        chosen = chosen ^ 1;
        nump[chosen]++;
        sm = 1 << (nump[0] + nump[1]);
        prev_depth = curr_node->depth;
        //      printf("---------------------------------------\n");
      }
      else 
        break;
    }

    //  printf("[%d,%d] -> ",curr_node->Row,curr_node->Column);

    for (int k = 0;k < 2;k++)
    {
      curr_node->cuts[dim[chosen]] = 2;
      curr_node->cuts[dim[chosen ^ 1]] = 1;

      node* child = new node;
      child->depth = curr_node->depth + 1;

      for (int i = 0;i < MAXDIMENSIONS;i++)
        if (i == dim[chosen])
          offset[i] = k;
        else
          offset[i] = 0;

      child->boundary = get_bound(curr_node,offset);
      child->children.clear();

      for (int i=0;i < MAXDIMENSIONS;i++)
        child->cuts[i] = 1;

      for (list <pc_rule*>::iterator rule = curr_node->classifier.begin();rule != curr_node->classifier.end();
          ++rule) 
      {
        if (is_present(child->boundary,*rule) == true)
        {
          child->classifier.push_back(*rule);
        }
      }

      if (chosen == 0)
      {
        child->Row = (2 * curr_node->Row) + k;
        child->Column = curr_node->Column;
      }
      else
      {
        child->Column = (2 * curr_node->Column) + k;
        child->Row = curr_node->Row;
      }

      child->is_compressed = false;

      //    printf("[%d,%d] ",child->Row,child->Column);

      sm += child->classifier.size();
      if (child->boundary.field[0].low == child->boundary.field[0].high &&
          child->boundary.field[1].low == child->boundary.field[1].high &&
          child->boundary.field[2].low == child->boundary.field[2].high &&
          child->boundary.field[3].low == child->boundary.field[3].high &&
          child->boundary.field[4].low == child->boundary.field[4].high )
        if (child->classifier.size() > 1)
        {
          printf("Box 1X1X1X1X1 cannot contain more than 1 rule!\n");
          exit(1);
        }
      childlist.push_back(child);
	  //printf("pushed! %u\n", childlist.size());

    }

    //  printf("\n");

    childlist.pop_front();
	//printf("\tpopped %u\n", childlist.size());

    ClearMem(curr_node);

	//printf("%u\n", childlist.empty());
  }

  root->cuts[dim[0]] = ( 1 << nump[0]);
  root->cuts[dim[1]] = ( 1 << nump[1]);

  //printf("%d X %d\n",root->cuts[dim[0]],root->cuts[dim[1]]);
  if (compressionON)
  {
    LinearizeChildren(root->cuts[dim[0]]);
    SortChildren();
  }

  // printf("Exited with %u children\n", childlist.size());

}

void calc_cuts(node *curr_node)
{
  int select_dim[MAXDIMENSIONS];
  int chosen_dim[2];
  int chosen_cnt = 0;

  calc_dimensions_to_cut(curr_node,select_dim);

  for (int i = 0;i < MAXDIMENSIONS;++i)
    if (select_dim[i])
      chosen_dim[chosen_cnt++] = i;

// for (int i = 0;i < chosen_cnt;i++)
 // printf("chosen_dim[%d] = %d [%llu - %llu]\n",i,chosen_dim[i],
     // curr_node->boundary.field[chosen_dim[i]].low,
     // curr_node->boundary.field[chosen_dim[i]].high);

	// cout << "depth = " << curr_node->depth << endl;
	// cout << curr_node->classifier.size() << " rules" << endl;
	 
  if (chosen_cnt > 2)
  {
    printf("Error: More than 2 dimensions are cut!\n");
    exit(1);
  }

  if (chosen_cnt > 1 && hypercuts == 0) 
  {
    printf("Error: Hicut: More than 1 dimensions are cut!\n");
    exit(1);
  }

  if (chosen_cnt == 0)
  {
    printf("Error: Atleast 1 dimension needs to be cut!\n");
    exit(1);
  }

  if (chosen_cnt == 2)
  {
    if (!fineOn)
		calc_num_cuts_2D(curr_node,chosen_dim);
	else
		CalcEquiCuts2D(curr_node, chosen_dim);
  } 
  else if (chosen_cnt == 1)
  {
    if (!fineOn)
		calc_num_cuts_1D(curr_node,chosen_dim[0]);
	else 
		CalcMultiEquiCuts1D(curr_node, chosen_dim[0]);
		//CalcEquiCuts1D(curr_node, chosen_dim[0]);
  }
	


// for (int i = 0;i < chosen_cnt;i++)
  // printf("chosen_dim[%d] = %d, cuts = %d\n",i,chosen_dim[i],curr_node->cuts[chosen_dim[i]]);

}

void AddRuleToTree(node* rootNode, pc_rule* rule)
{
	
	if (rootNode->actual_children.size() == 0)
	{
		// We're actually going to charge as though inserting into a vector
		updateReads += rootNode->classifier.size();
		updateWrites++; // New rule is added
		
		int addCost = rootNode->classifier.size();
		list<pc_rule*>::iterator iter;
		for (iter = rootNode->classifier.begin();
				iter != rootNode->classifier.end() && (*iter)->priority < rule->priority;
				iter++)
		{
			addCost--; // Don't have to copy any rule that comes before
		}
		rootNode->classifier.insert(iter, rule);
		updateWrites += addCost;
		
		if (rootNode->classifier.size() > bucketSize)
		{
			CutRecursive(rootNode);
		}
	}
	else
	{
		// Adding the rule to this list simply because of how the stats tallying later work
		rootNode->classifier.push_front(rule);
		
		// Check children
		for (list<node*>::iterator iter = rootNode->actual_children.begin(); 
				iter != rootNode->actual_children.end(); iter++)
		{
			updateReads++; // Must look at each child node
			bool didAdd = false;
			if (DoRulesIntersect(rule, &(*iter)->boundary))
			{
				AddRuleToTree(*iter, rule);
				didAdd = true;
			}
			if (!didAdd)
			{
				cout << "Forgot to add rule" << endl;
				PrintRule(rule);
			}
		}
	}
	
	if (rootNode->classifier.size() > bucketSize && rootNode->children.size() == 0)
	{
		printf("Node has too many rules and no children");
		exit(1);
	}
}

void RemoveRuleFromTree(node* rootNode, pc_rule* rule)
{
	// Cost of visiting this node
	updateReads++;
	
	// 1st: seek rule and remove
	int numMoved = rootNode->classifier.size();
	for (list<pc_rule*>::iterator iter = rootNode->classifier.begin();
			iter != rootNode->classifier.end(); iter++)
	{
		
		if (myequal(rule, *iter))
		{
			rootNode->classifier.erase(iter);
			break;
		}
		numMoved--; // Deleted rule counts : need a new blank rule at the end of the list
	}
	
	if (rootNode->actual_children.size() > 0)
	{
		// We did actually just delete this from the node
		// Charged as though deleted from an array list
		updateReads += rootNode->classifier.size();
		updateWrites += numMoved;
	}
	
	if (rootNode->classifier.size() < bucketSize)
	{
		// Drop children
		for (list<node*>::iterator iter = rootNode->actual_children.begin(); 
				iter != rootNode->actual_children.end(); iter++)
		{
			// Must read the child's rule list to copy it
			updateReads += (*iter)->classifier.size();
			// TODO : get delete correct
			//delete * iter;
		}
		
		// Cost of writing the "new" rule list
		updateWrites += rootNode->classifier.size();
		// TODO : this cost is still slightly undersized
		// Doesn't consider the difficulty of figuring out how big the rule list actually is
		// Without keeping the actual sublists around
		
		rootNode->actual_children.clear();
	}
	else
	{
		// Recurse
		for (list<node*>::iterator iter = rootNode->actual_children.begin(); 
				iter != rootNode->actual_children.end(); iter++)
		{
			if (DoRulesIntersect(rule, &(*iter)->boundary))
			{
				RemoveRuleFromTree(*iter, rule);
			}
		}
	}
}

node* CreateRootNode(list<pc_rule*> p_classifier)
{
	// create a currNode node, put all rules in it. 
	node* currNode = new node;
	currNode->depth = 1;
	for (int i = 0;i < MAXDIMENSIONS;++i) {
		currNode->boundary.field[i].low = 0;
		if (i < 2)
			currNode->boundary.field[i].high = 0xffffffff;
		else if (i < 4)
			currNode->boundary.field[i].high = 0xffff;
		else 
			currNode->boundary.field[i].high = 0xff;
	}
	currNode->children.clear();
	for (int i=0;i < MAXDIMENSIONS;i++)
		currNode->cuts[i] = 1;

	for (list <pc_rule*>::iterator i = p_classifier.begin();i != p_classifier.end();++i) 
	{
		currNode->classifier.push_back((*i));
	}

	currNode->is_compressed = false;

	int count = currNode->classifier.size();
	remove_redund(currNode);
	if (count != currNode->classifier.size()) {
		cout << "Redundant rules removed!" << endl;
	}

	// printf("currNode stats\n");
	// printf("About to add a new rule\n");
	// printf("currNode has rules: %u\n", currNode->node_has_rule);
	// printf("currNode rule count: %u\n", currNode->classifier.size());
	// printf("currNode depth: %u\n", currNode->depth);
	// printf("currNode children count: %u\n", currNode->children.size());
	
	return currNode;
}

void create_tree(list <pc_rule*> p_classifier)
{

  printf("Incoming No of Rules in this tree = %d\n",p_classifier.size());

  list <node*> worklist;
  
  

  node *curr_node;

  root = CreateRootNode(p_classifier);
  
  cout << "Initial root size = " << root->classifier.size() << endl;
  
	if (root->classifier.size() > bucketSize)
		worklist.push_back(root);
	else
	{
		root->problematic = 0;
		NodeStats(root);
	}

	while (!worklist.empty())
	{
		curr_node = worklist.back();

		// printf("Popped node!\n");
		// printf("Depth: %u\n", curr_node->depth);
		
		worklist.pop_back();

		list<node*> topush;
		CutNode(curr_node, topush);
	
		for (list <node*>::iterator item = topush.begin();
				item != topush.end();++item)
		{
			
			if ((*item)->classifier.size() > bucketSize)
			{
				if ((*item)->boundary.field[0].low == curr_node->boundary.field[0].low 
						&& (*item)->boundary.field[0].high == curr_node->boundary.field[0].high 
						&& (*item)->boundary.field[1].low == curr_node->boundary.field[1].low 
						&& (*item)->boundary.field[1].high == curr_node->boundary.field[1].high 
						&& (*item)->boundary.field[2].low == curr_node->boundary.field[2].low 
						&& (*item)->boundary.field[2].high == curr_node->boundary.field[2].high 
						&& (*item)->boundary.field[3].low == curr_node->boundary.field[3].low 
						&& (*item)->boundary.field[3].high == curr_node->boundary.field[3].high 
						&& (*item)->boundary.field[4].low == curr_node->boundary.field[4].low 
						&& (*item)->boundary.field[4].high == curr_node->boundary.field[4].high 
						&& (*item)->classifier.size() == curr_node->classifier.size())
				{
					printf("Warning: parent and child are identical with %d rules!\n",curr_node->classifier.size());
					(*item)->problematic = 1;
					NodeStats(*item);
					ClearMem(*item);
				}
				else
				{
					worklist.push_back(*item);
					if (worklist.size() > Max_WorklistSize)
					{
						Max_WorklistSize = worklist.size();
						if (Max_WorklistSize % 100 == 0)
							printf("Worklist.size() = %lld\n",Max_WorklistSize);
					}
				}
			}
			else
			{
				if (! (*item)->classifier.empty())
				{
					(*item)->problematic = 0;
					NodeStats(*item);
				}
				ClearMem(*item);
			}
		}
		
		curr_node->problematic = 0;
		NodeStats(curr_node);

		ClearMem(curr_node);

	}
	
	printf("Outgoing number of rules in this tree = %u\n", root->classifier.size());

}

void CutRecursive(node* curr_node)
{
	updateWrites++; // Cost of updating this node and it's child pointers
	
	if (curr_node->classifier.size() > bucketSize)
	{
		// Not charging read cost: already know all of the rules
		// updateReads += curr_node->classifier.size()
		
		list<node*> topush;
		CutNode(curr_node, topush);
		for (list<node*>::iterator iter = curr_node->actual_children.begin();
				iter != curr_node->actual_children.end(); iter++)
		{
			// Charge writing for all of the children
			updateWrites += (*iter)->classifier.size();
			// Make sure that the child isn't itself too big
			CutRecursive(*iter);
		}
	}
	
}

void CutNode(node* curr_node, list<node*> &topush)
{
	//cout << "Time to cut: " << curr_node->depth << " " << curr_node->classifier.size() << endl;
	// HEURISTIC 3
	if (hypercuts) {
		regionCompaction(curr_node);
	}
	calc_cuts(curr_node);

	for (list <node*>::iterator item = childlist.begin();
			item != childlist.end();++item)
	{
		(*item)->depth = curr_node->depth + 1;
		for (int i=0;i<MAXDIMENSIONS;i++)
		(*item)->cuts[i] = 1;
	}

	for (list <node*>::iterator item = childlist.begin();
			item != childlist.end();++item)
	{
		curr_node->children.push_back(*item);
	}

	childlist.clear();

	if (compressionON) {
		moveRulesUp(curr_node);
		  
		// backup the number of children, incase compression 
		// can't fit in!
		list <node*> Backup;
		for (list <node*>::iterator item = curr_node->children.begin();
			item != curr_node->children.end();++item)
		{
			// create a new node and make a copy
			node *child_copy = new node;
			cp_node(*item,child_copy);
			Backup.push_back(child_copy);
		}

		curr_node->is_compressed = NodeCompress(curr_node->children);

		if (curr_node->children.size() > num_intervals)
		{
			// printf("Before: %d\n",curr_node->children.size());
			// clear the current children 
			for (list <node*>::iterator item = curr_node->children.begin();
				item != curr_node->children.end();++item)
			{
				ClearMem(*item);
			}

			curr_node->children.clear();

			// reload Backup
			for (list <node*>::iterator item = Backup.begin();
				item != Backup.end();++item)
			{
				curr_node->children.push_back(*item);
			}

			curr_node->is_compressed = false;

			// printf("After: %d\n",curr_node->children.size());
		}
		else
		{
			// empty Backup No longer needed
			for (list <node*>::iterator item = Backup.begin();
					item != Backup.end();++item)
			{
				ClearMem(*item);
			}
		}

		Backup.clear();
	}

	
	if (compressionON == 0 && hypercuts) {
		// HEURISTIC 4
		moveRulesUp(curr_node);
	}
	
	// HEURISTIC 1 - create a list of nodes that should actually exist - both leaf and non-leaf
	topush = nodeMerging(curr_node);

	// HEURISTIC 2
	for (list <node*>::iterator item = topush.begin();item != topush.end();++item) 
	{
		remove_redund(*item);
		curr_node->actual_children.push_back(*item);
	}
}



int mainNormal(int argc, char* argv[])
{
	int i,j; 
	int header[MAXDIMENSIONS];
	int matchid, fid;
	char *s = (char *)calloc(200, sizeof(char));
	bool isCorrect = false;

	struct timeval startTime, endTime;
	long elapsedTimeMicroSec;
	
	parseargs(argc, argv);

	while(fgets(s, 200, fpr) != NULL)numrules++;
	rewind(fpr);


	printf("number of rules read from file = %d\n", numrules);

	gettimeofday(&startTime, NULL);
	
	classifier.clear();
	int numrules1 = loadrule(fpr);

	if (numrules1 != numrules)
	{
		printf("Error: Number of rules read != Number of rules loaded!\n");
		exit(1);
	}
	fclose(fpr);

	ComputeCutoffs();

	if (binningON == 1)
	{
		binRules(classifier);
		if (mergingON == 1)
			MergeTrees();

		for (int i = 0; i < 5; i++) {
			if (!(bigrules[i].empty())) {
				InitStats(bigrules[i].size());
				create_tree(bigrules[i]);
				RecordTreeStats();
				bigrules[i].clear();
			}
		}
		for (int j = 0; j < 10; j++) {
			if (!(kindabigrules[j].empty())) {
				InitStats(kindabigrules[j].size());
				create_tree(kindabigrules[j]);
				RecordTreeStats();
				kindabigrules[j].clear();
			}
		}
		for (int k = 0; k < 10; k++) {
			if (!(mediumrules[k].empty())) {
				InitStats(mediumrules[k].size());
				create_tree(mediumrules[k]);
				RecordTreeStats();
				mediumrules[k].clear();
			}
		}
		for (int l = 0; l < 5; l++) {
			if (!(littlerules[l].empty())) {
				InitStats(littlerules[l].size());
				create_tree(littlerules[l]);
				RecordTreeStats();
				mediumrules[l].clear();
			}
		}
		if (!(smallrules.empty())) {
			InitStats(smallrules.size());
			create_tree(smallrules);
			RecordTreeStats();
			smallrules.clear();
		}
		/*else
		{
			LoadRulePtr(classifier,p_classifier);
			InitStats(p_classifier.size());
			create_tree(p_classifier);
			RecordTreeStats();
			p_classifier.clear();
		}*/
		
		// TODO : correctness testing
	}
	else
	{
		int start = 0;
		int end = 0;
		for (int i = 0;i < Num_Junk;i++)
		{
			if (i == Num_Junk - 1)
				end = classifier.size() - 1;
			else
				end = start + Cutoffs[i] - 1;
			LoadRulePtr(classifier, p_classifier, start, end);
			start = end + 1;
			InitStats(p_classifier.size());
			create_tree(p_classifier);
			RecordTreeStats();
			p_classifier.clear();
		}
		list<TreeDetails> trees;
		TreeDetails details;
		details.root = root;
		trees.push_back(details);
		isCorrect = CheckTrees(trees, p_classifier);
	}
	
	gettimeofday(&endTime, NULL);
	elapsedTimeMicroSec = (endTime.tv_sec - startTime.tv_sec) * 1000000;
	elapsedTimeMicroSec += (endTime.tv_usec - startTime.tv_usec);
	
	// Statistics
	PrintStats();

//  BinPack(1,Statistics);
//  BinPack(2,Statistics);
//  BinPack(3,Statistics);
//  BinPack(4,Statistics);
	printf("Correct: %u\n", isCorrect);
	printf("Duration: %u us\n", elapsedTimeMicroSec);
}


int mainAdd(int argc, char* argv[]) {
	int i,j; 
	int header[MAXDIMENSIONS];
	int matchid, fid;
	char *s = (char *)calloc(200, sizeof(char));
	bool isCorrect;

	parseargs(argc, argv);

	while(fgets(s, 200, fpr) != NULL)numrules++;
	rewind(fpr);


	printf("number of rules read from file = %d\n", numrules);

	classifier.clear();
	int numrules1 = loadrule(fpr);

	if (numrules1 != numrules)
	{
		printf("Error: Number of rules read != Number of rules loaded!\n");
		exit(1);
	}
	fclose(fpr);

	ComputeCutoffs();
	
	
	int start = 0;
	int end = classifier.size();
	int split = end / 2;
	list<pc_rule*> addedRules;
	LoadRulePtr(classifier, addedRules, start, split);
	LoadRulePtr(classifier, p_classifier, start, end);
	
	cout << classifier.size() << endl;
	cout << addedRules.size() << endl;
	cout << p_classifier.size() << endl;
	
	if (binningON)
	{
		list<TreeDetails> trees;
		
		list<pc_rule> baseRuleList;
		
		list<pc_rule>::iterator indexer = classifier.begin();
		for (int i = 0; i < split; i++)
			indexer++;
		for ( ; indexer != classifier.end(); indexer++)
			baseRuleList.push_back(*indexer);
			
		cout << "Base rule list size: " << baseRuleList.size() << endl;
		
		binRules(baseRuleList);
		
		//cout << "Num Trees: " << numTrees << endl;
		
		if (mergingON)
		{
			cout << "Merging trees" << endl;
			MergeTrees();
		}
		else
		{
			cout << "Skipping merge step" << endl;
		}
		
		//printf("Preparing to make trees\n");
		
		int count = 0;
		for (int i = 0; i < 5; i++) {
			count += bigrules[i].size();
			Treeify(trees, bigrules[i]);
		}
		
		//printf("Big Trees made\n");
		
		for (int j = 0; j < 10; j++) {
			count += kindabigrules[j].size();
			Treeify(trees, kindabigrules[j]);
		}
		
		//printf("Kinda big trees made\n");
		
		for (int k = 0; k < 10; k++) {
			count += mediumrules[k].size();
			Treeify(trees, mediumrules[k]);
		}
		
		//printf("Medium trees made\n");
		
		for (int l = 0; l < 5; l++) {
			count += mediumrules[l].size();
			Treeify(trees, littlerules[l]);
		}
		
		//printf("Little trees made\n");
		
		count += smallrules.size();
		Treeify(trees, smallrules);
		
		cout << "Base has " << count << " rules" << endl;
		
		//printf("All trees made\n");
		
		int ruleCount = 0;
		for (list<TreeDetails>::iterator iter = trees.begin();
				iter != trees.end(); iter++)
		{
			ruleCount += iter->root->classifier.size();
			//cout << iter->root->classifier.size() << endl;
			//PrintRuleList(iter->root->classifier);
			// break;
		}
		cout << ruleCount << endl;
		//cout << addedRules.size() << endl;
		//cout << p_classifier.size() << endl;
		
		
		for (list<pc_rule*>::iterator iter = addedRules.begin();
				iter != addedRules.end();
				iter++)
		{
			node* tree = FindSuitableTree(trees, *iter);
			if (tree) {
				AddRuleToTree(tree, *iter);
				//cout << "Rule was added" << endl;
			} else {
				cout << "Tree was null" << endl;
				exit(1);
			}
		}
		
		
		for (list<TreeDetails>::reverse_iterator iter = trees.rbegin();
				iter != trees.rend();
				iter++)
		{
			InitStats(iter->root->classifier.size());
			StatNode(iter->root);
			root = iter->root;
			RecordTreeStats();
		}
		
		
		
		isCorrect = CheckTrees(trees, p_classifier);
	}
	else
	{
		list<pc_rule*> baseRuleList;
		LoadRulePtr(classifier, baseRuleList, split + 1, end);
		InitStats(p_classifier.size());
		create_tree(baseRuleList);
		
		//PrintTree(root);
		
		for (list<pc_rule*>::reverse_iterator iter = addedRules.rbegin();
			iter != addedRules.rend();
			iter++)
		{
			//printf("About to add a new rule\n");
			AddRuleToTree(root, *iter);
			//PrintRule(*iter);
		}
		
		// printf("More root stats\n");
		// printf("Root has rules: %u\n", root->node_has_rule);
		// printf("Root rule count: %u\n", root->classifier.size());
		// printf("Root depth: %u\n", root->depth);
		// printf("Root children count: %u\n", root->children.size());
		
		InitStats(classifier.size());
		StatNode(root);
		
		
		RecordTreeStats();
		
		
		list<TreeDetails> trees;
		TreeDetails details;
		details.root = root;
		trees.push_back(details);
		isCorrect = CheckTrees(trees, p_classifier);
		
		delete root;
		
		p_classifier.clear();
	}
	
	printf("Classifier size: %u\n", classifier.size());
	printf("Added classifier size: %u\n", addedRules.size());
	printf("Base classifier size: %u\n", p_classifier.size());
	
	PrintStats();
	printf("Correct: %u\n", isCorrect);
}

bool IsFieldWide(pc_rule* rule, int dim)
{
	if (dim < 2)
	{
		return ((double) (rule->field[dim].high - rule->field[dim].low))/0xFFFFFFFF >= IPbin;
	}
	else if (dim < 4)
	{
		return ((double) (rule->field[dim].high - rule->field[dim].low))/65535 >= bin;
	}
	else
	{
		return (rule->field[dim].low == 0) && (rule->field[dim].high == 0xFF);
	}
}

void Treeify(list<TreeDetails> &trees, list<pc_rule*> &rules)
{
	//cout << rules.size() << endl;
	if (!rules.empty())
	{
		create_tree(rules);
		TreeDetails details;
		details.root = root;
		
		// This part isn't quite right because of tree merging
		pc_rule* rule = *rules.begin();
		for (int i = 0; i < MAXDIMENSIONS; i++)
		{
			details.wideFields[i] = IsFieldWide(rule, i);
		}
		trees.push_back(details);
		rules.clear();
		//cout << "Added a tree!" << endl;
	}
}

node* FindSuitableTree(list<TreeDetails> & trees, pc_rule* rule)
{
	int count = 0;
	for (list<TreeDetails>::reverse_iterator iter = trees.rbegin();
			iter != trees.rend(); iter++)
	{
		count++;
		bool isAllowed = true;
		for (int i = 0; i < MAXDIMENSIONS; i++)
		{
			//cout << IsFieldWide(rule, i) << " ";
			if (IsFieldWide(rule, i) && !iter->wideFields[i])
			{
				isAllowed = false;
				break;
			}
		}
		//cout << endl;
		
		if (isAllowed)
		{
			//cout << "Using tree # " << count << " of " << trees.size() << endl;
			return iter->root;
		}
	}
	
	cout << "no matching tree found" << endl;
	PrintRule(rule);
	
	for (list<TreeDetails>::iterator iter = trees.begin();
			iter != trees.end(); iter++)
	{
		for (int i = 0; i < MAXDIMENSIONS; i++)
		{
			cout << iter->wideFields[i] << " ";
		}
		cout << endl;
	}
	return NULL;
}

int mainSub(int argc, char* argv[]) {
	int i,j; 
	int header[MAXDIMENSIONS];
	int matchid, fid;
	char *s = (char *)calloc(200, sizeof(char));
	bool isCorrect;

	parseargs(argc, argv);

	while(fgets(s, 200, fpr) != NULL)numrules++;
	rewind(fpr);


	printf("number of rules read from file = %d\n", numrules);

	classifier.clear();
	int numrules1 = loadrule(fpr);

	if (numrules1 != numrules)
	{
		printf("Error: Number of rules read != Number of rules loaded!\n");
		exit(1);
	}
	fclose(fpr);

	ComputeCutoffs();
	
	
	int start = 0;
	int end = classifier.size();
	int split = end / 2;
	
	list<pc_rule*> subbedRules;
	list<pc_rule*> remainingRules;
	LoadRulePtr(classifier, subbedRules, start, split);
	LoadRulePtr(classifier, remainingRules, split+1, end);
	LoadRulePtr(classifier, p_classifier, start, end);
	
	
	if (binningON)
	{
		list<TreeDetails> trees;
		
		binRules(classifier);
		
		if (mergingON)
		{
			cout << "Merging trees" << endl;
			MergeTrees();
		}
		else
		{
			cout << "Skipping merge step" << endl;
		}
		
		int count = 0;
		for (int i = 0; i < 5; i++) {
			count += bigrules[i].size();
			Treeify(trees, bigrules[i]);
		}
		
		for (int j = 0; j < 10; j++) {
			count += kindabigrules[j].size();
			Treeify(trees, kindabigrules[j]);
		}
		
		for (int k = 0; k < 10; k++) {
			count += mediumrules[k].size();
			Treeify(trees, mediumrules[k]);
		}
		
		for (int l = 0; l < 5; l++) {
			count += mediumrules[l].size();
			Treeify(trees, littlerules[l]);
		}
		
		count += smallrules.size();
		Treeify(trees, smallrules);
		
		cout << "Base has " << count << " rules" << endl;
		
		//printf("All trees made\n");
		
		int ruleCount = 0;
		for (list<TreeDetails>::iterator iter = trees.begin();
				iter != trees.end(); iter++)
		{
			ruleCount += iter->root->classifier.size();
		}
		
		cout << ruleCount << endl;
		
		
		for (list<pc_rule*>::iterator iter = subbedRules.begin();
				iter != subbedRules.end();
				iter++)
		{
			node* tree = FindSuitableTree(trees, *iter);
			if (tree) {
				RemoveRuleFromTree(tree, *iter);
				//cout << "Rule was added" << endl;
			} else {
				cout << "Tree was null" << endl;
				exit(1);
			}
		}
		
		
		for (list<TreeDetails>::iterator iter = trees.begin();
				iter != trees.end();
				iter++)
		{
			if (iter->root->classifier.size() > 0)
			{
				InitStats(iter->root->classifier.size());
				StatNode(iter->root);
				root = iter->root;
				RecordTreeStats();
			}
		}
		
		isCorrect = CheckTrees(trees, remainingRules);
	}
	else
	{
		InitStats(p_classifier.size());
		create_tree(p_classifier);
		
		//PrintTree(root);
		
		for (list<pc_rule*>::iterator iter = subbedRules.begin();
			iter != subbedRules.end();
			iter++)
		{
			//printf("About to add a new rule\n");
			RemoveRuleFromTree(root, *iter);
		}
		
		// printf("More root stats\n");
		// printf("Root has rules: %u\n", root->node_has_rule);
		// printf("Root rule count: %u\n", root->classifier.size());
		// printf("Root depth: %u\n", root->depth);
		// printf("Root children count: %u\n", root->children.size());
		
		InitStats(classifier.size());
		StatNode(root);
		
		
		RecordTreeStats();
		
		
		list<TreeDetails> trees;
		TreeDetails details;
		details.root = root;
		trees.push_back(details);
		isCorrect = CheckTrees(trees, remainingRules);
		
		delete root;
		
		p_classifier.clear();
	}
	
	printf("Classifier size: %u\n", classifier.size());
	printf("Subtracted classifier size: %u\n", remainingRules.size());
	printf("Base classifier size: %u\n", p_classifier.size());
	
	PrintStats();
	printf("Correct: %u\n", isCorrect);
}

int main(int argc, char* argv[]) {
	setNumReps(1);
	mainNormal(argc, argv);
}
