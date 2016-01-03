#include "ucbproto.h"
//#include "dali.h" ARU
//#include "dali_rel_mgr.h" ARU

#include "sql_defs.h"
#include "sql_error.h"
#include "prim_types.h"

/*************************/

const char *
type_to_string(PrimType type) 
{
    switch(type) 
        {
          case D_UnknownType:  return "type_unknown";
          case D_Int:          return "INT"; 
          case D_Float:        return "FLOAT"; 
          case D_Double:       return "DOUBLE";
          case D_String:       return "CHAR";
          case D_Varchar:      return "VARCHAR";
          case D_Truthval:     return "BOOLEAN";
          case D_Null:         return "NULL"; 
          case D_Star:         return "*"; 
          default:             return "ERROR:bad_type";
    }
}

const char *
argSourceToString(ArgSource source_a)
{
        switch(source_a)
        {
          case PARAMETER: return "PARAMETER";
          case RESULT:    return "RESULT";
          case LEFT:      return "LEFT";
          case RIGHT:     return "RIGHT";
        }
        return "Illegal source type";
}

/* ******** */

ArgType::ArgType(PrimType t) {
    type = t;
    switch(t)
    {
      case D_UnknownType:  len = 0; break;
      case D_Int:          len = sizeof_D_Int; break;
      case D_Float:        len = sizeof_D_Float; break;
      case D_Double:       len = sizeof_D_Double; break;
      case D_Truthval:     len = sizeof_D_Truthval; break;
      case D_Null:         len = 0;  break;
      case D_Star:         len = 0; break;
          default:
                fprintf(stderr,"Illegal use of ArgType::ArgType(PrimType)\n");
                DALI_R_ASSERT(0);   
    }
}

bool
ArgType::IsAsgCompatible(ArgType &lhs, ArgType &rhs)
{
        switch(lhs.type)
        {
          case D_Truthval: return (rhs.type == D_Truthval);
          case D_String:   return (rhs.type  == D_String);
                // FIX: Is that behaviour conforming to the standard?
          case D_Int: case D_Double: case D_Float:
                return ((rhs.type == D_Int) || (rhs.type == D_Float) ||
                  (rhs.type == D_Double));
          default: return false;
        }
}

void
ArgType::PtrToPtr(void *lhs, ArgType &lhsType, void *rhs, ArgType &rhsType)
{
        switch(lhsType.type)
        {
          case D_Int:
                switch(rhsType.type)
                {
                  case D_Int: case D_Null:
                        *((int *)lhs) = *((int *)rhs);
                        return;
                  case D_Float: 
                        *((int *)lhs) = int(*((float *)rhs));
                        return;
                  case D_Double: 
                        *((int *)lhs) = int(*((double *)rhs));
                        return;
                  default:
                        SQLErrE("ArgType::PtrToPtr: type mis-match", -1)
                }
          case D_Float:
                switch(rhsType.type)
                {
                  case D_Int: 
                        *((float *)lhs) = float(*((int *)rhs));
                        return;
                  case D_Float: case D_Null:
                        *((float *)lhs) = *((float *)rhs);
                        return;
                  case D_Double: 
                        *((float *)lhs) = float(*((double *)rhs));
                        return;
                  default:
                        SQLErrE("ArgType::PtrToPtr: type mis-match", -1)
                }
          case D_Double:
                switch(rhsType.type)
                {
                  case D_Int: 
                        *((double *)lhs) = double(*((int *)rhs));
                        return;
                  case D_Float: 
                        *((double *)lhs) = double(*((float *)rhs));
                        return;
                  case D_Double: case D_Null:
                        *((double *)lhs) = *((double *)rhs);
                        return;
                  default:
                        SQLErrE("ArgType::PtrToPtr: type mis-match", -1)
                }
          case D_String:
                switch(rhsType.type)
                {
                  case D_String: case D_Null:
                        strncpy((char *)lhs, (char *)rhs,
                          lhsType.length() - 1);
                        ((char *)lhs)[lhsType.length() - 1] = '\0';
                        return;
                  default:
                        SQLErrE("ArgType::PtrToPtr: type mis-match", -1)
                }
          case D_Truthval:
                switch(rhsType.type)
                {
                  case D_Truthval: case D_Null:
                        *((bool *)lhs) = *((bool *)rhs);
                    return;
                  default:
                        SQLErrE("ArgType::PtrToPtr: type mis-match", -1)
                }
          default:
                SQLErrE("ArgType::PtrToPtr: argument of illegal type", -1)
        }
}

bool 
ArgType::isEq(void *lhs, ArgType &lhsType, void *rhs, ArgType &rhsType)
{
        if((lhsType.type == D_String) && (rhsType.type == D_String))
                /* It would have been better to call IsStringType() instead
                   of this hard-coded check, but we want run-time efficiency. */
                return (strcmp((char *)lhs, (char *)rhs) == 0);

        // Now both have to be of numeric types
        switch(lhsType.type)
        {
          case D_Int:
                switch(rhsType.type)
                {
                  case D_Int:           return (*((int *)lhs) == *((int *)rhs));
                  case D_Float:         return (*((int *)lhs) == *((float *)rhs));
                  case D_Double:        return (*((int *)lhs) == *((double *)rhs));
                  default:              break;

                }
                break;
          case D_Float:
                switch(rhsType.type)
                {
                  case D_Int:           return (*((float *)lhs) == *((int *)rhs));
                  case D_Float:         return (*((float *)lhs) == *((float *)rhs));
                  case D_Double:        return (*((float *)lhs) == *((double *)rhs));
                  default:              break;
                }
                break;
          case D_Double:
                switch(rhsType.type)
                {
                  case D_Int:           return (*((double *)lhs) == *((int *)rhs));
                  case D_Float:         return (*((double *)lhs) == *((float *)rhs));
                  case D_Double:        return (*((double *)lhs) == *((double *)rhs));
                  default:              break;
                }
          default:              break;
        }
        SQLErrE("ArgType::isEq: type mismatch of operands", -1)
        return false;
}

bool 
ArgType::isGT(void *lhs, ArgType &lhsType, void *rhs, ArgType &rhsType)
{
        if((lhsType.type == D_String) && (rhsType.type == D_String))
                /* It would have been better to call IsStringType() instead
                   of this hard-coded check, but we want run-time efficiency. */
                return (strcmp((char *)lhs, (char *)rhs) > 0);

        // Now both have to be of numeric types
        switch(lhsType.type)
        {
          case D_Int:
                switch(rhsType.type)
                {
                  case D_Int:           return (*((int *)lhs) > *((int *)rhs));
                  case D_Float:         return (*((int *)lhs) > *((float *)rhs));
                  case D_Double:        return (*((int *)lhs) > *((double *)rhs));
                  default:              break;
                }
                break;
          case D_Float:
                switch(rhsType.type)
                {
                  case D_Int:           return (*((float *)lhs) > *((int *)rhs));
                  case D_Float:         return (*((float *)lhs) > *((float *)rhs));
                  case D_Double:        return (*((float *)lhs) > *((double *)rhs));
                  default:              break;
                }
                break;
          case D_Double:
                switch(rhsType.type)
                {
                  case D_Int:           return (*((double *)lhs) > *((int *)rhs));
                  case D_Float:         return (*((double *)lhs) > *((float *)rhs));
                  case D_Double:        return (*((double *)lhs) > *((double *)rhs));
                  default:              break;
                }
          default:              break;
        }
        SQLErrE("ArgType::isGT: type mismatch of operands", -1)
        return false;
}

bool 
ArgType::isLT(void *lhs, ArgType &lhsType, void *rhs, ArgType &rhsType)
{
        if((lhsType.type == D_String) && (rhsType.type == D_String))
                /* It would have been better to call IsStringType() instead
                   of this hard-coded check, but we want run-time efficiency. */
                return (strcmp((char *)lhs, (char *)rhs) < 0);

        // Now both have to be of numeric types
        switch(lhsType.type)
        {
          case D_Int:
                switch(rhsType.type)
                {
                  case D_Int:           return (*((int *)lhs) < *((int *)rhs));
                  case D_Float:         return (*((int *)lhs) < *((float *)rhs));
                  case D_Double:        return (*((int *)lhs) < *((double *)rhs));
                  default:              break;
                }
                break;
          case D_Float:
                switch(rhsType.type)
                {
                  case D_Int:           return (*((float *)lhs) < *((int *)rhs));
                  case D_Float:         return (*((float *)lhs) < *((float *)rhs));
                  case D_Double:        return (*((float *)lhs) < *((double *)rhs));
                  default:              break;
                }
                break;
          case D_Double:
                switch(rhsType.type)
                {
                  case D_Int:           return (*((double *)lhs) < *((int *)rhs));
                  case D_Float:         return (*((double *)lhs) < *((float *)rhs));
                  case D_Double:        return (*((double *)lhs) < *((double *)rhs));
                  default:              break;
                }
          default:              break;
        }
        SQLErrE("ArgType::isLT: type mismatch of operands", -1)
        return false;
}

void 
ArgType::add(void *result, ArgType &resultType, void *lhs,
  ArgType &lhsType, void *rhs, ArgType &rhsType)
{
        // WARNING: The following code performs no run-time type-checking.
        switch(resultType.type)
        {
          case D_Int:
                *((int *)result) = *((int *)lhs) + *((int *)rhs); return;
          case D_Float:
                switch(lhsType.type)
                {
                  case D_Float:
                        switch(rhsType.type)
                        {
                          case D_Int:
                                *((float *)result) = *((float *)lhs) + *((int *)rhs); return;
                          case D_Float:
                                *((float *)result) = *((float *)lhs) + *((float *)rhs); return;
                          default:              break;
                        }
                        break;
                  case D_Int:
                        *((float *)result) = *((int *)lhs) + *((float *)rhs); return;
                  default:              break;
                }
                break;
          case D_Double:
                switch(lhsType.type)
                {
                  case D_Double:
                        switch(rhsType.type)
                        {
                          case D_Int:
                                *((double *)result) = *((double *)lhs) + *((int *)rhs);
                                return;
                          case D_Float:
                                *((double *)result) = *((double *)lhs) + *((float *)rhs);
                                return;
                          case D_Double:
                                *((double *)result) = *((double *)lhs) + *((double *)rhs);
                                return;
                          default:              break;
                        }
                        break;
                  case D_Float:
                        *((double *)result) = *((float *)lhs) + *((double *)rhs); return;
                  case D_Int:
                        *((double *)result) = *((int *)lhs) + *((double *)rhs); return;
                  default:              break;
                }
          default:              break;
        }
        SQLErrE("ArgType::add: type mismatch of operands", -1)
}

void 
ArgType::subtract(void *result, ArgType &resultType, void *lhs,
  ArgType &lhsType, void *rhs, ArgType &rhsType)
{
        // WARNING: The following code performs no run-time type-checking.
        switch(resultType.type)
        {
          case D_Int:
                *((int *)result) = *((int *)lhs) - *((int *)rhs); return;
          case D_Float:
                switch(lhsType.type)
                {
                  case D_Float:
                        switch(rhsType.type)
                        {
                          case D_Int:
                                *((float *)result) = *((float *)lhs) - *((int *)rhs); return;
                          case D_Float:
                                *((float *)result) = *((float *)lhs) - *((float *)rhs); return;
                          default:              break;
                        }
                        break;
                  case D_Int:
                        *((float *)result) = *((int *)lhs) - *((float *)rhs); return;
                  default:              break;
                }
                break;
          case D_Double:
                switch(lhsType.type)
                {
                  case D_Double:
                        switch(rhsType.type)
                        {
                          case D_Int:
                                *((double *)result) = *((double *)lhs) - *((int *)rhs);
                                return;
                          case D_Float:
                                *((double *)result) = *((double *)lhs) - *((float *)rhs);
                                return;
                          case D_Double:
                                *((double *)result) = *((double *)lhs) - *((double *)rhs);
                                return;
                          default:              break;
                        }
                        break;
                  case D_Float:
                        *((double *)result) = *((float *)lhs) - *((double *)rhs); return;
                  case D_Int:
                        *((double *)result) = *((int *)lhs) - *((double *)rhs); return;
                  default:              break;
                }
          default:              break;
        }
        SQLErrE("ArgType::add: type mismatch of operands", -1)
}

void 
ArgType::multiply(void *result, ArgType &resultType, void *lhs,
  ArgType &lhsType, void *rhs, ArgType &rhsType)
{
        // WARNING: The following code performs no run-time type-checking.
        switch(resultType.type)
        {
          case D_Int:
                *((int *)result) = *((int *)lhs) * *((int *)rhs); return;
          case D_Float:
                switch(lhsType.type)
                {
                  case D_Float:
                        switch(rhsType.type)
                        {
                          case D_Int:
                                *((float *)result) = *((float *)lhs) * *((int *)rhs); return;
                          case D_Float:
                                *((float *)result) = *((float *)lhs) * *((float *)rhs); return;
                          default:              break;
                        }
                        break;
                  case D_Int:
                        *((float *)result) = *((int *)lhs) * *((float *)rhs); return;
                  default:              break;
                }
                break;
          case D_Double:
                switch(lhsType.type)
                {
                  case D_Double:
                        switch(rhsType.type)
                        {
                          case D_Int:
                                *((double *)result) = *((double *)lhs) * *((int *)rhs);
                                return;
                          case D_Float:
                                *((double *)result) = *((double *)lhs) * *((float *)rhs);
                                return;
                          case D_Double:
                                *((double *)result) = *((double *)lhs) * *((double *)rhs);
                                return;
                          default:              break;
                        }
                        break;
                  case D_Float:
                        *((double *)result) = *((float *)lhs) * *((double *)rhs); return;
                  case D_Int:
                        *((double *)result) = *((int *)lhs) * *((double *)rhs); return;
                  default:              break;
                }
          default:              break;
        }
        SQLErrE("ArgType::add: type mismatch of operands", -1)
}

void 
ArgType::divide(void *result, ArgType &resultType, void *lhs,
  ArgType &lhsType, void *rhs, ArgType &rhsType)
{
        // WARNING: The following code performs no run-time type-checking.
        switch(resultType.type)
        {
          case D_Int:
                *((int *)result) = *((int *)lhs) / *((int *)rhs); return;
          case D_Float:
                switch(lhsType.type)
                {
                  case D_Float:
                        switch(rhsType.type)
                        {
                          case D_Int:
                                *((float *)result) = *((float *)lhs) / *((int *)rhs); return;
                          case D_Float:
                                *((float *)result) = *((float *)lhs) / *((float *)rhs); return;
                          default:              break;
                        }
                        break;
                  case D_Int:
                        *((float *)result) = *((int *)lhs) / *((float *)rhs); return;
                  default:              break;
                }
                break;
          case D_Double:
                switch(lhsType.type)
                {
                  case D_Double:
                        switch(rhsType.type)
                        {
                          case D_Int:
                                *((double *)result) = *((double *)lhs) / *((int *)rhs);
                                return;
                          case D_Float:
                                *((double *)result) = *((double *)lhs) / *((float *)rhs);
                                return;
                          case D_Double:
                                *((double *)result) = *((double *)lhs) / *((double *)rhs);
                                return;
                          default:              break;
                        }
                        break;
                  case D_Float:
                        *((double *)result) = *((float *)lhs) / *((double *)rhs); return;
                  case D_Int:
                        *((double *)result) = *((int *)lhs) / *((double *)rhs); return;
                  default:              break;
                }
          default:              break;
        }
        SQLErrE("ArgType::add: type mismatch of operands", -1)
}
