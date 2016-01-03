// transformation rules

#ifndef __TRANSRULE_H__
#define __TRANSRULE_H__

class Memo_t;
class LogicalOp_t;
class Equivalence_t;
class Predicate_t;

// (AB)(CD) <--> (AC)(BD) and variations due to commutativity
class JoinExchange_t {
    static void BuildJoinTree(Equivalence_t *eqABCD,
                        Equivalence_t *inpA, Equivalence_t *inpB,
                        Equivalence_t *inpC, Equivalence_t *inpD,
                        Predicate_t *pred, Memo_t& memo);

public:
    static void Apply(Join_t *op, Memo_t& memo);
};

// A(BC) <--> (AB)C and variations due to commutativity
class JoinAssoc_t {
    static void BuildJoinTree(Equivalence_t *eqABC,
                        Equivalence_t *inpA, Equivalence_t *inpB,
                        Equivalence_t *inpC, Predicate_t *pred, Memo_t& memo);
    static void Apply(Equivalence_t *eqABC, Predicate_t *predABC,
                        Equivalence_t *eqA, Equivalence_t *eqBC, Memo_t &memo);

public:
    static void Apply(Join_t *op, Memo_t& memo);
};

// predicate pushdown for join
class JoinPushPred_t {
    static Equivalence_t *Push(Equivalence_t *inpEq,
                                Predicate_t *pred, Memo_t& memo);

public:
    static void Apply(Join_t *op, Memo_t& memo);
};

// predicate pushdown for select
class SelectPushPred_t {
public:
    static void Apply(Select_t *op, Memo_t& memo);
};

// project pushdown
class PushProj_t {
public:
    static void Apply(Project_t *op, Memo_t& memo);
};

// Ravi: New class for transforming nested predicate to Apply op
class NestedPredToApplyOp_t {
    static Predicate_t *removeInExpr(Predicate_t *pred,
                                     InExpr_t **inExpr);
    static void updateOuterVariables(Equivalence_t *innerEq,
                                     Equivalence_t *outerEq);
public:
    static void Apply(Select_t *op, Memo_t& memo);
};

#endif // __TRANSRULE_H__
