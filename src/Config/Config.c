// configuration parameters

#include <iostream>

#include "limits.h"
#include "typedefs.h"
#include "Config.h"

using namespace std;

// shar computation with unification?
int isUnifShar = 0;

// seek time
CostVal_t SeekTime = 8;

// read without seek
CostVal_t ReadTime = 2;

// write without seek
CostVal_t WriteTime = 4;

// number of buffers
Card_t NumBuffers = 8000;

int  Config_t::DebugLevel;
bool Config_t::DisableApply;
bool Config_t::ExhaustiveEIO;
bool Config_t::PostgresHeuristicEIO;
bool Config_t::PlainEIO;
bool Config_t::DisablePartialSort;
bool Config_t::DisableNLJ;
bool Config_t::DisableIndexNLJ;
bool Config_t::DisableMergeJoin;


// cost as a function of number of ios and sum of blocks input and out
CostVal_t Config_t::Cost(Card_t io_Cost, Card_t cpu_Blocks)
{
    assert(io_Cost >= 0);
    assert(cpu_Blocks >= 0);

    // Ravi: Reducing the CPU cost fudge factor from 0.2 to 0.05
    //CostVal_t costVal = CostVal_t(io_Cost + 0.05 * cpu_Blocks);
    CostVal_t costVal = CostVal_t(io_Cost + 0.2 * cpu_Blocks);
    assert(costVal >= 0);

    return costVal/1000;
}

// block size in number of bytes
DataSize_t Config_t::BlockSize_Bytes(void)
{ return 4096; }

// max rounding error for comparing costs
CostVal_t Config_t::MaxErr(void)
{ return CostVal_t(0.5); }

// upper bound on plan costs
CostVal_t Config_t::CostLimit(void)
{ return CostVal_t(1000000000); }

// upper bound on size (to check overflows)
Card_t Config_t::CardLimit(void)
{ return Card_t(ULONG_MAX); }

// subsumption enabled?
int Config_t::IsSubsumption(void)
{ return 1; }

// pruning enabled?
int Config_t::IsPruning(void)
{ return 1; }

// indexing enabled?
int Config_t::IsIndexing(void)
{ return 1; }

// sharable node computation by unification?
int Config_t::IsUnifShar(void)
{ return isUnifShar; }

// index fanout
int Config_t::IndexFanout(void)
{ return 20; }

// print DAG expansion time
int Config_t::PrintDAGExpansionTime(void)
{ return 0; }

// debugging mode --- 0: off; 1: on
int Config_t::IsDebug(int level)
{ return level >= DebugLevel; }

//Ravi:Added method
void Config_t::DebugPrintln(char *message, int level)
{
    if (IsDebug(level)) 
    { 
        cout << message << endl;
    }
}

//Ravi:Added method
void Config_t::DebugPrint(char *message, int level)
{
    if (IsDebug(level)) 
    { 
        cout << message;
    }
}

