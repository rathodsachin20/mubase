#####################
# The following gets in all the standard dependencies, compilers, etc,
# defined in Make.include
####################
OPTPATH =/home/sysad/Desktop/sachin4/PYRO
LIBPATH = $(OPTPATH)/lib
DALIPARSERPATH = $(OPTPATH)/src/DaliParser

# Which are the compilers to use
CPlus= g++ -ggdb -Wno-deprecated
C= gcc
AR= ar ru ./driver

########
# Following defines which directories have include files
# It is usually good to have an include directory where you place all
# your include files
########
SYSINCLUDES= 
INCLUDES= -I$(OPTPATH)/src/Include ${SYSINCLUDES}  -I.
PARINCS = -I$(OPTPATH)/src/Include/DaliParser
########
# for example: 
#   -O for optimization, 
#   -g to debug, 
#   if omitted no debugging information is generated, and no optimization done
########
#OPTFLAG=-ggdb -Wall -Wcast-qual -Wpointer-arith -Wstrict-prototypes -Wcast-align #-DLGMDEBUG #-O3 -DNDEBUG #
#OPTFLAG=-O3 -DNDEBUG
#Ravi: Removing optimization
OPTFLAG=-DNDEBUG
#OPTFLAG=-pg -DNDEBUG

########
# Following defines which directories have libraries to include
# eg. to include ../lib/mylib.a use the following
# LIBS= -L../lib/mylib
# Default below is set to no libraries
########
LIBS= -L$(LIBPATH) -lopt -lparser -lm
#LIBS=  -L$(DALILIBPATH) -ldali -lsocket -lnsl -ldl -lintl -lucb -lm
#LIBS=  -L$(DALILIBPATH) -lucb -lm
###########################################################################
CFLAGS= $(INCLUDES) $(OPTFLAG)
CPlusFlags= $(CFLAGS)

#########
# The following definition is used by the makedepend command when it is
# not configured properly.  For now it's set to default
#########
# CPlusROOT=
# OTHERINCLUDES= -I$(CPlusROOT)
#OTHERINCLUDES= /usr/local/SUNWspro/SC3.0.1/include/CC $(PARINCS)
OTHERINCLUDES= /usr/include/g++ $(PARINCS)

###########################################################################
# Standard dependencies.  Comes predefined on some versions of make
#########

.SUFFIXES: .C .c .cc

.C.o:
	$(CPlus) -c $(CPlusFlags) $*.C -o $@

.cc.o:
	$(CPlus) -c $(CPlusFlags) $*.cc -o $@

.c.o:   
	$(CPlus) -c $(CPlusFlags) $*.c -o $@

#    uncomment the following if you don't want to see how the make 
#    proceeds -  
#
#.SILENT:
###########################################################################

#########
# Define what are the source files
#########
OPTSRCS = $(OPTPATH)/src/Mubase/dm.cpp  $(OPTPATH)/src/Mubase/bm.cpp  $(OPTPATH)/src/Mubase/sm.cpp  $(OPTPATH)/src/Mubase/smutil.cpp  $(OPTPATH)/src/Mubase/bt.cpp  $(OPTPATH)/src/Mubase/sputil.c  $(OPTPATH)/src/Mubase/sp.cpp  $(OPTPATH)/src/Mubase/rm.cpp  $(OPTPATH)/src/Mubase/sort.cpp  $(OPTPATH)/src/Mubase/iter.cpp $(OPTPATH)/src/Config/Config.c $(OPTPATH)/src/Catalog/Catalog.c $(OPTPATH)/src/Cost/Cost.c $(OPTPATH)/src/Cost/CostList.c $(OPTPATH)/src/Equivalence/Equivalence.c $(OPTPATH)/src/Expr/Expr.c $(OPTPATH)/src/Interface/ExprInterface.c $(OPTPATH)/src/ExhSearch/ExhSearch.c $(OPTPATH)/src/Greedy/Greedy.c $(OPTPATH)/src/Interface/Interface.c $(OPTPATH)/src/LogProp/LogProp.c $(OPTPATH)/src/LogicalOp/LogicalOp.c $(OPTPATH)/src/Memo/Memo.c $(OPTPATH)/src/Node/Node.c $(OPTPATH)/src/PhyProp/PhyProp.c $(OPTPATH)/src/PhyPropGroup/PhyPropGroup.c $(OPTPATH)/src/PhysicalOp/PhysicalOp.c $(OPTPATH)/src/Plan/Plan.c $(OPTPATH)/src/TransRule/TransRule.c $(OPTPATH)/src/Volcano/Volcano.c $(OPTPATH)/src/Eval/Eval.c

#DALISRCS = $(DALIPARSERPATH)/ddl_stmts.c $(DALIPARSERPATH)/evaluate.c $(DALIPARSERPATH)/expr.c $(DALIPARSERPATH)/optimize.c $(DALIPARSERPATH)/prim_types.c $(DALIPARSERPATH)/rel_alg.c scanner.c parser.c
DALISRCS = $(DALIPARSERPATH)/expr.c $(DALIPARSERPATH)/prim_types.c scanner.c parser.c $(DALIPARSERPATH)/rel_alg.c
#DALISRCS = $(DALIPARSERPATH)/prim_types.c scanner.c parser.c

SRCS = driver.c $(OPTSRCS) $(DALISRCS)

#########
# Automatically define what are object files
#########
#OBJS = $(SRCS:.c=.o)  

OPTOBJS = dm.o bm.o sm.o smutil.o bt.o sputil.o sp.o rm.o sort.o iter.o Catalog.o Config.o Cost.o CostList.o Equivalence.o Expr.o ExprInterface.o ExhSearch.o Greedy.o Interface.o LogProp.o LogicalOp.o Memo.o Node.o PhyProp.o PhyPropGroup.o PhysicalOp.o Plan.o TransRule.o Volcano.o Eval.o

#PARSEROBJS = ddl_stmts.o evaluate.o expr.o optimize.o prim_types.o rel_alg.o scanner.o parser.o
PARSEROBJS = expr.o prim_types.o scanner.o parser.o rel_alg.o
#PARSEROBJS = prim_types.o scanner.o parser.o

########
# All object files will be made first, then executables are made
# We have already
########
all: perfeval driver
perfeval: perfeval.o $(LIBPATH)/libopt.a $(LIBPATH)/libparser.a 
	$(CPlus) -o $@ $(CPlusFlags) perfeval.o $(LIBS)

perfeval.o: perfeval.c
	$(CPlus) -c $(CPlusFlags) perfeval.c -o $@

driver: driver.o $(LIBPATH)/libopt.a $(LIBPATH)/libparser.a 
	$(CPlus) -o $@ $(CPlusFlags) driver.o $(LIBS)

#driver: driver.o $(OPTOBJS) $(PARSEROBJS)
#	$(CPlus) -o $@ $(CPlusFlags) driver.o $(OPTOBJS) $(PARSEROBJS) $(LIBS)

driver.o: driver.c
	$(CPlus) -c $(CPlusFlags) driver.c -o $@

lib: $(LIBPATH)/libparser.a $(LIBPATH)/libopt.a

$(LIBPATH)/libparser.a: $(PARSEROBJS)
	ar ru $(LIBPATH)/libparser.a $(PARSEROBJS)

$(LIBPATH)/libopt.a : $(OPTOBJS)
	ar ru $(LIBPATH)/libopt.a $(OPTOBJS)

dm.o: $(OPTPATH)/src/Mubase/dm.cpp
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Mubase/dm.cpp -o $@

bm.o: $(OPTPATH)/src/Mubase/bm.cpp
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Mubase/bm.cpp -o $@

sm.o: $(OPTPATH)/src/Mubase/sm.cpp
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Mubase/sm.cpp -o $@

smutil.o: $(OPTPATH)/src/Mubase/smutil.cpp
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Mubase/smutil.cpp -o $@

bt.o: $(OPTPATH)/src/Mubase/bt.cpp
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Mubase/bt.cpp -o $@

sputil.o: $(OPTPATH)/src/Mubase/sputil.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Mubase/sputil.c -o $@

sp.o: $(OPTPATH)/src/Mubase/sp.cpp
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Mubase/sp.cpp -o $@

rm.o: $(OPTPATH)/src/Mubase/rm.cpp
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Mubase/rm.cpp -o $@

sort.o: $(OPTPATH)/src/Mubase/sort.cpp
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Mubase/sort.cpp -o $@

iter.o: $(OPTPATH)/src/Mubase/iter.cpp
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Mubase/iter.cpp -o $@



Catalog.o: $(OPTPATH)/src/Catalog/Catalog.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Catalog/Catalog.c -o $@
	
Config.o: $(OPTPATH)/src/Config/Config.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Config/Config.c -o $@
	
Cost.o: $(OPTPATH)/src/Cost/Cost.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Cost/Cost.c -o $@

CostList.o: $(OPTPATH)/src/Cost/CostList.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Cost/CostList.c -o $@

Equivalence.o: $(OPTPATH)/src/Equivalence/Equivalence.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Equivalence/Equivalence.c -o $@

Expr.o: $(OPTPATH)/src/Expr/Expr.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Expr/Expr.c -o $@

ExprInterface.o: $(OPTPATH)/src/Interface/ExprInterface.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Interface/ExprInterface.c -o $@

ExhSearch.o: $(OPTPATH)/src/ExhSearch/ExhSearch.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/ExhSearch/ExhSearch.c -o $@

Greedy.o: $(OPTPATH)/src/Greedy/Greedy.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Greedy/Greedy.c -o $@

Interface.o: $(OPTPATH)/src/Interface/Interface.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Interface/Interface.c -o $@

LogProp.o: $(OPTPATH)/src/LogProp/LogProp.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/LogProp/LogProp.c -o $@

LogicalOp.o: $(OPTPATH)/src/LogicalOp/LogicalOp.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/LogicalOp/LogicalOp.c -o $@

Memo.o: $(OPTPATH)/src/Memo/Memo.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Memo/Memo.c -o $@

Node.o: $(OPTPATH)/src/Node/Node.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Node/Node.c -o $@

PhyProp.o: $(OPTPATH)/src/PhyProp/PhyProp.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/PhyProp/PhyProp.c -o $@

PhyPropGroup.o: $(OPTPATH)/src/PhyPropGroup/PhyPropGroup.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/PhyPropGroup/PhyPropGroup.c -o $@

PhysicalOp.o: $(OPTPATH)/src/PhysicalOp/PhysicalOp.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/PhysicalOp/PhysicalOp.c -o $@

Plan.o: $(OPTPATH)/src/Plan/Plan.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Plan/Plan.c -o $@

TransRule.o: $(OPTPATH)/src/TransRule/TransRule.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/TransRule/TransRule.c -o $@

Volcano.o: $(OPTPATH)/src/Volcano/Volcano.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Volcano/Volcano.c -o $@

Eval.o: $(OPTPATH)/src/Eval/Eval.c
	$(CPlus) -c $(CPlusFlags) $(OPTPATH)/src/Eval/Eval.c -o $@

#ddl_stmts.o: $(DALIPARSERPATH)/ddl_stmts.c
#	$(CPlus) -c $(CPlusFlags) $(PARINCS) $(DALIPARSERPATH)/ddl_stmts.c -o $@

#evaluate.o: $(DALIPARSERPATH)/evaluate.c
#	$(CPlus) -c $(CPlusFlags) $(PARINCS) $(DALIPARSERPATH)/evaluate.c -o $@

expr.o: $(DALIPARSERPATH)/expr.c
	$(CPlus) -c $(CPlusFlags) $(PARINCS) $(DALIPARSERPATH)/expr.c -o $@

#optimize.o: $(DALIPARSERPATH)/optimize.c
#	$(CPlus) -c $(CPlusFlags) $(PARINCS) $(DALIPARSERPATH)/optimize.c -o $@

prim_types.o: $(DALIPARSERPATH)/prim_types.c
	$(CPlus) -c $(CPlusFlags) $(PARINCS) $(DALIPARSERPATH)/prim_types.c -o $@

rel_alg.o: $(DALIPARSERPATH)/rel_alg.c
	$(CPlus) -c $(CPlusFlags) $(PARINCS) $(DALIPARSERPATH)/rel_alg.c -o $@

scanner.c:  $(DALIPARSERPATH)/scanner.l parser.h
	flex $(DALIPARSERPATH)/scanner.l
	mv lex.yy.c scanner.c

parser.h parser.c:  $(DALIPARSERPATH)/parser.y
#	bison_1.2 -d $(DALIPARSERPATH)/parser.y
	bison -d $(DALIPARSERPATH)/parser.y
	mv $(DALIPARSERPATH)/parser.tab.c parser.c
	mv $(DALIPARSERPATH)/parser.tab.h parser.h

clean:   
	rm -f *.o "mydb.txt" ../lib/*.a

backup: clean
	(cd ~; tar cvf - Optimizer | gzip -9 > Archive/Optimizer.tar.gz)

depend:
	makedepend -- $(CFLAGS) -I$(OTHERINCLUDES) -- $(SRCS)


