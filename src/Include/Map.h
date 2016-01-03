// A key-value map currently implemented using a list. 
// TODO: make this implementation over the hash based lookup table.

#ifndef __MAP_H__
#define __MAP_H__

#include <assert.h>
#include "List.h"
#include "PhyProp.h"
#include "typedefs.h"

template<class MapKey_t, class MapValue_t>
class MapEntry_t {
    MapKey_t   *key;
    MapValue_t *value;
   
    public: 
    MapEntry_t(MapKey_t *a_key, MapValue_t *a_value) 
              : key(a_key), value(a_value)
    { 
        assert(key);
        assert(value);
    }
    
    MapKey_t *Key()
    { return key; }
   
 
    MapValue_t *Value()
    { return value; }

    void SetValue(MapValue_t *a_value)
    { value = a_value; }
};

/*
template<class MapKey_t, class MapValue_t>
inline int operator ==(MapEntry_t<MapKey_t, MapValue_t>& x, 
                       MapEntry_t<MapKey_t, MapValue_t>& y)
{
    assert(x.key);
    assert(y.key);

    // expects the keys to have implemented == properly
    return x.key==y.key;
}
*/

template<class MapKey_t, class MapValue_t>
class Map_t : public RefC_t {
    AppendList_t <MapEntry_t<MapKey_t, MapValue_t>*> list;

public:
    // Returns the corresponding value or NULL if key not found
    MapValue_t *Get(MapKey_t *key);
    // Puts the value against the key replacing old if any.
    // Old value when present or NULL is returned
    MapValue_t *Put(MapKey_t *key, MapValue_t *value);
};

template<class MapKey_t, class MapValue_t>
MapValue_t* Map_t<MapKey_t, MapValue_t>::Get(MapKey_t *key)
{
    MapEntry_t<MapKey_t, MapValue_t> *entry;

    ListIter_t<MapEntry_t<MapKey_t, MapValue_t> *> entryIter;
    entryIter.Attach(&list);

    while(!entryIter.IsEnd())
    {
        entry = entryIter.Next();
        assert(entry);
        if (entry->Key() == key)
            return entry->Value();
    }
    return NULL;
}

template<class MapKey_t, class MapValue_t>
MapValue_t* Map_t<MapKey_t, MapValue_t>::Put(MapKey_t *key, 
                                             MapValue_t *value)
{
    assert(key);

    MapEntry_t<MapKey_t, MapValue_t> *entry;
    MapValue_t *ret = NULL;

    ListIter_t<MapEntry_t<MapKey_t, MapValue_t> *> entryIter;
    entryIter.Attach(&list);

    int found = 0;
    while(!entryIter.IsEnd())
    {
        entry = entryIter.Next();
        assert(entry);
        if (entry->Key() == key) {
            found = 1;
            ret = entry->Value();
            entry->SetValue(value);
            break;
        }
    }
    if (!found) {
        entry = new MapEntry_t<MapKey_t, MapValue_t>(key, value);
        list.Insert(entry);
    }
    return ret;
}



/************************* Classes for PGTable *********************/
 
// key for the lookup table
class PlanGroupTableKey_t {
    Order_t     *pso;
    Card_t       callCount;

    public :
    PlanGroupTableKey_t(Order_t *a_pso, Card_t a_callCount=1) 
                        : pso(a_pso), callCount(a_callCount)
    { assert(pso); }

    // Currently unused. Will be needed for a hashtable based impl.
    int HashValue(void)
    { 
        int h = (int) callCount;
        // int h = 0;
       
        const int *order = pso->Order(); 
        for (int i = 0; i < pso->OrderSize(); i++) {
            h += order[i];
        }

        // return h%PLANGROUPTABLESIZE; 
        return h;
    }

    inline int operator ==(PlanGroupTableKey_t& x) 
    {
        assert(x.pso);
        // return x.pso->IsEquivalent(pso);

        if (x.callCount != callCount)
            return 0;

        if(pso->OrderSize() != (x.pso)->OrderSize())
            return 0;

        const int *xOrder  = (x.pso)->Order();
        const int *myOrder = pso->Order();

        for (int i = 0; i < pso->OrderSize(); i++) {
            if (myOrder[i] != xOrder[i])
                return 0;
        }
        return 1;
    }

    ~PlanGroupTableKey_t(void) { }
};

/*
inline int operator ==(PlanGroupTableKey_t& x, PlanGroupTableKey_t& y)
{
    assert(x.pso);
    assert(y.pso);

    // Currently not matching the callCount
    return x.pso->IsEquivalent(y.pso);
}
*/

typedef Map_t<PlanGroupTableKey_t, PlanGroup_t> PlanGroupTable_t;

#endif // __MAP_H__

