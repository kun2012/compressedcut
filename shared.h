#pragma once
#ifndef EFFI_SHARED
#define EFFI_SHARED

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

#define DIMS_PER_REP 5
#define NUM_JUNK 5

extern int Num_Junk;

extern unsigned long long Percents[NUM_JUNK];

extern vector<unsigned long long> UpperBounds;

extern unsigned long long Cutoffs[NUM_JUNK];

extern int bucketSize;
extern double spfac;
extern FILE *fpr;
extern int hypercuts;
extern int compressionON;
extern int binningON;
extern int mergingON;
extern int fineOn;
extern int num_intervals;
extern double bin;
extern double IPbin;
extern int thirtyone;
extern int Num_Rules_Moved_Up;

extern int numTrees;

// tree related
extern list <pc_rule> classifier;
extern list <pc_rule*> p_classifier;
extern int numrules;
extern node *root;
extern list <node*> childlist;

extern int rulelists[31];
extern list<pc_rule*> bigrules[5];
extern list<pc_rule*> kindabigrules[10];
extern list<pc_rule*> mediumrules[10];
extern list<pc_rule*> littlerules[5];
extern list<pc_rule*> smallrules;

extern int Num_Partitions;
extern int Avg_Degree;
extern int Max_Degree;
extern unsigned long long Max_WorklistSize;
// Statistics
// live records 
extern int Max_Depth;
extern int Max_Levels;
extern int Max_Cuts;
extern int Max_Access64Bit;
extern int Max_Access128Bit;
extern int Rules_at_the_Leaf;
extern int Rules_along_path;
extern unsigned long long Total_Rule_Size;
extern unsigned long long Total_Rules_Moved_Up;
extern unsigned long long Total_Array_Size;
extern unsigned long long Node_Count;
extern unsigned long long Problematic_Node_Count;
extern unsigned long long NonLeaf_Node_Count;
extern unsigned long long Compressed_NonLeaf_Node_Count;
extern unsigned long long Uncompressed_NonLeaf_Node_Count;
extern map <unsigned,unsigned long long> interval_per_node;
extern map <unsigned,unsigned long long> cuts_per_node;
// accumulated records
extern int treecount;
extern TreeStat* p_record;
extern list <TreeStat*> Statistics;

extern int updateReads;
extern int updateWrites;

int setNumReps(int reps);
#endif