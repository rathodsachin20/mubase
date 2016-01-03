#include "iter.h"

class Sort : public RIterator {
    public:
    virtual void open();
    virtual Record next();
    virtual void close();
};
