#include "database.hpp"
#include "convert.hpp"
#include "crc.hpp"

#include <cstdlib>
#include <string.h>
#include <math.h>

/**
 *
 * @brief Инициализация, сделана не в конструкторе, потому что в нем она срабатывает не на каждой платформе
 *
 */

void Database::init()
{

    mutex.lock();

    entriesCount = 0;
    entries      = 0;

    mutex.unlock();

}

/**
 *
 * @brief Деинициализация, сделана не в деструкторе, потому что в нем она срабатывает не на каждой платформе
 *
 */

void Database::deinit()
{

    mutex.lock();

    if( entries == 0 )
    {
        return;
    }

    for( uint32_t i = 0 ; i < entriesCount; i++ )
    {

        if( entries[i].type == STRING )
        {

            char* value = (char*) *((uintptr_t*)&(entries[i].value));

            if( value != 0 )
            {

                free(value);

            }

        }
        else if( entries[i].type == DATABASE )
        {

            Database* value = (Database*) *((uintptr_t*)&(entries[i].value));

            delete value;

        }

    }

    free(entries);

    mutex.unlock();

}

/**

   @brief Сравнить значение переменной с другой переменной

    Правила сравнения:

        Сравниваются переменные типов: FLOAT, INT, STRING, CHAR

        Если типы переменных совпадают, то они сравниваются напрямую

        Если одна из переменных строка, то она приводится к типу другой нестроковой переменной

        Если обе переменные числовые(FLOAT,INT) то они приводятся к типу FLOAT и сравниваются.

 * @param leftKey Ключ первой переменной в базе данных
 * @param rightKey ключ второй переменной в базе данных
 *
 * @return результат сравнения
 *
 */

Database::COMPARE_RESULT Database::compare( char* leftKey, char* rightKey )
{

    mutex.lock();

    entry* leftEntry  = _findEntry(leftKey);
    entry* rightEntry = _findEntry(rightKey);

    if( ( leftEntry == 0 ) || ( rightEntry == 0 ) )
    {
        mutex.unlock();
        return EQUALS;
    }

    if( leftEntry->type == rightEntry->type )
    {

        // Типы совпадают, сравниваем напрямую

        if( leftEntry->type == INT )
        {

            int leftValue  = *((int*)&(leftEntry->value));
            int rightValue = *((int*)&(rightEntry->value));

            mutex.unlock();

            if( leftValue == rightValue )
            {

                return EQUALS;

            }
            else if( leftValue > rightValue )
            {

                return LONGER;

            }
            else
            {

                return SMALLER;

            }

        }
        else if( leftEntry->type == FLOAT )
        {

            float leftValue  = *((float*)&(leftEntry->value));
            float rightValue = *((float*)&(rightEntry->value));

            mutex.unlock();

            if( fabs( leftValue - rightValue ) < __FLT_EPSILON__ )
            {

                return EQUALS;

            }
            else if( leftValue > rightValue )
            {

                return LONGER;

            }
            else
            {

                return SMALLER;

            }

        }
        else if( leftEntry->type == STRING )
        {

            char* leftValue  = (char*)(*((uintptr_t*)&(leftEntry->value)));
            char* rightValue = (char*)(*((uintptr_t*)&(rightEntry->value)));

            int state = strcmp( leftValue, rightValue );

            mutex.unlock();

            if( state == 0 )
            {

                return EQUALS;

            }
            else if( state > 0 )
            {

                return LONGER;

            }
            else
            {

                return SMALLER;

            }

        }
        else if( leftEntry->type == CHAR )
        {

            if( leftEntry->value == rightEntry->value )
            {

                mutex.unlock();

                return EQUALS;

            }
            else if( leftEntry->value > rightEntry->value )
            {

                mutex.unlock();

                return LONGER;

            }
            else
            {

                mutex.unlock();

                return SMALLER;

            }


        }
        else
        {

            // Остальные типы не сравниваются

            mutex.unlock();

            return EQUALS;

        }

    }
    else if( ( leftEntry->type == STRING ) || ( rightEntry->type == STRING ) )
    {

        // Одна из переменных строка, она приводится к типу переменной

        if( leftEntry->type == STRING )
        {

            char* leftValue  = (char*)(*((uintptr_t*)&(leftEntry->value)));

            if( rightEntry->type == FLOAT )
            {

                float rightVal = *((float*)&(rightEntry->value));
                float leftVal  = atof( leftValue );

                mutex.unlock();

                if( fabs( leftVal - rightVal ) < __FLT_EPSILON__ )
                {

                    return EQUALS;

                }
                else if( leftVal > rightVal )
                {

                    return LONGER;

                }
                else
                {

                    return SMALLER;

                }

            }
            else if( rightEntry->type == INT )
            {

                int rightVal = *((int*)&(rightEntry->value));
                int leftVal  = atoi( leftValue );

                mutex.unlock();

                if( leftVal == rightVal )
                {

                    return EQUALS;

                }
                else if( leftVal > rightVal )
                {

                    return LONGER;

                }
                else
                {

                    return SMALLER;

                }

            }
            else
            {

                mutex.unlock();

                return EQUALS;

            }

        }
        else
        {

            char* rightValue  = (char*)(*((uintptr_t*)&(rightEntry->value)));

            if( leftEntry->type == FLOAT )
            {

                float leftVal  = *((float*)&(leftEntry->value));
                float rightVal = atof( rightValue );

                mutex.unlock();

                if( fabs( leftVal - rightVal ) < __FLT_EPSILON__ )
                {

                    return EQUALS;

                }
                else if( leftVal > rightVal )
                {

                    return LONGER;

                }
                else
                {

                    return SMALLER;

                }

            }
            else if( leftEntry->type == INT )
            {

                int leftVal = *((int*)&(leftEntry->value));
                int rightVal  = atoi( rightValue );

                mutex.unlock();

                if( leftVal == rightVal )
                {

                    return EQUALS;

                }
                else if( leftVal > rightVal )
                {

                    return LONGER;

                }
                else
                {

                    return SMALLER;

                }

            }
            else
            {

                mutex.unlock();

                return EQUALS;

            }

        }

    }
    else
    {

        // Ни одна из переменных не строка
        //  Обе приводим к типу float и сравниваем

        float leftVal;
        float rightVal;

        if( leftEntry->type == INT  )
        {

            leftVal = (float) *((int*)&(leftEntry->value));

        }
        else if( leftEntry->type == FLOAT )
        {

            leftVal = *((float*)&(leftEntry->value));

        }
        else
        {

            mutex.unlock();

            return EQUALS;

        }

        if( rightEntry->type == INT )
        {

            rightVal = (float) *((int*)&(rightEntry->value));

        }
        else if( rightEntry->type == FLOAT )
        {

            rightVal = *((float*)&(rightEntry->value));

        }
        else
        {

            mutex.unlock();

            return EQUALS;

        }

        mutex.unlock();

        if( fabs( leftVal - rightVal ) < __FLT_EPSILON__ )
        {

            return EQUALS;

        }
        else if( leftVal > rightVal )
        {

            return LONGER;

        }
        else
        {

            return SMALLER;

        }

    }

    mutex.unlock();

    return EQUALS;

}

/**
 *
 * @brief Сравнить значение переменной со строкой, при этом строка приводится к типу переменной
 *
 * @param key ключ переменной
 * @param str строка, с которой сравнивается значение переменной
 *
 * @return результат сравнения
 *
 */

Database::COMPARE_RESULT Database::compareWithString( char* key, const char* str )
{

    mutex.lock();

    entry* pentry = _findEntry(key);

    if( pentry == 0 )
    {

        mutex.unlock();

        return FIELD_NOT_FOUND;

    }

    if( pentry->type == INT )
    {

        // atoi

        int value    = *((int*)&(pentry->value));
        int strValue = atoi(str);

        mutex.unlock();

        if( value == strValue )
        {

            return EQUALS;

        }
        else if( value > strValue )
        {

            return LONGER;

        }
        else
        {

            return SMALLER;

        }


    }
    else if( pentry->type == FLOAT )
    {

        // atof

        float value    = *((float*)&(pentry->value));
        float strValue = atof(str);

        mutex.unlock();

        if( fabs( value - strValue ) < __FLT_EPSILON__ )
        {

            return EQUALS;

        }
        else if( value > strValue )
        {

            return LONGER;

        }
        else
        {

            return SMALLER;

        }

    }
    else if( pentry->type == STRING )
    {

       char* value = (char*)(*((uintptr_t*)&(pentry->value)));

       int state = strcmp( value, str );

       mutex.unlock();

       if( state == 0 )
       {

           return EQUALS;

       }
       else if( state > 0 )
       {

           return LONGER;

       }
       else
       {

           return SMALLER;

       }

    }
    else if( pentry->type == CHAR )
    {

        if( pentry->value == (uintptr_t) str[0] )
        {

            mutex.unlock();

            return EQUALS;

        }
        else if( pentry->value > (uintptr_t) str[0] )
        {

            mutex.unlock();

            return LONGER;

        }
        else
        {

            mutex.unlock();

            return SMALLER;

        }

    }
    else
    {

        mutex.unlock();

        return EQUALS;

    }

    mutex.unlock();

    return FIELD_NOT_FOUND;

}

/**
 * @brief Database::add
 * @param key
 * @param value
 * @return
 */

RSTATUS Database::add( const char* key, const char value )
{

    mutex.lock();

    if ( SUCCESS_STATUS != _getMemoryForSecondEntry() )
    {

        mutex.unlock();

        return ERROR_STATUS;

    }

    uint32_t hash = CRC::crc32( 0, (uint8_t*)key, strlen(key) );

    entries[entriesCount].name = hash;
    entries[entriesCount].type = CHAR;

    entries[entriesCount].value = value;

    entriesCount += 1;

    RSTATUS status = _sort();

    mutex.unlock();

    return status;

}

/**
 * @brief Database::add
 * @param key
 * @param value
 * @return
 */

RSTATUS Database::add( const char *key, const char* value )
{

    mutex.lock();

    if ( SUCCESS_STATUS != _getMemoryForSecondEntry() )
    {
        mutex.unlock();
        return ERROR_STATUS;
    }

    uint32_t hash = CRC::crc32( 0, (uint8_t*)key, strlen(key) );

    entries[entriesCount].name = hash;
    entries[entriesCount].type = STRING;

    if( value == 0 )
    {

        entries[entriesCount].value = 0;

    }
    else
    {

        char* str = (char*) malloc(strlen(value)+1);

        strcpy( str, value );
        *( (uintptr_t*) &(entries[entriesCount].value ) ) = (uintptr_t) str;

    }

    entriesCount += 1;

    RSTATUS status = _sort();

    mutex.unlock();

    return status;

}

/**
 * @brief Database::add
 * @param key
 * @param value
 * @return
 */

RSTATUS Database::add( const char* key, const int value )
{

    mutex.lock();

    if ( SUCCESS_STATUS != _getMemoryForSecondEntry() )
    {
        mutex.unlock();
        return ERROR_STATUS;
    }

    uint32_t hash = CRC::crc32( 0, (uint8_t*)key, strlen(key) );

    entries[entriesCount].name = hash;
    entries[entriesCount].type = INT;

    entries[entriesCount].value = value;

    entriesCount += 1;

    RSTATUS status = _sort();

    mutex.unlock();

    return status;

}

/**
 * @brief Database::add
 * @param key
 * @param value
 * @return
 */

RSTATUS Database::add( const char* key, Database *value )
{

    mutex.lock();

    if ( SUCCESS_STATUS != _getMemoryForSecondEntry() )
    {
        mutex.unlock();
        return ERROR_STATUS;
    }

    uint32_t hash = CRC::crc32( 0, (uint8_t*)key, strlen(key) );

    entries[entriesCount].name = hash;
    entries[entriesCount].type = DATABASE;

    *((uintptr_t*)&entries[entriesCount].value) = (uintptr_t)value;

    entriesCount += 1;

    RSTATUS status = _sort();

    mutex.unlock();

    return status;


}

/**
 * @brief Database::add
 * @param key
 * @param value
 * @return
 */

RSTATUS Database::add( const char* key, void* value )
{

    mutex.lock();

    if ( SUCCESS_STATUS != _getMemoryForSecondEntry() )
    {
        mutex.unlock();
        return ERROR_STATUS;
    }

    uint32_t hash = CRC::crc32( 0, (uint8_t*)key, strlen(key) );

    entries[entriesCount].name = hash;
    entries[entriesCount].type = FUNCTION;

    *((uintptr_t*)&entries[entriesCount].value) = (uintptr_t)value;

    entriesCount += 1;

    RSTATUS status = _sort();

    mutex.unlock();

    return status;


}

/**
 * @brief Database::add
 * @param key
 * @param value
 * @return
 */

RSTATUS Database::add( const char* key, const double value )
{

    return add( key, (float) value );

}

/**
 * @brief Database::add
 * @param key
 * @param value
 * @return
 */

RSTATUS Database::add( const char* key, const float value )
{

    mutex.lock();

    if ( SUCCESS_STATUS != _getMemoryForSecondEntry() )
    {
        mutex.unlock();
        return ERROR_STATUS;
    }

    uint32_t hash = CRC::crc32( 0, (uint8_t*)key, strlen(key) );

    entries[entriesCount].name = hash;
    entries[entriesCount].type = FLOAT;

    *((float*)&entries[entriesCount].value) = value;

    entriesCount += 1;

    RSTATUS status = _sort();

    mutex.unlock();

    return status;

}

/**
 *
 * @brief Устанавливается новое значение переменной
 *
 * @param key название переменной
 * @param newvalue новое значение переменной в виде строки, строковое значение приводится к типу переменной
 *
 * @return
 *
 */

RSTATUS Database::change( const char* key, const char* newvalue )
{

    mutex.lock();

    entry* pentry = _findEntry(key);

    if( pentry == 0 )
    {

        mutex.unlock();

        return ERROR_STATUS;

    }
    else
    {

        if( pentry->type == STRING )
        {

            char* str = (char*)(*((uintptr_t*)&(pentry->value)));

            if( str != 0 )
            {
                free((void*)str);
            }

            if( newvalue == 0 )
            {

                pentry->value = 0;

            }
            else
            {

                str = (char*) malloc(strlen(newvalue)+1);

                strcpy( str, newvalue );

                *( (uintptr_t*) &( pentry->value ) ) = (uintptr_t) str;

            }

        }
        else if( pentry->type == INT )
        {

            *((int*)&pentry->value) = atoi(newvalue);

        }
        else if( pentry->type == FLOAT )
        {

            *((float*)&pentry->value) = atof(newvalue);

        }
        else if( pentry->type == CHAR )
        {

            pentry->value = newvalue[0];

        }
        else
        {

            mutex.unlock();

            return ERROR_STATUS;

        }

        mutex.unlock();

        return SUCCESS_STATUS;

    }

    mutex.unlock();

    return ERROR_STATUS;

}

/**
 * @brief Database::change
 * @param key
 * @param newvalue
 * @return
 */

RSTATUS Database::change( const char* key, const int newvalue )
{

    mutex.lock();

    entry* pentry = _findEntry(key);

    if( ( pentry == 0 ) || ( ( pentry->type != INT )  && ( pentry->type != FLOAT ) )  )
    {

        mutex.unlock();

        return ERROR_STATUS;

    }
    else
    {

        if( pentry->type == FLOAT )
        {

            *((float*)&pentry->value) = (float) newvalue;

        }
        else
        {

            *((int*)&pentry->value) = newvalue;

        }

        mutex.unlock();

        return SUCCESS_STATUS;

    }

    mutex.unlock();

    return ERROR_STATUS;

}

/**
 * @brief Database::change
 * @param key
 * @param newvalue
 * @return
 */

RSTATUS Database::change( const char* key, const char newvalue )
{

    mutex.lock();

    entry* pentry = _findEntry(key);

    if( ( pentry != 0 ) && ( pentry->type == CHAR )  )
    {

        pentry->value = newvalue;

        mutex.unlock();

        return SUCCESS_STATUS;

    }

    mutex.unlock();

    return ERROR_STATUS;

}

/**
 * @brief Database::change
 * @param key
 * @param newvalue
 * @return
 */

RSTATUS Database::change( const char* key, const double newvalue )
{

    return change( key, (float) newvalue );

}

/**
 * @brief Database::change
 * @param key
 * @param newvalue
 * @return
 */

RSTATUS Database::change( const char* key, const float newvalue )
{

    mutex.lock();

    entry* pentry = _findEntry(key);

    if( ( pentry == 0 ) || ( pentry->type != FLOAT ) )
    {

        mutex.unlock();

        return ERROR_STATUS;

    }
    else
    {

        *((float*)&pentry->value) = newvalue;

        mutex.unlock();

        return SUCCESS_STATUS;

    }

    mutex.unlock();

    return ERROR_STATUS;

}

/**
 *
 * @brief Получить строковое значение, потоконебезопасная функция, использовать в связке с lock()/unlock()
 *
 * @param key
 *
 * @return
 *
 */

void Database::getStringValue( const char* key, char* stringValue )
{

    mutex.lock();

    entry* pentry = _findEntry(key);

    if( pentry == 0 )
    {

        stringValue[0] = '\0';

    }
    else if( pentry->type == INT )
    {

        int value = *((int*)&(pentry->value));

        stringValue [ Convert::integerToString( &(stringValue[0]), value, 16 ) ] = '\0';

    }
    else if( pentry->type == FLOAT )
    {

        float value = *((float*)&(pentry->value));

        stringValue[ Convert::floatToString( &(stringValue[0]), value, 16 ) ] = '\0';

    }
    else if( pentry->type == STRING )
    {

        char* str = (char*)(*((uintptr_t*)&(pentry->value)));

        if( str != 0 )
        {

            int i = 0;

            while( str[i] != '\0' )
            {
                stringValue[i] = str[i];
                i += 1;
            }

            stringValue[i] = '\0';

        }
        else
        {

            stringValue[0] = '\0';

        }

    }
    else if( pentry->type == CHAR )
    {

        stringValue[0] = pentry->value;
        stringValue[1] = '\0';

    }
    else
    {

        stringValue[0] = '\0';

    }

    mutex.unlock();

// ----------------------------------------------------
/*
    if( ( pentry == 0 ) || ( pentry->type != STRING ))
    {

        value[0] = '\0';

    }
    else
    {

        char* databaseValue = (char*)(*((uintptr_t*)&(pentry->value)));

        int i = 0;

        while( databaseValue[i] != '\0' )
        {
            value[i] = databaseValue[i];
            i += 1;
        }

        value[i] = '\0';

    }
*/


}

/**
 *
 * @brief Получить значение по ключу, потоконебезопасная функция!!!
 *         использовать в связке с lock()/unlock
 *
 * @param key
 *
 */

/*
char* Database::operator[] ( const char* key )
{

    entry* pentry = _findEntry(key);

    if( pentry == 0 )
    {

        stringValue[0] = '\0';

    }
    else if( pentry->type == INT )
    {

        int value = *((int*)&(pentry->value));

        stringValue [ Convert::integerToString( &(stringValue[0]), value, 16 ) ] = '\0';

    }
    else if( pentry->type == FLOAT )
    {

        float value = *((float*)&(pentry->value));

        stringValue[ Convert::floatToString( &(stringValue[0]), value, 16 ) ] = '\0';

    }
    else if( pentry->type == STRING )
    {

        char* str = (char*)(*((uintptr_t*)&(pentry->value)));

        if( str != 0 )
        {

            return str;

        }
        else
        {

            stringValue[0] = '\0';

        }

    }
    else if( pentry->type == CHAR )
    {

        stringValue[0] = pentry->value;
        stringValue[1] = '\0';

    }
    else
    {

        stringValue[0] = '\0';

    }

    return &(stringValue[0]);

}
*/
/**
 * @brief Database::getIntegerValue
 * @param key
 * @return
 */

int Database::getIntegerValue( const char* key )
{

    mutex.lock();

    entry* pentry = _findEntry(key);

    if( ( pentry != 0 ) && ( pentry->type == INT ) )
    {

        int value = *((int*)&(pentry->value));

        mutex.unlock();

        return value;

    }

    mutex.unlock();

    return 0;

}

/**
 * @brief Database::getFloatValue
 * @param key
 * @return
 */

float Database::getFloatValue( const char* key )
{

    mutex.lock();

    entry* pentry = _findEntry(key);

    if( ( pentry != 0 ) && ( pentry->type == FLOAT ) )
    {

        float value = *((float*)&(pentry->value));

        mutex.unlock();

        return value;

    }

    mutex.unlock();

    return 0;

}

/**
 *
 * @brief Получить строковое значение, потоконебезопасная функция, использовать вместе с lock()/unlock()
 *
 * @param key
 *
 * @return
 *
 */

void Database::getStringValue( uint32_t key, char* value )
{

    mutex.lock();

    entry* pentry = _findEntry(key);

    if( ( pentry == 0 ) || ( pentry->type != STRING ))
    {

        value[0] = '\0';

    }
    else
    {

        char* databaseValue = (char*)(*((uintptr_t*)&(pentry->value)));

        int i = 0;

        while( databaseValue[i] != '\0' )
        {
            value[i] = databaseValue[i];
            i += 1;
        }

        value[i] = '\0';

    }

    mutex.unlock();

}

/**
 * @brief Database::getCharValue
 * @param key
 * @return
 */

char Database::getCharValue( const char* key )
{

    mutex.lock();

    entry* pentry = _findEntry(key);

    if( ( pentry != 0 ) && ( pentry->type == CHAR ) )
    {

        char value = pentry->value;

        mutex.unlock();

        return value;

    }

    mutex.unlock();

    return 0;

}

/**
 *
 * @brief Получить другую базу данных
 *
 * @param key
 *
 * @return
 *
 */

Database* Database::getDatabase( uint32_t key )
{

    mutex.lock();

    entry* pentry = _findEntry(key);

    if( ( pentry != 0 ) && ( pentry->type == DATABASE ) )
    {

        Database* databasep = (Database*)(*((uintptr_t*)&(pentry->value)));

        mutex.unlock();

        return databasep;

    }

    mutex.unlock();

    return 0;

}

/**
 *
 * @brief Получить другую базу данных
 *
 * @param key
 *
 * @return
 *
 */

Database* Database::getDatabase( const char* key )
{

    mutex.lock();

    entry* pentry = _findEntry(key);

    if( ( pentry != 0 ) && ( pentry->type == DATABASE ))
    {

        Database* databasep = (Database*)(*((uintptr_t*)&(pentry->value)));

        mutex.unlock();

        return databasep;

    }

    mutex.unlock();

    return 0;


}

/**
 *
 * @brief Получить фукнцию
 *
 * @param key
 *
 * @return
 *
 */

void* Database::getFunction( uint32_t key )
{

    mutex.lock();

    entry* pentry = _findEntry(key);

    if( ( pentry != 0 ) && ( pentry->type == FUNCTION ))
    {

        void* func = (void*)(*((uintptr_t*)&(pentry->value)));

        mutex.unlock();

        return func;

    }

    mutex.unlock();

    return 0;

}

/**
 *
 * @brief Получить фукнцию
 *
 * @param key
 *
 * @return
 *
 */

void* Database::getFunction( const char* key )
{

    mutex.lock();

    entry* pentry = _findEntry(key);

    if( ( pentry != 0 ) && ( pentry->type == FUNCTION ))
    {

        void* func = (void*)(*((uintptr_t*)&(pentry->value)));

        mutex.unlock();

        return func;

    }

    mutex.unlock();

    return 0;

}

/**
 * @brief Ищем ключ двоичным поиском
 * @param hash
 * @return
 */

Database::entry* Database::_findEntry( uint32_t hash )
{

    if( entriesCount == 0 )
    {

        return 0;

    }

    _sort();

    return _bsearch( hash );

}

/**
 * @brief Ищем ключ двоичным поиском
 * @param key
 * @return
 */

Database::entry* Database::_findEntry( const char* key )
{

    if( entriesCount == 0 )
    {

        return 0;

    }

    _sort();

    return _bsearch( CRC::crc32( 0, (uint8_t*)key, strlen(key) ) );

}

/**
 * @brief Database::getKeyType
 * @param key
 * @return
 */

Database::TYPE Database::getKeyType( const char* key )
{

    entry* pentry = _findEntry(key);

    return pentry->type;

}


/**

    @brief Двоичный поиск записи по её хэшу

    За основу функции взял исходный код функциии двоичного поиска bsearch из стандартной библиотеки
     Напрямую её не получилось использовать, потому что она требует функцию сравнения
     (не метод), а мне требовалось чтобы фукнция сравнения была методом
     класса Database.

    @param hashkey хэш ключа записи

    @return найденная запись или 0

*/

Database::entry* Database::_bsearch ( const uint32_t hashKey )
{

  uint32_t l = 0;
  uint32_t u = entriesCount;

  while ( l < u )
  {

      uint32_t idx = ( l + u ) / 2;

      entry* p = (entry *) (((const char *) &(entries[0])) + (idx * sizeof(entry)));

      if( hashKey < p->name )
      {

          u = idx;

      }
      else if( hashKey > p->name )
      {

          l = idx + 1;

      }
      else
      {

          return p;

      }

    }

    return 0;

}

/**
 *
 * @brief Получить количество записанных ключей
 *
 * @return
 *
 */

uint32_t Database::size()
{

    mutex.lock();

    uint32_t ecount = entriesCount;

    mutex.unlock();

    return ecount;

}

#include "debug.hpp"

/**
 *
 * @brief Загрузка сохраненной в постоянной(энергонезависимой) памяти базы данных
 *
 * @param settings
 *
 */

RSTATUS Database::load( Settings& settings )
{

    mutex.lock();

    // ДОБАВИТЬ УДАЛЕНИЕ СТРОК ПРИ ОШИБКЕ!!!

    if ( ( entries != 0 ) || (entriesCount != 0) )
    {

        mutex.unlock();

        return ERROR_STATUS;

    }

    Debug::print("Load settings");

    Debug::print("Setting length: ", settings.getLength() );

    for( uint32_t readIndex = 0; readIndex < settings.getLength(); )
    {

        Debug::print("Read index: ", readIndex );

        entry loadEntry;

        if( sizeof(uint32_t) != settings.read(&(loadEntry.name), sizeof(uint32_t)) )
        {
            if( entries != 0 ){ free(entries); entries = 0; }

            Debug::print("Error on read NAME");

            mutex.unlock();
            return ERROR_STATUS;
        }

        readIndex += sizeof(uint32_t);

        uint32_t typeOfEntry = 0;

        if( sizeof(uint32_t) != settings.read( &typeOfEntry, sizeof(uint32_t) ) )
        {
            if( entries != 0 ){ free(entries); entries = 0; }
            Debug::print("Error on read type: ");
            mutex.unlock();
            return ERROR_STATUS;
        }

        loadEntry.type = (Database::TYPE) typeOfEntry;

        readIndex += sizeof(uint32_t);

        if( loadEntry.type == STRING )
        {

            uint32_t strLength;

            if( sizeof(uint32_t) != settings.read( &strLength, sizeof(uint32_t) ) )
            {
                if( entries != 0 ){ free(entries); entries = 0; }
                Debug::print("Error on read STRING: ");
                mutex.unlock();
                return ERROR_STATUS;
            }

            readIndex += sizeof(uint32_t);

            char* str = (char*) malloc( strLength + 1 );

            if( 0 == str )
            {
                if( entries != 0 ){ free(entries); entries = 0; }
                Debug::print("Error - string is NULL!");
                mutex.unlock();
                return ERROR_STATUS;
            }

            if( strLength != settings.read( str, strLength ) )
            {
                if( entries != 0 ){ free(entries); entries = 0; }
                Debug::print("Error string length is NULL!");
                mutex.unlock();
                return ERROR_STATUS;
            }

            readIndex += strLength;

            str[ strLength ] = '\0';

            *( (uintptr_t*) &(loadEntry.value) ) = (uintptr_t) str;

        }
        else
        {

            if( sizeof(uint32_t) != settings.read( &(loadEntry.value), sizeof(uint32_t) ) )
            {
                if( entries != 0 ){ free(entries); entries = 0; }
                Debug::print("Error on read int/float!");
                mutex.unlock();
                return ERROR_STATUS;
            }

            readIndex += sizeof(uint32_t);

        }

        if ( SUCCESS_STATUS != _getMemoryForSecondEntry() )
        {
            if( entries != 0 ){ free(entries); entries = 0; }
            Debug::print("Error on get memory for second entry!");
            mutex.unlock();
            return ERROR_STATUS;
        }

        *(&entries[entriesCount]) = loadEntry;

        entriesCount += 1;

        Debug::print("Entries count ++ ", entriesCount );

    }

    if ( ERROR_STATUS == _sort() )
    {
        mutex.unlock();
        return ERROR_STATUS;
    }

    mutex.unlock();

    return SUCCESS_STATUS;

}

/**
 *
 * @brief Сохранение базы данных в постоянной(энергонезависимой) памяти
 *
 * @param settings
 *
 */

RSTATUS Database::store( Settings& settings )
{

    mutex.lock();

    Debug::print("Store: ", entriesCount );

    // Сохранение базы данных в настройках

    for( uint32_t i = 0 ; i < entriesCount ; i++ )
    {

        Debug::print("Store entry: ", i );

        if( sizeof(uint32_t) != settings.write( &(entries[i].name), sizeof(uint32_t) ) )
        {
            Debug::print("Error on write name!");
            mutex.unlock();
            return ERROR_STATUS;
        }

        uint32_t type = entries[i].type;

        if( sizeof(uint32_t) != settings.write( &type, sizeof(uint32_t) ) )
        {
            Debug::print("Error on write type");
            mutex.unlock();
            return ERROR_STATUS;
        }

        if( entries[i].type == STRING )
        {

            Debug::print("Write entry with type STRING");

            char* str = (char*)(*((uintptr_t*)&(entries[i].value)));

            uint32_t strLength = strlen(str);

            if( sizeof(uint32_t) != settings.write( &strLength, sizeof(uint32_t) ) )
            {
                Debug::print("Error on write STRING length");
                mutex.unlock();
                return ERROR_STATUS;
            }

            if( strlen(str) != settings.write( str, strlen(str) ) )
            {
                Debug::print("Error on write STRING");
                mutex.unlock();
                return ERROR_STATUS;
            }

        }
        else
        {

            Debug::print("Write entry with type int/float");

            if( sizeof(uint32_t) != settings.write( &(entries[i].value), sizeof(uint32_t) ) )
            {
                mutex.unlock();
                return ERROR_STATUS;
            }

        }

    }

    Debug::print("Store success!");

    RSTATUS status = settings.commit();

    mutex.unlock();

    return status;


}

/**
 *
 * @brief Сортировка записей по возрастанию хэша
 *
 * @return
 *
 */

RSTATUS Database::_sort()
{

    for ( uint32_t i = 0; i < entriesCount-1; i++)
    {

        for ( uint32_t j = 0; j < entriesCount-i-1; j++)
        {

            if ( entries[j].name > entries[j+1].name )
            {

                entry temp   = entries[j];
                entries[j]   = entries[j+1];
                entries[j+1] = temp;

            }

        }

    }

    if( _isHasDuplicateKeys() )
    {

        return ERROR_STATUS;

    }
    else
    {

        return SUCCESS_STATUS;

    }

}

/**
 *
 *  @brief Есть ли одинаковые ключи или хэши(коллизии хэш функции)
 *
 *    С целью экономии памяти и ускорения поиска для хэширования используется
 *      алгоритм подсчета контрольной суммы CRC32 который возвращает 4 байта.
 *
 *    Есть небольшая вероятность того что 2 или более одинаковых ключа дадут одну хэш сумму.
 *
 *    Если все-таки возникла коллизия, нужно отказаться от использования имени ключа/ключей
 *     приводящих к коллизии.
 *
 *   @return true - ошибка есть дубликаты или коллизии, false - все нормально дубликатов или коллизий нет
 *
*/

bool Database::_isHasDuplicateKeys()
{

    for( uint32_t i = 0 ; i < (entriesCount-1) ; i++ )
    {

        if( entries[i].name == entries[i+1].name )
        {

            // Найдены одинаковые ключи или произошла коллизия хэш функции

            return true;

        }

    }

    return false;

}

/**
 * @brief Database::_getMemoryForSecondEntry
 * @return
 */

RSTATUS Database::_getMemoryForSecondEntry()
{

    if( entries == 0 )
    {

        entries = (entry*) malloc(sizeof(entry));

        if( entries != 0 )
        {
            return SUCCESS_STATUS;
        }
        else
        {
            return ERROR_STATUS;
        }

    }
    else
    {

        void* newSpace;

        newSpace = realloc( &(entries[0]), sizeof(entry) * (entriesCount+1) );

        if( newSpace != NULL )
        {

            entries = (entry*) newSpace;

            return SUCCESS_STATUS;

        }
        else
        {

            return ERROR_STATUS;

        }

    }

    return ERROR_STATUS;

}

Database database;
