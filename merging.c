#include "shared.h"

void modifyrule(pc_rule boundary,pc_rule *rule)
{
    for (int i = 0;i < MAXDIMENSIONS;i++)
    {
        if (rule->field[i].low < boundary.field[i].low)
            rule->field[i].low = boundary.field[i].low;
        if (rule->field[i].high > boundary.field[i].high)
            rule->field[i].high = boundary.field[i].high;
    }
}

void remove_redund(node *curr_node)
{
    //kun: do nothing?
}

void remove_redund_skipped(node *curr_node)
{
    list <pc_rule*> rulelist;
    rulelist.clear();
    for (list<pc_rule*>::iterator rule = curr_node->classifier.begin();
            rule != curr_node->classifier.end();++rule)
    {
        int found = 0;
        for (list<pc_rule*>::iterator mule = rulelist.begin();
                mule != rulelist.end();++mule)
        {
            if (is_equal(**mule,**rule, curr_node->boundary) == true)
            {
                found = 1;
        //cout << "Redundant rule" << endl;
            }
        }
        if (found != 1)
        {
            rulelist.push_back(*rule);
        }
    }
    // Now add back the rules
    curr_node->classifier.clear();
    curr_node->classifier = rulelist;
    curr_node->classifier.unique(myequal);
}


void LinearizeChildren(int RowSize)
{
    for (list <node*>::iterator item = childlist.begin();
            item != childlist.end();++item)
    {
        (*item)->Index = (*item)->Column * RowSize + (*item)->Row;
    }

}

void SortChildren()
{
    list <node*> childlist_sorted;

    childlist_sorted.clear();

    for (int i = 0; i < childlist.size();i++)
    {
        int found = 0;
        for (list <node*>::iterator item = childlist.begin();
                item != childlist.end();++item)
        {
            if ((*item)->Index == i)
            {
                childlist_sorted.push_back(*item);
                found = 1;
                break;
            }
        }
        if (found == 0)
        {
            printf("Error: Index = %d not found in childlist\n",i);
            exit(1);
        }
    }

    if (childlist.size() != childlist_sorted.size())
    {
        printf("Error: Please check Index calculation\n");
        exit(1);
    }

    childlist.clear();

    childlist = childlist_sorted;

}


// makes a = a + b
void createBoundary(node *a,node *b,node *c)
{
    for (int i = 0;i < MAXDIMENSIONS;i++)
    {
        list<unsigned long long> EndPoints;
        EndPoints.clear();
        EndPoints.push_back(a->boundary.field[i].low);
        EndPoints.push_back(a->boundary.field[i].high);
        EndPoints.push_back(b->boundary.field[i].low);
        EndPoints.push_back(b->boundary.field[i].high);
        EndPoints.sort();
        c->boundary.field[i].low    = EndPoints.front();
        c->boundary.field[i].high = EndPoints.back();
    }

}

int LogicalMerge(node* a,node* b,int Max)
{
    node *c = new node;
    cp_node(a,c);
    createBoundary(a,b,c);
    c->classifier.clear();


    // create a sort list of rules in a or b in rules
    list<pc_rule*> templist = b->classifier;
    c->classifier = a->classifier;
    c->classifier.merge(templist,mycomparison);
    c->classifier.unique(myequal);

    // remove redundant rules
    remove_redund(c);

    int asize = a->classifier.size();
    int bsize = b->classifier.size();
    int csize = c->classifier.size();
    int maxsize = (asize > bsize) ? asize : bsize;

    if (c->classifier.size() <= bucketSize ||
            ((c->classifier.size() <= maxsize) &&
             (c->classifier.size() < Max)) )
    {
        cp_node(c,a);
        ClearMem(c);
        return 1;
    }
    else
    {
        ClearMem(c);
        return 0;
    }
}


bool NodeCompress(list <node*> &nodelist)
{
    int Max;
    int merge_possible = compressionON;
    int original_size = nodelist.size();
    while (merge_possible)
    {
        merge_possible = 0;

        // find the max. rules among all child nodes
        Max = 0;
        for (list <node*>::iterator item = nodelist.begin();
                item != nodelist.end();++item)
        {
            if ((*item)->classifier.size() > Max)
                Max = (*item)->classifier.size();
        }


        for (list <node*>::iterator item = nodelist.begin();
                item != nodelist.end();++item)
        {
            ++item;
            list <node*>::iterator item_p1 = item;
            --item;
            if (item_p1 != nodelist.end())
            {
                if (LogicalMerge(*item,*item_p1,Max))
                {
                    nodelist.erase(item_p1);
                    ClearMem(*item_p1);
                    merge_possible = 1;
                }
            }
        }
    }

    if (nodelist.size() > original_size)
    {
        printf("Error: Compression resulted in increase of nodes!\n");
        exit(1);
    }

    if (nodelist.size() < original_size)
        return true;
    else
        return false;

}


void moveRulesUp(node* curr_node) {
    curr_node->node_has_rule = 0;
    list<pc_rule*> rulesMovedUp, setToCheck;
    int emptyIntersect = 1;
    rulesMovedUp = ((curr_node->children).front())->classifier; // start with this set
    // get list of rules that exist in all
    for (list <node*>::iterator item = (curr_node->children).begin();item != (curr_node->children).end();++item) {
        if (emptyIntersect) {
            setToCheck.clear();
            setToCheck = rulesMovedUp;
            rulesMovedUp.clear();
            for (list<pc_rule*>::iterator ptr = (*item)->classifier.begin(); ptr != (*item)->classifier.end(); ptr++) {
                for (list<pc_rule*>::iterator setptr = setToCheck.begin();setptr != setToCheck.end();setptr++) {
                    if (*setptr == *ptr) {
                        rulesMovedUp.push_back(*setptr);
                        break;
                    }
                }
            }
            if (rulesMovedUp.empty()) {
                emptyIntersect = 0;
            }
        }
    }

    if (rulesMovedUp.size() > Num_Rules_Moved_Up) {
        // truncate to first bucketSize children
        rulesMovedUp.resize(Num_Rules_Moved_Up);
    }

    // remove duplicate rules from all children
    if (emptyIntersect) {
        for(list<pc_rule*>::iterator setptr = rulesMovedUp.begin();setptr != rulesMovedUp.end();setptr++)
        {
        //setptr--;
        for (list <node*>::iterator item = (curr_node->children).begin();item != (curr_node->children).end();++item) {
            for (list<pc_rule*>::iterator ptr = (*item)->classifier.begin();ptr != (*item)->classifier.end();ptr++) {
                if (*setptr == *ptr) {
                    ptr = (*item)->classifier.erase(ptr);
                    break;
                }
            }
        }
        Total_Rule_Size++;
        Total_Rules_Moved_Up++;
        curr_node->node_has_rule = 1;
        }
    }
    //printf("Rules moved up: %d\n", rulesMovedUp.size());
}



list<node*> nodeMerging(node * curr_node) {
    list<node*> newlist = curr_node->children;
    int num = 0;
    list<node*>::iterator itr2;
/*  for(list<node*>::iterator junk = curr_node->children.begin(); junk != curr_node->children.end();junk++) {
        remove_redund(junk++);
    }*/

    for (list<node*>::iterator itr1 = newlist.begin(); itr1 != newlist.end(); itr1++) {
        itr2 = itr1;
        itr2++;
        while(itr2 != newlist.end()) {
            if (samerules(*itr1, *itr2)) {
                num++;
                //printf("samerules returned true\n");
                for (int i = 0; i < MAXDIMENSIONS; i++) {
                    if ((**itr1).boundary.field[i].low > (**itr2).boundary.field[i].low) {
                        (**itr1).boundary.field[i].low = (**itr2).boundary.field[i].low;
                    }
                    if ((**itr1).boundary.field[i].high < (**itr2).boundary.field[i].high) {
                        (**itr1).boundary.field[i].high = (**itr2).boundary.field[i].high;
                    }
                }
                ClearMem(*itr2);
                itr2 = newlist.erase(itr2);
            } else {
                itr2++;
            }
        }
    }
    if (num > curr_node->children.size()) {
        printf("Odd: Of %d children, %d were identical\n",(int)newlist.size(),(int)num);
    }
    return newlist;
}

void regionCompaction(node * curr_node) {
    list<unsigned long long> f0, f1, f2, f3, f4;
    for (list<pc_rule*>::iterator itr = (curr_node->classifier).begin();itr != (curr_node->classifier).end();itr++) {
        if ((**itr).field[0].low < curr_node->boundary.field[0].low) { f0.push_back(curr_node->boundary.field[0].low);}
        else { f0.push_back((**itr).field[0].low);}
        if ((**itr).field[0].high > curr_node->boundary.field[0].high) {f0.push_back(curr_node->boundary.field[0].high);}
        else { f0.push_back((**itr).field[0].high);}

        if ((**itr).field[1].low < curr_node->boundary.field[1].low) { f1.push_back(curr_node->boundary.field[1].low);}
        else { f1.push_back((**itr).field[1].low);}
        if ((**itr).field[1].high > curr_node->boundary.field[1].high) {f1.push_back(curr_node->boundary.field[1].high);}
        else { f1.push_back((**itr).field[1].high);}

        if ((**itr).field[2].low < curr_node->boundary.field[2].low) { f2.push_back(curr_node->boundary.field[2].low);}
        else { f2.push_back((**itr).field[2].low);}
        if ((**itr).field[2].high > curr_node->boundary.field[2].high) {f2.push_back(curr_node->boundary.field[2].high);}
        else { f2.push_back((**itr).field[2].high);}

        if ((**itr).field[3].low < curr_node->boundary.field[3].low) { f3.push_back(curr_node->boundary.field[3].low);}
        else { f3.push_back((**itr).field[3].low);}
        if ((**itr).field[3].high > curr_node->boundary.field[3].high) {f3.push_back(curr_node->boundary.field[3].high);}
        else { f3.push_back((**itr).field[3].high);}

        if ((**itr).field[4].low < curr_node->boundary.field[4].low) { f4.push_back(curr_node->boundary.field[4].low);}
        else { f4.push_back((**itr).field[4].low);}
        if ((**itr).field[4].high > curr_node->boundary.field[4].high) {f4.push_back(curr_node->boundary.field[4].high);}
        else { f4.push_back((**itr).field[4].high);}
    }
    f0.sort();
    f1.sort();
    f2.sort();
    f3.sort();
    f4.sort();
    curr_node->boundary.field[0].low = f0.front();
    curr_node->boundary.field[0].high = f0.back();
    curr_node->boundary.field[1].low = f1.front();
    curr_node->boundary.field[1].high = f1.back();
    curr_node->boundary.field[2].low = f2.front();
    curr_node->boundary.field[2].high = f2.back();
    curr_node->boundary.field[3].low = f3.front();
    curr_node->boundary.field[3].high = f3.back();
    curr_node->boundary.field[4].low = f4.front();
    curr_node->boundary.field[4].high = f4.back();
}


void binRules(list<pc_rule> &ruleList) {
    int min, wild;
    int secondmin, thirdmin;
    double field[5];

    int count = 0;
    for (list<pc_rule>::iterator itr = ruleList.begin(); itr != ruleList.end(); itr++) {
        count++;
        wild = 0;
        field[0] = ((double) ((*itr).field[0].high - (*itr).field[0].low))/0xFFFFFFFF;
        field[1] = ((double) ((*itr).field[1].high - (*itr).field[1].low))/0xFFFFFFFF;
        field[2] = ((double) ((*itr).field[2].high - (*itr).field[2].low))/65535;
        field[3] = ((double) ((*itr).field[3].high - (*itr).field[3].low))/65535;
        if (((*itr).field[4].low == 0) && ((*itr).field[4].high == 0xFF)) {
            field[4] = 1;
            wild++;
        } else {
            field[4] = 0;
        }
        /*for (int i = 0; i < 5; i++) {
            printf("Field %d is %f, "i, field[i],priority);
            }*/
        //printf("\n");
        min = 0;
        if (field[0] >= IPbin) { wild++; }
        if (field[1] >= IPbin) { wild++; }
        if (field[2] >= bin) { wild++; }
        if (field[3] >= bin) { wild++; }
        for (int i = 0; i < 4; i++) {
            if (field[i] < field[min]) {
                min = i;
            }
        }
        if (wild >= 4) {
            if ((field[0] > IPbin) && (field[1] > IPbin) && (field[2] > bin) && (field[3] > bin) && (field[4] != 1))
                bigrules[4].push_back(&(*itr));
            else {
                bigrules[min].push_back(&(*itr));
            }
        } else if (wild == 3) {
            if ((field[0] < IPbin) && (field[1] < IPbin)) {
                kindabigrules[9].push_back(&(*itr));    // wc except 0 and 1
            } else if ((field[0] < IPbin) && (field[2] < bin)){
                kindabigrules[8].push_back(&(*itr));    // wc except 0 and 2
            } else if ((field[0] < IPbin) && (field[3] < bin)) {
                kindabigrules[7].push_back(&(*itr));    // wc except 0 and 3
            } else if ((field[0] < IPbin) && (field[4] < bin)) {
                kindabigrules[6].push_back(&(*itr));    // wc except 0 and 4
            } else if ((field[1] < IPbin) && (field[2] < bin)) {
                kindabigrules[5].push_back(&(*itr));    // wc except 1 and 2
            } else if ((field[1] < IPbin) && (field[3] < bin)) {
                kindabigrules[4].push_back(&(*itr));    // wc except 1 and 3
            } else if ((field[1] < IPbin) && (field[4] < bin)) {
                kindabigrules[3].push_back(&(*itr));    // wc except 1 and 4
            } else if ((field[2] < bin) && (field[3] < bin)) {
                kindabigrules[2].push_back(&(*itr));    // wc except 2 and 3
            } else if ((field[2] < bin) && (field[4] < bin)) {
                kindabigrules[1].push_back(&(*itr));    // wc except 2 and 4
            } else if ((field[3] < bin) && (field[4] < bin)) {
                kindabigrules[0].push_back(&(*itr));    // wc except 3 and 4
            } else {
                printf("ERROR: Rule had 3 wc but did not match any of the bins!\n");
            }
        } else if (wild == 2) {
            if ((field[0] < IPbin) && (field[1] < IPbin) && (field[2] < bin)) {
                mediumrules[9].push_back(&(*itr));  // wc except 0, 1 and 2
            } else if ((field[0] < IPbin) && (field[1] < IPbin) && (field[3] < bin)){
                mediumrules[8].push_back(&(*itr));  // wc except 0, 1 and 3
            } else if ((field[0] < IPbin) && (field[1] < IPbin) && (field[4] < bin)) {
                mediumrules[7].push_back(&(*itr));  // wc except 0, 1 and 4
            } else if ((field[0] < IPbin) && (field[2] < bin) && (field[3] < bin)) {
                mediumrules[6].push_back(&(*itr));  // wc except 0, 2 and 3
            } else if ((field[0] < IPbin) && (field[2] < bin) && (field[4] < bin)) {
                mediumrules[5].push_back(&(*itr));  // wc except 0, 2 and 4
            } else if ((field[0] < IPbin) && (field[3] < bin) && (field[4] < bin)) {
                mediumrules[4].push_back(&(*itr));  // wc except 0, 3 and 4
            } else if ((field[1] < IPbin) && (field[2] < bin) && (field[3] < bin)) {
                mediumrules[3].push_back(&(*itr));  // wc except 1, 2 and 3
            } else if ((field[1] < IPbin) && (field[2] < bin) && (field[4] < bin)) {
                mediumrules[2].push_back(&(*itr));  // wc except 1, 2 and 4
            } else if ((field[1] < IPbin) && (field[3] < bin) && (field[4] < bin)) {
                mediumrules[1].push_back(&(*itr));  // wc except 1, 3 and 4
            } else if ((field[2] < bin) && (field[3] < bin) && (field[4] < bin)) {
                mediumrules[0].push_back(&(*itr));  // wc except 2, 3 and 4
            } else {
                printf("ERROR: Rule had 2 wc but did not match any of the bins!: %lf, %lf, %lf, %lf, %lf\n",field[0],field[1],field[2],field[3],field[4]);
            }
        } else {
            if (thirtyone) {
                if (wild == 1) {
                    if (field[0] >= IPbin) {
                        littlerules[0].push_back(&(*itr));
                        //printf("littlerules[0]\n");
                    } else if (field[1] >= IPbin){
                        //printf("littlerules[1]\n");
                        littlerules[1].push_back(&(*itr));
                    } else if (field[2] >= IPbin) {
                        littlerules[2].push_back(&(*itr));
                        //printf("littlerules[1]\n");
                    } else if (field[3] >= IPbin) {
                        //printf("littlerules[3]\n");
                        littlerules[3].push_back(&(*itr));
                    } else if (field[4] >= IPbin) {
                        //printf("littlerules[4]\n");
                        littlerules[4].push_back(&(*itr));
                    } else {
                        printf("ERROR: Rule had 1 wc but did not match any of the bins!\n");
                    }
                } else {
                    smallrules.push_back(&(*itr));
                }
            } else {
                smallrules.push_back(&(*itr));
            }
        }
    }
    numTrees = 0;

    for (int i = 0; i < 5; i++) {
        if (bigrules[i].size() > 0) {
            numTrees++;
            rulelists[i] = 1;
        } else {
            rulelists[i] = 0;
        }
    }
    for (int j = 0; j < 10; j++) {
        if (kindabigrules[j].size() > 0) {
            numTrees++;
            rulelists[(j+5)] = 1;
        } else {
            rulelists[(j+5)] = 0;
        }
    }
    for (int k = 0; k < 10; k++) {
        if (mediumrules[k].size() > 0) {
            numTrees++;
            rulelists[k+15] = 1;
        } else {
            rulelists[k+15] = 0;
        }
    }
    for (int l = 0; l < 5; l++) {
        if (littlerules[l].size() > 0) {
            numTrees++;
        }
    }
    if (smallrules.size() > 0) {
        numTrees++;
        rulelists[25] = 1;
    } else {
        rulelists[25] = 0;
    }

#ifndef KUN_TEST
    cout << "Number of rules binned: " << count << endl;
#endif
}

/*
 *  Method to merge trees together
 *  Will try to merge trees that have no more than one field that is not overlapping (i.e. where one tree is WC and one tree is not)
 */
void MergeTrees() {
#ifndef KUN_TEST
    printf("Number of trees before merge: %d\n",numTrees);
#endif
    int merged[26]; // array - if the value is 0 than that try is not merged, if it is 1 it has been and is NOT a candidate for merging anymore!
    for (int i = 0; i < 26; i++) { merged[i] = 0; } // make sure array is initialized to 0

    // try to merge any of the 1* into a 2* if it exists
    if (rulelists[0] == 1) {
        if (rulelists[11] == 1 && !(merged[11])) {
            bigrules[0].merge(kindabigrules[6],mycomparison);
            rulelists[11] = 0;
            merged[0] = 1;
            numTrees--;
        } else if (rulelists[12] == 1 && !(merged[12])) {
            bigrules[0].merge(kindabigrules[7],mycomparison);
            rulelists[12] = 0;
            merged[0] = 1;
             numTrees--;
        } else if (rulelists[13] == 1 && !(merged[13])) {
            bigrules[0].merge(kindabigrules[8],mycomparison);
            rulelists[13] = 0;
            merged[0] = 1;
            numTrees--;
        } else if (rulelists[14] == 1 && !(merged[14])) {
            bigrules[0].merge(kindabigrules[9],mycomparison);
            rulelists[14] = 0;
            merged[0] = 1;
            numTrees--;
        }
    }

    if (rulelists[1] == 1) {
         if (rulelists[8] == 1 && !(merged[8])) {
            bigrules[1].merge(kindabigrules[3],mycomparison);
            rulelists[8] = 0;
            merged[1] = 1;
            numTrees--;
        } else if (rulelists[9] == 1 && !(merged[9])) {
            bigrules[1].merge(kindabigrules[4],mycomparison);
            rulelists[9] = 0;
            merged[1] = 1;
            numTrees--;
        } else if (rulelists[10] == 1 && !(merged[10])) {
            bigrules[1].merge(kindabigrules[5],mycomparison);
            rulelists[10] = 0;
            merged[1] = 1;
            numTrees--;
        } else if (rulelists[14] == 1 && !(merged[14])) {
            bigrules[1].merge(kindabigrules[9],mycomparison);
            rulelists[14] = 0;
            merged[1] = 1;
            numTrees--;
        }
    }
     if (rulelists[2] == 1) {
         if (rulelists[6] == 1 && !(merged[6])) {
            bigrules[2].merge(kindabigrules[1],mycomparison);
            rulelists[6] = 0;
            merged[2] = 1;
            numTrees--;
        } else if (rulelists[7] == 1 && !(merged[7])) {
            bigrules[2].merge(kindabigrules[2],mycomparison);
            rulelists[7] = 0;
            merged[2] = 1;
            numTrees--;
        } else if (rulelists[10] == 1 && !(merged[10])) {
            bigrules[2].merge(kindabigrules[5],mycomparison);
            rulelists[10] = 0;
            merged[2] = 1;
            numTrees--;
        } else if (rulelists[13] == 1 && !(merged[13])) {
            bigrules[2].merge(kindabigrules[8],mycomparison);
            rulelists[13] = 0;
            merged[2] = 1;
            numTrees--;
        }
    }
    if (rulelists[3] == 1) {
         if (rulelists[5] == 1 && !(merged[5])) {
            bigrules[3].merge(kindabigrules[0],mycomparison);
            rulelists[5] = 0;
            merged[3] = 1;
            numTrees--;
        } else if (rulelists[7] == 1 && !(merged[7])) {
            bigrules[3].merge(kindabigrules[2],mycomparison);
            rulelists[7] = 0;
            merged[3] = 1;
            numTrees--;
        } else if (rulelists[9] == 1 && !(merged[9])) {
            bigrules[3].merge(kindabigrules[4],mycomparison);
            rulelists[9] = 0;
            merged[3] = 1;
            numTrees--;
        } else if (rulelists[12] == 1 && !(merged[12])) {
            bigrules[3].merge(kindabigrules[7],mycomparison);
            rulelists[12] = 0;
            merged[3] = 1;
            numTrees--;
        }
    }
    if (rulelists[4] == 1) {
        if (rulelists[5] == 1 && !(merged[5])) {
            bigrules[4].merge(kindabigrules[0],mycomparison);
            rulelists[5] = 0;
            merged[4] = 1;
            numTrees--;
        } else if (rulelists[6] == 1 && !(merged[6])) {
            bigrules[4].merge(kindabigrules[1],mycomparison);
            rulelists[6] = 0;
            merged[4] = 1;
            numTrees--;
        } else if (rulelists[8] == 1 && !(merged[8])) {
            bigrules[4].merge(kindabigrules[3],mycomparison);
            rulelists[8] = 0;
            merged[4] = 1;
            numTrees--;
        } else if (rulelists[11] == 1 && !(merged[11])) {
            bigrules[4].merge(kindabigrules[6],mycomparison);
            rulelists[11] = 0;
            merged[4] = 1;
            numTrees--;
        }
    }
    if (rulelists[5] == 1) {
        if (rulelists[15] == 1 && !(merged[15])) {
            kindabigrules[0].merge(mediumrules[0],mycomparison);
            rulelists[15] = 0;
            merged[5] = 1;
            numTrees--;
        } else if (rulelists[16] == 1 && !(merged[16])) {
            kindabigrules[0].merge(mediumrules[1],mycomparison);
            rulelists[16] = 0;
            merged[5] = 1;
            numTrees--;
        } else if (rulelists[19] == 1 && !(merged[19])) {
            kindabigrules[0].merge(mediumrules[4],mycomparison);
            rulelists[19] = 0;
            merged[5] = 1;
            numTrees--;
        }
    }
    if (rulelists[6] == 1) {
        if (rulelists[15] == 1 && !(merged[15])) {
            kindabigrules[1].merge(mediumrules[0],mycomparison);
            rulelists[15] = 0;
            merged[6] = 1;
            numTrees--;
        } else if (rulelists[17] == 1 && !(merged[17])) {
            kindabigrules[1].merge(mediumrules[2],mycomparison);
            rulelists[17] = 0;
            merged[6] = 1;
            numTrees--;
        } else if (rulelists[20] == 1 && !(merged[20])) {
            kindabigrules[1].merge(mediumrules[5],mycomparison);
            rulelists[20] = 0;
            merged[6] = 1;
            numTrees--;
        }
    }
    if (rulelists[7] == 1) {
        if (rulelists[15] == 1 && !(merged[15])) {
            kindabigrules[2].merge(mediumrules[0],mycomparison);
            rulelists[15] = 0;
            merged[7] = 1;
            numTrees--;
        } else if (rulelists[18] == 1 && !(merged[18])) {
            kindabigrules[2].merge(mediumrules[3],mycomparison);
            rulelists[18] = 0;
            merged[7] = 1;
            numTrees--;
        } else if (rulelists[21] == 1 && !(merged[21])) {
            kindabigrules[2].merge(mediumrules[6],mycomparison);
            rulelists[21] = 0;
            merged[7] = 1;
            numTrees--;
        }
    }
    if (rulelists[8] == 1) {
        if (rulelists[16] == 1 && !(merged[16])) {
            kindabigrules[3].merge(mediumrules[1],mycomparison);
            rulelists[16] = 0;
            merged[8] = 1;
            numTrees--;
        } else if (rulelists[17] == 1 && !(merged[17])) {
            kindabigrules[3].merge(mediumrules[2],mycomparison);
            rulelists[17] = 0;
            merged[8] = 1;
            numTrees--;
        } else if (rulelists[22] == 1 && !(merged[22])) {
            kindabigrules[3].merge(mediumrules[7],mycomparison);
            rulelists[22] = 0;
            merged[8] = 1;
            numTrees--;
        }
    }
    if (rulelists[9] == 1) {
        if (rulelists[16] == 1 && !(merged[16])) {
            kindabigrules[4].merge(mediumrules[1],mycomparison);
            rulelists[16] = 0;
            merged[9] = 1;
            numTrees--;
        } else if (rulelists[18] == 1 && !(merged[18])) {
            kindabigrules[4].merge(mediumrules[3],mycomparison);
            rulelists[18] = 0;
            merged[9] = 1;
            numTrees--;
        } else if (rulelists[23] == 1 && !(merged[23])) {
            kindabigrules[4].merge(mediumrules[8],mycomparison);
            rulelists[23] = 0;
            merged[9] = 1;
            numTrees--;
        }
    }
    if (rulelists[10] == 1) {
        if (rulelists[17] == 1 && !(merged[17])) {
            kindabigrules[5].merge(mediumrules[2],mycomparison);
            rulelists[17] = 0;
            merged[10] = 1;
            numTrees--;
        } else if (rulelists[18] == 1 && !(merged[18])) {
            kindabigrules[5].merge(mediumrules[3],mycomparison);
            rulelists[18] = 0;
            merged[10] = 1;
            numTrees--;
        } else if (rulelists[24] == 1 && !(merged[24])) {
            kindabigrules[5].merge(mediumrules[9],mycomparison);
            rulelists[24] = 0;
            merged[10] = 1;
            numTrees--;
        }
    }
    if (rulelists[11] == 1) {
        if (rulelists[19] == 1 && !(merged[19])) {
            kindabigrules[6].merge(mediumrules[4],mycomparison);
            rulelists[19] = 0;
            merged[11] = 1;
            numTrees--;
        } else if (rulelists[20] == 1 && !(merged[20])) {
            kindabigrules[6].merge(mediumrules[5],mycomparison);
            rulelists[20] = 0;
            merged[11] = 1;
            numTrees--;
        } else if (rulelists[22] == 1 && !(merged[22])) {
            kindabigrules[6].merge(mediumrules[7],mycomparison);
            rulelists[22] = 0;
            merged[11] = 1;
            numTrees--;
        }
    }
    if (rulelists[12] == 1) {
        if (rulelists[19] == 1 && !(merged[19])) {
            kindabigrules[7].merge(mediumrules[4],mycomparison);
            rulelists[19] = 0;
            merged[12] = 1;
            numTrees--;
        } else if (rulelists[21] == 1 && !(merged[21])) {
            kindabigrules[7].merge(mediumrules[6],mycomparison);
            rulelists[21] = 0;
            merged[12] = 1;
            numTrees--;
        } else if (rulelists[23] == 1 && !(merged[23])) {
            kindabigrules[7].merge(mediumrules[8],mycomparison);
            rulelists[23] = 0;
            merged[12] = 1;
            numTrees--;
        }
    }
    if (rulelists[13] == 1) {
        if (rulelists[20] == 1 && !(merged[20])) {
            kindabigrules[8].merge(mediumrules[5],mycomparison);
            rulelists[20] = 0;
            merged[13] = 1;
            numTrees--;
        } else if (rulelists[21] == 1 && !(merged[21])) {
            kindabigrules[8].merge(mediumrules[6],mycomparison);
            rulelists[21] = 0;
            merged[13] = 1;
            numTrees--;
        } else if (rulelists[24] == 1 && !(merged[24])) {
            kindabigrules[8].merge(mediumrules[9],mycomparison);
            rulelists[24] = 0;
            merged[13] = 1;
            numTrees--;
        }
    }
    if (rulelists[14] == 1) {
        if (rulelists[22] == 1 && !(merged[22])) {
            kindabigrules[9].merge(mediumrules[7],mycomparison);
            rulelists[22] = 0;
            merged[14] = 1;
            numTrees--;
        } else if (rulelists[23] == 1 && !(merged[23])) {
            kindabigrules[9].merge(mediumrules[8],mycomparison);
            rulelists[23] = 0;
            merged[14] = 1;
            numTrees--;
        } else if (rulelists[24] == 1 && !(merged[24])) {
            kindabigrules[9].merge(mediumrules[9],mycomparison);
            rulelists[24] = 0;
            merged[14] = 1;
            numTrees--;
        }
    }
#if 0
    for (int i = 0; i < 9; i++) {
        if (rulelists[i+15] == 1 && rulelists[25] == 1) {
            mediumrules[i].merge(smallrules,mycomparison);
            merged[i+15] = 1;
            rulelists[25] = 0;
            numTrees--;
            break;
        }
    }
#endif
#ifndef KUN_TEST
    printf("Number of trees after merge: %d\n",numTrees);
#endif
}


void BinPack(int bins,list <TreeStat*> Statistics)
{
    list <MemBin*> Memory;
    for (int i = 0;i < bins;i++)
    {
        MemBin* newbin = new MemBin;
        newbin->Max_Depth = 0;
        newbin->Max_Levels = 0;
        newbin->Max_Access64Bit = 0;
        newbin->Max_Access128Bit = 0;
        newbin->total_memory = 0;
        newbin->Trees.clear();
        Memory.push_back(newbin);
    }
    while (!Statistics.empty())
    {
        Statistics.sort(mystatsort);
        TreeStat* selected_tree = Statistics.front();
        // printf("tree %d allocated!\n",selected_tree->Id);

        Memory.sort(mymemsort);
        (Memory.front())->Trees.push_back(selected_tree);
        (Memory.front())->Max_Depth      += selected_tree->Max_Depth;
        (Memory.front())->Max_Levels        += selected_tree->Max_Levels;
        (Memory.front())->Max_Access64Bit+= selected_tree->Max_Access64Bit;
        (Memory.front())->Max_Access128Bit+= selected_tree->Max_Access128Bit;
        (Memory.front())->total_memory  += selected_tree->total_memory;
        Statistics.pop_front();
    }


    printf("******************************************\n");
    printf("Memory Channels = %d\n",bins);
    printf("******************************************\n");
    int count = 0;
    int ADJUSTED_OVERALL_DEPTH = 0;
    int ADJUSTED_OVERALL_LEVELS = 0;
    int accessCosts64 = 0;
    int accessCosts128 = 0;
    for (auto iter = Memory.begin(); iter != Memory.end();++iter)
    {
        if ((*iter)->Max_Depth > ADJUSTED_OVERALL_DEPTH)
            ADJUSTED_OVERALL_DEPTH = (*iter)->Max_Depth;

        if ((*iter)->Max_Levels > ADJUSTED_OVERALL_LEVELS)
            ADJUSTED_OVERALL_LEVELS = (*iter)->Max_Levels;

        if ((*iter)->Max_Access64Bit > accessCosts64)
            accessCosts64 = (*iter)->Max_Access64Bit;
        if ((*iter)->Max_Access128Bit > accessCosts128)
            accessCosts128 = (*iter)->Max_Access128Bit;

        printf("Channel %d: Depth = %d Levels = %d Memory = %llu;Trees - ",count++,
                            (*iter)->Max_Depth,(*iter)->Max_Levels,(*iter)->total_memory);
        for (auto sub_iter = (*iter)->Trees.begin(); sub_iter != (*iter)->Trees.end();sub_iter++)
        {
            printf("{%d} ",(*sub_iter)->Id);
        }
        printf("\n");
    }
    printf("ADJUSTED_OVERALL_DEPTH: %d\n",ADJUSTED_OVERALL_DEPTH);
    printf("ADJUSTED_OVERALL_LEVELS: %d\n",ADJUSTED_OVERALL_LEVELS);
    printf("ADJUSTED_Access_Cost_64Bit: %d\n", accessCosts64);
    printf("ADJUSTED_Access_Cost_128Bit: %d\n", accessCosts128);


}
