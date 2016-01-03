// system configuration

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "typedefs.h"
#include "Cost.h"

#define DEBUG_ERROR         4
#define DEBUG_WARN          3
#define DEBUG_INFO          2
#define DEBUG_VERBOSE_INFO  1

extern CostVal_t SeekTime, ReadTime, WriteTime;
extern Card_t NumBuffers;

class Config_t {
public:
    static int  DebugLevel;
            
    // if true, disables NQO
    static bool DisableApply;

    // if true, enumerates interesting orders exhaustively
    static bool ExhaustiveEIO;

    // if true, enumerates interesting orders using postgres heuristic 
    static bool PostgresHeuristicEIO;
    
    // if true, generates a single arbitrary interesting order
    static bool PlainEIO;

    // if true, turns off benefits for partial sort
    static bool DisablePartialSort;

    // if true, disables NLJ
    static bool DisableNLJ;

    // if true, disables hash-join 
    static bool DisableIndexNLJ;

    // if true, disables hash-join 
    static bool DisableMergeJoin;

    // block size in number of bytes
    static DataSize_t BlockSize_Bytes(void);

    // max rounding error for comparing costs
    static CostVal_t MaxErr(void);

    // upper bound on plan costs
    static CostVal_t CostLimit(void);

    // cardinality limit
    static Card_t CardLimit(void);

    // cost as a function of number of ios and sum of blocks input and out
    static CostVal_t Cost(Card_t io_Blocks, Card_t out_Blocks);

    // subsumption enabled?
    static int IsSubsumption(void);

    // pruning enabled?
    static int IsPruning(void);

    // indexing enabled?
    static int IsIndexing(void);

    // sharable node computation by unification?
    static int IsUnifShar(void);

    // index fanout
    static int IndexFanout(void);

    // print DAG expansion time
    static int PrintDAGExpansionTime(void);

    // debugging mode --- 0: off; 1: on
    static int IsDebug(int level = 10);

    // Ravi:Added new method - ideally should not belong to this class
    static void DebugPrintln(char *message, int level=100);
    
    // Ravi:Added new method - ideally should not belong to this class
    static void DebugPrint(char *message, int level=100);
};

#endif // __CONFIG_H__
