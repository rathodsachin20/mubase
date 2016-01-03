/* %W%  %G%     %R% */
/*********************************************************************/
/**        Copyright (c) 1993-94 AT&T.   All Rights Reserved.       **/
/**        Dali Storage Manager.                                    **/
/**        AT&T Bell Labs, Lab 1138.                                **/
/**                                                                 **/
/**        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T.     **/
/**        The copyright notice above does not evidence any         **/
/**        actual or intended publication of such source code.      **/
/**                                                                 **/
/**   THIS SOFTWARE  COMES WITH NO  WARRANTIES.  ANY LIABILITY OF   **/
/**   ANY KIND OF DAMAGES RESULTING FROM THE USE OF THIS SOFTWARE   **/
/**   IS DISCLAIMED.                                                **/
/*********************************************************************/

/**
*********************************************************************
*
*  Overview:
*    This file contains class and other header definitions related 
* to the implementation of attribute types in the Dali Relational
* Manager.  Implementations are in rel_mgr/reltypes.c.  
*
*********************************************************************
**/
/* The class heirarchy based on DaliRelType is used to provide type dependent 
 * functionality in a clean an object oriented manner.  New types can be added
 * by adding an overloaded class, a new value to the AttrType enum, and a
 * new entry in an array of pointers to types, set up by DaliAttrTypeCode::init()
*/
#ifndef RELTYPES_H
#define RELTYPES_H

enum DaliAttrType {
    DALI_UNKNOWN_TYPE = -1,
    DALI_INTEGER = 0,
    DALI_STRING = 1,
    DALI_FLOAT = 2,
    DALI_DOUBLE = 3,
    DALI_VARSTRING = 4,
    DALI_VARBINARY = 5,
    DALI_DATE = 6,
    DALI_TIME = 7,
    DALI_BINARY = 8
//    DALI_TIMESTAMP = 9
};

#define DREL_NUM_DEFINED_TYPES (10)

class DaliAttrTypeCode {

public:

    virtual int defaultLength() = 0;
    virtual int dataLength(const void * /* value */, int schema_length) {
        /* Should be overridden for external types */
        return schema_length; 
    }

    /* compareVal returns < 0 if first_arg is less than second_arg, 
     * 0 if they are equal, and > 0 if first_arg is greater than second_arg.
     */
    virtual int compareVal(const void *first_arg, const void *second_arg) = 0;


    virtual unsigned int computeHashVal(const void *data) = 0;
    virtual int convertFromString(void *value, const char *string) = 0;
    virtual int convertToString(const void *value, char *string,
                                int max_length, char *format = 0) = 0;
    virtual void *mallocAndCopy(const void* data, int schema_length)
    {
        int len = dataLength(data,schema_length);
        void *rval = malloc(len);
        dali_bcopy((char *)data,(char *)rval,len);
        return rval;
    }
    virtual int maxPrintLength(int schemaLength = 0) = 0;
    virtual DaliAttrType type() const;
    virtual char *typeString() const;
    virtual char *typeInCString() const;

    static DaliAttrTypeCode * typeObject(DaliAttrType type);
    static int compareVal(DaliAttrType type, void *v1, void *v2);
    static int defaultLength(DaliAttrType type);
    static int defaultInternalLength(DaliAttrType type);

    static int dataLength(DaliAttrType type, const void *value, int schema_length);
    static void *mallocAndCopy(DaliAttrType type,
                               const void* data, 
                               int schema_length);
    
    static int computeHashVal(DaliAttrType type, void *v1);
    static int convertFromString(DaliAttrType type, void *value, const char *string);
    static int convertToString(DaliAttrType type,const void *value, char *string,
                                int max_length, char *format = 0);
    static int maxPrintLength(DaliAttrType type, int schemaLength = 0);
    static char *typeString(DaliAttrType type);
    static char *typeInCString(DaliAttrType type);

    static int init();
protected:
    static DaliAttrTypeCode **TypeObjects;
};

inline int isDaliTypeExternal(DaliAttrType type)  {
    return (type == DALI_VARSTRING || type == DALI_VARBINARY);
}
#define DALI_EXTERNAL_POINTER_LENGTH  (sizeof(DbOffset))

class DaliTableHandle;
class DaliRelInternal;

class DaliVarString{
  DbOffset offset;
  
 public:
  DaliVarString(){offset = DbOffset_NULL;}
  int init(const TransID &tid, DaliTableHandle &handle, char *str);
  int init(const TransID &tid, DaliRelInternal &internal, char *str);
  int init(const TransID &tid, DaliDB *DB, char *str);

  int update(const TransID &tid, DaliTableHandle &handle, char *str);
  int update(const TransID &tid, DaliRelInternal &info, char *str);
  int update(const TransID &tid, DaliDB *db, char *str);

  int free(const TransID &tid, DaliDB *db);

  char *ptr(DaliTableHandle &handle);
  char *ptr(DaliDB *db);

};

struct DaliVarBinaryRep {
    int size;
    char data[1];
};


class DaliVarBinary{
  DbOffset offset;
 public:
  DaliVarBinary(){offset = DbOffset_NULL;}
  int init(const TransID &tid, DaliTableHandle &handle, void *str);
  int init(const TransID &tid, DaliTableHandle &handle, void *str, int sz);
  int init(const TransID &tid, DaliRelInternal &internal, void *str);
  int init(const TransID &tid, DaliRelInternal &internal, void *str, int sz);
  int init(const TransID &tid, DaliDB *db, void *str, int sz);

  int update(const TransID &tid, DaliTableHandle &handle, void *str);
  int update(const TransID &tid, DaliTableHandle &handle, void *str, int sz);
  int update(const TransID &tid, DaliRelInternal &info, void *str);
  int update(const TransID &tid, DaliRelInternal &info, void *str, int sz);
  int update(const TransID &tid, DaliDB *db, void *str, int sz);

  int free(const TransID &tid, DaliDB *db);

  void *ptr(DaliTableHandle &handle);
  void *ptr(DaliDB *db);

  int size(DaliDB *db); 
  int size(DaliTableHandle &handle); 

  DaliVarBinaryRep *repPtr(DaliTableHandle &handle);
  DaliVarBinaryRep *repPtr(DaliDB *db);
};



typedef DaliInt32 DaliJulianRep;

class DaliDate {  // The class a user would declare to hold a Dali date

public:
    DaliDate() {julian_date = 0;}
    DaliDate(int year, int month, int day);
    DaliDate(DaliJulianRep julian) : julian_date(julian) {;}
    DaliDate(const DaliDate &d2) { julian_date = d2.julian_date; }
    DaliDate(const struct tm *tmptr) { set(tmptr); }
    DaliDate& operator=(const DaliDate& d2) {
        julian_date=d2.julian_date; 
        return *this;
    }

    int set(int year, int month, int day);
    int set(const struct tm *tmptr);
    int get(int &year, int &month, int &day) const;
    int get(struct tm *tmptr) const;
    int isValid() const;
    void setNull() { julian_date = 0;}

    int dayOfWeek() const;
    char *dayOfWeekName() const;
    char *dayOfWeekAbbr() const;
    static char * dayOfWeekName(int day); // 0--> Sunday
    static char * dayOfWeekAbbr(int day); // 0--> Sunday
    static int dayOfWeek(DaliJulianRep juldate);

    int dayOfMonth() const;
    int dayOfYear() const;

    int month() const;
    char * monthName() const;
    char * monthAbbr() const;
    static char * monthName(int month);
    static char * monthAbbr(int month);

    size_t strftime(char *s, size_t maxsize, const char *format);
    int parseFrom(const char *s);

    DaliDate operator++(int) { return DaliDate(julian_date++);}
    DaliDate operator--(int) { return DaliDate(julian_date--);}

    DaliDate operator+=(int days) { return DaliDate(julian_date+= days);}
    DaliDate operator-=(int days) { return DaliDate(julian_date-= days);}

    int year() const;

    static int isValidDate(int year, int month, int day);

    friend DaliDate operator+(const DaliDate &d1, int days);
    friend DaliDate operator+(int days, const DaliDate &d1);
    friend DaliDate operator-(const DaliDate &d1, int days);
    friend int operator-(const DaliDate &d1, const DaliDate& d2);
    friend int operator<(const DaliDate &d1 ,const DaliDate &d2 );
    friend int operator>(const DaliDate &d1 ,const DaliDate &d2 );
    friend int operator<=(const DaliDate &d1 ,const DaliDate &d2 );
    friend int operator>=(const DaliDate &d1 ,const DaliDate &d2 );
    friend int operator==(const DaliDate &d1 ,const DaliDate &d2 );
    friend int operator!=(const DaliDate &d1 ,const DaliDate &d2 );
 
    friend class DaliAttr_DATE;
#if TESTING_VERY_LOCAL
    friend main();
#endif

protected:
    DaliJulianRep julian_date;

static int isLeapYear(int year);
static int daysInMonth(int month, int year);
static int YMDToJulian(int year, int mon, int day, DaliJulianRep &julian);
static int julianToYMD(DaliJulianRep julian, int &year, int &month, int &day);
};




class DaliTime {  // The class a user would declare to hold a Dali date

public:
    DaliTime() {ssm = 0;}
    DaliTime(int hours, int mins, int secs);
    DaliTime(int total_secs) : ssm(total_secs) {;}
    DaliTime(const DaliTime &d2) { ssm = d2.ssm; }
    DaliTime(const struct tm *tmptr) { set(tmptr); }
    DaliTime& operator=(const DaliTime& d2) {
        ssm=d2.ssm; 
        return *this;
    }

    int set(int hours, int mins, int secs);
    int set(const struct tm *tmptr);
    int get(int &hours, int &mins, int &secs) const;
    int get(struct tm *tmptr) const;
    int isValid() const;
    void setNull() { ssm = -1;}

    int secondsSinceMidnight() const { return ssm;}
    int seconds() const;
    int minutes() const;
    int hours() const;

    int parseFrom(const char *s);
    size_t strftime(char *s, size_t maxsize, const char *format);

    DaliTime operator++(int) { return DaliTime(ssm++);}
    DaliTime operator--(int) { return DaliTime(ssm--);}

    DaliTime operator+=(int seconds) { return DaliTime(ssm+= seconds);}
    DaliTime operator-=(int seconds) { return DaliTime(ssm-= seconds);}

    static int isValidTime(int hours, int mins, int secs);

    friend DaliTime operator+(const DaliTime &t1, int seconds);
    friend DaliTime operator+(int seconds, const DaliTime &t1);
    friend DaliTime operator-(const DaliTime &t1, int seconds);
    friend int operator-(const DaliTime &t1, const DaliTime& t2);
    friend int operator<(const DaliTime &t1 ,const DaliTime &t2 );
    friend int operator>(const DaliTime &t1 ,const DaliTime &t2 );
    friend int operator<=(const DaliTime &t1 ,const DaliTime &t2 );
    friend int operator>=(const DaliTime &t1 ,const DaliTime &t2 );
    friend int operator==(const DaliTime &t1 ,const DaliTime &t2 );
    friend int operator!=(const DaliTime &t1 ,const DaliTime &t2 );

    friend class DaliAttr_TIME;
#if TESTING_VERY_LOCAL
    friend main();
#endif

protected:
    int ssm;

};


class DaliVal{
 public:
  DaliAttrType const_typ;
  int length;
  void *val;
  DaliVal() { const_typ = DALI_UNKNOWN_TYPE; length = 0; val = 0;} 
  DaliVal(int _val) {
      const_typ = DALI_INTEGER; 
      length = sizeof(int);
      val = (void *)malloc(length); 
      *(int *)val = _val;
  }
  DaliVal(float _val) {
      const_typ = DALI_FLOAT;
      length = sizeof(float);
      val = (void *)malloc(length); 
      *(float *)val = _val;
  }
  DaliVal(double _val) {
      const_typ = DALI_DOUBLE;
      length = sizeof(double);
      val = (void *)malloc(length); 
      *(double *)val = _val;
  }
  DaliVal(char *_val) {
    const_typ = DALI_STRING;
    length = strlen(_val) + 1;
    val = (void *)strdup(_val);
  }
  DaliVal(void *_val, int sz) {
    const_typ = DALI_VARBINARY;
    length = sizeof(int) + sz;
                /* kvr- Doesn't this expose the structure of 
                   $struct DaliVarBinaryRep$?
                   Also in the next line, use malloc(length). */
    val = (void *) malloc(sizeof(int) + sz);
    DaliVarBinaryRep *vrp = (DaliVarBinaryRep *)val;
    vrp->size = sz;
    dali_bcopy((char *)_val, vrp->data, sz);
  }
  DaliVal(DaliAttrType type, void *_val, int sz) {
    const_typ = type;
    length = sz;
    val = (void *) malloc(sz);
    dali_bcopy((char *)_val, val, sz);
  }
  DaliVal(DaliDate _val) {
      const_typ = DALI_DATE;
      length = sizeof(DaliDate);
      val = (void *)malloc(length); 
      *(DaliDate *)val = _val;
  }
  DaliVal(DaliTime _val) {
      const_typ = DALI_TIME;
      length = sizeof(DaliTime);
      val = (void *)malloc(length); 
      *(DaliTime *)val = _val;
  }
  DaliVal(const DaliVal & src) {
      *this = src;
  }
  DaliVal & operator = (const DaliVal & src) {
      const_typ = src.const_typ;
      length = src.length;
      val = src.copy();
      return *this;
  }
  void *copy() const {
      void *retPtr = val;
      if(val) {
          void *rval = (void *)malloc(length);
          dali_bcopy((char *)val,(char *)rval,length);
          retPtr = rval;
      }
      return retPtr;
  }
  int destroy() {
    free(val);
    val = 0;
    length = 0;
    const_typ = DALI_UNKNOWN_TYPE;
    return DALI_SUCCESS;
  }
  ~DaliVal(){
      destroy();
  }
};    



static inline DaliAttrTypeCode *DaliAttrTypeCode::typeObject(DaliAttrType type)
{
    DaliAttrTypeCode *retPtr = NULL;
    if(!(type < 0 ||  type > DREL_NUM_DEFINED_TYPES)) {
        retPtr = TypeObjects[type];
    }
    return retPtr;
}
static inline int DaliAttrTypeCode::compareVal(DaliAttrType type, 
                                              void *v1, void *v2) {
    DaliAttrTypeCode *obj = typeObject(type);
    DALI_D_ASSERT(obj);
    return obj->compareVal(v1,v2);
}

static inline int DaliAttrTypeCode::computeHashVal(DaliAttrType type, void
                                                   *v1)
{
    DaliAttrTypeCode *obj = typeObject(type);
    DALI_D_ASSERT(obj);
    return obj->computeHashVal(v1);
}

static inline int DaliAttrTypeCode::convertFromString(DaliAttrType type, 
                                                     void *value, 
                                                     const char *string)
{
    DaliAttrTypeCode *obj = typeObject(type);
    DALI_D_ASSERT(obj);
    return obj->convertFromString(value,string);

}
static inline int DaliAttrTypeCode::convertToString(DaliAttrType type,
                                                   const void *value, 
                                                   char *string,
                                                   int max_length, 
                                                   char *format)
{
    DaliAttrTypeCode *obj = typeObject(type);
    DALI_D_ASSERT(obj);
    return obj->convertToString(value,string,max_length,format);

}
static inline int DaliAttrTypeCode::maxPrintLength(DaliAttrType type, int schemaLength)
{

    DaliAttrTypeCode *obj = typeObject(type);
    DALI_D_ASSERT(obj);
    return obj->maxPrintLength(schemaLength);
}
static inline int DaliAttrTypeCode::defaultLength(DaliAttrType type)
{

    DaliAttrTypeCode *obj = typeObject(type);
    DALI_D_ASSERT(obj);
    return obj->defaultLength();
}
static inline int DaliAttrTypeCode::defaultInternalLength(DaliAttrType type)
{
    int len;
    if(isDaliTypeExternal(type)) {
         len = DALI_EXTERNAL_POINTER_LENGTH;
    }
    else {
        DaliAttrTypeCode *obj = typeObject(type);
        DALI_D_ASSERT(obj);
        len= obj->defaultLength();
    }
    return len;
}

static inline int DaliAttrTypeCode::dataLength(DaliAttrType type, 
                                               const void *value, 
                                               int schema_length)
{
    DaliAttrTypeCode *obj = typeObject(type);
    DALI_D_ASSERT(obj);
    return obj->dataLength(value, schema_length);

}
static inline void *DaliAttrTypeCode::mallocAndCopy(DaliAttrType type,
                                                    const void* data, 
                                                    int schema_length)
{
    DaliAttrTypeCode *obj = typeObject(type);
    DALI_D_ASSERT(obj);
    return obj->mallocAndCopy(data, schema_length);
    
}

static inline char *DaliAttrTypeCode::typeString(DaliAttrType type)
{
    DaliAttrTypeCode *obj = typeObject(type);
    DALI_D_ASSERT(obj);
    return obj->typeString();
    
}
static inline char *DaliAttrTypeCode::typeInCString(DaliAttrType type)
{
    DaliAttrTypeCode *obj = typeObject(type);
    DALI_D_ASSERT(obj);
    return obj->typeInCString();

}

inline char *DaliAttrTypeToString(DaliAttrType typ)
{
    return DaliAttrTypeCode::typeString(typ);
}

inline char *DaliAttrTypeToCTypeString(DaliAttrType typ)
{
    return DaliAttrTypeCode::typeInCString(typ);
}

extern int DaliTypeAlign(DaliAttrType atype);

class daliNULL{
};

extern const daliNULL DALI_NULL;
#endif 
/* RELTYPES_H */
