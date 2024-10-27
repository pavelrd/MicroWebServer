#include "convert.hpp"

#include <cstring>
#include <stdlib.h>

/**
 * @brief Преобразование числа с плавающей точкой в строку
 *
 * @param string
 * @param number
 * @param stringLength
 *
 * @return
 *
 */

uint8_t Convert::floatToString(char* string, float number, const uint8_t stringLength )
{

    // 1234567
    // 1234567.3456
    // -

    if( stringLength == 0 )
    {
        return 0;
    }

    uint8_t pt;
    uint32_t t;

    if( number < 0 )
    {

        // -

        string[0] = '-';

        pt = 1;

        number = ((float)0) - number;

        t = (uint32_t) number;

        pt += unsignedIntegerToString( string+1, t, stringLength-1 );

        if( pt >= ( stringLength - 1 ) )
        {
            string[pt] = '\0';
            return pt;
        }

        string[pt] = '.';

        pt += 1;

    }
    else
    {

        // +

        pt = 0;

        t = (uint32_t) number;

        pt += unsignedIntegerToString( string, t, stringLength );

        if( pt >= (stringLength-1) )
        {
            string[pt] = '\0';
            return pt;
        }

        string[pt] = '.';

        pt += 1;

    }

    number -= t;

    // 0.0790996

    for( uint8_t i = 0; i < 5; i++ )
    {

        if( pt >= (stringLength-1) )
        {
            string[pt] = '\0';
            return pt;
        }

        number *= 10;

        uint8_t dec = (uint8_t)number;

        number -= dec;

        string[pt] = dec;
        string[pt] += ASCII_NUM_START;
        pt += 1;


    }

    string[pt] = '\0';

    return pt;

}

/**
 *
 * @brief Преобразование целого числа в строку
 *
 * @param str
 * @param num
 * @param length
 *
 * @return
 *
 */

uint8_t Convert::integerToString( char *str, int32_t num, const uint8_t length )
{

    if( length == 0 ) return 0;

    if( num < 0 )
    {

        str[0] = '-';

        return ( 1 + unsignedIntegerToString(str+1,(~num)+1,length-1) );

    } else {

        return unsignedIntegerToString( str, num, length );

    }

}

/**
 * @brief unsignedCharToString
 * @param str
 * @param num
 * @param length
 * @return
 */

uint8_t Convert::unsignedCharToString( char *str, uint8_t num, const uint8_t length )
{

    if( length > 0 )
    {
        str[0] = '\0';
    }
    num = num;
    return 0;

}

/**
 * @brief charToString
 * @param str
 * @param num
 * @param length
 * @return
 */

uint8_t Convert::charToString( char *str, int8_t num, const uint8_t length)
{
    if( length > 0 )
    {
        str[0] = '\0';
    }
    num = num;
    return 0;
}

/**
 * @brief unsignedShortToString
 * @param str
 * @param num
 * @param length
 * @return
 */

uint8_t Convert::unsignedShortToString( char *str, uint16_t num, const uint8_t length)
{
    if( length > 0 )
    {
        str[0] = '\0';
    }
    num = num;
    return 0;
}

/**
 * @brief shortToString
 * @param str
 * @param num
 * @param length
 * @return
 */

uint8_t Convert::shortToString( char *str, int16_t num, const uint8_t length)
{
    if( length > 0 )
    {
        str[0] = '\0';
    }
    num = num;
    return 0;
}

/**
 *
 * @brief stringToInteger
 *
 * @param str
 *
 * @return
 *
 */

int32_t Convert::toInteger( uint8_t* str )
{
    return atoi( (const char*) str );
}

/**
 *
 * @brief Convert::toInteger
 *
 * @param str
 *
 * @return
 *
 */

int32_t Convert::toInteger( char* str )
{
    return atoi( (const char*) str );
}

/**
 *
 * @brief Convert::toInteger
 *
 * @param str
 *
 * @return
 *
 */

int32_t Convert::toInteger( const char* str )
{
    return atoi( (const char*) str );
}

/**
 *
 * @brief Convert::toFloat
 *
 * @param str
 *
 * @return
 *
 */

int32_t Convert::toFloat( const char* str )
{
    return atof( (const char*) str );
}


/**
 * @brief Convert::unsignedIntegerToString
 * @param str
 * @param num
 * @param length
 * @return
 */

uint8_t Convert::unsignedIntegerToString( char *str, uint32_t num, const uint8_t length )
{

    uint8_t pt = 0;

    if( pt >= length ) return pt;

    uint8_t flag = 0;

    if( num >= 1000000000 )
    {
        flag = 1;
        uint8_t dec = 0;
        while(num >= 1000000000)
        {
            num -= 1000000000;
            dec += 1;
        }
        str[pt] = dec;
        str[pt] += ASCII_NUM_START;
        pt += 1;
    }

    if( pt >= length ) return pt;

    if( 1 == flag || num >= 100000000 )
    {
        flag = 1;
        uint8_t dec = 0;
        while(num >= 100000000)
        {
            num -= 100000000;
            dec += 1;
        }
        str[pt] = dec;
        str[pt] += ASCII_NUM_START;
        pt += 1;
    }

    if( pt >= length ) return pt;

    if( 1 == flag || num >= 10000000 )
    {
        flag = 1;
        uint8_t dec = 0;
        while(num >= 10000000)
        {
            num -= 10000000;
            dec += 1;
        }
        str[pt] = dec;
        str[pt] += ASCII_NUM_START;
        pt += 1;
    }

    if( pt >= length ) return pt;

    if( 1 == flag || num >= 1000000 )
    {
        flag = 1;
        uint8_t dec = 0;
        while(num >= 1000000)
        {
            num -= 1000000;
            dec += 1;
        }
        str[pt] = dec;
        str[pt] += ASCII_NUM_START;
        pt += 1;
    }

    if( pt >= length ) return pt;

    if( 1 == flag || num >= 100000 )
    {
        flag = 1;
        uint8_t dec = 0;
        while(num >= 100000)
        {
            num -= 100000;
            dec += 1;
        }
        str[pt] = dec;
        str[pt] += ASCII_NUM_START;
        pt += 1;
    }

    if( pt >= length ) return pt;

    if( 1 == flag || num >= 10000 )
    {
        flag = 1;
        uint8_t dec = 0;
        while(num >= 10000)
        {
            num -= 10000;
            dec += 1;
        }
        str[pt] = dec;
        str[pt] += ASCII_NUM_START;
        pt += 1;
    }

    if( pt >= length ) return pt;

    if( 1 == flag || num >= 1000 )
    {
        flag = 1;
        uint8_t dec = 0;
        while(num >= 1000)
        {
            num -= 1000;
            dec += 1;
        }
        str[pt] = dec;
        str[pt] += ASCII_NUM_START;
        pt += 1;
    }

    if( pt >= length ) return pt;

    if( 1 == flag || num >= 100 )
    {
        flag = 1;
        uint8_t dec = 0;
        while(num >= 100)
        {
            num -= 100;
            dec += 1;
        }
        str[pt] = dec;
        str[pt] += ASCII_NUM_START;
        pt += 1;
    }

    if( pt >= length ) return pt;

    if( 1 == flag || num >= 10 )
    {
        flag = 1;
        uint8_t dec = 0;
        while(num >= 10)
        {
            num -= 10;
            dec += 1;
        }
        str[pt] = dec;
        str[pt] += ASCII_NUM_START;
        pt += 1;
    }

    if( pt >= length ) return pt;

    str[pt] = num;
    str[pt] += ASCII_NUM_START;

    pt += 1;

    return pt;

}


/**
 *
 * @brief Convert::unsignedIntegerToHexString
 *
 * @param str
 * @param num
 * @param length
 *
 * @return
 *
 */

uint8_t Convert::unsignedIntegerToHexString( char *str, uint32_t num, const uint8_t length )
{

    uint8_t flag     = 0;
    uint8_t strIndex = 0;

    if( strIndex >= length ) return 0;

    if( ( flag == 1 ) || num >= 0x10000000 )
    {
        flag        = 1;
        uint8_t dec = 0;
        while( num >= 0x10000000 )
        {
            num -= 0x10000000;
            dec += 1;
        }
        str[strIndex] = hexToChar[dec];
        strIndex     += 1;
    }

    if( strIndex >= length ) return 0;

    if( ( flag == 1 ) || num >= 0x01000000 )
    {
        flag        = 1;
        uint8_t dec = 0;
        while( num >= 0x01000000 )
        {
            num -= 0x01000000;
            dec += 1;
        }
        str[strIndex] = hexToChar[dec];
        strIndex     += 1;
    }

    if( strIndex >= length ) return 0;

    if( ( flag == 1 ) || num >= 0x00100000 )
    {
        flag        = 1;
        uint8_t dec = 0;
        while( num >= 0x00100000 )
        {
            num -= 0x00100000;
            dec += 1;
        }
        str[strIndex] = hexToChar[dec];
        strIndex     += 1;
    }

    if( strIndex >= length ) return 0;

    if( ( flag == 1 ) || num >= 0x00010000 )
    {
        flag        = 1;
        uint8_t dec = 0;
        while( num >= 0x00010000 )
        {
            num -= 0x00010000;
            dec += 1;
        }
        str[strIndex] = hexToChar[dec];
        strIndex     += 1;
    }

    if( strIndex >= length ) return 0;

    if( ( flag == 1 ) || num >= 0x00001000 )
    {
        flag        = 1;
        uint8_t dec = 0;
        while( num >= 0x00001000 )
        {
            num -= 0x00001000;
            dec += 1;
        }
        str[strIndex] = hexToChar[dec];
        strIndex     += 1;
    }

    if( strIndex >= length ) return 0;

    if( ( flag == 1 ) || num >= 0x00000100 )
    {
        flag        = 1;
        uint8_t dec = 0;
        while( num >= 0x00000100 )
        {
            num -= 0x00000100;
            dec += 1;
        }
        str[strIndex] = hexToChar[dec];
        strIndex     += 1;
    }

    if( strIndex >= length ) return 0;

    if( ( flag == 1 ) || num >= 0x00000010 )
    {
        flag        = 1;
        uint8_t dec = 0;
        while( num >= 0x00000010 ) {
            num -= 0x00000010;
            dec += 1;
        }
        str[strIndex] = hexToChar[dec];
        strIndex     += 1;
    }

    if( strIndex >= length ) return 0;

    uint8_t dec = 0;

    while( num >= 1 )
    {
        num -= 1;
        dec += 1;
    }

    str[strIndex] = hexToChar[dec];
    strIndex     += 1;

    return strIndex;

}

/**

    \brief Перекодирует символы в строке содержащей URL

    \param str строка с URL над которой делается перекодировка

        Стандарт URL использует набор символов US-ASCII.
        Это имеет серьёзный недостаток, поскольку разрешается использовать лишь латинские буквы, цифры и несколько знаков пунктуации.
        Все другие символы необходимо перекодировать.
        Например, перекодироваться должны буквы кириллицы, буквы с диакритическими знаками, лигатуры, иероглифы.
        Перекодирующая кодировка описана в стандарте RFC 3986 и называется URL-encoding, URLencoded или percent‐encoding.

    Например, поданная на вход методу строка:

        https://ru.wikipedia.org/wiki/%D0%9C%D0%B8%D0%BA%D1%80%D0%BE%D0%BA%D1%80%D0%B5%D0%B4%D0%B8%D1%82

    Будет преобразована в:

        https://ru.wikipedia.org/wiki/Микрокредит

*/

void Convert::urlToUnicodeString( char *str )
{

    uint16_t strLength = strlen(str);

    for( uint16_t i = 0; ( i < strLength ) && ( str[i] != '\0' ) ; i++ )
    {

        if( str[i] == '%' )
        {

            if( str[i+1] != '\0' && ( str[i+2] != '\0') )
            {

                str[i] = hexStringToUnsignedChar( &(str[i+1]), 2 );

                // Преобразование зазерервированных символов

                switch (str[i])
                {
                    case 0x20 : str[i] = ' ';  break;
                    case 0x21 : str[i] = '!';  break;
                    case 0x22 : str[i] = '"';  break;
                    case 0x23 : str[i] = '#';  break;
                    case 0x24 : str[i] = '$';  break;
                    case 0x25 : str[i] = '%';  break;
                    case 0x26 : str[i] = '&';  break;
                    case 0x27 : str[i] = '\''; break;
                    case 0x2A : str[i] = '*';  break;
                    case 0x2C : str[i] = ',';  break;
                    case 0x3A : str[i] = ':';  break;
                    case 0x3B : str[i] = ';';  break;
                    case 0x3C : str[i] = '<';  break;
                    case 0x3D : str[i] = '=';  break;
                    case 0x3E : str[i] = '>';  break;
                    case 0x3F : str[i] = '?';  break;
                    case 0x5B : str[i] = '[';  break;
                    case 0x5D : str[i] = ']';  break;
                    case 0x5E : str[i] = '^';  break;
                    case 0x60 : str[i] = '`';  break;
                    case 0x7B : str[i] = '{';  break;
                    case 0x7C : str[i] = '|';  break;
                    case 0x7D : str[i] = '}';  break;
                }

                //

                uint16_t j;

                for( j = (i+1); j < (strLength-2) ; j++ )
                {

                    str[j] = str[j+2];

                }

                str[j] = '\0';

            }

         }

    }

}

/**

    @brief Преобразует unicode строку в url строку. Делает преобразование, обратное функции urlToUnicodeString

    @param str unicode строка
    @param outputStr выходная url строка
    @param outputStrLength максимальная длина выходной строки

    Например, поданная на вход методу строка:

        https://ru.wikipedia.org/wiki/Микрокредит

    Будет преобразована в:

        https://ru.wikipedia.org/wiki/%D0%9C%D0%B8%D0%BA%D1%80%D0%BE%D0%BA%D1%80%D0%B5%D0%B4%D0%B8%D1%82

*/

void Convert::unicodeStringToUrl(char* inputStr, char* outputStr, uint8_t outputStrLength )
{

    uint16_t j = 0;

    for( uint16_t i = 0 ; inputStr[i] != '\0' ; i++ )
    {

        if( inputStr[i] & 0x80 )
        {

            // Символ кодируется более чем одним байтом
            //  берем 2 байта

            outputStr[j] = '%';
            j += 1;

            Convert::unsignedCharToHexString( inputStr[i], &(outputStr[j]), outputStrLength - j );
            j += 2;
            i += 1;

            outputStr[j] = '%';
            j += 1;

            Convert::unsignedCharToHexString( inputStr[i], &(outputStr[j]), outputStrLength - j );
            j += 2;

        }
        else
        {

            uint8_t code = 0;

            switch ( inputStr[i] )
            {
                case ' '  : code = 0x20; break;
                case '!'  : code = 0x21; break;
                case '"'  : code = 0x22; break;
                case '#'  : code = 0x23; break;
                case '$'  : code = 0x24; break;
                case '%'  : code = 0x25; break;
                case '&'  : code = 0x26; break;
                case '\'' : code = 0x27; break;
                case '*'  : code = 0x2A; break;
                case ','  : code = 0x2C; break;
                case ':'  : code = 0x3A; break;
                case ';'  : code = 0x3B; break;
                case '<'  : code = 0x3C; break;
                case '='  : code = 0x3D; break;
                case '>'  : code = 0x3E; break;
                case '?'  : code = 0x3F; break;
                case '['  : code = 0x5B; break;
                case ']'  : code = 0x5D; break;
                case '^'  : code = 0x5E; break;
                case '`'  : code = 0x60; break;
                case '{'  : code = 0x7B; break;
                case '|'  : code = 0x7C; break;
                case '}'  : code = 0x7D; break;
            }

            if( code != 0 )
            {

                // Специальный символ

                outputStr[j] = '%';
                j += 1;

                Convert::unsignedCharToHexString( code, &(outputStr[j]), (outputStrLength - j) );

                j += 2;

            }
            else
            {

                outputStr[j] = inputStr[i];

                j+=1;

            }

        }

    }

    outputStr[j] = '\0';

}

/**

    @brief Преобразование строки в беззнаковое целое 8 битное число

    @param str входная строка
    @param num длина входной строки

    @return число

*/

uint8_t Convert::hexStringToUnsignedChar( const char *str, const uint8_t len )
{

    // Пропускаем ведущие нули, если они есть

    uint16_t i;

    for( i = 0 ; ( i < len ) && (str[i] != 0 ) ; i++ ){

        if( str[i] != '0' ) break;

    }

    //

    uint8_t outnum = 0;

    for( ; ( i < len ) && ( str[i] != '\0' ) ; i++ ){

        if( str[i] == ' ' ) continue;

        outnum <<= 4; // *16

        switch(str[i]){
            case '0' : outnum += 0;  break;
            case '1' : outnum += 1;  break;
            case '2' : outnum += 2;  break;
            case '3' : outnum += 3;  break;
            case '4' : outnum += 4;  break;
            case '5' : outnum += 5;  break;
            case '6' : outnum += 6;  break;
            case '7' : outnum += 7;  break;
            case '8' : outnum += 8;  break;
            case '9' : outnum += 9;  break;
            case 'A' : outnum += 10; break;
            case 'a' : outnum += 10; break;
            case 'B' : outnum += 11; break;
            case 'b' : outnum += 11; break;
            case 'C' : outnum += 12; break;
            case 'c' : outnum += 12; break;
            case 'D' : outnum += 13; break;
            case 'd' : outnum += 13; break;
            case 'E' : outnum += 14; break;
            case 'e' : outnum += 14; break;
            case 'F' : outnum += 15; break;
            case 'f' : outnum += 15; break;
            default : return 0; // Ошибка, в строке встречен недопустимый символ
        }

    }

    return outnum;

}

/**

    @brief Преобразование беззнакового целого 8 битного числа в строку

    @param num входное число
    @param str вывходная строка
    @param num максимальная длина выходной строки

*/

void Convert::unsignedCharToHexString( uint8_t num, char* str, const uint8_t len )
{

    if( num >= 16 )
    {
        uint8_t dec = 0;
        while( num >= 16 )
        {
            num -= 16;
            dec += 1;
        }
        str[0] = hexToChar[dec];
    }
    else
    {
        str[0] = '0';
    }

    if( 1 >= len )
    {

        str[0] = '\0';

        return;

    }

    str[1] = hexToChar[num];

    if( 2 >= len )
    {

        str[1] = '\0';

    }
    else
    {

        str[2] = '\0';

    }

}

/**
 *
 * @brief Получить значение байта из hex строки по номеру
 *
 * @param str 16 ричная строка следующего вида: C8600067B16D
 * @param byteNumber номер запрашиваемого байта, 0-5
 *                    например: 0 - C8
 *                              1 - 60
 *                               ...
 *                              5 - 6D
 * @return значение байта
 *
 */

uint8_t Convert::getByteFromHexString( const char* str, uint8_t byteNumber )
{

    return hexStringToUnsignedChar( str + ( byteNumber * 2 ), 2 );

}

/**
 *
 * @brief Получить значение байта из строки с ip адресом
 *
 * @param str строка с ip адресом вида: "192.168.38.2"
 * @param byteNumber номер запрашиваемого байта, 0-3
 *                    например: 0 - 192
 *                              1 - 168
 *                              2 -  38
 *                              3 -   2
 * @return значение байта
 *
 */

uint16_t Convert::getNumberFromIpString( const char* str, uint8_t byteNumber )
{

    if( str == 0 )
    {
        return 0;
    }

    uint8_t start;

    for( start = 0 ; ( str[start] != 0 ) && ( byteNumber > 0 ) ; start++ )
    {
        if( str[start] == '.' )
        {
            byteNumber -= 1;
        }
    }

    char     buffer[4]; // 255\0
    uint8_t  i = 0;

    for( ; ( i < 3 ) && ( str[start] != '.' )  && ( str[start] != '\0' ) ; start++, i++ )
    {
        buffer[i] = str[start];
    }

    buffer[i] = '\0';

    return atoi(buffer);

}

const char Convert::hexToChar[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
