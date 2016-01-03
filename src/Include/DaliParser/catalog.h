/* %W%  %G%     %R% */

#ifndef _CATALOG_H
#define _CATALOG_H

class CatalogDBPtr{
public:
    unsigned int mptr;
    // Constructors
    
    inline CatalogDBPtr(void) {mptr =0;};
    inline CatalogDBPtr(const CatalogDBPtr &ptr) {mptr = ptr.mptr;};
    inline CatalogDBPtr(const DaliPtr &ptr) { mptr = ptr.mptr;};
    inline CatalogDBPtr(int _mptr) {mptr = _mptr;};
    inline CatalogDBPtr(void *_mptr);
    inline CatalogDBPtr(tuplePtr _tptr){mptr = _tptr.mptr;};
    inline CatalogDBPtr(DbOffset _ptr){mptr = _ptr.mptr;};
    
    // Operators
    inline CatalogDBPtr& operator =(const CatalogDBPtr& pr2){
        mptr = pr2.mptr;
        return *this;
    };
    
    friend inline int operator == (const CatalogDBPtr& pr1, const CatalogDBPtr& pr2){
        return (pr1.mptr == pr2.mptr);
    };
    
    inline int operator == (const int i) const {
        return (mptr == i);
    };
    
    friend inline int operator !=(const CatalogDBPtr& pr1, const CatalogDBPtr& pr2) {
        return(pr1.mptr != pr2.mptr);
    };
    inline int operator !() const { return (mptr == 0); }
    
    friend inline CatalogDBPtr operator + (const CatalogDBPtr& pr1, int i);
    friend inline CatalogDBPtr operator + (int i, const CatalogDBPtr& pr1);
    
    inline operator int() { return mptr;} 
    inline operator DaliPtr() { return dali_ptr();}
    inline DaliPtr dali_ptr();
    inline operator void *() { return mem_ptr();}
    inline unsigned int offset() {return mptr;};
    inline void *mem_ptr();

    friend inline ostream & operator << (ostream &, const CatalogDBPtr&);
};

inline ostream& operator << (ostream& s, const CatalogDBPtr& cDBPtr)
{
    return s << cDBPtr.mptr; 
}
inline CatalogDBPtr operator + (const CatalogDBPtr& pr1, int i) {
    return( CatalogDBPtr(pr1.mptr+i));
}

inline CatalogDBPtr operator + (int i, const CatalogDBPtr& pr1) {
    return( CatalogDBPtr(pr1.mptr+i));
}

extern const CatalogDBPtr CatalogDBPtr_NULL;


/****************** CatalogDBPtr Functions ***********************/


template <class Type> class CatalogDBPtr_of : public CatalogDBPtr {
public:
    inline CatalogDBPtr_of() { };
    inline CatalogDBPtr_of(CatalogDBPtr t) {mptr =t.mptr; }; 
    inline CatalogDBPtr_of(DaliPtr ptr) {mptr = ptr.mptr; };
    inline CatalogDBPtr_of(tuplePtr ptr) {mptr = ptr.mptr; };
    inline CatalogDBPtr_of(DbOffset ptr) {mptr = ptr.mptr; };
    inline CatalogDBPtr_of(int t) : CatalogDBPtr(t) { };
    inline CatalogDBPtr_of(Type *v) : CatalogDBPtr((void*) v) { };       
    inline CatalogDBPtr_of(const DaliDB *, Type *v) 
        : CatalogDBPtr((void *)v) { };
    
    inline Type *mem_ptr() const {
        return (Type*) ((CatalogDBPtr*)this)->mem_ptr();                
    };
    inline CatalogDBPtr_of<Type> operator+ (int i) const;
    inline Type& operator* () const { return *mem_ptr();}                   
    inline Type* operator-> () const { return mem_ptr();}                   
    inline operator Type* () const { return mem_ptr();}                    
    inline friend CatalogDBPtr_of<Type> 
                 operator+ (int i, const CatalogDBPtr_of<Type>& pr1) 
                 { return pr1 + i; }                                        
    inline int operator! () const {
        return ! *((CatalogDBPtr *) this);
    }
};

template <class Type>
inline CatalogDBPtr_of<Type> CatalogDBPtr_of<Type>::operator+ (int i) const {
    CatalogDBPtr_of<Type> tmp = *this;
    tmp.mptr +=  (i * sizeof(Type));
    return tmp;
}

#endif
