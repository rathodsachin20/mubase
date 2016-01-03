VOL: Original optimizer without any extensions (Volcano Ord)
NQO: Optimizer with NQO extensions

Files in the test case dirs:

.sql       : Query
_nqo.plan  : Plan with NQO
-orig.plan : Plan with VOL
Catalog    : Catalog used for the case
.dump      : stdout with debug messages (you can ignore this)

case 01:
  A simple 2 level nested query. Inner relation not sorted.  
  NQO introduces an Apply, a state retaining scan and an explicit
  enforcer on the inner relation. VOL produces a plain NI plan.

case 02:
  Same as case 01 but inner relation has a built-in index (assumed 
  to be clustered). NQO plan same as before but wihout an enforcer.
  VOL plan same as before.

case 03:
  A nested query with 3 levels.

case 04:
  A nested query with 10 levels. All needed indexes present and for each
  block, the required sort order matches the paramete order and hence 
  no enforcers needed.

  Original volcano fails to produce a plan for any allowed cost limt.
  This is because of a bug in the cost computation. The cost of a block
  nested at level n gets multipled by numDistinct at each level, 
  irrespective of whether it's correlated with that block or not.

case 05:
  
    3 levels. Required order and guaranteed order for the middle block 
    do not match. An enforcer is introduced.


