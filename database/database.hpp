#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "settings.hpp"
#include "mutex.hpp"
#include "platform.hpp"

/**
 *
 * @brief База данных, хранит пары ключ-значение
 *
 */

class Database
{
public:

    enum TYPE
    {
        INT,
        FLOAT,
        STRING,
        CHAR,
        FUNCTION,
        DATABASE,
        UNKNOWN_TYPE
    };

    typedef enum
    {
        EQUALS,
        SMALLER,
        LONGER,
        FIELD_NOT_FOUND
    } COMPARE_RESULT ;

    void init();
    void deinit();

    RSTATUS add( const char* key, const char*  value );
    RSTATUS add( const char* key, const int    value );   
    RSTATUS add( const char* key, const char   value );
    RSTATUS add( const char* key, const float  value );
    RSTATUS add( const char* key, const double value );
    RSTATUS add( const char* key, Database* value );
    RSTATUS add( const char* key, void* value );

    RSTATUS change( const char* key, const char*  newvalue );
    RSTATUS change( const char* key, const int    newvalue );
    RSTATUS change( const char* key, const char   newvalue );
    RSTATUS change( const char* key, const float  newvalue );
    RSTATUS change( const char* key, const double newvalue );

    COMPARE_RESULT compare( char* leftKey, char* rightKey );

    COMPARE_RESULT compareWithString( char* key, const char* str );

    void      getStringValue( const char* key, char* stringValue );
    void      getStringValue(  uint32_t key, char* value    );
    int       getIntegerValue( const char* key );
    float     getFloatValue(   const char* key );   // float and double
    char      getCharValue(    const char* key );
    Database* getDatabase(     const char* key );
    Database* getDatabase( uint32_t key );
    void*     getFunction(     const char* key );
    void*     getFunction( uint32_t key );

   // char*     operator[] ( const char* key );

    RSTATUS load( Settings& settings );
    RSTATUS store( Settings& settings );

    TYPE getKeyType( const char* key );

    uint32_t size();

public:

    typedef struct
    {
        uint32_t  name;
        TYPE      type;
        uintptr_t value; // Хранит в себе указатель на строку, либо int число либо float число в зависимости от типа данных
    } entry;

    Mutex mutex;

    entry*   entries;        // динамический массив!!
    uint32_t entriesCount;

    char stringValue[16];

    entry* _findEntry( const char* key );
    entry* _findEntry( uint32_t key );
    entry* _bsearch( uint32_t hashKey );
    RSTATUS _getMemoryForSecondEntry();
    RSTATUS _sort();
    bool   _isHasDuplicateKeys();

};

extern Database database;

#endif
