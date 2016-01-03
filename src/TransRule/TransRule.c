// algebraic transformation rules

#include <assert.h>

#include "Config.h"
#include "Memo.h"
#include "TransRule.h"
#include "Expr.h"

void JoinAssoc_t::Apply(Join_t *op, Memo_t& memo)
{
    assert(op);

    Join_t *joinOuter = (Join_t *) op;
    Equivalence_t *eqClassOuter = joinOuter->EqClass();
    assert(eqClassOuter);

    Predicate_t *predOuter = joinOuter->Predicate();
    Equivalence_t *leftInpOuter = joinOuter->Input(0);
    Equivalence_t *rightInpOuter = joinOuter->Input(1);

    // left associativity
    Apply(eqClassOuter, predOuter, leftInpOuter, rightInpOuter, memo);

    // right associativity
    Apply(eqClassOuter, predOuter, rightInpOuter, leftInpOuter, memo);
}

// A(BC) --> (AB)C
void JoinAssoc_t::Apply(Equivalence_t *eqABC, Predicate_t *predABC,
                    Equivalence_t *eqA, Equivalence_t *eqBC, Memo_t& memo)
{
    assert(eqABC);
    assert(predABC);
    assert(eqA);
    assert(eqBC);

    const List_t<LogicalOp_t *>& exprList = eqBC->LogExprList();

    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&exprList);

    eqBC->MarkInProgress();

    if( Config_t::IsDebug(DEBUG_INFO) )
        cout << "JoinAssoc_t::Apply Iterating over " << eqBC->ID() << endl;

    while( !iter.IsEnd() ) {
        LogicalOp_t *childOp = iter.Next();
        assert(childOp);
        if( childOp->IsDetached() )
            continue;

        if( childOp->Type() != JOIN_T )
            continue;

        Join_t *joinBC = (Join_t *) childOp;

        Predicate_t *predBC = joinBC->Predicate();

        // no transformations involving cross products
        // if( predBC->IsEquivalent(ConstPreds.True()) )
        //       continue;

        Predicate_t *newPred = NULL;
        if( predABC->IsEquivalent(ConstPreds.True()) ||
                                        predBC->Implies(predABC) ) {
            newPred = predBC;
        } else if( predBC->IsEquivalent(ConstPreds.True()) ||
                                        predABC->Implies(predBC) ) {
            newPred = predABC;
        } else {
            newPred = new And_t(predABC, predBC);
        }

        Refer(newPred);

        Equivalence_t *eqB = joinBC->Input(0);
        Equivalence_t *eqC = joinBC->Input(1);

        // two cases due to commutativity of (BC)
        BuildJoinTree(eqABC, eqA, eqB, eqC, newPred, memo);
        BuildJoinTree(eqABC, eqA, eqC, eqB, newPred, memo);

        DeRefer(newPred);
    }

    eqBC->UnMarkInProgress();

    if( Config_t::IsDebug(DEBUG_INFO) )
        cout << "JoinAssoc_t::Apply Finished Iterating over " <<
                                                eqBC->ID() << endl;
}

// builds (AB)C and inserts into eqClass
// pred is the overall join predicate
// -- decomposed into inner and outer predicates depending on the inner
// table schema
void JoinAssoc_t::BuildJoinTree(Equivalence_t *eqClassABC,
                        Equivalence_t *inpA, Equivalence_t *inpB,
                        Equivalence_t *inpC, Predicate_t *pred, Memo_t& memo)
{
    assert(inpA);
    assert(inpB);
    assert(inpC);
    assert(pred);

    LogProp_t *lpABC = eqClassABC->LogicalProp();
    assert(lpABC);
    LogProp_t *lpA = inpA->LogicalProp();
    assert(lpA);
    LogProp_t *lpB = inpB->LogicalProp();
    assert(lpB);

    Schema_t *schemaABC = lpABC->Schema();
    assert(schemaABC);
    Schema_t *schemaA = lpA->Schema();
    assert(schemaA);
    Schema_t *schemaB = lpB->Schema();
    assert(schemaB);

    // merge the schemas of A and B to get the inner table schema
    Schema_t *schemaAB = MergeSchema(schemaA, schemaB);
    Refer(schemaAB);

    // decompose the predicate based on the inner table schema
    Predicate_t *predAB = NULL;
    Predicate_t *predABC = NULL;
    pred->Decompose(schemaABC, schemaAB, &predABC, &predAB);
    DeRefer(schemaAB);

    // PRASAN: don't introduce cross products
    if( !predAB->IsEquivalent(ConstPreds.True()) ) {
        LogicalOp_t *joinAB = new Join_t(predAB);
        joinAB->MarkDerived();

        joinAB->SetInput(0, inpA);
        joinAB->SetInput(1, inpB);

        Equivalence_t *eqClassAB = NULL;

        // search for the inner join expression in memo table
        LogicalOp_t *prevExprAB = memo.Search(joinAB);
        if( prevExprAB ) {
            // inner join expression already present
            delete joinAB;
            joinAB = NULL;
            eqClassAB = prevExprAB->EqClass();
        } else {
            // not present
            LogProp_t *logProp = joinAB->LogicalProp();
            Refer(logProp);

            eqClassAB = new Equivalence_t(logProp);
            DeRefer(logProp);

            joinAB->SetEqClass(eqClassAB);
            eqClassAB->AddLogExpr(joinAB);

            memo.Insert(joinAB);
        } 

        Join_t *joinABC = new Join_t(predABC);
        joinABC->MarkDerived();

        joinABC->SetInput(0, eqClassAB);
        joinABC->SetInput(1, inpC);

        // to avoid duplicate expressions
        joinABC->DisableTrans();

        LogicalOp_t *prevExprABC = memo.Search(joinABC);
        if( prevExprABC ) {
            // present

            delete joinABC;
            joinABC = NULL;

            Equivalence_t *prevEqClassABC = prevExprABC->EqClass();
            assert(prevEqClassABC);

            if( prevEqClassABC->ID() != eqClassABC->ID() ) {
                // unification required here

                if( memo.IsUnify() ) {
                    eqClassABC->Unify(prevEqClassABC, memo);
                    delete prevEqClassABC;
                    prevEqClassABC = NULL;

                    prevEqClassABC = prevExprABC->EqClass();
                    assert(prevEqClassABC->ID() == eqClassABC->ID());
                }
            }
        } else {
            // not present
            joinABC->SetEqClass(eqClassABC);
            eqClassABC->AddLogExpr(joinABC);

            memo.Insert(joinABC);
        }
    }

    DeRefer(predAB);
    DeRefer(predABC);
}

// (AB)(CD) --> (AC)(BD)
void JoinExchange_t::Apply(Join_t *op, Memo_t& memo)
{
    assert(op);

    Join_t *joinABCD = (Join_t *) op;
    Equivalence_t *eqABCD = joinABCD->EqClass();
    assert(eqABCD);
    Predicate_t *predABCD = joinABCD->Predicate();
    assert(predABCD);

    Equivalence_t *eqAB = joinABCD->Input(0);
    assert(eqAB);
    Equivalence_t *eqCD = joinABCD->Input(1);
    assert(eqCD);

    const List_t<LogicalOp_t *>& exprListAB = eqAB->LogExprList();
    const List_t<LogicalOp_t *>& exprListCD = eqCD->LogExprList();

    ListIter_t<LogicalOp_t *> iterAB;
    ListIter_t<LogicalOp_t *> iterCD;

    iterAB.Attach(&exprListAB);
    eqAB->MarkInProgress();

    if( Config_t::IsDebug(DEBUG_INFO) )
        cout << "JoinExchange_t::Apply Iterating over " << eqAB->ID() << endl;

    while( !iterAB.IsEnd() ) {
        LogicalOp_t *childOp = iterAB.Next();
        assert(childOp);
        if( childOp->IsDetached() )
            continue;

        if( childOp->Type() != JOIN_T )
            continue;

        Join_t *joinAB = (Join_t *) childOp;

        Predicate_t *predAB = joinAB->Predicate();

        Predicate_t *leftPred = NULL;
        if( predABCD->IsEquivalent(ConstPreds.True()) ||
                                        predAB->Implies(predABCD) ) {
            leftPred = predAB;
        } else if( predAB->IsEquivalent(ConstPreds.True()) ||
                                        predABCD->Implies(predAB) ) {
            leftPred = predABCD;
        } else {
            leftPred = new And_t(predABCD, predAB);
        }

        Refer(leftPred);

        Equivalence_t *eqA = joinAB->Input(0);
        Equivalence_t *eqB = joinAB->Input(1);

        iterCD.Attach(&exprListCD);
        eqCD->MarkInProgress();

        while( !iterCD.IsEnd() ) {
            LogicalOp_t *childOp = iterCD.Next();
            assert(childOp);
            if( childOp->IsDetached() )
                continue;

            if( childOp->Type() != JOIN_T )
                continue;

            Join_t *joinCD = (Join_t *) childOp;

            Predicate_t *predCD = joinCD->Predicate();

            Predicate_t *newPred = NULL;
            if( leftPred->IsEquivalent(ConstPreds.True()) ||
                                            predCD->Implies(leftPred) ) {
                newPred = predCD;
            } else if( predCD->IsEquivalent(ConstPreds.True()) ||
                                            leftPred->Implies(predCD) ) {
                newPred = leftPred;
            } else {
                newPred = new And_t(leftPred, predCD);
            }

            Refer(newPred);

            Equivalence_t *eqC = joinCD->Input(0);
            Equivalence_t *eqD = joinCD->Input(1);

            // two cases due to commutativity of (AB)
            BuildJoinTree(eqABCD, eqA, eqB, eqC, eqD, newPred, memo);
            BuildJoinTree(eqABCD, eqB, eqA, eqC, eqD, newPred, memo);
        }

        eqCD->UnMarkInProgress();
        DeRefer(leftPred);
    }

    eqAB->UnMarkInProgress();

    if( Config_t::IsDebug(DEBUG_INFO) )
        cout << "JoinExchange_t::Apply Finished Iterating over " <<
                                                eqAB->ID() << endl;
}

// (AB)(CD) --> (AC)(BD)
// builds (AC), (BD), (AC)(BD) and inserts into respective classes
// pred is the overall join predicate
// -- decomposed into inner and outer predicates depending on the inner
// table schema
void JoinExchange_t::BuildJoinTree(Equivalence_t *eqClassABCD,
                        Equivalence_t *inpA, Equivalence_t *inpB,
                        Equivalence_t *inpC, Equivalence_t *inpD,
                        Predicate_t *pred, Memo_t& memo)
{
    assert(inpA);
    assert(inpB);
    assert(inpC);
    assert(inpD);
    assert(pred);

    LogProp_t *lpABCD = eqClassABCD->LogicalProp();
    assert(lpABCD);

    LogProp_t *lpA = inpA->LogicalProp();
    assert(lpA);
    LogProp_t *lpB = inpB->LogicalProp();
    assert(lpB);
    LogProp_t *lpC = inpC->LogicalProp();
    assert(lpC);
    LogProp_t *lpD = inpD->LogicalProp();
    assert(lpD);

    Schema_t *schemaABCD = lpABCD->Schema();
    assert(schemaABCD);
    Schema_t *schemaA = lpA->Schema();
    assert(schemaA);
    Schema_t *schemaB = lpB->Schema();
    assert(schemaB);
    Schema_t *schemaC = lpC->Schema();
    assert(schemaC);
    Schema_t *schemaD = lpD->Schema();
    assert(schemaD);

    // merge the schemas of A and C to get the left table schema
    Schema_t *schemaAC = MergeSchema(schemaA, schemaC);
    Refer(schemaAC);

    // merge the schemas of B and D to get the left table schema
    Schema_t *schemaBD = MergeSchema(schemaB, schemaD);
    Refer(schemaBD);

    // decompose the predicate based on the inner tables' schema
    Predicate_t *predAC = NULL;
    Predicate_t *predBD = NULL;
    Predicate_t *predABCD = NULL;
    Predicate_t *tmpPred = NULL;
    pred->Decompose(schemaABCD, schemaAC, &tmpPred, &predAC);
    DeRefer(schemaAC);

    tmpPred->Decompose(schemaABCD, schemaBD, &predABCD, &predBD);
    DeRefer(schemaBD);

    DeRefer(tmpPred);

    LogicalOp_t *joinAC = new Join_t(predAC);
    joinAC->MarkDerived();

    joinAC->SetInput(0, inpA);
    joinAC->SetInput(1, inpC);

    Equivalence_t *eqClassAC = NULL;

    // search for the inner join expression in memo table
    LogicalOp_t *prevExprAC = memo.Search(joinAC);
    if( prevExprAC ) {
        // inner join expression already present
        delete joinAC;
        joinAC = NULL;
        eqClassAC = prevExprAC->EqClass();
    } else {
        // not present
        LogProp_t *logProp = joinAC->LogicalProp();
        Refer(logProp);

        eqClassAC = new Equivalence_t(logProp);
        DeRefer(logProp);

        joinAC->SetEqClass(eqClassAC);
        eqClassAC->AddLogExpr(joinAC);

        memo.Insert(joinAC);
    } 

    LogicalOp_t *joinBD = new Join_t(predBD);
    joinBD->MarkDerived();

    joinBD->SetInput(0, inpB);
    joinBD->SetInput(1, inpD);

    Equivalence_t *eqClassBD = NULL;

    // search for the inner join expression in memo table
    LogicalOp_t *prevExprBD = memo.Search(joinBD);
    if( prevExprBD ) {
        // inner join expression already present
        delete joinBD;
        joinBD = NULL;
        eqClassBD = prevExprBD->EqClass();
    } else {
        // not present
        LogProp_t *logProp = joinBD->LogicalProp();
        Refer(logProp);

        eqClassBD = new Equivalence_t(logProp);
        DeRefer(logProp);

        joinBD->SetEqClass(eqClassBD);
        eqClassBD->AddLogExpr(joinBD);

        memo.Insert(joinBD);
    } 

    Join_t *joinABCD = new Join_t(predABCD);
    joinABCD->MarkDerived();

    joinABCD->SetInput(0, eqClassAC);
    joinABCD->SetInput(1, eqClassBD);

    // to avoid duplicate expressions
    joinABCD->DisableTrans();

    LogicalOp_t *prevExprABCD = memo.Search(joinABCD);
    if( prevExprABCD ) {
        // present

        delete joinABCD;
        joinABCD = NULL;

        Equivalence_t *prevEqClassABCD = prevExprABCD->EqClass();
        assert(prevEqClassABCD);

        if( prevEqClassABCD->ID() != eqClassABCD->ID() ) {
            // unification required here

            if( memo.IsUnify() ) {
                eqClassABCD->Unify(prevEqClassABCD, memo);
                delete prevEqClassABCD;
                prevEqClassABCD = NULL;

                prevEqClassABCD = prevExprABCD->EqClass();
                assert(prevEqClassABCD->ID() == eqClassABCD->ID());
            }
        }
    } else {
        // not present
        joinABCD->SetEqClass(eqClassABCD);
        eqClassABCD->AddLogExpr(joinABCD);

        memo.Insert(joinABCD);
    }

    DeRefer(predAC);
    DeRefer(predBD);
    DeRefer(predABCD);
}

void JoinPushPred_t::Apply(Join_t *op, Memo_t& memo)
{
    assert(op);

    Join_t *join = (Join_t *) op;

    LogProp_t *lp = op->LogicalProp();
    Refer(lp);

    Schema_t *schema = lp->Schema();
    Refer(schema);
    DeRefer(lp);

    Predicate_t *pred = join->Predicate();
    if( pred->IsEquivalent(ConstPreds.True()) )
        return;

    Equivalence_t *eqClass = join->EqClass();
    assert(eqClass);

    int numInputs = join->NumInputs();

    // push predicates across one input at a time
    int i = 0;
    for( i = 0 ; i < numInputs ; i++ ) {
        Equivalence_t *inp = join->Input(i);
        assert(inp);

        LogProp_t *inpLP = inp->LogicalProp();
        assert(inpLP);

        Schema_t *inpSchema = inpLP->Schema();

        Predicate_t *parPred = NULL;
        Predicate_t *childPred = NULL;
        pred->Decompose(schema, inpSchema, &parPred, &childPred);

        if( !parPred->IsEquivalent(join->Predicate()) ) {
            // some part of the predicate pushed in
            // create the new join node and set its inputs

            // push childPred into the input
            Equivalence_t *newInp = Push(inp, childPred, memo);
            assert(newInp);

            // instantiate a join node, set its inputs and insert in the
            // eq class
            Join_t *newJoin = new Join_t(parPred);
            newJoin->MarkDerived();
            newJoin->SetInput(i, newInp);
            for( int k = 0 ; k < numInputs ; k++ ) {
                if( k != i )
                    newJoin->SetInput(k, join->Input(k));
            }

            // search for the join expression in memo table
            LogicalOp_t *prevExpr = memo.Search(newJoin);
            if( prevExpr ) {
                // present

                delete newJoin;
                newJoin = NULL;

                Equivalence_t *prevEqClass = prevExpr->EqClass();
                assert(prevEqClass);

                if( prevEqClass->ID() != eqClass->ID() ) {
                    // unification required here

                    if( memo.IsUnify() ) {
                        eqClass->Unify(prevEqClass, memo);
                        delete prevEqClass;
                        prevEqClass = NULL;

                        prevEqClass = prevExpr->EqClass();
                        assert(prevEqClass->ID() == eqClass->ID());
                    }
                }
            } else {
                // not present

                newJoin->SetEqClass(eqClass);
                eqClass->AddLogExpr(newJoin);

                memo.Insert(newJoin);
            }
        }

        DeRefer(parPred);
        DeRefer(childPred);
    }

    // push predicates across all inputs at once
    Equivalence_t **inpList = new Equivalence_t *[numInputs];
    Refer(pred);
    for( i = 0 ; i < numInputs ; i++ ) {
        Equivalence_t *inp = join->Input(i);
        assert(inp);

        LogProp_t *inpLP = inp->LogicalProp();
        assert(inpLP);

        Schema_t *inpSchema = inpLP->Schema();

        Predicate_t *parPred = NULL;
        Predicate_t *childPred = NULL;
        pred->Decompose(schema, inpSchema, &parPred, &childPred);

        // update the predicate
        DeRefer(pred);
        pred = parPred;
        Refer(pred);

        inpList[i] = Push(inp, childPred, memo);

        DeRefer(parPred);
        DeRefer(childPred);
    }

    if( !pred->IsEquivalent(join->Predicate()) ) {
        // some part of the predicate pushed in
        // create the new join node and set its inputs

        // instantiate a join node and insert in the eq class
        Join_t *newJoin = new Join_t(pred);
        newJoin->MarkDerived();

        for( int j = 0 ; j < numInputs ; j++ ) {
            assert(inpList[j]);
            newJoin->SetInput(j, inpList[j]);
        }

        // search for the join expression in memo table
        LogicalOp_t *prevExpr = memo.Search(newJoin);
        if( prevExpr ) {
            // present

            delete newJoin;
            newJoin = NULL;

            Equivalence_t *prevEqClass = prevExpr->EqClass();
            assert(prevEqClass);

            if( prevEqClass->ID() != eqClass->ID() ) {
                // unification required here

                if( memo.IsUnify() ) {
                    eqClass->Unify(prevEqClass, memo);
                    delete prevEqClass;
                    prevEqClass = NULL;

                    prevEqClass = prevExpr->EqClass();
                    assert(prevEqClass->ID() == eqClass->ID());
                }
            }
        } else {
            // not present

            newJoin->SetEqClass(eqClass);
            eqClass->AddLogExpr(newJoin);

            memo.Insert(newJoin);
        }
    }

    DeRefer(schema);
    DeRefer(pred);

    delete[] inpList;
}

// put filter predicate above the equivalence node 
Equivalence_t *JoinPushPred_t::Push(Equivalence_t *inpEq,
                                        Predicate_t *pred, Memo_t& memo)
{
    assert(pred);
    assert(inpEq);

    if( pred->IsEquivalent(ConstPreds.True()) ) {
        // nothing to push
        return inpEq;
    }

    // push the predicate down as a select over inpEq
    Select_t *parSelect = new Select_t(pred);
    parSelect->MarkDerived();

    parSelect->SetInput(0, inpEq);

    Equivalence_t *eq = NULL;

    LogicalOp_t *prevSel = memo.Search(parSelect);
    if( prevSel ) {
        // already present
        delete parSelect;
        parSelect = NULL;

        eq = prevSel->EqClass();
    } else {
        LogProp_t *lp = parSelect->LogicalProp();
        Refer(lp);

        eq = new Equivalence_t(lp);
        DeRefer(lp);

        parSelect->SetEqClass(eq);
        eq->AddLogExpr(parSelect);

        memo.Insert(parSelect);
    }

    return eq;
}

// Ravi:Com:This transformation separates the correlated and uncorrelated 
// Ravi:Com:predicates for making it possible to materialize the result of 
// Ravi:applying the uncorrelated predicates.
void SelectPushPred_t::Apply(Select_t *op, Memo_t& memo)
{
    assert(op);

    Select_t *parSelect = (Select_t *) op;
    Equivalence_t *eq = parSelect->EqClass();
    assert(eq);

    Predicate_t *pred = parSelect->Predicate();
    Refer(pred);
    // redundant selects should not exist
    if( pred->IsEquivalent(ConstPreds.True()) )
        return;

    Equivalence_t *inpEq = parSelect->Input(0);
    assert(inpEq);

    // decompose into the correlated and uncorrelated predicates
    LogProp_t *parLP = parSelect->LogicalProp();
    Refer(parLP);

    Schema_t *parSchema = parLP->Schema();
    assert(parSchema);
    DeRefer(parLP);

    Predicate_t *unCorrelatedPred = NULL;
    Predicate_t *correlatedPred = NULL;

    // predicate should be separated into correlated and uncorrelated
    // parts to enable uncorrelated part to be materialized
    // usual option of pushing in the predicate to the join is also
    // retained so that correlated selection can be used
    pred->Decompose(NULL, parSchema, &correlatedPred, &unCorrelatedPred);
    if( !correlatedPred->IsEquivalent(ConstPreds.True()) &&
                !unCorrelatedPred->IsEquivalent(ConstPreds.True()) ) {
        // the predicate has a correlated component; separate it
        
        LogicalOp_t *unCorrelSel = new Select_t(unCorrelatedPred);
        unCorrelSel->SetInput(0, inpEq);

        Equivalence_t *unCorrelEq = NULL;
        LogicalOp_t *prevUnCorrelSel = memo.Search(unCorrelSel);
        if( prevUnCorrelSel ) {
            // already present
            delete unCorrelSel;
            unCorrelSel = prevUnCorrelSel;

            unCorrelEq = prevUnCorrelSel->EqClass();
        } else {
            LogProp_t *lp = unCorrelSel->LogicalProp();
            Refer(lp);

            unCorrelEq = new Equivalence_t(lp);
            DeRefer(lp);

            unCorrelSel->SetEqClass(unCorrelEq);
            unCorrelEq->AddLogExpr(unCorrelSel);

            memo.Insert(unCorrelSel);
        }

        LogicalOp_t *correlSel = new Select_t(correlatedPred);
        correlSel->SetInput(0, unCorrelEq);

        LogicalOp_t *prevCorrelSel = memo.Search(correlSel);
        if( prevCorrelSel ) {
            // already present
            delete correlSel;
            correlSel = NULL;

            Equivalence_t *prevCorrelEq = prevCorrelSel->EqClass();
            assert(prevCorrelEq);

            if( prevCorrelEq->ID() != eq->ID() ) {
                // unification required here

                if( memo.IsUnify() ) {
                    eq->Unify(prevCorrelEq, memo);
                    delete prevCorrelEq;

                    prevCorrelEq = prevCorrelSel->EqClass();
                    assert(prevCorrelEq->ID() == eq->ID());
                }
            }
        } else {
            correlSel->SetEqClass(eq);
            eq->AddLogExpr(correlSel);

            memo.Insert(correlSel);
        }
    }

    // check if some join/sel can be merged with select
    const List_t<LogicalOp_t *>& exprList = inpEq->LogExprList();

    inpEq->MarkInProgress();

    if( Config_t::IsDebug(DEBUG_INFO) )
        cout << "SelectPushPred_t::Apply Iterating over " <<
                                                inpEq->ID() << endl;
    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&exprList);

    while( !iter.IsEnd() ) {
        LogicalOp_t *childOp = iter.Next();
        assert(childOp);
        if( childOp->IsDetached() )
            continue;

        LogicalOp_t *newChildOp = childOp->PushInPred(pred, memo);

        // newOp is NULL if no new operator is generated
        if( !newChildOp )
            continue;

        LogicalOp_t *prev = memo.Search(newChildOp);
        if( prev ) {
            // present

            delete newChildOp;
            newChildOp = NULL;

            Equivalence_t *prevEq = prev->EqClass();
            assert(prevEq);

            if( prevEq->ID() != eq->ID() ) {
                // unification required here

                if( memo.IsUnify() ) {
                    eq->Unify(prevEq, memo);
                    delete prevEq;

                    prevEq = prev->EqClass();
                    assert(prevEq->ID() == eq->ID());
                }
            }
        } else {
            // not present
            newChildOp->SetEqClass(eq);
            eq->AddLogExpr(newChildOp);

            memo.Insert(newChildOp);
        }
    }

    inpEq->UnMarkInProgress();

    if( Config_t::IsDebug(DEBUG_INFO) )
        cout << "SelectPushPred_t::Apply Finished Iterating over " <<
                                                        inpEq->ID() << endl;

    DeRefer(pred);
}

void PushProj_t::Apply(Project_t *op, Memo_t& memo)
{
    assert(op);

    Project_t *proj = (Project_t *) op;

    Equivalence_t *eq = proj->EqClass();
    assert(eq);

    LogProp_t *projLP = eq->LogicalProp();
    assert(projLP);

    // check if the project can be pushed down across an input
    Equivalence_t *inpEq = proj->Input(0);

    const List_t<LogicalOp_t *>& exprList = inpEq->LogExprList();

    inpEq->MarkInProgress();

    if( Config_t::IsDebug(DEBUG_INFO) )
        cout << "PushProj_t::Apply Iterating over " << inpEq->ID() << endl;
    ListIter_t<LogicalOp_t *> iter;
    iter.Attach(&exprList);

    while( !iter.IsEnd() ) {
        LogicalOp_t *op = iter.Next();
        assert(op);
        if( op->IsDetached() )
            continue;

        LogicalOp_t *newOp = op->PushInProj(proj, memo);

        // newOp is NULL if no new operator is generated
        if( !newOp )
            continue;

        LogicalOp_t *prev = memo.Search(newOp);
        if( prev ) {
            // present

            delete newOp;
            newOp = NULL;

            Equivalence_t *prevEq = prev->EqClass();
            assert(prevEq);

            if( prevEq->ID() != eq->ID() ) {
                // unification required here

                if( memo.IsUnify() ) {
                    eq->Unify(prevEq, memo);
                    delete prevEq;
                    prevEq = NULL;

                    prevEq = prev->EqClass();
                    assert(prevEq->ID() == eq->ID());
                }
            }
        } else {
            // not present
            newOp->SetEqClass(eq);
            eq->AddLogExpr(newOp);

            memo.Insert(newOp);
        }
    }

    inpEq->UnMarkInProgress();

    if( Config_t::IsDebug(DEBUG_INFO) )
        cout << "PushProj_t::Apply Finished Iterating over " <<
                                                inpEq->ID() << endl;
}

// Ravi: New method for nested predicate to Apply op transformation
void NestedPredToApplyOp_t::Apply(Select_t *op, Memo_t& memo)
{
    if(Config_t::IsDebug(DEBUG_INFO)) {
        cout << endl << "Applying Transformation: NestedPredToApplyOp." 
             << endl << "On Select_t node: " << op->ID() << endl;
    }
    
    assert(op);
    Select_t *sel = (Select_t *) op;

    Equivalence_t *eq = sel->EqClass();
    assert(eq);

    LogProp_t *selLP = eq->LogicalProp();
    assert(selLP);

    Predicate_t *pred = sel->Predicate();
    InExpr_t *inCond = (InExpr_t *) pred->FindIn();     

    // Check if at least one IN predicate exists
    if (inCond == NULL) {
        if(Config_t::IsDebug(DEBUG_INFO))
            cout << endl << "No nested predicate in logexpr: " 
                 << op->ID() << endl;
        return;
    }

    Card_t numDist = inCond->NumDistinct(selLP);

    if(Config_t::IsDebug(DEBUG_INFO)) {
        cout << endl << "Nested predicate present in logexpr: " 
             << op->ID() << endl;
        cout << "Original Predicate (Before removing nested pred):" << endl;
        pred->PrintExpr(1);
    }

    InExpr_t *inExpr = NULL;
    // newPredicate is the predicate after removing the IN. Initially 
    // make a deep copy of the original predicate.
    Predicate_t *newPredicate = (Predicate_t *) pred->Copy(); 

    newPredicate = removeInExpr(newPredicate, &inExpr);

    assert(inExpr);
    assert(newPredicate);

    if(Config_t::IsDebug(DEBUG_INFO)) {
        cout << endl << "Modified Predicate (After removing nested pred):" 
             << endl;
        newPredicate->PrintExpr(1);
    }

    if(Config_t::IsDebug(DEBUG_INFO)) {
        cout << endl << "Exracted Nested Predicate: " << endl;
        inExpr->PrintExpr(1);
    }

    Equivalence_t *applyOpChild; // Points the eq node that should be 
                                 // the left input of the apply op.

    if (!newPredicate->IsEquivalent(ConstPreds.True())) {
        Select_t *newSelectOp = newSelectOp = new Select_t(newPredicate);
        newSelectOp->SetInput(0, sel->Input(0));
        // Add the new select op to a new eq class. 
        // TODO: Recompute the logical properties for the new eq class. 
        // Currently using the old ones.
        applyOpChild = new Equivalence_t(selLP);
        //Refer(applyOpChild);
        newSelectOp->SetEqClass(applyOpChild);
        applyOpChild->AddLogExpr(newSelectOp);
        memo.Insert(newSelectOp);
        if (Config_t::IsDebug(DEBUG_INFO)) {
            cout << endl << "ID of the new Select_t: " << newSelectOp->ID() 
                 << endl;
            cout << endl << "EQ node for the new Select_t: " 
                 << applyOpChild->ID() << endl;
        }
    } 
    else {
        applyOpChild = sel->Input(0);
        //Refer(applyOpChild);
    }

    LeftSemiJoin_t *innerOp = new LeftSemiJoin_t(applyOpChild->LogicalProp(),
                                            inExpr->InpEq()->LogicalProp(),
                                            inExpr);
   // Apply_t *applyOp = new Apply_t(inExpr);
    Apply_t *applyOp = new Apply_t(innerOp);
    applyOp->SetInput(0, applyOpChild);
    applyOp->SetInput(1, inExpr->InpEq());
    applyOp->SetNumDistinct(numDist);
    //updateOuterVariables(applyOp->Input(1), applyOp->Input(0));

    applyOp->SetEqClass(eq);
    eq->AddLogExpr(applyOp);
    memo.Insert(applyOp);
    //DeRefer(applyOpChild);

    if (Config_t::IsDebug(DEBUG_INFO)) {
        cout << endl << "ID of the new Apply_t: " << applyOp->ID() << endl;
        cout << endl << "EQ node for the new Apply_t : " << eq->ID() << endl;
    }

    if(Config_t::IsDebug(DEBUG_INFO))
        cout << endl << "NestedPredToApplyOp_t::Apply Finished" << endl;
}


/**
 * Assumptions:
 * 1. The predicate is only conjunctive.
 * 2. Only one IN expression exists in the entire predicate
 */
Predicate_t *NestedPredToApplyOp_t::removeInExpr(Predicate_t *pred,
                                                 InExpr_t **inExpr)
{
    if(Config_t::IsDebug(DEBUG_INFO))
        cout << endl << "Inside method NestedPredToApplyOp_t::removeInExpr()" 
             << endl;

    assert(pred);

    if (pred->PredOpType() == IN_T) {
        *inExpr = (InExpr_t *) pred;
        Refer(*inExpr);
        return new True_t();
    }

    int n = pred->NumInputs();
    for (int i = 0; i < n; i++) {
        Expr_t *childExpr = pred->Input(i);
        if (childExpr->ExprType() == PREDICATE_T) {
            Predicate_t *newChild = removeInExpr((Predicate_t*)childExpr , 
                                                  inExpr);
            Refer(newChild);
            pred->SetInput(i, newChild);
            DeRefer(newChild);
        }
    }
    return pred;
}

/**
 * Updates the outer variable list of innerEq with the variables 
 * bound in the outerEq.
void NestedPredToApplyOp_t::updateOuterVariables(Equivalence_t *innerEq,
                                                 Equivalence_t *outerEq) {
        innerEq->PushOnScope(outerEq->LogicalProp());
}
*/

