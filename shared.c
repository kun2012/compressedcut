#include "shared.h"

int Num_Junk;

unsigned long long Percents[NUM_JUNK] =
{
  83,
  10,
  4,
  2,
  1
};

vector<unsigned long long> UpperBounds;

unsigned long long Cutoffs[NUM_JUNK];

int bucketSize = 16;
double spfac = 8.0;
FILE *fpr;
FILE *fpt; //Added by kun for trace file
int hypercuts = 1;
int compressionON = 1; //kun: compress contiguous sibling nodes or not
int binningON = 1; //kun: separable trees
int mergingON = 1; //kun: selective tree merging
int fineOn = 1; //kun: use equi-dense cut or not
int num_intervals = 7; //kun: upperbound of a node's children number
//kun: identifying separable rules
double bin = 0.5; //for non-ip dimension
double IPbin = 0.05; //for ip dimension
int thirtyone = 0; //thirtyone == 1 --> make a difference between rules with one and no wildcards
int Num_Rules_Moved_Up = 0; //kun: compressedcut do not used the move up optimation.

int trace_rule_num; //Added by kun

int numTrees = 0;

// tree related
list <pc_rule> classifier;
list <pc_rule*> p_classifier;
int numrules=0;
node *root;
list <node*> childlist;

int rulelists[31];
list<pc_rule*> bigrules[5];
list<pc_rule*> kindabigrules[10];
list<pc_rule*> mediumrules[10];
list<pc_rule*> littlerules[5];
list<pc_rule*> smallrules;

int Num_Partitions;
int Avg_Degree;
int Max_Degree;
unsigned long long Max_WorklistSize;
// Statistics
// live records
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
// accumulated records
int treecount = 0;
TreeStat* p_record;
list <TreeStat*> Statistics;

int updateReads = 0;
int updateWrites = 0;

int setNumReps(int reps) {
	MAXDIMENSIONS = DIMS_PER_REP * reps;
	numReps = reps;
	UpperBounds.resize(MAXDIMENSIONS);
	for (int i = 0; i < reps; i++) {
		int index = i * DIMS_PER_REP;
		UpperBounds[index] = 4294967295;
		UpperBounds[index + 1] = 4294967295;
		UpperBounds[index + 2] = 65535;
		UpperBounds[index + 3] = 65535;
		UpperBounds[index + 4] = 255;
	}
}
