#ifndef __EVAL_H__
#define __EVAL_H__

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



void eval(Plan_t* plan);
void PrintOps(PhysicalOp_t *root, int ind);



#endif //__EVAL_H__
