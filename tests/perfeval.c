// performance evaluation


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

extern Catalog_t *Catalog;

extern CostVal_t RelThreshold;
extern CostVal_t AbsThreshold;
extern int isUnifShar;

using namespace std;

int main(int argc, char **argv)
{
    if(argc < 2) {
	cout << "Usage: " << argv[0] << " algo" << endl;
	cout << endl;
	cout << "Algo: " << endl;
	cout << "greedy-unifshar :\t\tGreedy-UnifShar" << endl;
	cout << "greedy :\t\tGreedy" << endl;
	cout << "exhaustive :\t\tExhaustive" << endl;
	cout << "cached-exhaustive :\tCached-EXH" << endl;
	cout << "volcano :\t\tVolcano" << endl;
	cout << "volcano-sh :\t\tVolcano-SH" << endl;
	cout << "volcano-ru :\t\tVolcano-RU" << endl;
	cout << "volcano-revru :\t\tVolcano-RevRU" << endl;
	cout << "volcano-nounif :\tVolcano-NOUNIF" << endl;
	cout << "volcano-nounif-sh :\tVolcano-NOUNIF-SH" << endl;
	cout << "volcano-nounif-ru :\tVolcano-NOUNIF-RU" << endl;
	exit(0);
    }

    Catalog = new Catalog_t;
    Optimizer_t *opt = NULL;

    if( !strcmp(argv[1], "greedy") ) {
	if( argv[2] ) {
	    RelThreshold = atof(argv[2]);
	}
	cout << "ALGO Greedy" << endl;
	cout << "RelThreshold = " << RelThreshold << endl;
	cout << "AbsThreshold = " << AbsThreshold << endl;
	isUnifShar = 0;
	opt = new Greedy_t(1, 1);
    } else if( !strcmp(argv[1], "greedy-unifshar") ) {
	if( argv[2] ) {
	    RelThreshold = atof(argv[2]);
	}
	cout << "ALGO Greedy-UnifShar" << endl;
	cout << "RelThreshold = " << RelThreshold << endl;
	cout << "AbsThreshold = " << AbsThreshold << endl;
	isUnifShar = 1;
	opt = new Greedy_t(1, 1);
    } else if( !strcmp(argv[1], "greedy-noprune") ) {
	cout << "ALGO Greedy-NOPRUNE" << endl;
	opt = new Greedy_t(1, 0);
    } else if( !strcmp(argv[1], "exhaustive") ) {
	cout << "ALGO Exhaustive" << endl;
	opt = new ExhSearch_t;
    } else if( !strcmp(argv[1], "cached-exhaustive") ) {
	cout << "ALGO Cached-EXH" << endl;
	opt = new CachedExhSearch_t;
    } else if( !strcmp(argv[1], "volcano-app") ) {
	cout << "ALGO Volcano-APP" << endl;
	opt = new VolcanoOrd_t(1,0); 
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

    struct rusage rusageBeg, rusageEnd;

    getrusage(RUSAGE_SELF, &rusageBeg);
    Equivalence_t *eq = Interface_t::ExpressionDAG(*opt);
    assert(eq);

    CostVal_t cost = opt->BestPlanCost(eq);

    if( Config_t::IsDebug() ) {
	cout << "Expression DAG:" << endl;
	eq->PrintExpressionDAG(0);
	cout << endl;
    }
    
    getrusage(RUSAGE_SELF, &rusageEnd);

    int utimetaken =
	    (rusageEnd.ru_utime.tv_sec-rusageBeg.ru_utime.tv_sec)*1000 +
	    (rusageEnd.ru_utime.tv_usec-rusageBeg.ru_utime.tv_usec)/1000;
    if (utimetaken < 10) utimetaken = 10; // < 10 msec must be noise!
    int stimetaken =
	    (rusageEnd.ru_stime.tv_sec-rusageBeg.ru_stime.tv_sec)*1000 +
	    (rusageEnd.ru_stime.tv_usec-rusageBeg.ru_stime.tv_usec)/1000;

    cout << "COST " << cost << endl;
    cout << "USRT " << utimetaken << endl;
    cout << "SYST " << stimetaken << endl;

    cout << "PHYOPNODES " << PhysicalOp_t::NodeCount() << endl;
    cout << "PHYEQNODES " << PlanGroup_t::NodeCount() << endl;

    cout << "LOGOPNODES " << LogicalOp_t::NodeCount() << endl;
    cout << "EQUIVNODES " << Equivalence_t::NodeCount() << endl;

    delete eq;
    delete opt;
    delete Catalog;
    exit(0);
}
