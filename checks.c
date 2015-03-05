#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <limits>

using namespace std;

#include "compressedcuts.h"
#include "shared.h"

bool DoesRuleContainPoint(pc_rule* rule, unsigned long long* pt)
{
	for (int i = 0; i < MAXDIMENSIONS; i++)
	{
		if (rule->field[i].low > pt[i] || rule->field[i].high < pt[i])
		{
			return false;
		}
	}
	return true;
}

bool DoRulesIntersect(pc_rule* r1, pc_rule* r2)
{
	for (int i = 0; i < MAXDIMENSIONS; i++)
	{
		if (r1->field[i].high < r2->field[i].low || r1->field[i].low > r2->field[i].high)
		{
			return false;
		}
	}
	return true;
}

bool mycomparison(pc_rule* first,pc_rule* second)
{
  return (first->priority < second->priority);
}

bool myequal(pc_rule* first,pc_rule* second)
{
  return (first->priority == second->priority);
}

bool mystatsort(TreeStat* first,TreeStat* second)
{
  if (first->Max_Depth > second->Max_Depth)
  {
    return true;
  }
  else
  {
    if (first->Max_Depth == second->Max_Depth)
    {
      if (first->total_memory > second->total_memory)
      {
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }
}

bool mymemsort(MemBin* first,MemBin* second)
{
  if (first->Max_Depth < second->Max_Depth)
  {
    return true;
  }
  else
  {
    if (first->Max_Depth == second->Max_Depth)
    {
      if (first->total_memory < second->total_memory)
      {
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }
}

int CheckIPBounds(range fld)
{
  if (fld.low > 0xFFFFFFFF)
  {
    printf("Error: IPRange is buggy!(%llu)\n",fld.low);
    return 1;
  }
  if (fld.high > 0xFFFFFFFF)
  {
    printf("Error: IPRange is buggy!(%llu)\n",fld.high);
    return 1;
  }
  if (fld.low > fld.high)
  {
    printf("Error: IPRange is buggy!(%llu - %llu)\n",fld.low,fld.high);
    return 1;
  }
  return 0;
}

int CheckPortBounds(range fld)
{
  if (fld.low > 0xFFFF)
  {
    printf("Error: PortRange is buggy!(%llu)\n",fld.low);
    return 1;
  }
  if (fld.high > 0xFFFF)
  {
    printf("Error: PortRange is buggy!(%llu)\n",fld.high);
    return 1;
  }
  if (fld.low > fld.high)
  {
    printf("Error: PortRange is buggy!(%llu - %llu)\n",fld.low,fld.high);
    return 1;
  }
  return 0;
}

int CheckProtoBounds(range fld)
{
  if (fld.low > 0xFF)
  {
    printf("Error: ProtoRange is buggy!(%llu)\n",fld.low);
    return 1;
  }
  if (fld.high > 0xFF)
  {
    printf("Error: ProtoRange is buggy!(%llu)\n",fld.high);
    return 1;
  }
  if (fld.low > fld.high)
  {
    printf("Error: ProtoRange is buggy!(%llu - %llu)\n",fld.low,fld.high);
    return 1;
  }
  return 0;
}

bool IsPowerOfTwo(int x)
{
  return (x & (x - 1)) == 0;
}

bool is_present(pc_rule boundary,pc_rule *rule)
{
  if ( ((rule->field[0].low  <= boundary.field[0].low  && rule->field[0].high >= boundary.field[0].low)  ||  // cuts to the left of range
        (rule->field[0].high >= boundary.field[0].high && rule->field[0].low  <= boundary.field[0].high) ||  // cuts to the right of range
        (rule->field[0].low  >= boundary.field[0].low  && rule->field[0].high <= boundary.field[0].high)) && // completely inside the range
      ((rule->field[1].low  <= boundary.field[1].low  && rule->field[1].high >= boundary.field[1].low)  ||  // cuts to the left of range
       (rule->field[1].high >= boundary.field[1].high && rule->field[1].low  <= boundary.field[1].high) ||  // cuts to the right of range
       (rule->field[1].low  >= boundary.field[1].low  && rule->field[1].high <= boundary.field[1].high)) && // completely inside the range
      ((rule->field[2].low  <= boundary.field[2].low  && rule->field[2].high >= boundary.field[2].low)  ||  // cuts to the left of range
       (rule->field[2].high >= boundary.field[2].high && rule->field[2].low  <= boundary.field[2].high) ||  // cuts to the right of range
       (rule->field[2].low  >= boundary.field[2].low  && rule->field[2].high <= boundary.field[2].high)) && // completely inside the range
      ((rule->field[3].low  <= boundary.field[3].low  && rule->field[3].high >= boundary.field[3].low)  ||  // cuts to the left of range
       (rule->field[3].high >= boundary.field[3].high && rule->field[3].low  <= boundary.field[3].high) ||  // cuts to the right of range
       (rule->field[3].low  >= boundary.field[3].low  && rule->field[3].high <= boundary.field[3].high)) && // completely inside the range
      ((rule->field[4].low  <= boundary.field[4].low  && rule->field[4].high >= boundary.field[4].low)  ||  // cuts to the left of range
       (rule->field[4].high >= boundary.field[4].high && rule->field[4].low  <= boundary.field[4].high) ||  // cuts to the right of range
       (rule->field[4].low  >= boundary.field[4].low  && rule->field[4].high <= boundary.field[4].high)) )  // completely inside the range
  {
    return true;
  }
  else
  {
    return false;
  }

}

bool is_equal(pc_rule rule1,pc_rule rule2, pc_rule boundary)
{
  int count = 0;
  range r1, r2;
  for (int i = 0;i < MAXDIMENSIONS;i++)
  {
    if (rule1.field[i].low > boundary.field[i].low) {
      r1.low = rule1.field[i].low;
    } else {
      r1.low = boundary.field[i].low;
    }
    if (rule1.field[i].high < boundary.field[i].high) {
      r1.high = rule1.field[i].high;
    } else {
      r1.high = boundary.field[i].high;
    }
    if (rule2.field[i].low > boundary.field[i].low) {
      r2.low = rule2.field[i].low;
    } else {
      r2.low = boundary.field[i].low;
    }
    if (rule2.field[i].high < boundary.field[i].high) {
      r2.high = rule2.field[i].high;
    } else {
      r2.high = boundary.field[i].high;
    }
    if (r1.low <= r2.low && r1.high >= r2.high)
    {
      count++;
    }
  }

  if (count == MAXDIMENSIONS)
    return true;
  else
    return false;
}

int samerules(node * r1, node * r2) {
  if (r1->classifier.empty() || r2->classifier.empty()) {
    return 0;
  }
  if (r1->classifier.size() != r2->classifier.size()) {
    return 0;
  }
  int found = 0;
  int num = 0;
  for (list<pc_rule*>::iterator itr1 = r1->classifier.begin();itr1 != r1->classifier.end();itr1++) {
    found = 0;
    for (list<pc_rule*>::iterator itr2 = r2->classifier.begin();itr2 != r2->classifier.end();itr2++) {
      if ((**itr1).priority == (**itr2).priority) {
        found = 1;
        num++;
        break;
      }
    }
    if (!found) {
      return 0;
    }
  }
  if (num != r1->classifier.size()) {
    printf("ERROR: Too many or too few rules matched\n");
  }
  return 1;
}

int ColorOfList(list<pc_rule*> rules, unsigned long long *pt)
{
	for (list<pc_rule*>::iterator iter = rules.begin(); iter != rules.end(); iter++)
	{
		bool isMatch = true;
		for (int d = 0; d < MAXDIMENSIONS; d++)
		{
			if (pt[d] < (*iter)->field[d].low || pt[d] > (*iter)->field[d].high)
			{
				isMatch = false;
				break;
			}
		}
		if (isMatch)
		{
			return (*iter)->priority;
		}
	}

	// No match
	return -1;
}

int ColorOfTree(node* tree, unsigned long long * pt)
{
	for (int dim = 0; dim < MAXDIMENSIONS; dim++)
	{
		if (pt[dim] < tree->boundary.field[dim].low || pt[dim] > tree->boundary.field[dim].high)
			return -1; // Out of bounds
	}

	if (tree->actual_children.size() == 0)
	{
		// Check list
		return ColorOfList(tree->classifier, pt);
	}
	else
	{
		// Check children

		int color = -1;

		for (list<node*>::iterator iter = tree->actual_children.begin();
				iter != tree->actual_children.end(); iter++)
		{
			if (DoesRuleContainPoint(&(*iter)->boundary, pt))
			{
				int c = ColorOfTree(*iter, pt);
				if (color < 0 || (c < color && c >= 0))
					color = c;
			}
		}

		return color;

		/**
		// Apparently they have some combination of keeping around multiple copies and not doing so
		// Which makes figuring out which child we want a complete pain in the butt.
		// So we don't actually try to compute it because it will be wrong

		// Compute Index
		int index = 0;
		cout << "Building an index" << endl;

		for (int i = 0; i < MAXDIMENSIONS; i++)
		{
			range bound = tree->boundary.field[i];
			unsigned long long boundSize = bound.high - bound.low + 1;
			int numParts = tree->cuts[i];
			boundSize /= numParts;
			int x = (pt[i] - bound.low);
			int dimIndex = x / boundSize;

			if (dimIndex >= numParts)
			{
				cout << "bound: [" << bound.low << "," << bound.high << "]" << endl;
				cout << "point: " << pt[i] << endl;
				cout << "num cuts: " << numParts << endl;
				cout << "index: " << dimIndex << endl;
			}

			index *= numParts;
			index += dimIndex;

			cout << numParts << endl;
		}

		cout << "Index is " << index << endl;

		if (index > tree->children.size())
		{
			cout << "... but there are only " << tree->children.size() << " children" << endl;
			cout << tree->actual_children.size() << endl;
			cout << tree->Row << " " << tree->Column << endl;
			exit(1);
		}

		// <rant>why is it a list instead of a vector? we need random access</rant>
		list<node*>::iterator iter = tree->children.begin();
		for (int i = 0; i < index; i++)
		{
			iter++;
		}

		return ColorOfTree(*iter, pt);

		*/
	}

	// No match
	return -1;
}

int ColorOfTrees(list<TreeDetails> trees, unsigned long long *pt)
{
	int color = -1;

	for (list<TreeDetails>::iterator iter = trees.begin(); iter != trees.end(); iter++)
	{
		int c = ColorOfTree(iter->root, pt);
        cout<<c<<endl;
		//cout << "Found color: " << c << endl;
		if (color < 0 || (c < color && c >= 0))
			color = c;
	}

	return color;
}

bool CheckRule(list<TreeDetails> trees, list<pc_rule*> rules, pc_rule* rule,
		unsigned long long *pt, int dim)
{
	if (dim >= MAXDIMENSIONS)
	{
		//cout << "Searching for rule!" << endl;
		// Compare tree to list at the given point
		int listColor = ColorOfList(rules, pt);
		//cout << "Expected color: " << listColor << endl;
		int treeColor = ColorOfTrees(trees, pt);

		if (listColor == treeColor)
		{
			return true;
		}
		else
		{
			PrintRuleList(rules);
			for (int i = 0; i < MAXDIMENSIONS; i++)
			{
				cout << pt[i] << " ";
			}
			cout << endl;
			cout << "expected " << listColor << " found " << treeColor << endl;

			return false;
		}
	}
	else if (rule->field[dim].low > rule->field[dim].high)
	{
		// Rule has no area : vacuously true
		return true;
	}
	else
	{
		// Build the point up
		// Test lower outside
		if (rule->field[dim].low > 0)
		{
			pt[dim] = rule->field[dim].low - 1;
			if (!CheckRule(trees, rules, rule, pt, dim + 1))
				return false;
		}

		// Test upper outside
		if (rule->field[dim].high < UpperBounds[dim])
		{
			pt[dim] = rule->field[dim].high - 1;
			if (!CheckRule(trees, rules, rule, pt, dim + 1))
				return false;
		}

		// Test lower inside
		pt[dim] = rule->field[dim].low;
		if (!CheckRule(trees, rules, rule, pt, dim + 1))
			return false;

		// Test upper inside
		pt[dim] = rule->field[dim].high;
		if (!CheckRule(trees, rules, rule, pt, dim + 1))
			return false;

		return true;
	}
}

bool CheckTrees(list<TreeDetails> trees, list<pc_rule*> rules)
{
	for (list<pc_rule*>::iterator iter = rules.begin(); iter != rules.end(); iter++)
	{
		unsigned long long pt[MAXDIMENSIONS];
		if (!CheckRule(trees, rules, *iter, pt, 0))
			return false;
	}
#ifndef KUN_SPEED_TEST
	cout << "All Matched" << endl;
#endif
	return true;
}
