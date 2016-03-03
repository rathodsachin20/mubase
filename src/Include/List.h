// list data structure

#ifndef __LIST_H__
#define __LIST_H__

#include <assert.h>
#include <stdlib.h>

#include <iostream>
#include "Config.h"

using namespace std;

// node in the linked list
template <class T>
class ListEntry_t {
public:
    T data;
    ListEntry_t<T> *next;

    ListEntry_t(T& newdata) : data(newdata), next(NULL) { }
    ~ListEntry_t(void) { }
};

// iterator for the list
template <class T>
class ListIter_t;

// prepend only ordered list
template <class T>
class List_t {
protected:
    ListEntry_t<T> *first;              // last element in the list
    int size;                           // size of the list

    // instantiate a new list entry
    virtual ListEntry_t<T> *NewListEntry(T& newdata)
    { return new ListEntry_t<T>(newdata); }

public:
    List_t(void) : first(NULL), size(0) { }

    // is the list empty?
    int IsEmpty(void) const
    { assert(size >= 0); return size == 0; }

    // insert element into the list
    virtual void Insert(T& newdata) = 0;

    // delete an element at the start of the list
    // does not care if an iterator is sitting on the deleted element
    virtual T DeleteTop(void) = 0;

    // make the list empty
    virtual void MakeEmpty(void)
    {
        ListEntry_t<T> *p = NULL;

        while( first != NULL ) {
            p = first->next;
            delete first;
            first = p;
            size--;
        }
        assert(size == 0);
    }

    int Size(void) const
    { return size; }

    // delete the elements in the list
    virtual ~List_t(void)
    { MakeEmpty(); }

    friend class ListIter_t<T>;         // associated iterator class
};


// append only ordered list
template<class T>
class AppendList_t : public List_t<T> {
protected:
    using List_t<T>::first;
    using List_t<T>::size;
    ListEntry_t<T> *last;               // last element in the list

public:
    AppendList_t(void) : last(NULL) { }

    // insert an element
    void Insert(T& newdata)
    {
        ListEntry_t<T> *p = this->NewListEntry(newdata);

        p->next = NULL;
        if( last )
            last->next = p;
        else first = p;
        last = p;
        size++;
    }

    T DeleteTop(void)
    {
        assert(size > 0);

        ListEntry_t<T> *p = first;
        first = first->next;
        if( !first )
            last = NULL;

        T data = p->data;

        delete p;
        size--;
        assert(size >= 0);

        return data;
    }

    void MakeEmpty(void)
    {
        List_t<T>::MakeEmpty();
        last = NULL;
    }

    virtual ~AppendList_t(void) { }
};

// prepend only ordered list
template<class T>
class PrependList_t : public List_t<T> {
protected:
    using List_t<T>::first;
    using List_t<T>::size;
public:
    PrependList_t(void) { }

    // insert element at the start of the list
    void Insert(T& newdata)
    {
        ListEntry_t<T> *p = this->NewListEntry(newdata);

        // prepend
        p->next = first;
        first = p;
        size++;
    }

    T DeleteTop(void)
    {
        assert(size > 0);
        assert(first);

        ListEntry_t<T> *p = first;
        first = first->next;

        T data = p->data;

        delete p;
        size--;
        assert(size >= 0);

        return data;
    }

    virtual ~PrependList_t(void) { }
};

// append-only list supporting deletion
template<class T>
class AppendDelList_t : public AppendList_t<T> {
protected:
    using List_t<T>::first;
    using AppendList_t<T>::last;
    using List_t<T>::size;
    int ID(const T& x) const;
    int IsEqual(const T& x, const T& y) const;

public:
    // delete an arbitrary element of the list --- assumes an
    // IsEqual(T, T) function exists
    void Delete(T& data)
    {
        if( Config_t::IsDebug(DEBUG_INFO) ) {
            ListEntry_t<T> *cur = first;
            cout << "Before Deletion" << endl;
            cur = first;
            while( cur != NULL ) {
                cout << "["<< ID(cur->data) <<"]";
                cur = cur->next;
            }
            cout << endl;
            flush(cout);
        }

        ListEntry_t<T> *prev = NULL;
        ListEntry_t<T> *cur = first;

        while( cur != NULL ) {
            if( IsEqual(data, cur->data) )
                break;

            prev = cur;
            cur = cur->next;
        }

        assert(cur);

        ListEntry_t<T> *next = cur->next;
        if( prev )
            prev->next = next;
        else first = next;
        if( !next )
            last = prev;

        delete cur;
        size--;
        assert(size >= 0);

        if( Config_t::IsDebug(DEBUG_INFO) ) {
            cout << "After Deletion" << endl;
            ListEntry_t<T> *cur = first;
            while( cur != NULL ) {
                cout << "["<< ID(cur->data) <<"]";
                cur = cur->next;
            }
            cout << endl;
            flush(cout);
        }
    }
};

// prepend-only list supporting deletion
template<class T>
class PrependDelList_t : public PrependList_t<T> {
protected:
    using List_t<T>::first;
    using List_t<T>::size;
    int ID(const T& x) const;
    int IsEqual(const T& x, const T& y) const;

public:
    // delete an arbitrary element of the list --- assumes an
    // IsEqual(T, T) function exists
    void Delete(T& data)
    {
        ListEntry_t<T> *prev = NULL;
        ListEntry_t<T> *cur = first;

        while( cur != NULL ) {
            if( IsEqual(data, cur->data) )
                break;

            prev = cur;
            cur = cur->next;
        }

        assert(cur);

        ListEntry_t<T> *next = cur->next;
        if( prev )
            prev->next = next;
        else first = next;

        delete cur;
        size--;
        assert(size >= 0);
    }
};

// iterator for the linked list
template <class T>
class ListIter_t {
    const List_t<T> *list;
    ListEntry_t<T> *cur;

public:
    ListIter_t(void) : list(NULL), cur(NULL)
    { }

    // attach the iterator to a list
    void Attach(const List_t<T> *list_a)
    {
        list = list_a;
        Reset();
    }

    // check for end of list
    int IsEnd(void) const
    {
        assert(list);
//      cout << "\n\n\n\n DEBUG: In IsEnd list address : " << list;
        if( !list->first )
            return 1;

        if( cur ) {
            if( !cur->next )
                return 1;
        }

        return 0;
    }

    // next element
    const T& Next(void)
    {
        assert(list);
        if( !cur )
            cur = list->first;
        else cur = cur->next;

        assert(cur);
        const T& data = cur->data;

        return data;
    }

    // reset to the beginning
    void Reset(void) { cur = NULL; }

    ~ListIter_t(void)
    { }
};

#endif    // __LIST_H__
