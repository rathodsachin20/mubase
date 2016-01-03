// Equivalence node class implementation

#include "Config.h"
#include "Memo.h"
#include "Node.h"
#include "Plan.h"
#include "PhysicalOp.h"
#include "Volcano.h"
#include "Equivalence.h"

int Equivalence_t::CurID = 0;           // id generator
int Equivalence_t::Count = 0;           // instance count
int Equivalence_t::TopCount = 0;        // topological numbering count

// constructor
Equivalence_t::Equivalence_t(LogProp_t *logProp_a, int isBaseRel_a)
        : id(CurID++), topNo(-1), logProp(logProp_a), planGroupBase(NULL),
            numPlanGroups(0), degree(0), progressCount(0), isExpanded(0),
            isPruned(0), isSubsExpanded(0), isAffected(0),
            isBaseRel(isBaseRel_a), isCorrelated(0),
            costLB(-1), useCost(NULL), isSharable(0), isRoot(0), 
            isOuterVarLogPropSet(0)
{
    Refer(logProp);
    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Created eq " << ID() << endl;
        cout << "Schema: " ;
        logProp_a->Schema()->PrintSQL(logProp_a->SchemaProp());
    }

    Count++;
}

// add a parent
void Equivalence_t::AddParent(LogicalOp_t *lop)
{
    assert(lop);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Adding parent logExpr " << lop->ID() << " to " << ID() << endl;

    parents.Insert(lop);
}

// add a parent
void Equivalence_t::DeleteParent(LogicalOp_t *lop)
{
    assert(lop);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Deleting parent logExpr " << lop->ID() << " from " <<
                                                                ID() << endl;
    parents.Delete(lop);
}

// add a logexp
void Equivalence_t::AddLogExpr(LogicalOp_t *lexp)
{
    assert(lexp);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Adding child logExpr " << lexp->ID() << " to " << ID() << endl;

    logExprs.Insert(lexp);

    CostVal_t lexpCostLB = lexp->CostLB();
    if( costLB == -1 || lexpCostLB < costLB )
        costLB = lexpCostLB;
}

// delete a logexp
void Equivalence_t::DeleteLogExpr(LogicalOp_t *lexp)
{
    assert(lexp);

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Deleting child logExpr " << lexp->ID() << " from " <<
                                                                ID() << endl;
    logExprs.Delete(lexp);
}

// get the plan group for a physical property --- add if not present
PlanGroup_t *Equivalence_t::PlanGroup(PhyPropGroup_t *propGroup, 
                                      Order_t *gpso, Card_t callCount)
{
    assert(propGroup);

    // schema prop
    assert(logProp);
    SchemaProp_t *schemaProp = logProp->SchemaProp();

    // Ravi: BEGIN
    PlanGroup_t* pgBase = planGroupBase;
    PlanGroupTableKey_t *key;
    if (gpso) {
        // TODO: pgBase = GetPlanGroupBase(gpso);
        // cout << "Setting planGroupBase to NULL for testing";
        // planGroupBase = NULL;
        key = new PlanGroupTableKey_t(gpso, callCount);
        pgBase = pgTable.Get(key);
    }
    // Ravi: END

    // create the plan group base if not present
    if( !pgBase) {
        PhyPropGroup_t *emptyProp = new PhyPropGroup_t;
        Refer(emptyProp);
        pgBase = new PlanGroup_t(this, emptyProp);
        pgBase->SetParameterSortOrder(gpso);
        pgBase->SetCallCount(callCount);
        DeRefer(emptyProp);
        Refer(pgBase);
        numPlanGroups++;
        // Ravi: Added. Above changed planGroupBase to pgBase
        if (gpso) {
            pgTable.Put(key, pgBase);
        }
        else {
            planGroupBase = pgBase;
        }
    }

    // update the visit marker value
    int travID = PlanGroup_t::StartScan();

    PlanGroup_t *pg = NULL;
    // Ravi: changed planGroupBase to pgBase in the line below.
    Traverse(travID, schemaProp, pgBase, propGroup, &pg, gpso, callCount);

    assert(pg);
    return pg;
}

// traverses the partial order below base, visiting the plan groups
// whose physical properties are covered by propGroup.
// If a plan group for propGroup is found, it is returned as *pg
// else a new plan group for propGroup is created, inserted at the
// appropriate place in the partial order and returned as *pg
// returns 1 if the plan group for prop was already present; 0 otherwise
// ASSUMPTION: base->prop is covered by prop
int Equivalence_t::Traverse(int travID, SchemaProp_t *schemaProp,
                PlanGroup_t *base, PhyPropGroup_t *prop, PlanGroup_t **pg,
                Order_t *gpso, Card_t callCount)
{
    assert(schemaProp);
    assert(base);
    assert(prop);
    assert(pg);

    if( base->IsVisited(travID) )
        return 0;

    base->Visit(travID);

    PhyPropGroup_t *baseProp = base->PropGroup();
    assert(baseProp);

    // guaranteed that baseProp is covered by prop
    if( prop->IsCovered(schemaProp, baseProp) ) {
        // prop and baseProp identical
        *pg = base;
        return 1;
    }

    // iterate over the least plan groups with properties covered by baseProp
    POEdgeList_t& super = base->SuperList();

    POEdgeIter_t iter;
    iter.Attach(&super);

    int inserted = 0;
    PlanGroup_t *nextPG = NULL;
    while( (nextPG = iter.Next()) ) {
        PhyPropGroup_t *nextProp = nextPG->PropGroup();
        assert(nextProp);

        if( nextProp->IsCovered(schemaProp, prop) ) {
            // nextProp covered by prop -- descend further
            int isPresent = Traverse(travID, schemaProp, nextPG, prop, pg,
                                     gpso, callCount);
            if( isPresent ) {
                // plan group already present
                return 1;
            }
            inserted = 1;
        } else if( prop->IsCovered(schemaProp, nextProp) ) {
            // nextProp not covered by prop
            // but prop is covered by nextProp
            // therefore, insert plan group for prop between baseProp
            // and nextProp
            // delete nextPG from base->super
            iter.Delete();

            // delete base from nextPG->sub
            POEdgeList_t& nextSub = nextPG->SubList();

            POEdgeIter_t nextIter;
            nextIter.Attach(&nextSub);

            for( ; ; ) {
                PlanGroup_t *p = nextIter.Next();
                assert(p);
                if( p->ID() == base->ID() ) {
                    // found the entry for base in nextPG->sub
                    // delete the same
                    nextIter.Delete();
                    break;
                }
            }

            nextIter.Detach();

            // insert plan group for prop between base and newPG
            InsertPlanGroup(prop, base, nextPG, pg, gpso, callCount);

            DeRefer(nextPG);    // was deleted from base->super
            (*pg)->Visit(travID);

            inserted = 1;
        }
    }

    if( !inserted ) {
        // prop not covered by any property associated with any plan
        // group in super -- branch off at base
        InsertPlanGroup(prop, base, NULL, pg, gpso, callCount);
        (*pg)->Visit(travID);
    }

    iter.Detach();

    assert(*pg);
    return 0;
}

// insert plan group corresponding to prop between base and next
void Equivalence_t::InsertPlanGroup(PhyPropGroup_t *prop,
        PlanGroup_t *base, PlanGroup_t *nextPG, PlanGroup_t **pg,
        Order_t *gpso, Card_t callCount)
{
    assert(prop);
    assert(base);
    assert(pg);

    if( !*pg ) {
        // plan group for prop not instantiated yet
        *pg = new PlanGroup_t(this, prop);
        (*pg)->SetParameterSortOrder(gpso);
        (*pg)->SetCallCount(callCount);
        numPlanGroups++;

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
            cout << "Added new plan group id = " << (*pg)->ID() <<
                                            " for eq id = " << ID() << endl;
            cout << "Number of properties = " << prop->NumProps() << endl;
            prop->Print();
            cout << endl;
        }
    }

    // insert *pg in base->super
    POEdgeList_t& super = base->SuperList();
    super.Insert(*pg);
    Refer(*pg);

    if( nextPG ) {
        // insert *pg in nextPG->sub
        POEdgeList_t& nextSub = nextPG->SubList();
        nextSub.Insert(*pg);
    }

    // insert base in (*pg)->sub
    POEdgeList_t& sub = (*pg)->SubList();
    sub.Insert(base);

    if( nextPG ) {
        // insert nextPG in (*pg)->super
        POEdgeList_t& super = (*pg)->SuperList();

        super.Insert(nextPG);
        Refer(nextPG);
    }
}

// unify two semantically identical equivalent nodes
void Equivalence_t::Unify(Equivalence_t *eq, Memo_t& memo)
{
    assert(eq);
    assert(ID() != eq->ID());
    assert(memo.IsUnify());

    assert(!eq->IsInProgress());

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Unifying [EQCLASS" << eq->ID() << "] into [EQCLASS" <<
                                                        ID() << "]" << endl;
        PrintExpressionDAG(0);
        cout << endl;
        eq->PrintExpressionDAG(0);
        cout << endl;
    }

    // put the parent logExprs to eq into a list to prevent consistency
    // problems while changing pointers
    List_t<LogicalOp_t *>& eqParents = eq->ParList();
    ListIter_t<LogicalOp_t *> iter;

    int numEqParents = eqParents.Size();
    // plus one to take care of zero size
    LogicalOp_t **eqParList = new LogicalOp_t *[numEqParents + 1];

    int i = 0;
    iter.Attach(&eqParents);
    while( !iter.IsEnd() ) {
        LogicalOp_t *parOp = iter.Next();
        assert(parOp);

        assert(i < numEqParents);
        eqParList[i++] = parOp;
    }
    assert(i == numEqParents);

    // remove all parent logExprs of eq from the hash table
    // next, make them point to $this$ instead of eq
    // and reinsert into the hash table, recursively unifying the parent
    // equivalence nodes if need be

    for( i = 0 ; i < numEqParents ; i++ ) {
        LogicalOp_t *parOp = eqParList[i];
        assert(parOp);

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
            cout << "Updating the inputs of LogExpr = " << parOp->ID() << endl;

        memo.Remove(parOp);

        // update the inputs to point to $this$ instead
        int n = parOp->NumInputs();
        for( int j = 0 ; j < n ; j++ ) {
            Equivalence_t *inp = parOp->Input(j);
            assert(inp);

            if( inp->ID() == eq->ID() )
                parOp->SetInput(j, this);
        }

        LogicalOp_t *prevParOp = memo.Search(parOp);
        if( prevParOp ) {
            // parOp already present as a parent of $this$
            Equivalence_t *parEq = parOp->EqClass();
            assert(parEq);

            Equivalence_t *prevParEq = prevParOp->EqClass();
            assert(prevParEq);

            // parOp should be removed from parEq->logExprs 
            parEq->DeleteLogExpr(parOp);
            parOp->SetEqClass(NULL);

            if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
                cout << "Deleting LogExpr = " << parOp->ID() << endl;
            // --- DESPERATE FIX --- //
#if 0 
            delete parOp;
#else
            parOp->Detach();
#endif

            parOp = NULL;

            if( parEq->ID() != prevParEq->ID() ) {
                // unification needed here
                if( prevParEq->IsInProgress() ) {
                    prevParEq->Unify(parEq, memo);

                    delete parEq;
                    parEq = NULL;
                } else {
                    parEq->Unify(prevParEq, memo);

                    delete prevParEq;
                    prevParEq = NULL;
                }
            }
        } else {
            memo.Insert(parOp);
        }
    }

    delete[] eqParList;

    numEqParents = eq->NumParents();
    assert(numEqParents == 0);

    // now merge eq->LogExprList() with logExprs ---
    // guaranteed to be disjoint

    const List_t<LogicalOp_t *>& eqLogExprs = eq->LogExprList();
    int numEqLogExprs = eqLogExprs.Size();

    // plus one to take care of zero size
    LogicalOp_t **eqLogExprList = new LogicalOp_t *[numEqLogExprs + 1];

    iter.Attach(&eqLogExprs);

    i = 0;
    while( !iter.IsEnd() ) {
        LogicalOp_t *op = iter.Next();
        assert(op);
        if( op->IsDetached() )
            continue;

        assert(i < numEqLogExprs);
        eqLogExprList[i++] = op;
    }

    // actual number of logical exprs
    numEqLogExprs = i;

    for( i = 0 ; i < numEqLogExprs ; i++ ) {
        LogicalOp_t *op = eqLogExprList[i];

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
            cout << "Moving LogExpr " << op->ID() << " from eq " << eq->ID()
                << " to " << ID() << endl;
        eq->DeleteLogExpr(op);
        op->SetEqClass(this);
        AddLogExpr(op);
    }

    delete[] eqLogExprList;
    assert(eq->NumLogExprs() == 0);

    // merge the plan groups next
    PlanGroup_t *pg = eq->PlanGroupBase();
    if( pg ) {
        int travID = PlanGroup_t::StartScan();

        if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
            cout << "Merging PG list of eq " << eq->ID()
                                << " into eq " << ID() << endl;
        MergePlanGroupList(travID, eq->PlanGroupBase());
    }

    MarkSharable();
}

// merge plan group lists
void Equivalence_t::MergePlanGroupList(int travID, PlanGroup_t *pg)
{
    assert(pg);
    if( pg->IsVisited(travID) )
        return;

    pg->Visit(travID);

    // get the associated physical property
    PhyPropGroup_t *pgPropGroup = pg->PropGroup();
    assert(pgPropGroup);

    // gets the corresponding plangroup in this equivalence class
    PlanGroup_t *myPG = PlanGroup(pgPropGroup);
    assert(myPG);
    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Merging PlanGroup " << pg->ID() <<
                        " with " << myPG->ID() << endl;

    // merge the two into myPG
    myPG->Merge(pg);

    POEdgeList_t& super = pg->SuperList();
    POEdgeIter_t pgIter;
    pgIter.Attach(&super);

    PlanGroup_t *nextPG = NULL;
    while( (nextPG = pgIter.Next()) )
        MergePlanGroupList(travID, nextPG);

    pgIter.Detach();
}

// create the list of degree of sharing given the lists of the children
// logical op nodes
// the children lists are topologically sorted wrt the eq nodes
AppendList_t<DegShar_t *> *Equivalence_t::CreateDegreeList(void)
{
    // new list
    AppendList_t<DegShar_t *> *list = new AppendList_t<DegShar_t *>;

    if( TopNo() != -1  )
        return list;

    // number of child logical op nodes
    int numLogExprs = logExprs.Size();

    if( numLogExprs > 0 ) {
        int i = -1;

        // input lists
        List_t<DegShar_t *> **inpList =
                        new List_t<DegShar_t *> *[numLogExprs];
        // is this the last reference for the input lists
        int *isLast = new int[numLogExprs];

        // iterators on the input lists
        ListIter_t<DegShar_t *> *inpIter =
                        new ListIter_t<DegShar_t *>[numLogExprs];

        // attach the iterators
        ListIter_t<LogicalOp_t *> logExprIter;
        logExprIter.Attach(&logExprs);

        for( i = 0 ; i < numLogExprs ; i++ ) {
            LogicalOp_t *inp = logExprIter.Next();
            assert(inp);

            inpList[i] = inp->DegreeList(&isLast[i]);
            assert(inpList[i]);

            inpIter[i].Attach(inpList[i]);
        }

        // initialize the current entries
        DegShar_t **curEntry  = new DegShar_t *[numLogExprs];
        for( i = 0 ; i < numLogExprs ; i++ ) {
            if( !inpIter[i].IsEnd() ) {
                curEntry[i] = inpIter[i].Next();
                assert(curEntry[i]);
            } else {
                curEntry[i] = NULL;
            }
        }

        for( ; ; ) {
            int minTopNo = -1;

            // get the min topNo
            for( i = 0 ; i < numLogExprs ; i++ ) {
                if( !curEntry[i] )
                    continue;

                const Equivalence_t *eq = curEntry[i]->EqNode();
                assert(eq);
                int eqTopNo = eq->TopNo();

                if( minTopNo == -1 || eqTopNo < minTopNo )
                    minTopNo = eqTopNo;
            }

            // if all input lists finished, break
            if( minTopNo == -1 )
                break;

            // take the max of degrees of the children with min topNo
            int maxDegree = 0;
            DegShar_t *maxDegShar = NULL;

            for( i = 0 ; i < numLogExprs ; i++ ) {
                if( !curEntry[i] )
                    continue;

                const Equivalence_t *eq = curEntry[i]->EqNode();
                assert(eq);
                int eqTopNo = eq->TopNo();

                if( eqTopNo != minTopNo )
                    continue;

                int curDegree = curEntry[i]->Degree();

                if( maxDegree < curDegree ) {
                    maxDegree = curDegree;
                    maxDegShar = curEntry[i];
                }

                if( !inpIter[i].IsEnd() )
                    curEntry[i] = inpIter[i].Next();
                else curEntry[i] = NULL;
            }
            assert(maxDegree > 0);

            // insert the max entry into the list
            Refer(maxDegShar);
            list->Insert(maxDegShar);
        }

        // delete the input lists if required
        for( i = 0 ; i < numLogExprs ; i++ ) {
            if( isLast[i] )
                DeleteDegreeList(inpList[i]);
        }

        delete[] curEntry;
        delete[] inpList;
        delete[] isLast;
        delete[] inpIter;
    }

    // set the topological number
    SetTopNo();

    DegShar_t *newDegShar = new DegShar_t(this, 1);
    Refer(newDegShar);
    list->Insert(newDegShar);

    return list;
}

// cost for using the materialized intermediate result
CostVal_t Equivalence_t::UseCost(void) const
{ return RelScan_t::Cost(logProp); }

// cost adt for using the materialized intermediate result
Cost_t *Equivalence_t::UseCostADT(Volcano_t *opt)
{
    if( !useCost ) {
        useCost = opt->CreateCost(NULL, UseCost());
        Refer(useCost);
    }

    return useCost;
}

// // cost for materializing the intermediate result
// CostVal_t Equivalence_t::MatCost(void) const
// { return isBaseRel ? 0 : RelScan_t::Cost(logProp); }

// print for debugging
void Equivalence_t::PrintExpressionDAG(int indentLevel) const
{
    for(int i = 0; i < indentLevel; i++)
        cout << "| ";
 
    cout << "[EQCLASS" << ID() << "]";
    if( isCorrelated )
        cout << " CORRELATED";
    cout << endl;
    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&logExprs);
 
    while( !iter.IsEnd() ) {
        LogicalOp_t *next = iter.Next();
        next->PrintExpressionDAG(indentLevel+1);
    }
}

void Equivalence_t::PrintExpressionDAGXML(int indentLevel) const
{
    for(int i = 0; i < indentLevel; i++)
        cout << "  ";
 
    cout << "<EQCLASS id=\"" << ID() << "\" isCorrelated=\"" 
         << (isCorrelated? "true" : "false") << "\">" << endl;

    if (isCorrelated) {
        assert(outerVarLogProp);
        cout << "<OuterVariables>" << endl;
        int nCorr = outerVarLogProp->Schema()->NumAttr();

        for (int i = 0; i < nCorr; i++) {
            Attr_t *attr = outerVarLogProp->Schema()->Attribute(i);
            cout << "<Attr relname=\"" << attr->RelName() << "\" attrname=\""
                 << attr->Name() << "\"/>" << endl;
        }
        cout << "</OuterVariables>" << endl;
    }
 
    /** Print the Scope 
    cout << "<OUTERSCOPE>" << endl;
    ListIter_t<LogProp_t *> scopeIter;
    scopeIter.Attach(&outerVariables);
    int level = outerVariables.Size();
    while( !scopeIter.IsEnd()) {
        cout << endl << "<Level-" << --level << ">" << endl;
        LogProp_t *next = scopeIter.Next();
        next->Schema()->PrintSQL();
        cout << endl << "</Level-" << level << ">" << endl;
    }
    cout << "</OUTERSCOPE>" << endl;
    */

    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&logExprs);
 
    while( !iter.IsEnd() ) {
        LogicalOp_t *next = iter.Next();
        next->PrintExpressionDAGXML(indentLevel+1);
    }
    cout << "</EQCLASS>" << endl;
}


// clean up --- free the allocated memory
Equivalence_t::~Equivalence_t(void)
{
    assert(!IsInProgress());

    if( planGroupBase ) {
        planGroupBase->Detach();
        DeRefer(planGroupBase);
    }

    DeRefer(logProp);
    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Deleted eq " << ID() << endl;

    assert(parents.Size() == 0);
    while( !logExprs.IsEmpty() ) {
        LogicalOp_t *op = logExprs.DeleteTop();
        assert(op);

        op->SetEqClass(NULL);
        delete op;
    }

    if( useCost ) DeRefer(useCost);

    Count--;
}

// find the degree list for the nested query
// invMult gives the invocation multiplicity
// TODO: equivalence class should have a list of corr attributes
// Ravi: What this method does? This is used only in Greedy.c 
// Ravi: How is this only related to Greedy?
int Equivalence_t::NQEqList(List_t<Equivalence_t *> *list, int isNested)
{
    assert(list);

    // check if already visited
    if( topNo != -1 )
        return isCorrelated;

    // get the schema
    assert(logProp);
    Schema_t *schema = logProp->Schema();
    assert(schema);

    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&logExprs);
 
    isCorrelated = 0;
    while( !iter.IsEnd() ) {
        LogicalOp_t *op = iter.Next();
        assert(op);

        if( op->Type() == SELECT_T || op->Type() == JOIN_T ) {
            // check the predicate
            Filter_t *f = (Filter_t *) op;
            Predicate_t *p = f->Predicate();
            assert(p);

            if( !isCorrelated ) {
                // Ravi: Bug:
                if( !p->IsValidSchema(NULL, schema) ) {
                    isCorrelated = 1;
                    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
                        cout << "Found Correlated Predicate in Eq = " <<
                                                                ID() << endl;
                }
            }

            InExpr_t *inP = (InExpr_t *) p->FindIn();
            if( inP ) {
                // nested predicate
                Equivalence_t *inpEq = inP->InpEq();
                assert(inpEq);

                inpEq->NQEqList(list, 1);
            }
        }

        int n = op->NumInputs();
        for( int i = 0 ; i < n ; i++ ) {
            Equivalence_t *eq = op->Input(i);
            assert(eq);

            int isInpCorrelated = eq->NQEqList(list, isNested);
            if( !isCorrelated ) {
                isCorrelated = isInpCorrelated;
                if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
                    if( isCorrelated )
                        cout << "Found Correlated Child Eq = " << eq->ID()
                                            << " in Eq = " << ID() << endl;
                }
            }

        }
    }

    // set the topological number
    if( isNested )
        SetTopNo();

    if( isNested && !isCorrelated ) {
        Equivalence_t *eq = this;
        list->Insert(eq);
    }

    if( isCorrelated && Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Correlated Eq = " << ID() << endl;

    return isCorrelated;
}


/**
 * Populates the outerVarLogProp
 */
void Equivalence_t::SetOuterVarLogProp()
{
    if (isOuterVarLogPropSet)
        return;

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Setting parameter logical props for Eq = " << ID() << endl;

    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&logExprs);

    // The correlation variables under  one child are same as 
    // the correlation  variables under any other child.  However, 
    // traversing all the children is a must since we need
    // to set the correlation vars for all equivalence nodes and
    // predicates beneath.
    while( !iter.IsEnd() ) {
        LogicalOp_t *op = iter.Next();
        assert(op);
        op->SetOuterVarLogProp();
    }
    isOuterVarLogPropSet = 1; 

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Setting paramLogProp for Eq = " << ID() 
             << " Done" << endl;
}

/**
 * this->OuterVarLogProp = this->OuterVarLogProp UNION aLogProp 
 */
void Equivalence_t::UnionOuterVarLogProp(LogProp_t *aLogProp)
{
    AppendList_t<Attr_t *> tmpList;
    int currAttrCount;

    // Put all the current attributes into the tmp list    
    if (!outerVarLogProp) {
        currAttrCount = 0;
    }
    else {
        currAttrCount = outerVarLogProp->Schema()->NumAttr();
    }
    for (int i = 0; i < currAttrCount; i++) {
        Attr_t *tmpAttr = outerVarLogProp->Schema()->Attribute(i);
        tmpList.Insert(tmpAttr);
    }


    // From the supplied props put those that are not already present
    int extraAttrCount;
    if (!aLogProp) {
        extraAttrCount = 0;
    }
    else {
        extraAttrCount = aLogProp->Schema()->NumAttr();
    }

    for (int i = 0; i < extraAttrCount; i++) {
        Attr_t *attr = aLogProp->Schema()->Attribute(i);
        assert(attr);

        // Insert only if not already present.
        int found = 0;
        for (int j = 0; j < currAttrCount; j++) {
            Attr_t *myattr = outerVarLogProp->Schema()->Attribute(j);
            assert(myattr);
            if (myattr->IsEquivalent(attr)) {
                found = 1;
                break;
            }
        }

        if (!found)
            tmpList.Insert(attr);
    }
   
    // Now convert the list to a new schema and set it.
    if (outerVarLogProp){
        delete(outerVarLogProp);
    }
    outerVarLogProp = MakeLogProp(MakeSchema(tmpList));
}


/**
 * plmap[i] tells the level of the outerVar identified by 
 * outerVarLogProp->Schema->Attribute(i) of this EQ node
 * TODO: For now skipping the check for valid sort orders
 */
List_t<Order_t *>* Equivalence_t::GetInterestingOrders(int *plmap, int level) 
{
    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) )
        cout << "Getting interesting props for Eq = " << ID() << endl;

    // TODO: Skipping the memoization of sort orders
    // If the set of interesting orders i ords for (e,plm) is already found
    //    return i ords

    // Create an empty set result of sort orders for (e,plm)
    List_t<Order_t *> *intOrds = new AppendList_t<Order_t *>();
    //  cout << "DEBUG: intOrds address : " << intOrds << endl;
    // TODO: Skipping the memoization

    if (!outerVarLogProp) {
        // cout << "Returning empty interesting orders for Eq " << ID() << endl;
        return intOrds;  // No outer vars used. Return empty set.
    }

    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&logExprs);
 
    while( !iter.IsEnd() ) {
        LogicalOp_t *logOp = iter.Next();
        logOp->GetInterestingOrders(intOrds, plmap, level);
    }

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
      // cout << "Returning: Interesting orders for Eq " << ID() << ":" << endl;
        PrintOrders(intOrds, OuterVarLogProp()->Schema());
    }
        
    return intOrds;
}

List_t<Order_t *>* Equivalence_t::GetFavorableOrders()
{
    if (favOrds != NULL) {
        return favOrds;
    }
    
    favOrds = new AppendList_t<Order_t *>();

    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&logExprs);
    while( !iter.IsEnd() ) {
        LogicalOp_t *logOp = iter.Next();
        logOp->GetFavorableOrders(favOrds);
    }

    if( Config_t::IsDebug(DEBUG_VERBOSE_INFO) ) {
        cout << "Computed the favorable orders for Eq " << ID() << endl;
        PrintOrders(favOrds, LogicalProp()->Schema());
    }
        
    return favOrds;
}
