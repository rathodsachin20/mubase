// dali interface class

#ifndef __INTERFACE_H__
#define __INTERFACE_H__

class DaliTableSchema;          // dali table schema structure
class Optimizer_t;              // optimizer class
struct StringKey_t;
class AliasTable;

using namespace std;

//const int ALIAS_TABLE_SIZE = 10;

// dali interface class
class Interface_t
{
public:
    static AliasTable aliasTable; // Globally accissible alias table
    static SortOrder_t *reqOrder ; // OrderBy on the whole query
    // get the expression dag
    static Equivalence_t *ExpressionDAG(Optimizer_t& opt, char* inpFile=NULL);
};


/**
 * A table that maintains mapping of table aliases used in the query to the
 * table names.
 *
 * PERF_FIX: Ideally we should keep the name -> originalName mapping in 
 * an efficient lookup structure. For now storing in a list. May not make much
 * difference since typically not more than 10 aliases would be used in a
 * query.
 */
class AliasTable
{
    // For now using a list to keep the mapping from alias name to the table
    // name. Alternate elements store alias name and corresponding table name.
    AppendList_t<const char *> aliasTable;

public:
    AliasTable() {}

    // Returns the table name corresponding to the alias name.
    // Alias names are taken to be case sensitive as are table names.
    // Returns NULL if no matching alias found.
    char *getTableName(const char *aliasName)
    {
        assert(aliasName);

        ListIter_t<const char *> iter;
        iter.Attach(&aliasTable);
        char *tableName = NULL;
        
        int found = 0;
        while(!iter.IsEnd()) {
            const char *alias = iter.Next(); 
            assert(alias);
            assert(!iter.IsEnd());   // List always contains even no. of elts
            const char *tableName = iter.Next(); 
            assert(tableName);
            if (strcmp(alias, aliasName)) {
                found = 1;
                break;
            }
        }
        
        if (found)
            return tableName;

        return NULL;
    }

    void  addMapping(const char *aliasName, const char *tableName)
    {
        assert(aliasName);
        assert(tableName);

        aliasTable.Insert(aliasName);
        aliasTable.Insert(tableName);
    }

    AppendList_t<const char *>& getAliasList()
    {
        return aliasTable;
    }

    void PrintTable()
    {
        cout << "Alias Table" << endl;
        ListIter_t<const char *> iter;
        iter.Attach(&aliasTable);

        while(!iter.IsEnd()) {
            const char *alias = iter.Next(); 
            assert(alias);
            assert(!iter.IsEnd());   // List always contains even no. of elts
            const char *tableName = iter.Next(); 
            assert(tableName);

            cout << "[Alias: " << alias << ", Table: " << tableName 
                               << "]" << endl;
        }
    }
        

    ~AliasTable() {}
};

/*
Lookup_t<ALIAS_TABLE_SIZE, StringKey_t> aliasTable; 

// A lookey for strings 
struct StringKey_t {
    char* str;

    StringKey_t(char *x = NULL) : str(x)
    { assert(str); }

    int HashValue(void)
    { 
        int val = 0;
        for (int i = 0; str[i] !='\0'; i++)
            val += str[i];
        
        return val%ALIAS_TABLE_SIZE;
    }

    ~StringKey_t(void) { }
};

inline int operator ==(StringKey_t& x, StringKey_t& y)
{
    assert(x.str);
    assert(y.str);

    return (strcmp(x.str, y.str) == 0);
}
*/

#endif  // __INTERFACE_H__
