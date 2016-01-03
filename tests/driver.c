#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>

#include <iostream>

#include "Catalog.h"
#include "Equivalence.h"
#include "PhysicalOp.h"
#include "PhyPropGroup.h"
#include "Interface.h"
#include "Volcano.h"
#include "ExhSearch.h"
#include "Greedy.h"

//added by sachin
#include "Eval.h"
//End - added ny sachin

extern Catalog_t *Catalog;

extern CostVal_t RelThreshold;
extern CostVal_t AbsThreshold;
extern int isUnifShar;

using namespace std;

int main(int argc, char **argv)
{
    if(argc < 2) {
        cout << "Usage: " << argv[0] << " algo [costlimit] [rel-threshold] [config-args]" << endl;
        cout << endl;
        cout << "algo: " << endl;
        cout << "\tgreedy :\t\tGreedy" << endl;
        cout << "\tgreedy-unifshar :\tGreedy-UnifShar" << endl;
        cout << "\texhaustive :\t\tExhaustive" << endl;
        cout << "\tcached-exhaustive :\tCached-EXH" << endl;
        cout << "\tvolcano :\t\tVolcano" << endl;
        cout << "\tvolcano-sh :\t\tVolcano-SH" << endl;
        cout << "\tvolcano-ru :\t\tVolcano-RU" << endl;
        cout << "\tvolcano-revru :\t\tVolcano-RevRU" << endl;
        cout << "\tvolcano-nounif :\tVolcano-NOUNIF" << endl;
        cout << "\tvolcano-nounif-sh :\tVolcano-NOUNIF-SH" << endl;
        cout << "\tvolcano-nounif-ru :\tVolcano-NOUNIF-RU" << endl;
        cout << "config-args: " << endl;
        cout << "\t--catalog" << endl;
        cout << "\t--disable-apply" << endl;
        cout << "\t\tDisables NQO" << endl;
        cout << "\t--exhaustive-eio" << endl;
        cout << "\t--pgheuristic-eio" << endl;
        cout << "\t--disable-nlj" << endl;
        cout << "\t--disable-index-nlj" << endl;
        cout << "\t--disable-merge-join" << endl;
        cout << "\t--debug-level <level>" << endl;
        cout << "\t\t[1-4] 1:Very verbose, 4:Least messages" << endl;
        exit(0);
    }

    Optimizer_t *opt = NULL;

    if( !strcmp(argv[1], "greedy") ) {
        if( argv[3] ) {
            RelThreshold = atof(argv[2]);
        }
        cout << "ALGO Greedy" << endl;
        cout << "RelThreshold = " << RelThreshold << endl;
        cout << "AbsThreshold = " << AbsThreshold << endl;
        isUnifShar = 0;
        opt = new Greedy_t(1, 1);
    } else if( !strcmp(argv[1], "greedy-unifshar") ) {
        if( argv[3] ) {
            RelThreshold = atof(argv[2]);
        }
        cout << "ALGO Greedy-UnifShar" << endl;
        cout << "RelThreshold = " << RelThreshold << endl;
        cout << "AbsThreshold = " << AbsThreshold << endl;
        isUnifShar = 1;
        opt = new Greedy_t(1, 1);
    } else if( !strcmp(argv[1], "exhaustive") ) {
        cout << "ALGO Exhaustive" << endl;
        opt = new ExhSearch_t;
    } else if( !strcmp(argv[1], "cached-exhaustive") ) {
        cout << "ALGO Cached-EXH" << endl;
        opt = new CachedExhSearch_t;
    } else if( !strcmp(argv[1], "volcano") ) {
        cout << "ALGO Volcano" << endl;
        opt = new VolcanoOrd_t(1,1); 
    } else if( !strcmp(argv[1], "volcano-sh") ) {
        cout << "ALGO Volcano-SH" << endl;
        opt = new VolcanoShar_t(1,1); 
    } else if( !strcmp(argv[1], "volcano-ru") ) {
        cout << "ALGO Volcano-RU" << endl;
        opt = new VolcanoRU_t(1,1); 
    } else if( !strcmp(argv[1], "volcano-revru") ) {
        cout << "ALGO Volcano-RevRU" << endl;
        opt = new VolcanoRevRU_t(1,1); 
    } else if( !strcmp(argv[1], "greedy-nounif") ) {
        cout << "ALGO Greedy-NOUNIF" << endl;
        opt = new Greedy_t(0);
    } else if( !strcmp(argv[1], "volcano-nounif") ) {
        cout << "ALGO Volcano-NOUNIF" << endl;
        opt = new VolcanoOrd_t(0,1); 
    } else if( !strcmp(argv[1], "volcano-nounif-sh") ) {
        cout << "ALGO Volcano-NOUNIF-SH" << endl;
        opt = new VolcanoShar_t(0,1); 
    } else if( !strcmp(argv[1], "volcano-nounif-ru") ) {
        cout << "ALGO Volcano-NOUNIF-RU" << endl;
        opt = new VolcanoRU_t(0,1); 
    } else {
        cout << "Unknown algorithm: " << argv[1] << endl;
        exit(0);
    }

    // Set the default values for config params
    Config_t::DisableApply     = false;
    Config_t::ExhaustiveEIO    = false;
    Config_t::PostgresHeuristicEIO = false;
    Config_t::PlainEIO         = false;
    Config_t::DebugLevel       = 4;
    Config_t::DisableNLJ       = false;
    Config_t::DisableIndexNLJ  = false;
    Config_t::DisableMergeJoin = false;

    char *catalogFile = NULL;
    if(argc > 2) {
        for (int i = 2; i < argc; i++) {
            if( !strcmp(argv[i], "--catalog") ) {
                catalogFile = argv[i+1];
                cout << "Catalog file:"  << catalogFile << endl;
            }
            else if( !strcmp(argv[i], "--disable-apply") ) {
                cout << "Disabling Apply (NQO)" << endl;
                Config_t::DisableApply = true;
            }
            else if( !strcmp(argv[i], "--exhaustive-eio") ) {
                cout << "Interesting orders to be generated exhaustively" << endl;
                Config_t::ExhaustiveEIO = true;
            }
            else if( !strcmp(argv[i], "--pgheuristic-eio") ) {
                cout << "Interesting orders to be generated using Postgres heuristic" << endl;
                Config_t::PostgresHeuristicEIO = true;
            }
            else if( !strcmp(argv[i], "--plain-eio") ) {
                cout << "Interesting orders to be generated using PlainEIO" << endl;
                Config_t::PlainEIO = true;
            }
            else if( !strcmp(argv[i], "--disable-partial-sort") ) {
                cout << "Partial Sort to have the same cost as Full Sort" << endl;
                Config_t::DisablePartialSort = true;
            }
            else if( !strcmp(argv[i], "--debug-level") ) {
                Config_t::DebugLevel = atoi(argv[i+1]);
                if (Config_t::DebugLevel > 4) { // 4 is the max
                    Config_t::DebugLevel = 4;
                }
                cout << "DebugLevel set to " << Config_t::DebugLevel << endl;
            }
            else if( !strcmp(argv[i], "--disable-nlj") ) {
                cout << "Disabling NLJ" << endl;
                Config_t::DisableNLJ = true;
            }
            else if( !strcmp(argv[i], "--disable-index-nlj") ) {
                cout << "Disabling Index NLJ" << endl;
                Config_t::DisableIndexNLJ = true;
            }
            else if( !strcmp(argv[i], "--disable-merge-join") ) {
                cout << "Disabling Merge Join" << endl;
                Config_t::DisableMergeJoin = true;
            }
        }
    }

    Catalog = new Catalog_t(catalogFile);

    // Get timing
    struct rusage rusageBeg, rusageEnd;
    getrusage(RUSAGE_SELF, &rusageBeg);

    Equivalence_t *eq = Interface_t::ExpressionDAG(*opt);
    assert(eq);

    // cost limit
    PlanGroup_t *planGroup = NULL;
    CostVal_t costLimit = -1;
    if( argc <= 2 ) {
        cout << "Cost Limit = NULL" << endl;

        planGroup = opt->FindBestPlan(eq, NULL, Interface_t::reqOrder);
    } else {
        costLimit = atol(argv[2]);
        cout << "Cost Limit = " << costLimit << endl;

        planGroup = opt->FindBestPlan(eq, &costLimit, Interface_t::reqOrder);
    }

    // For timing
    getrusage(RUSAGE_SELF, &rusageEnd);
    int utimetaken =
            (rusageEnd.ru_utime.tv_sec-rusageBeg.ru_utime.tv_sec)*1000 +
            (rusageEnd.ru_utime.tv_usec-rusageBeg.ru_utime.tv_usec)/1000;
    // if (utimetaken < 10) utimetaken = 10; // < 10 msec must be noise!
    int stimetaken =
            (rusageEnd.ru_stime.tv_sec-rusageBeg.ru_stime.tv_sec)*1000 +
            (rusageEnd.ru_stime.tv_usec-rusageBeg.ru_stime.tv_usec)/1000;
    cout << "USRT " << utimetaken << endl;
    cout << "SYST " << stimetaken << endl;
    // End timing

    Plan_t *plan = NULL;
    if( planGroup )
        plan = planGroup->BestPlan();

    if( plan ) {
    cout << endl;
        cout << "Optimization Successful: Plan Cost = " << plan->CostVal() <<
                                        " Cost Limit = " << costLimit << endl;
        cout << endl;
        plan->PrintPlan();
        cout << endl;
//added by sachin
eval(plan);
//end- added ny sachin

    } else {
        cout << "Optimization Failed for Limit " << costLimit << endl;
    }

    cout << endl;

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Expression DAG:" << endl;
        eq->PrintExpressionDAG(0);
        cout << endl;
    }

    delete eq;
    delete opt;
    delete Catalog;
    exit(0);
}
