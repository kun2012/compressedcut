#pragma once
#ifndef EFFI_COMPRESSED_H
#define EFFI_COMPRESSED_H

#include <stdio.h>

#include <list>
#include <map>
#include <vector>

/**
 * Header file for the EffiCuts program
 * Original program written by Balajee Vamanan, Gwendolyn Voskuilen and T. N. Vijaykumar
 * of Purdue University
 * Changes made by James Daly as marked
 */
extern int MAXDIMENSIONS;
extern int numReps;
#define MAX_ALLOWED_LEVELS 200

#define PTR_SIZE 4
#define HEADER_SIZE 4
#define BOUNDARY_SIZE 16
#define INTERVAL_SIZE 2

#define INTERNAL_NODE_SIZE (HEADER_SIZE + BOUNDARY_SIZE + PTR_SIZE)
#define LEAF_NODE_SIZE HEADER_SIZE

#define INTERNAL_COST_64	3
#define INTERNAL_COST_128	2
#define RULE_COST_64	3
#define RULE_COST_128	2

#define MAX_MEMBINS 4

#define TOO_MUCH 16

using namespace std;


struct range{
  unsigned long long low;
  unsigned long long high;
};

struct pc_rule{

  pc_rule() { field.resize(MAXDIMENSIONS); }

  int priority;
  //struct range field[MAXDIMENSIONS];
  vector<struct range> field;
  int siplen, diplen;
  unsigned sip[4], dip[4];
};

struct node 
{
  int depth;
  int problematic;
  int node_has_rule;
  pc_rule boundary;
  list <pc_rule*> classifier;
  list <node *> children;
  list <node *> actual_children;
  vector<int> cuts;
  //int cuts[MAXDIMENSIONS];
  // this is used only if this node is a result 
  // of cutting in 2D
  int Row;
  int Column;
  int Index;

  bool is_compressed;
  
  int count;
  
public:
	node();
	~node();
};

struct TreeStat
{
  // independent vars
  int Id;
  int No_Rules;
  int Max_Depth;
  int Max_Levels;
  int Max_Cuts;
  int Max_Access64Bit;
  int Max_Access128Bit;
  int Rules_at_the_Leaf;
  int Rules_along_path;
  unsigned long long Total_Rule_Size;
  unsigned long long Total_Rules_Moved_Up;
  unsigned long long Total_Array_Size;
  unsigned long long Node_Count;
  unsigned long long Problematic_Node_Count;
  unsigned long long NonLeaf_Node_Count;
  unsigned long long Compressed_NonLeaf_Node_Count;
  unsigned long long Uncompressed_NonLeaf_Node_Count;
  map <unsigned,unsigned long long> interval_per_node;
  map <unsigned,unsigned long long> cuts_per_node;
  // dependent vars
  unsigned long long ruleptr_memory;
  unsigned long long array_memory;
  unsigned long long leaf_node_memory;
  unsigned long long compressed_int_node_memory;
  unsigned long long uncompressed_int_node_memory;
  unsigned long long total_memory;
  unsigned long long total_memory_in_KB;
};

struct MemBin
{
  int Max_Depth;
  int Max_Levels;
  int Max_Access64Bit;
  int Max_Access128Bit;
  unsigned long long total_memory;
  list<TreeStat*> Trees;
};

struct TreeDetails
{
	TreeDetails() { wideFields.resize(MAXDIMENSIONS); }
	node* root;
	//bool wideFields[MAXDIMENSIONS];
	vector<bool> wideFields;
};

/*
 * Function declarations added by James Daly
 * Mostly for my own sanity
 */
//Comparers
bool mycomparison(pc_rule* first,pc_rule* second);
bool myequal(pc_rule* first,pc_rule* second);
bool mystatsort(TreeStat* first,TreeStat* second);
bool mymemsort(MemBin* first,MemBin* second);
//Checkers
int CheckIPBounds(range fld);
int CheckPortBounds(range fld);
int CheckProtoBounds(range fld);

void parseargs(int argc, char *argv[]);
void ClearMem(node *curr_node);
bool IsPowerOfTwo(int x);

pc_rule get_bound(node *curr_node,int *offset);
bool is_present(pc_rule boundary,pc_rule *rule);
void modifyrule(pc_rule boundary,pc_rule *rule);
bool is_equal(pc_rule rule1,pc_rule rule2, pc_rule boundary);
void remove_redund(node *curr_node);

void calc_dimensions_to_cut(node *curr_node,int *select_dim);
void cp_node(node* src,node* dest);
void calc_num_cuts_1D(node *root,int dim);
void LinearizeChildren(int RowSize);
void SortChildren();
void calc_num_cuts_2D(node *root,int *dim);
void calc_cuts(node *curr_node);
void createBoundary(node *a,node *b,node *c);
int LogicalMerge(node* a,node* b,int Max);
bool NodeCompress(list <node*> &nodelist);

void InitStats(int No_Rules);
void NodeStats(node *curr_node);

void InterValHist(map <unsigned,unsigned long long> interval_per_node);
void CutsHist(map <unsigned,unsigned long long> cuts_per_node);

void PrintStatRecord(TreeStat *p_record);
void PrintStats();
void RecordTreeStats();

void moveRulesUp(node* curr_node);
int samerules(node * r1, node * r2);
list<node*> nodeMerging(node * curr_node); 
void regionCompaction(node * curr_node);
void create_tree(list <pc_rule*> p_classifier);
//void node* CreateTreeFromRuleList(list<pc_rule*> p_classifier);

void IP2Range(unsigned ip1,unsigned ip2,unsigned ip3,unsigned ip4,unsigned iplen,pc_rule *rule,int index);
int loadrule(FILE *fp);
void binRules(list<pc_rule> & ruleList);
void MergeTrees();
void LoadRulePtr(list <pc_rule> &rule_list,list <pc_rule*> &ruleptr_list,int start,int end);
void BinPack(int bins,list <TreeStat*> Statistics);
int ComputeCutoffs();

// Functions written by J. Daly
bool DoRulesIntersect(pc_rule* r1, pc_rule* r2);

unsigned long long BestSplitPoint(node * current, int dim);
void CalcMultiEquiCuts1D(node *root, int dim);
list<node*> CalcEquiCuts1D(node *root, int dim);
void CalcEquiCuts2D(node *root, int* dim);
node* SpawnChild(node* parent);

void AddRuleToTree(node *node, pc_rule* rule);
void RemoveRuleFromTree(node *node, pc_rule* rule);

void CutRecursive(node *curr_node);
void CutNode(node *curr_node, list<node*> &topush);
void StatNode(node* currNode);

void Treeify(list<TreeDetails> &trees, list<pc_rule*> &rules);
node* FindSuitableTree(list<TreeDetails> & trees, pc_rule* rule);

void PrintRule(pc_rule* rule);
void PrintNode(node* node);
void PrintRuleList(list<pc_rule*> &rules);

int ColorOfList(list<pc_rule*> rules, unsigned long long *pt);
int ColorOfTree(node* tree, unsigned long long * pt);
int ColorOfTrees(list<TreeDetails> trees, unsigned long long *pt);
bool CheckTrees(list<TreeDetails> trees, list<pc_rule*> rules);
#endif
