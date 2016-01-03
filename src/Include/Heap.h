// heap implementation

#ifndef __HEAP_H__
#define __HEAP_H__

#include <assert.h>

// define a heap
template<class HeapKey_t>
class Heap_t {
    int maxHeapSize;
    int heapSize;
    HeapKey_t *a;
    
    void AdjustUp(int i)
    {
        if( i == 0 )
            return;

        while( i > 0 ) {
            assert(2*i <= heapSize);

            // min of the children
            int j = -1;
            if( 2*i+1 <= heapSize )
                j = (a[2*i] <= a[2*i+1]) ? 2*i : 2*i+1;
            else j = 2*i;
                    
            if( a[i] <= a[j] )
                break;

            // par is greater -- swap min child with par
            HeapKey_t e = a[i];
            a[i] = a[j];
            a[j] = e;

            // adjust upwards
            i /= 2;
        }
    }

    void AdjustDown(int i)
    {
        while( 2*i <= heapSize ) {
            int j;

            // min of the children
            if( 2*i+1 <= heapSize )
                j = ( a[2*i] <= a[2*i+1] ) ? 2*i : 2*i+1;
            else j = 2*i;

            if( a[i] <= a[j] )
                break;

            // par is greater -- swap min child with par
            HeapKey_t e = a[i];
            a[i] = a[j];
            a[j] = e;

            // adjust downwards
            i = j;
        }
    }

public:
    Heap_t(int maxHeapSize_a)
        : maxHeapSize(maxHeapSize_a), heapSize(0)
    {
        // extra 1 to account for the dummy 0th element
        a = new HeapKey_t[1+maxHeapSize];
    }

    void Insert(HeapKey_t& e)
    {
        heapSize++;
        assert(heapSize <= maxHeapSize);
        a[heapSize] = e;
        AdjustUp(heapSize/2);
    }

    void UpdateTop(HeapKey_t& newTop)
    {
        assert(heapSize >= 1);
        a[1] = newTop;
        AdjustDown(1);
    }

    HeapKey_t& Top(void) const
    {
        assert(heapSize > 0);
        return a[1];
    }

    void DeleteTop(void)
    {
        assert(heapSize > 0);
        if( heapSize > 1 ) {
            a[1] = a[heapSize];
            a[heapSize] = a[0];         // a[0] is a dummy element
            heapSize--;
            AdjustDown(1);
        } else heapSize = 0;
    }

    int HeapSize(void) const
    { return heapSize; }

    int IsEmpty(void) const
    { return heapSize == 0; }

    int IsFull(void) const
    { return heapSize == maxHeapSize; }

    void Empty(void)
    { heapSize = 0; }

    void Populate(Heap_t<HeapKey_t> *h)
    {
        assert(IsEmpty());
        assert(h);

        int hSize = h->HeapSize();

        heapSize = (maxHeapSize < hSize) ? maxHeapSize : hSize;

        // now populate the heap --- fill up $a$ in sorted order
        for( int i = 1 ; i <= heapSize ; i++ ) {
            a[i] = h->Top();
            h->DeleteTop();
        }
    }

    ~Heap_t(void)
    { delete[] a; }
};

#endif // __HEAP_H__
