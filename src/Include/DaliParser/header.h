#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <iostream>
#include <malloc.h>
#include <stdio.h>
#include <assert.h>

typedef unsigned long BV_t;             // bitvector type
#include "define.h"
#include "operator.h"
#include "plan.h"
#include "algo.h"
#include "enforcer.h"
#include "rule.h"
#include "equivalence.h"
#include "cost.h"
#include "schema.h"
#include "newexpr.h"
#include "logicalprop.h"
#include "searchclass.h"
#include "multiset.h"
#include "stack.h"
#include "cache.h"
#include "functions.h"
#include "tables.h"

extern int trace_print;
