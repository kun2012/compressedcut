
using namespace std;

//#include "compressedcuts.h"
#include "shared.h"
#include <unistd.h>

void ClearMem(node *curr_node)
{
	// curr_node->classifier.clear();
	// curr_node->children.clear();
	// delete curr_node;
};

void PrintRule(pc_rule* rule)
{
	for (int i = 0; i < MAXDIMENSIONS; i++)
	{
		printf("[%u,%u]\t", (unsigned int)rule->field[i].low, (unsigned int)rule->field[i].high);
	}
	printf("%u\n", rule->priority);
}

void PrintNode(node *curr_node)
{
	printf("Node at depth %u\n", curr_node->depth);
	PrintRule(&curr_node->boundary);
	printf("%u rules\n", (unsigned int)curr_node->classifier.size());
	PrintRuleList(curr_node->classifier);
}

void PrintRuleList(list<pc_rule*> &rules)
{
	for (list<pc_rule*>::iterator iter = rules.begin();
			iter != rules.end(); iter++)
	{
		PrintRule(*iter);
	}
}

pc_rule get_bound(node *curr_node,int *offset)
{
	pc_rule boundary;
	unsigned long long interval;

	for (int i = 0;i < MAXDIMENSIONS;i++)
	{
		interval = curr_node->boundary.field[i].high - curr_node->boundary.field[i].low + 1;
		interval = interval / curr_node->cuts[i];
		boundary.field[i].low = curr_node->boundary.field[i].low + offset[i] * interval;
		if (offset[i] == curr_node->cuts[i] - 1)
			boundary.field[i].high = curr_node->boundary.field[i].high;
		else 
			if (interval == 0)
				boundary.field[i].high = boundary.field[i].low;
			else
				boundary.field[i].high = boundary.field[i].low + interval - 1;
	}
	// check the node's bounds
	if (CheckIPBounds(boundary.field[0]))
	{
		printf("Error: get_bound bounds check for 0 failed\n");
		printf("[%llu - %llu] => [%llu - %llu] @ %d\n",
			curr_node->boundary.field[0].low,curr_node->boundary.field[0].high,
			boundary.field[0].low,boundary.field[0].high,curr_node->cuts[0]);
		exit(1);
	}
	if (CheckIPBounds(boundary.field[1]))
	{
		printf("Error: get_bound bounds check for 1 failed\n");
		printf("[%llu - %llu] => [%llu - %llu] @ %d\n",
			curr_node->boundary.field[1].low,curr_node->boundary.field[1].high,
			boundary.field[1].low,boundary.field[1].high,curr_node->cuts[1]);
		exit(1);
	}
	if (CheckPortBounds(boundary.field[2]))
	{
		printf("Error: get_bound bounds check for 2 failed\n");
		printf("[%llu - %llu] => [%llu - %llu] @ %d\n",
			curr_node->boundary.field[2].low,curr_node->boundary.field[2].high,
			boundary.field[2].low,boundary.field[2].high,curr_node->cuts[2]);
		exit(1);
	}
	if (CheckPortBounds(boundary.field[3]))
	{
		printf("Error: get_bound bounds check for 3 failed\n");
		printf("[%llu - %llu] => [%llu - %llu] @ %d\n",
			curr_node->boundary.field[3].low,curr_node->boundary.field[3].high,
			boundary.field[3].low,boundary.field[3].high,curr_node->cuts[3]);
		exit(1);
	}
	if (CheckProtoBounds(boundary.field[4]))
	{
		printf("Error: get_bound bounds check for 4 failed\n");
		printf("[%llu - %llu] => [%llu - %llu] @ %d\n",
			curr_node->boundary.field[4].low,curr_node->boundary.field[4].high,
			boundary.field[4].low,boundary.field[4].high,curr_node->cuts[4]);
		exit(1);
	}
	return boundary;
}

void parseargs(int argc, char *argv[]) {
	int	c;
	bool	ok = 1;
	while ((c = getopt(argc, argv, "b:s:r:h:m:c:f:n:i:t:u:g:z:F:")) != -1) {
		switch (c) {
			case 'b':
				bucketSize = atoi(optarg);
				break;
			case 's':
				spfac = atof(optarg);
				break;
			case 'f':
				num_intervals = atoi(optarg);
				break;
			case 'r':
				fpr = fopen(optarg, "r");
				break;
			case 'm':
				hypercuts = atoi(optarg);
				break;
			case 'u':
				Num_Rules_Moved_Up = atoi(optarg);
				break;
			case 'c':
				compressionON = atoi(optarg);
				break;
			case 'g':
				binningON = atoi(optarg);
				break;
			case 'z':
				mergingON = atoi(optarg);
				break;
			case 'F':
				fineOn = atoi(optarg);
				break;
			case 'R':
				setNumReps(atoi(optarg));
				break;
			case 'n':
				bin = atof(optarg);
				break;
			case 'i':
				IPbin = atof(optarg);
				break;
			case 't':
				thirtyone = atoi(optarg);
				break;
			case 'h':
				printf("compressedcuts [-b bucketSize][-s spfac][-f comfac][-r ruleset][-m (0|1)][-u pushup][-c (0|1)][-n bin][-i IPbin][-t 0|1][-h]\n");
				exit(1);
				break;
			default:
				ok = 0;
		}
	}

	if(bucketSize <= 0){
		printf("bucketSize should be > 0\n");
		ok = 0;
	}	

	if(spfac < 0){
		printf("space factor should be >= 0\n");
		ok = 0;
	}

	if (compressionON > 1) {
		printf("c can be only 0 - no compress, 1 - linear\n");
		ok = 0;
	}

	if (binningON > 2) {
		printf("g can be only 0 - no binning, 1 - binning 2 - static\n");
		ok = 0;
	}

	if (hypercuts > 1) {
		printf("m can be only 0 - hicut, 1 - hypercut\n");
		ok = 0;
	}

	if(num_intervals < 0){
		printf("num_intervals should be >= 0\n");
		ok = 0;
	}

	if(fpr == NULL){
		printf("can't open ruleset file\n");
		ok = 0;
	}

	if (bin < 0.0 || bin > 1.0)
	{
		printf("bin should be [0,1]\n");
		ok = 0;
	}
	if (IPbin < 0.0 || IPbin > 1.0)
	{
		printf("IP bin should be [0,1]\n");
		ok = 0;
	}
	if (!ok || optind < argc) {
		//fprintf (stderr, "hypercut [-c (0|1)][-b bucketSize][-s spfac][-r ruleset][-c (0|1)][-f comfac][-n bin]\n");

		printf("compressedcuts [-b bucketSize][-s spfac][-f comfac][-r ruleset][-m (0|1)][-u pushup][-c (0|1)][-n bin][-i IPbin][-t 0|1][-h]\n");
		//fprintf (stderr, "Type \"hypercut -h\" for help\n");
		fprintf (stderr, "Type \"compressedcuts -h\" for help\n");
		exit(1);
	}

	printf("******************************************\n");
	printf("Bucket Size =	%d\n", bucketSize);
	printf("Space Factor = %f\n", spfac);
	printf("bin = %f\n",bin);
	printf("IPbin = %f\n",IPbin);
	printf("hypercuts = %d\n",hypercuts);
	printf("Num_Rules_Moved_Up = %d\n",Num_Rules_Moved_Up);
	printf("compressionON = %d\n",compressionON);
	printf("binningON = %d\n",binningON);
	printf("mergingON = %d\n",mergingON);
	printf("num_intervals = %d\n",num_intervals);
	printf("******************************************\n");
}

void cp_node(node* src,node* dest)
{
	dest->depth = src->depth;

	dest->boundary.priority = src->boundary.priority;
	for (int i = 0;i < MAXDIMENSIONS;i++)
	{
		dest->boundary.field[i].low	= src->boundary.field[i].low;
		dest->boundary.field[i].high = src->boundary.field[i].high;
	}

	dest->classifier = src->classifier;

	dest->children = src->children;

	for (int i = 0;i < MAXDIMENSIONS;i++)
		dest->cuts[i] = src->cuts[i];

	dest->Row = src->Row;
	dest->Column = src->Column;

	dest->Index = src->Index;

	dest->is_compressed = src->is_compressed;

}

void ReadIPRange(FILE* fp, pc_rule &rule, int dim)
{
	unsigned int ip1, ip2, ip3, ip4, iplen;
	if (fscanf(fp, "%u.%u.%u.%u/%u", &ip1, &ip2, &ip3, &ip4, &iplen) != 5) {
		printf("ill-formatted IP rule\n");
		exit(-1);
	}
	//printf("%d.%d.%d.%d/%d\n", ip1, ip2, ip3, ip4, iplen);
	IP2Range(ip1, ip2, ip3, ip4, iplen, &rule, dim);
}

void ReadPortRange(FILE* fp, pc_rule &rule, int dim)
{
	unsigned int min, max;
	if (fscanf(fp, "%d : %d", &min, &max) != 2) {
		printf("ill-formatted port field\n");
		exit(-1);
	}
	//printf("%d : %d\n", min, max);
	rule.field[dim].low = min;
	rule.field[dim].high = max;
}

void ReadProtocol(FILE* fp, pc_rule &rule, int dim)
{
	char dump = 0;
	unsigned int proto = 0, len = 0;
	//if (fscanf(fp, "%c%c%x%c%c%c%x", &dump, &dump, &proto, &dump, &dump, &dump, &len) != 7) {
	if (fscanf(fp, "%x/%x", &proto, &len) != 2) {
		printf("Ill-formatted protocol field\n");
		exit(-1);
	}
	//printf("%d/%d\n", proto, len);
	if (len == 0xff) {
		rule.field[dim].low = proto;
		rule.field[dim].high = proto;
	} else {
		rule.field[dim].low = 0x0;
		rule.field[dim].high = 0xff;
	}
}

int loadrule(FILE * fp) {
	char validfilter;
	pc_rule rule;
	
	int ruleCount = 0;
	
	while (!feof(fp)) {
		fscanf(fp, "%c", &validfilter);
		if (validfilter != '@') continue;
		
		printf("%d\n", ruleCount);
		
		for (int rep = 0; rep < numReps; rep++) {
			int dim = rep * DIMS_PER_REP;
			ReadIPRange(fp, rule, dim);
			ReadIPRange(fp, rule, dim + 1);
			ReadPortRange(fp, rule, dim + 2);
			ReadPortRange(fp, rule, dim + 3);
			ReadProtocol(fp, rule, dim + 4);
		}
		classifier.push_back(rule);
		ruleCount++;
	}
	return ruleCount;
}

// int loadrule(FILE *fp) {
	// int i = 0;
	// int wild = 0;
	// unsigned sip1, sip2, sip3, sip4, siplen;
	// unsigned dip1, dip2, dip3, dip4, diplen;
	// unsigned proto, protomask;
	// unsigned junk, junkmask;

	// pc_rule rule;

	// while(1) {
		// wild = 0;
		// if(fscanf(fp,"@%u.%u.%u.%u/%u\t%u.%u.%u.%u/%u\t%llu : %llu\t%llu : %llu\t%x/%x\t%x/%x\n",
					// &sip1, &sip2, &sip3, &sip4, &siplen, &dip1, &dip2, &dip3, &dip4, &diplen, 
					// &rule.field[2].low, &rule.field[2].high, &rule.field[3].low, &rule.field[3].high,
					// &proto, &protomask, &junk, &junkmask) != 18) break;
		// rule.siplen = siplen;
		// rule.diplen = diplen;
		// rule.sip[0] = sip1;
		// rule.sip[1] = sip2;
		// rule.sip[2] = sip3;
		// rule.sip[3] = sip4;
		// rule.dip[0] = dip1;
		// rule.dip[1] = dip2;
		// rule.dip[2] = dip3;
		// rule.dip[3] = dip4;

		// IP2Range(sip1,sip2,sip3,sip4,siplen,&rule,0);
		// IP2Range(dip1,dip2,dip3,dip4,diplen,&rule,1);

		// if(protomask == 0xFF){
			// rule.field[4].low = proto;
			// rule.field[4].high = proto;
		// }else if(protomask == 0){
			// rule.field[4].low = 0;
			// rule.field[4].high = 0xFF;
			// wild++;
		// }else{
			// printf("Protocol mask error\n");
			// return 0;
		// }
		// rule.priority = i;
		// if ((rule.field[2].low == 0) && (rule.field[2].high == 65535)) {
			// wild++;
		// }
		// if ((rule.field[3].low == 0) && (rule.field[3].high == 65535)) {
			// wild++;
		// }
		// if (wild != 5) {
			// classifier.push_back(rule);
		// }
		// i++;
	// }
	// return i;
// }

int ComputeCutoffs()
{
	if (binningON == 0)
	{
		Cutoffs[0] = numrules;
		Num_Junk = 1;
		return 0;
	}
	for (int i = 0;i < NUM_JUNK;i++)
	{
		Cutoffs[i] = numrules * Percents[i] / 100;
		printf("Cutoffs[%d] = %lld\n",i,Cutoffs[i]);
	}
	Num_Junk = NUM_JUNK;
}


void InitStats(int No_Rules)
{
	p_record = new TreeStat;
	p_record->Id = treecount++;
	p_record->No_Rules = No_Rules;

	Max_Depth = 0;
	Max_Levels = 0;
	Max_Cuts = 0;
	Max_Access64Bit = 0;
	Max_Access128Bit = 0;
	Rules_at_the_Leaf = 0;
	Rules_along_path = 0;
	Max_WorklistSize = 0;
	Node_Count = 0;
	Problematic_Node_Count = 0;
	NonLeaf_Node_Count = 0;
	Compressed_NonLeaf_Node_Count = 0;
	Uncompressed_NonLeaf_Node_Count = 0;
	Total_Array_Size = 0;
	Total_Rule_Size = 0;
	Total_Rules_Moved_Up = 0;
	Max_Degree = 0;
	Avg_Degree = 0;
	Num_Partitions = 0;

	interval_per_node.clear();
	cuts_per_node.clear();
}

void NodeStats(node *curr_node)
{

	if (curr_node->problematic == 1)
		Problematic_Node_Count++;


	Num_Partitions = curr_node->cuts[0] * curr_node->cuts[1] * curr_node->cuts[2] * curr_node->cuts[3] * curr_node->cuts[4];

	int mcuts = curr_node->cuts[0];
	for (int di = 1;di < MAXDIMENSIONS;di++)
		if (curr_node->cuts[di] > mcuts)
			mcuts = curr_node->cuts[di];

	if (mcuts > Max_Cuts)
		Max_Cuts = mcuts;

	// checks 
	if ( curr_node->classifier.size() > bucketSize && 
			curr_node->children.size() == 0 && curr_node->problematic == 0) 
	{
		printf("Error: This node is not cut further!\n");
	printf("\tIt has %u rules!\n", (unsigned int)curr_node->classifier.size());
	printf("\tactual-children: %u\n", (unsigned int)curr_node->actual_children.size());
	
	PrintNode(curr_node);
		exit(1);
	}

	if (curr_node->problematic == 1 && curr_node->classifier.size() > TOO_MUCH)
	{
		printf("Error: This problematic node has %d rules!\n",(int)curr_node->classifier.size());
		// Edit: JED 13/10/2014
		// Allow nodes to stop with too many children if they can't split further
		//exit(1);
	}

	if (Num_Partitions != curr_node->children.size() 
			&& curr_node->children.size() != 0 && compressionON == 0)
	{
		printf("Error: num children != partitions!(%d != %d)\n",(int)curr_node->children.size(),(int)Num_Partitions);
		exit(1);
	}

	for (int i = 0;i < MAXDIMENSIONS;++i)
		if (IsPowerOfTwo(curr_node->cuts[i]) == false && !fineOn)
		{
			printf("Error: ncuts[%d] = %d is not a power of 2!\n",i,curr_node->cuts[i]);
			exit(1);
		}

	// check the node's bounds
	if (CheckIPBounds(curr_node->boundary.field[0]))
	{
		printf("Error: NodeStat bounds check for 0 failed\n");
		exit(1);
	}
	if (CheckIPBounds(curr_node->boundary.field[1]))
	{
		printf("Error: NodeStat bounds check for 1 failed\n");
		exit(1);
	}
	if (CheckPortBounds(curr_node->boundary.field[2]))
	{
		printf("Error: NodeStat bounds check for 2 failed\n");
		exit(1);
	}
	if (CheckPortBounds(curr_node->boundary.field[3]))
	{
		printf("Error: NodeStat bounds check for 3 failed\n");
		exit(1);
	}
	if (CheckProtoBounds(curr_node->boundary.field[4]))
	{
		printf("Error: NodeStat bounds check for 4 failed\n");
		exit(1);
	}

	// stats
	Node_Count++;

	if (curr_node->children.size() != 0)
	{
		NonLeaf_Node_Count++;
		if (curr_node->is_compressed == true)
			Compressed_NonLeaf_Node_Count++;
		else
			Uncompressed_NonLeaf_Node_Count++;
	}

	if (curr_node->is_compressed == true && curr_node->children.size() == 0)
	{
		printf("Error: How the heck is leaf node compressed, exiting..\n");
		exit(1);
	}

	
	int Actual_Curr_Level = curr_node->depth;
	

	if (curr_node->is_compressed != true && !fineOn)
		Actual_Curr_Level++;

	if (Actual_Curr_Level > Max_Levels)
	{
		
		Max_Levels = Actual_Curr_Level;
		//printf("[Tree %d] currently at level %d ...with %d children\n",p_record->Id,Max_Levels,curr_node->children.size());
		if (Max_Levels > MAX_ALLOWED_LEVELS)
		{
			printf("Error: [Tree %d] more that %d levels!\n",
					p_record->Id,MAX_ALLOWED_LEVELS);
			exit(1);
		}

		if (curr_node->children.empty())
			Rules_at_the_Leaf = curr_node->classifier.size();

		if (curr_node->node_has_rule == 1)
			Rules_along_path++;
	}

	int depth = curr_node->depth + (curr_node->children.empty() ? curr_node->classifier.size() : 0);
	int cost64 = curr_node->depth * INTERNAL_COST_64 + 
		(curr_node->children.empty() ? curr_node->classifier.size() * RULE_COST_64 : 0);
	int cost128 = curr_node->depth * INTERNAL_COST_128 + 
		(curr_node->children.empty() ? curr_node->classifier.size() * RULE_COST_128 : 0);
	
	
	if (depth > Max_Depth)
		Max_Depth = depth;
	if (cost64 > Max_Access64Bit) {
		Max_Access64Bit = cost64;
	}
	if (cost128 > Max_Access128Bit)
		Max_Access128Bit = cost128;
	
	if (curr_node->children.size() != 0)
		Total_Array_Size += curr_node->children.size();
	else
	{
		Total_Rule_Size += curr_node->classifier.size();
	}

	if (curr_node->children.size() > Max_Degree)
		Max_Degree = curr_node->children.size();

	Avg_Degree += curr_node->children.size();

	// intervals per node
	if (curr_node->children.size() != 0 && curr_node->is_compressed == true)
	{
		map<unsigned,unsigned long long>::iterator iter = interval_per_node.find(curr_node->children.size());
		if (iter != interval_per_node.end())
		{
			unsigned long long count = iter->second;
			count++;
			interval_per_node[curr_node->children.size()] = count;
		}
		else
		{
			interval_per_node[curr_node->children.size()] = 1;
		}
	}

	// cuts per node
	if (curr_node->children.size() != 0)
	{
		map<unsigned,unsigned long long>::iterator iter = cuts_per_node.find(Num_Partitions);
		if (iter != cuts_per_node.end())
		{
			unsigned long long count = iter->second;
			count++;
			cuts_per_node[Num_Partitions] = count;
		}
		else
		{
			cuts_per_node[Num_Partitions] = 1;
		}
	}

}

void InterValHist(map <unsigned,unsigned long long> interval_per_node)
{
	for (map<unsigned,unsigned long long>::iterator iter = interval_per_node.begin();
			iter != interval_per_node.end();++iter)
	{
		printf("I %u,%llu\n",(*iter).first,(*iter).second);
	}
}
void CutsHist(map <unsigned,unsigned long long> cuts_per_node)
{
	for (map<unsigned,unsigned long long>::iterator iter = cuts_per_node.begin();
			iter != cuts_per_node.end();++iter)
	{
		printf("C %u,%llu\n",(*iter).first,(*iter).second);
	}
}

void PrintStatRecord(TreeStat *p_record)
{
	printf("******************************************\n");
	printf("Tree: %d\n",p_record->Id);
	printf("******************************************\n");
	printf("Rules: %d\n",p_record->No_Rules);
	printf("Cost64: %d\n",p_record->Max_Access64Bit);
	printf("Cost128: %d\n",p_record->Max_Access128Bit);
	printf("Depth: %d\n",p_record->Max_Depth);
	printf("Levels: %d\n",p_record->Max_Levels);
	printf("Cuts: %d\n",p_record->Max_Cuts);
	printf("Rules_at_the_Leaf: %d\n",p_record->Rules_at_the_Leaf);
	printf("Rules_along_path: %d\n",p_record->Rules_along_path);
	printf("Total_Rule_Size: %lld\n",p_record->Total_Rule_Size);
	printf("Total_Rules_Moved_Up: %lld\n",p_record->Total_Rules_Moved_Up);
	printf("Total_Array_Size: %lld\n",p_record->Total_Array_Size);
	printf("Node_Count: %lld\n",p_record->Node_Count);
	printf("Problematic_Node_Count: %lld\n",p_record->Problematic_Node_Count);
	printf("NonLeaf_Node_Count: %lld\n",p_record->NonLeaf_Node_Count);
	printf("Compressed_NonLeaf_Node_Count: %lld\n",p_record->Compressed_NonLeaf_Node_Count);
	printf("Uncompressed_NonLeaf_Node_Count: %lld\n",p_record->Uncompressed_NonLeaf_Node_Count);
	printf("------------------------------------------\n");
	printf("ruleptr_memory: %lld\n",p_record->ruleptr_memory);;
	printf("array_memory: %lld\n",p_record->array_memory);;
	printf("leaf_node_memory: %lld\n",p_record->leaf_node_memory);;
	printf("compressed_int_node_memory: %lld\n",p_record->compressed_int_node_memory);;
	printf("uncompressed_int_node_memory: %lld\n",p_record->uncompressed_int_node_memory);;
	printf("total_memory: %lld\n",p_record->total_memory);;
	printf("total_memory_in_KB: %lld\n",p_record->total_memory_in_KB);;
	printf("------------------------------------------\n");
	InterValHist(p_record->interval_per_node);
	printf("------------------------------------------\n");
	CutsHist(p_record->cuts_per_node);
}

void PrintStats()
{
	unsigned long long OVERALL_MEMORY = 0;
	int OVERALL_DEPTH = 0;
	int OVERALL_LEVELS = 0;
	int No_Rules = 0;
	int sumCost64 = 0;
	int sumCost128 = 0;

	for (list<TreeStat*>::iterator iter = Statistics.begin();
				iter != Statistics.end();iter++)
	{
		PrintStatRecord(*iter);
		OVERALL_MEMORY += (*iter)->total_memory_in_KB;
		No_Rules += (*iter)->No_Rules;
		if ((*iter)->Max_Depth > OVERALL_DEPTH)
			OVERALL_DEPTH = (*iter)->Max_Depth;
		if ((*iter)->Max_Levels > OVERALL_LEVELS)
			OVERALL_LEVELS = (*iter)->Max_Levels;
		sumCost64 += (*iter)->Max_Access64Bit;
		sumCost128 += (*iter)->Max_Access128Bit;
	}
	printf("******************************************\n");
	printf("OVERALL_MEMORY: %lld\n",OVERALL_MEMORY);
	printf("OVERALL_DEPTH: %d\n",OVERALL_DEPTH);
	printf("OVERALL_LEVELS: %d\n",OVERALL_LEVELS);
	
	printf("SUM_COST64: %d\n", sumCost64);
	printf("SUM_COST128: %d\n", sumCost128);
	
	printf("Update Reads: %d\n", updateReads);
	printf("Update Writes: %d\n", updateWrites);

	// some final checks
	if (No_Rules != classifier.size())
	{
		printf("Error: Some rules got dropped while binning!\n");
		//exit(1);
	}

}


void PrintTree(node* currNode)
{
	printf("Node: depth %u, %u children\n", (unsigned int)currNode->depth, (unsigned int)currNode->children.size());
	for (list<node*>::iterator iter = currNode->children.begin();
		iter != currNode->children.end(); iter++)
	{
		PrintTree(*iter);
	}
}

void StatNode(node* currNode)
{
	//printf("node size: %u rules %u children %u actual\n", currNode->classifier.size(), currNode->children.size(), currNode->actual_children.size());
	NodeStats(currNode);
	
	if (currNode->count > 0)
	{
		printf("Node has been visited %u times before!\n", currNode->count);
	}
	
	currNode->count++;
	
	for (list<node*>::iterator iter = currNode->actual_children.begin();
			iter != currNode->actual_children.end(); iter++)
	{
		StatNode(*iter);
	}
}

void RecordTreeStats()
{
	p_record->Max_Depth = Max_Depth;
	
	p_record->Max_Access64Bit = Max_Access64Bit;
	p_record->Max_Access128Bit = Max_Access128Bit;

	p_record->Max_Levels = Max_Levels;

	p_record->Max_Cuts = Max_Cuts;

	p_record->Rules_at_the_Leaf = Rules_at_the_Leaf;

	p_record->Rules_along_path = Rules_along_path;
	
	p_record->Total_Rule_Size = Total_Rule_Size;

	p_record->Total_Rules_Moved_Up = Total_Rules_Moved_Up;

	p_record->Total_Array_Size = Total_Array_Size;

	p_record->Node_Count = Node_Count;

	p_record->Problematic_Node_Count = Problematic_Node_Count;

	p_record->NonLeaf_Node_Count = NonLeaf_Node_Count;

	p_record->Compressed_NonLeaf_Node_Count = Compressed_NonLeaf_Node_Count;

	p_record->Uncompressed_NonLeaf_Node_Count = Uncompressed_NonLeaf_Node_Count;

	p_record->interval_per_node = interval_per_node;

	p_record->cuts_per_node = cuts_per_node;

	p_record->ruleptr_memory =	PTR_SIZE * Total_Rule_Size;

	p_record->array_memory = PTR_SIZE * Total_Array_Size;

	p_record->leaf_node_memory = LEAF_NODE_SIZE * (Node_Count - NonLeaf_Node_Count);

	p_record->compressed_int_node_memory = (INTERNAL_NODE_SIZE + INTERVAL_SIZE * num_intervals) * 
																				Compressed_NonLeaf_Node_Count;

	p_record->uncompressed_int_node_memory = INTERNAL_NODE_SIZE * Uncompressed_NonLeaf_Node_Count;

	p_record->total_memory = p_record->ruleptr_memory + p_record->array_memory + p_record->leaf_node_memory 
													+ p_record->compressed_int_node_memory + p_record->uncompressed_int_node_memory;

	p_record->total_memory_in_KB = p_record->total_memory / 1024;

	Statistics.push_back(p_record);

}

void IP2Range(unsigned ip1,unsigned ip2,unsigned ip3,unsigned ip4,unsigned iplen,pc_rule *rule,int index)
{
	unsigned tmp;
	unsigned Lo,Hi;

	if(iplen == 0){
		Lo = 0;
		Hi = 0xFFFFFFFF;
	}else if(iplen > 0 && iplen <= 8) {
		tmp = ip1 << 24;
		Lo = tmp;
		Hi = Lo + (1<<(32-iplen)) - 1;
	}else if(iplen > 8 && iplen <= 16){
		tmp =	ip1 << 24; 
		tmp += ip2 << 16;
		Lo = tmp;
		Hi = Lo + (1<<(32-iplen)) - 1;
	}else if(iplen > 16 && iplen <= 24){
		tmp = ip1 << 24; 
		tmp += ip2 << 16; 
		tmp += ip3 << 8; 
		Lo = tmp;
		Hi = Lo + (1<<(32-iplen)) - 1;
	}else if(iplen > 24 && iplen <= 32){
		tmp = ip1 << 24; 
		tmp += ip2 << 16; 
		tmp += ip3 << 8; 
		tmp += ip4;
		Lo = tmp;
		Hi = Lo + (1<<(32-iplen)) - 1;
	}else{
		printf("Error: Src IP length exceeds 32\n");
		exit(1);
	}

	rule->field[index].low	= Lo;
	rule->field[index].high = Hi;

	if (CheckIPBounds(rule->field[index]))
	{
		printf("Error: IP2Range bounds check for %d failed\n",index);
		exit(1);
	}
	//printf("\t Prefix: %u.%u.%u.%u/%u\n",ip1,ip2,ip3,ip4,iplen);
	//printf("\t Range : %llu : %llu\n",rule->field[index].low,rule->field[index].high);

}

void LoadRulePtr(list <pc_rule> &rule_list,list <pc_rule*> &ruleptr_list,int start,int end)
{
	printf("Rule:%d - %d\n",start,end);
	int count = 0;
	for (list <pc_rule>::iterator i = rule_list.begin();i != rule_list.end();++i) 
	{
		if (count >= start && count <= end)
			ruleptr_list.push_back(&(*i));
		count++;
	}
}
