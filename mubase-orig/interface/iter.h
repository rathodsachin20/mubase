// Interface to be implemented by all operators and base-rel scan
class Record;

class RIterator {
    public:
    virtual void open() = 0;
    virtual Record next() = 0;
    virtual void close() = 0;
};

