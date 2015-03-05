#include <iostream>
#include <stdio.h>
#include "shared.h"
#include "compressedcuts.h"
using namespace std;

struct flow* read_trace_file(FILE* traceFile) {
    struct flow *flows = new struct flow[MAX_TRACES];
    trace_rule_num = 0;
    int ret = 0;
    while (true) {
        struct flow f;
        int ret = fscanf(traceFile, "%u %u %u %u %u %u", &f.src_ip, &f.dst_ip, &f.src_port, &f.dst_port, &f.proto, &f.trueRID);
        if (ret != 6)
            break;
        flows[trace_rule_num++] = f;
    }
    fclose(traceFile);
    return flows;
}
