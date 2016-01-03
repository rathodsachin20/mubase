#include "config.h"

// typedef int bool;
// const bool true = 1, false = 0;

class Value;

enum ArgSource {
        PARAMETER = 1,
        RESULT    = 2,
        LEFT      = 3,
        RIGHT     = 4 
};

const char *argSourceToString(ArgSource source_a);
enum PrimType {
        D_UnknownType   = 0,
        D_Int       = 1,
        D_Float     = 2,
        D_Double    = 3,
          /* Warning: type checking is performed assuming that D_Double > 
             D_Float > D_Int. Hopefully only the member functions of 
                 $class ArgType$ make this assumption. */
        D_String    = 4,
        D_Varchar   = 5,
        D_Truthval  = 6,
        D_Null      = 7,
        D_Star      = 8,

        D_Tuple    = 19   
};

const char *type_to_string(PrimType type);
/** FIX:
   ArgType in Dali is currently more restricted (is an enum)
   This defn. should supercede the dali definitions eventually
**/

class ArgType {
  public:
        PrimType  type;
        int       len ; 
  private:
          /* NOTE: $len$ is being set for all $PrimType$s, not just strings etc. 
             MEANING: $len$ is the length occupied by an item of PrimType $type$
                          in the data base, in $SQLResult$ and in the child classes of
                                  $class Value$, except for $D_Star$,$D_Null$ and $D_Truthval$. 
             COMMENTS on this $len$ field:-
                        1. For D_String:  This SQL Code 
                           does not place any constraints on $len$s value, neither does
                           it provide a default value in the constructor. I.e. you
                           have to call $ArgType(PrimType, int)$.
                        2. For D_Truthval:  Because SQL Code
                           needs to convert between the representation in the db and
                           the $DaliTruthVal$ representation, it lays a constraint
                           on the db representation:-
                           i. The db representation shall occupy sizeof(bool) bytes.
                           I.e. $len$ will always be 1.
                           Hence the programmer should use the constructor 
                           $ArgType(PrimType)$, NOT $ArgType(PrimType, int)$.
                           Another constraint which is unrelated to $len$ is that
                           in the db and in $SQLResult$, the value $DaliTruthVal::DB_TRUE$ 
                           be used to represent $true$, and $DaliTruthVal::DB_FALSE$
                           be used to represent $false$.
                           Note that Dali itself does not provide a default length for
                           truth vals. So the SQL Code is not general enough. But see
                           the FIX message inside the definition of $class DaliTableSchema$
                           in "dali_rel_mgr.h".
                        3. For D_Int, D_Float, D_Double:  SQL Code places the constraint 
                           that $len$ be equal to the compiler defined size for the 
                           corresponding C++ basic type. Hence the programmer should NOT 
                           use $ArgType(PrimType, int)$.  
                        4. For D_Null, D_Star: $len$ is always zero. D_Star can never
                           occur in the db or in $SQLResult$. A D_Null in
                           the db is assumed to occupy as many bytes
                           as the type of that attribute. The SQL Code, right now, does
                           not recognize D_Nulls in the database or in $SQLResult$ objects.
                           Even Dali currently does not provide a way of remembering the
                           Null value for each attribute in a relation. */

                /* FIX: "external types" as in Dali not yet supported.
                   For an external type, Dali does not store the value in the
                   tuple. Instead it stores a $DbOffset$ to the value, in the
                   tuple. */

                /* In future, 
                        -1 could indicate ``varying length, allocated at 
                                            end of tuple''.
                        0  could indicate  ``default length of the prim. type''
                 */
        enum 
        {
                sizeof_D_Int = sizeof(int), sizeof_D_Double = sizeof(double),
                sizeof_D_Float = sizeof(float), 
                sizeof_D_Truthval = sizeof(bool)
                /* These sizes should be the same as the size of the $value$ member
                   in the corresponding child clases of $class Value$. */
        };

  public:
        ArgType () { type = D_UnknownType; len = 0; }
        ArgType (PrimType t); 
                /* Automatically sets the $len$ field  for Int, Float, Double 
                   and Truthval only. */
        ArgType (PrimType t, int l) 
                // Try not to use this constructor for Int, Float, Double and Truthval
        { 
                switch(t)
                {
                  case D_Int:     DALI_R_ASSERT(l == sizeof_D_Int); break;
                  case D_Double:  DALI_R_ASSERT(l == sizeof_D_Double); break;
                  case D_Float:   DALI_R_ASSERT(l == sizeof_D_Float); break;
                  case D_Truthval:DALI_R_ASSERT(l == sizeof_D_Truthval); break;
                  case D_Null:    DALI_R_ASSERT(l == 0); break;
                  case D_Star:    DALI_R_ASSERT(l == 0);
                  default:        break;
                }
                type = t;
                len = l; 
        }
        int operator == (PrimType t) { return type == t;}
        int print(FILE *file)
        {
                fprintf(file, " %s ", type_to_string(type));
                if(len > 0)
                        fprintf(file, "(%d)", len);
                return 1;
        }
        int length()
        {
                return len;
        }
        static bool IsAsgCompatible(ArgType &lhs_a, ArgType &rhs_a);
                /* This function checks whether a value of type $rhs_a$ can be 
                   assigned to a value of type $lhs_a$. 
                   Who uses this function? Guys like InsertTable::typeCheck
                   and UpdateTable::typeCheck. */
        void *DefaultValue()
                /* Returns the default value for $type$. This function should
                   be used only if the appropriate default value cannot be 
                   obtained from the catalogue. */
        {
                void *retVal = new char[len];
                for(int i = 0; i < len; ++i)
                        ((char *)retVal)[i] = 0;
                return retVal;
        }
        static void PtrToPtr(void *lhs, ArgType &lhsType, void *rhs, 
          ArgType &rhsType);
                /* $lhs$ should be a pointer to a data item of the type
                   represented by $lhsType$. $rhs$ is a pointer to a data item
                   of type $rhsType$. This function performs the requisite 
                   type-casting and assigns $*rhs$ to $*lhs$. 
                   Who uses this function? Guys like InsertTable::evaluate
                   and UpdateTable::evaluate. */
        static void *ValueToPtr(ArgType &lhsType, Value *value_a, 
          void *defaultValue);
        /* $defaultValue$ should be the default value appropriate for
           $lhsType$.
           Who uses this function? Guys like 
           RelValueIter::RelValueIter() */

        static bool isEq(void *lhs, ArgType &lhsType, void *rhs, 
          ArgType &rhsType);
        static bool isGT(void *lhs, ArgType &lhsType, void *rhs, 
          ArgType &rhsType);
        static bool isLT(void *lhs, ArgType &lhsType, void *rhs, 
          ArgType &rhsType);
        static void add(void *result, ArgType &resultType, void *lhs,
          ArgType &lhsType, void *rhs, ArgType &rhsType);
        static void subtract(void *result, ArgType &resultType, void *lhs,
          ArgType &lhsType, void *rhs, ArgType &rhsType);
        static void multiply(void *result, ArgType &resultType, void *lhs,
          ArgType &lhsType, void *rhs, ArgType &rhsType);
        static void divide(void *result, ArgType &resultType, void *lhs,
          ArgType &lhsType, void *rhs, ArgType &rhsType);

        static ArgType &MixedModeArithResult(ArgType &lhs, ArgType &rhs)
        {
                return (lhs.type > rhs.type) ? lhs : rhs;
        }
        bool IsNumericType()
        {
                return ((type == D_Int) || (type == D_Float) || (type == D_Double));
        }
        bool IsStringType()
        {
                return (type == D_String);
                // FIX: Later this function should return true for D_Varchar also
        }
};
