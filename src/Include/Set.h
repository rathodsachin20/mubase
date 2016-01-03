// set data structure

#ifndef __SET_H__
#define __SET_H__

using namespace std;

template <class T>
class Set_t {
public:
    T *elements;
    int size;

    Set_t<T>(const T *elts, int count) 
    { 
        size = count;
        elements = new T[size];

        for (int i = 0; i < size; i++)
        {
            elements[i] = elts[i];
        }
    }

    Set_t<T>()
    {
        size = 0;
        elements = NULL;
    }

    const int *Elements() const
    {
        return elements;
    }

    int Size() const
    {
        return size;
    }

    // Adds an element if not already present. Returns true
    // iff the element is added.
    bool AddElement(T& newElem)
    {
        for (int i = 0; i < size; i++) {
            if (elements[i] == newElem)
                return false;
        }

        T *newElements = new T[size + 1];
        for (int i = 0; i < size; i++) {
            newElements[i] = elements[i];
        }
        newElements[size] = newElem;
        if (elements)
            delete[] elements;
        
        elements = newElements;
        size++;
        return true;
    }

    // returns ths - aSet
    Set_t<T> *Minus(const Set_t<T> *aSet) const
    {
        T *newElts = new T[size];
        int cnt = 0;
        for (int i = 0; i < size; i++)
        {
            bool found = false;
            for (int j = 0; j < aSet->Size(); j++) {
                if (elements[i] == aSet->elements[j]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                newElts[cnt++] = elements[i];
            }
        }

        return new Set_t<T>(newElts, cnt);
    }

    void Print()
    {
        cout << "{ ";
        for (int i = 0; i < size; i++)
        {
            cout << elements[i] << " ";
        }
        cout << "} \n";
    }

    ~Set_t(void) 
    { 
        if(elements) 
            delete[] elements; 
    }
};
#endif    // __SET_H__
