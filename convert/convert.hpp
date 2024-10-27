#ifndef NUMBERS_HPP
#define NUMBERS_HPP

#include "platform.hpp"

/**
 *  \brief Преобразование:

        Сток в числа и чисел в строки
        URL(percent-encoding) в unicode строку и обратно
*/

class Convert
{

public:

    static uint8_t     floatToString( char* string, float number, const uint8_t stringLength );

    static uint8_t     integerToString( char *str, int32_t num, const uint8_t length);
    static uint8_t     unsignedCharToString( char *str, uint8_t num, const uint8_t length);
    static uint8_t     charToString( char *str, int8_t num, const uint8_t length);
    static uint8_t     unsignedShortToString( char *str, uint16_t num, const uint8_t length);
    static uint8_t     shortToString( char *str, int16_t num, const uint8_t length);

    static int32_t     toInteger( uint8_t* str );
    static int32_t     toInteger( char* str );
    static int32_t     toInteger( const char* str );
    static int32_t     toFloat( const char* str );

    static uint8_t     unsignedIntegerToString( char *str, uint32_t num, const uint8_t length );

    static uint8_t     unsignedIntegerToHexString( char *str, uint32_t num, const uint8_t length );

    static void        unsignedCharToHexString( uint8_t num, char* str, const uint8_t len );

    static uint8_t     hexStringToUnsignedChar( const char *str, const uint8_t len );

    static void        urlToUnicodeString( char* str );

    static void        unicodeStringToUrl( char* inputStr, char* outputStr, uint8_t ouputStrLength );

    static uint8_t     getByteFromHexString( const char* str, uint8_t byteNumber );

    static uint16_t    getNumberFromIpString( const char* str, uint8_t byteNumber );

private:

    enum CONSTANTS
    {
        ASCII_NUM_START = 0x30
    };

    static const char hexToChar[16];

};

#endif
