#include "httpheader.hpp"

#include <cstring>
#include <stdlib.h>

/**
 *
 * @brief Инициализация
 *
 */

HttpHeader::HttpHeader()
{

    clear();

}

/**
 *
 * @brief Очистка для начала нового распознавания заголовка
 *
 */

void HttpHeader::clear()
{

    currentParseState = FIND_START;

}

/**
 *
 * @brief Поместить следующий символ заголовка на обработку
 *
 * @param symbol символ(код символа)
 *
 */

void HttpHeader::put( uint8_t symbol )
{

    if( ( currentParseState == ERROR_VALUE_BUFFER_OVERFLOW ) ||
        ( currentParseState == ERROR_BAD_ENDLINE )           ||
        ( currentParseState == HEADER_FILLED ) )
    {

        return;

    }
    else if( PARSE_CGI_TOKEN_COMPLETED == currentParseState )
    {

        hashBuffer = 0;

        currentParseState = PARSE_CGI_NAME;

    }
    else if( PARSE_CGI_COMPLETED == currentParseState )
    {

        hashBuffer = 0;

        currentParseState = PARSE_PROTO;

    }

    if( FIND_START == currentParseState )
    {

        //  Пропуск всех \r\n и пробелов

        if( ( symbol == '\r' ) && ( symbol == '\n' ) && ( symbol == ' ' ) )
        {
            return;
        }

        // Встречен символ запроса

        type              = UNKNOWN_METHOD;
        protocolVersion   = UNKNOWN_VERSION;
        resourceNameHash  = 0;
        isChunked         = false;
        chunkStartByte    = 0;
        chunkEndByte      = 0;
        isCloseConnection = false;
        isCache           = false;
        contentLength     = 0;
        contentType       = Magic::PLAIN;
        postBoundary[0]   = '\0';

        cookieExpire = false;

        hashBuffer = CRC::crc32( 0, &symbol, 1 );

        currentParseState = PARSE_METHOD;


    }
    else if( PARSE_METHOD == currentParseState )
    {

        // Поиск метода запроса - GET,POST,OPTIONS...etc

        if( symbol == ' ' )
        {

            switch (hashBuffer)
            {
                case 0x6b9c6cfa : type = GET_METHOD;     break; // 0x6b9c6cfa - "GET" hash
                case 0x6c1f8139 : type = POST_METHOD;    break; // 0x6c1f8139 - "POST" hash
                case 0xef70049d : type = OPTIONS_METHOD; break; // 0xef70049d - "OPTIONS" hash
                case 0x91661b28 : type = HEAD_METHOD;    break; // 0x91661b28 - "HEAD" hash
                case 0x3837cb5e : type = PUT_METHOD;     break; // 0x3837cb5e - "PUT" hash
                case 0xcaeabb01 : type = DELETE_METHOD;  break; // 0xcaeabb01 - "DELETE" hash
                case 0xc60f1711 : type = TRACE_METHOD;   break; // 0x6b9c6cfa - "TRACE" hash
                default : type = UNKNOWN_METHOD;
            }

            resourceNameHash    = 0;

            currentParseState   = PARSE_NAME;

        }
        else
        {

            hashBuffer = CRC::crc32( hashBuffer, &symbol, 1 );

        }

    }
    else if( PARSE_NAME == currentParseState )
    {

        // Поиск имени запрашиваемого ресурса - /example.html, /, page.html

        if( symbol == '?' )
        {

            // Распознавание имени ресурса закончено, так как был найден cgi запрос

            hashBuffer = 0;

            currentParseState = PARSE_CGI_NAME;

        }
        else if( symbol == ' ' )
        {

            // Полностью закончено распознавание имени

            hashBuffer = 0;

            currentParseState = PARSE_PROTO;

        }
        else
        {

            // Копирование следующего байта(символа) имени

            resourceNameHash = CRC::crc32( resourceNameHash, &symbol, 1 );

        }

    }
    else if( PARSE_CGI_NAME == currentParseState )
    {

        if( symbol == '=' )
        {

            // Нашли имя, ищем значение

            stringBuffer[0]   = '\0';
            stringBufferIndex =   0 ;

            currentParseState = PARSE_CGI_VALUE;

        }
        else if ( symbol == ' ' )
        {

            stringBuffer[0]   = '\0';

            currentParseState = PARSE_CGI_COMPLETED;

        }
        else
        {

            hashBuffer = CRC::crc32( hashBuffer, &symbol, 1 );

        }

    }
    else if( PARSE_CGI_VALUE == currentParseState )
    {

        // Распознавание строки cgi запроса
        //  ?..=..&... HTTP/1.1\r\n

        if( symbol == ' ' )
        {

            stringBuffer[stringBufferIndex]  = '\0';

            Convert::urlToUnicodeString( &(stringBuffer[0]) );

            // Cтрока с cgi запросом распознана
            //  переход к распознаванию протокола

            currentParseState = PARSE_CGI_COMPLETED;

        }
        else if( symbol == '&' )
        {

            stringBuffer[stringBufferIndex]  = '\0';

            Convert::urlToUnicodeString( &(stringBuffer[0]) );

            currentParseState = PARSE_CGI_TOKEN_COMPLETED;

        }
        else
        {

            stringBuffer[stringBufferIndex]  = symbol;

            stringBufferIndex               += 1;

            if( stringBufferIndex >= (STRING_BUFFER_LENGTH-1) )
            {

                stringBufferIndex -= 1;

            }

        }

    }
    else if( PARSE_PROTO == currentParseState )
    {

        // Поиск протокола - HTTP/1.1 или HTTP/1.0

        if( symbol == '\r' )
        {

            // Поиск протокола завершен

            if ( hashBuffer == 0x17ea47f0 )
            { // "HTTP/1.0"

                protocolVersion = HTTP_1_0;

            }
            else if( hashBuffer == 0x60ed7766 )
            { // "HTTP/1.1"

                protocolVersion = HTTP_1_1;

            }
            else
            {

                protocolVersion = UNKNOWN_VERSION;

            }

            hashBuffer = CRC::crc32( 0, &(symbol), 1 );

            currentParseState = PARSE_ENDLINE;

        }
        else
        {

           hashBuffer = CRC::crc32( hashBuffer, (uint8_t*)&(symbol), 1 );

        }

    }
    else if( ( PARSE_ENDLINE == currentParseState ) || ( currentParseState == PARSE_COOKIE_TOKEN_COMPLETED ) )
    {

        // Поиск конца строки/запроса

        if( ( symbol == '\r' ) || ( symbol == '\n' ) )
        {

            hashBuffer = CRC::crc32( hashBuffer, &(symbol), 1 );

            if( hashBuffer == 0x8bc21544 ) // '\r\n\r\n'
            {

                hashBuffer = 0;

                currentParseState = HEADER_FILLED;

            }

        } else {

            if( hashBuffer == 0x14a285ac ) // '\r\n'
            {

                hashBuffer = CRC::crc32( 0, &(symbol), 1 );

                currentParseState = PARSE_TAGNAME;

            }
            else
            {

                currentParseState = ERROR_BAD_ENDLINE;

            }

        }

    }
    else if( PARSE_TAGNAME == currentParseState )
    {

        if( symbol == ':' )
        {

            stringBuffer[0] = '\0';

            stringBufferIndex = 0;

            currentParseState = PARSE_TAGVALUE;

        }
        else
        {

            hashBuffer = CRC::crc32( hashBuffer, &symbol, 1 );

        }

    }
    else if( PARSE_TAGVALUE == currentParseState )
    {

        if( ( symbol == '\r' ) || ( symbol == '\n' ) )
        {

            stringBuffer[stringBufferIndex] = '\0';

            // Значение тега полностью найдено

            switch ( hashBuffer )
            {
                case 0xb48ce802 : _accept( &(stringBuffer[0]) )              ; break ; // "Accept"
                case 0x147b1563 : _accept_charset( &(stringBuffer[0]) )      ; break ; // "Accept-Charset"
                case 0xd185f29d : _accept_encoding( &(stringBuffer[0]) )     ; break ; // "Accept-Encoding"
                case 0x6d167a67 : _accept_language( &(stringBuffer[0]) )     ; break ; // "Accept-Language"
                case 0x1a7b9e4a : _accept_ranges( &(stringBuffer[0]) )       ; break ; // "Accept-Ranges"
                case 0x997d5652 : _age( &(stringBuffer[0]) )                 ; break ; // "Age"
                case 0xbf886fec : _allow( &(stringBuffer[0]) )               ; break ; // "Allow"
                case 0xc913c01a : _authorization( &(stringBuffer[0]) )       ; break ; // "Authorization"
                case 0x7c4f56f8 : _cache_control( &(stringBuffer[0]) )       ; break ; // "Cache-Control"
                case 0x66aa70b6 : _connection( &(stringBuffer[0]) )          ; break ; // "Connection"
                case 0x36c7d4cd : _content_base( &(stringBuffer[0]) )        ; break ; // "Content-Base"
                case 0xae3e1d8  : _content_encoding( &(stringBuffer[0]) )    ; break ; // "Content-Encoding"
                case 0xb6706922 : _content_language( &(stringBuffer[0]) )    ; break ; // "Content-Language"
                case 0xd87452f2 : _content_length( &(stringBuffer[0]) )      ; break ; // "Content-Length"
                case 0x3c35915c : _content_location( &(stringBuffer[0]) )    ; break ; // "Content-Location"
                case 0xb7e43896 : _content_md5( &(stringBuffer[0]) )         ; break ; // "Content-MD5"
                case 0x4c11c6a0 : _content_range( &(stringBuffer[0]) )       ; break ; // "Content-Range"
                case 0x7aad7d85 : _content_type( &(stringBuffer[0]) )        ; break ; // "Content-Type"
                case 0xaac9844  : _date( &(stringBuffer[0]) )                ; break ; // "Date"
                case 0x490b5f62 : _etag( &(stringBuffer[0]) )                ; break ; // "ETag"
                case 0x55215110 : _expires( &(stringBuffer[0]) )             ; break ; // "Expires"
                case 0x19280e4e : _from( &(stringBuffer[0]) )                ; break ; // "From"
                case 0x6f15bcc3 : _host( &(stringBuffer[0]) )                ; break ; // "Host"
                case 0x4c2c1a6c : _if_modified_since( &(stringBuffer[0]) )   ; break ; // "If-Modified-Since"
                case 0x7d1c7484 : _if_match( &(stringBuffer[0]) )            ; break ; // "If-Match"
                case 0x66c775a0 : _if_none_match( &(stringBuffer[0]) )       ; break ; // "If-None-Match"
                case 0x94c0ebc8 : _if_range( &(stringBuffer[0]) )            ; break ; // "If-Range"
                case 0xebe8cf92 : _if_unmodified_since( &(stringBuffer[0]) ) ; break ; // "If-Unmodified-Since"
                case 0xb357ebc2 : _last_modified( &(stringBuffer[0]) )       ; break ; // "Last-Modified"
                case 0xa7e8eb9d : _location( &(stringBuffer[0]) )            ; break ; // "Location"
                case 0x51055c1f : _max_forwards( &(stringBuffer[0]) )        ; break ; // "Max-Forwards"
                case 0x1aba131c : _pragma( &(stringBuffer[0]) )              ; break ; // "Pragma"
                case 0xe01c36ba : _proxy_authenticate( &(stringBuffer[0]) )  ; break ; // "Proxy-Authenticate"
                case 0x9d3bead3 : _proxy_authorization(&(stringBuffer[0]) )  ; break ; // "Proxy-Authorization"
                case 0x3c182b2b : _public( &(stringBuffer[0]) )              ; break ; // "Public"
                case 0x5246754d : _range( &(stringBuffer[0]) )               ; break ; // "Range"
                case 0x9a610f19 : _referer( &(stringBuffer[0]) )             ; break ; // "Referer"
                case 0x73c40b41 : _retry_after( &(stringBuffer[0]) )         ; break ; // "Retry-After"
                case 0x5dc1d0c0 : _server( &(stringBuffer[0]) )              ; break ; // "Server"
                case 0x80796fda : _transfer_encoding( &(stringBuffer[0]) )   ; break ; // "Transfer-Encoding"
                case 0x78db4d86 : _upgrade( &(stringBuffer[0]) )             ; break ; // "Upgrade"
                case 0xc3fe7db  : _useragent( &(stringBuffer[0]) )           ; break ; // "User-Agent"
                case 0xb2e7fc99 : _vary( &(stringBuffer[0]) )                ; break ; // "Vary"
                case 0x19fa0a30 : _via( &(stringBuffer[0]) )                 ; break ; // "Via"
                case 0x8ff3a55a : _warning( &(stringBuffer[0]) )             ; break ; // "Warning"
                case 0x17a3ca99 : _www_authenticate( &(stringBuffer[0]) )    ; break ; // "WWW-Authenticate"
                case 0x8a2e0b68 : _dnt( &(stringBuffer[0]) )                 ; break ; // "DNT"
                case 0x8d4cbf50 : _cookie( &(stringBuffer[0]) )              ; break ; // "Cookie"
                default: break;
            }

            if( hashBuffer == 0x8d4cbf50 )
            {

                hashBuffer = CRC::crc32( 0, &symbol, 1 );

                currentParseState = PARSE_COOKIE_TOKEN_COMPLETED;

            }
            else
            {

                hashBuffer = CRC::crc32( 0, &symbol, 1 );

                currentParseState = PARSE_ENDLINE;

            }

        }
        else
        {

            stringBuffer[stringBufferIndex] = symbol;

            stringBufferIndex += 1;

            if( stringBufferIndex >= ( STRING_BUFFER_LENGTH - 1 ) )
            {

                stringBufferIndex -= 1;

            }

        }

    }

}

/**
 *
 * @brief Получить следующий символ заголовка(не реализовано)
 *
 * @return всегда 0
 *
 */

uint8_t HttpHeader::get()
{

    // Преобразуем заголовок в строку
    //  и выводим следующий байт строки

    return 0;

}

/**
 *
 * @brief Готовность к чтению следующего байта заголовка(не реализовано)
 *
 * @return всегда true
 *
 */

bool HttpHeader::isReadyRead()
{

    return true;

}

/**
 *
 * @brief Проверяет был ли полностью заполнен заголовок
 *
 * @return true - заполнен, false - заполняется, нужны новые данные
 *
 */

bool HttpHeader::isFilled()
{

    if( currentParseState == HEADER_FILLED )
    {
        return true;
    }
    else
    {
        return false;
    }

}

/**
 *
 * @brief Произошла ли ошибка при распознавании заголовка
 *
 * @return true - ошибка, false - ошибки нет
 *
 */

bool HttpHeader::isError()
{

    if( ( currentParseState == ERROR_BAD_ENDLINE ) || ( currentParseState == ERROR_VALUE_BUFFER_OVERFLOW ) ){
        return true;
    }
    else
    {
        return false;
    }

}

/**
 *
 * @brief Происходит ли сейчас распознавание параметра http запроса
 *
 * @return true - да, необходимо обработать параметр, false - нет
 *
 */


bool HttpHeader::isParameter()
{

    if( ( currentParseState == PARSE_CGI_TOKEN_COMPLETED ) || ( currentParseState == PARSE_CGI_COMPLETED ) )
    {
        return true;
    }
    else
    {
        return false;
    }

}

/**
 *
 * @brief Происходит ли сейчас распознавание cookies
 *
 * @return true - да, необходимо обработать cookies, false - нет
 *
 */

bool HttpHeader::isCookie()
{

    if( currentParseState == PARSE_COOKIE_TOKEN_COMPLETED ){
        return true;
    } else {
        return false;
    }

}

/**
 *
 * @brief Получить имя параметра http запроса
 *
 * @return хэш сумма имени параметра
 *
 */

uint32_t HttpHeader::getParameterNameHash()
{
    return hashBuffer;
}

/**
 *
 * @brief Получить значение параметра http запроса
 *
 * @return строка со значением запроса
 *
 */

char* HttpHeader::getParameterValue()
{

    return &(stringBuffer[0]);

}

/**
 *
 * @brief Получить имя cookie
 *
 * @return хэш сумма имени cookie
 *
 */

uint32_t HttpHeader::getCookieNameHash()
{
    return hashBuffer;
}

/**
 *
 * @brief Получить значение cookie
 *
 * @return строка со значением cookie
 *
 */

char* HttpHeader::getCookieValue()
{
    // urlDecode
    return &(stringBuffer[0]);
}

// ------------------------------------------------
// ------- Функции обработчики отдельных тэгов

void HttpHeader::_accept( char* data )
{
    data = data;
}

void HttpHeader::_accept_charset( char* data )
{
    data = data;
}

void HttpHeader::_accept_encoding( char* data )
{
    data = data;
}

void HttpHeader::_accept_language( char* data )
{
    data = data;
}

void HttpHeader::_accept_ranges( char* data )
{
    data = data;
}

void HttpHeader::_age( char* data )
{
    data = data;
}

void HttpHeader::_allow( char* data )
{
    data = data;
}

void HttpHeader::_authorization( char* data )
{
    data = data;
}

void HttpHeader::_cache_control( char* data )
{

    // no-cache

    // info->cache =

    // HTTP_CACHE_CONTROL_CACHE
    // HTTP_CACHE_CONTROL_NO_CACHE

    data = data;

}

void HttpHeader::_connection( char* data ){

    uint16_t i = 0;

    while(  (data[i] == ' ') && (data[i] != '\0') ){
        i += 1;
    }

    if ( 0 == strcmp( &(data[i]), "close" ) ) {

        isCloseConnection = true;

    }

}

void HttpHeader::_content_base( char* data )
{
    data = data;
}

void HttpHeader::_content_encoding( char* data )
{
    data = data;
}

void HttpHeader::_content_language( char* data )
{
    data = data;
}

void HttpHeader::_content_length( char* data )
{

    uint16_t i = 0;

    while(  (data[i] == ' ') && (data[i] != '\0') ){
        i += 1;
    }

    if( strlen( &(data[i]) ) < 10 ){

        contentLength = atoi( &(data[i]) );

    }

}

void HttpHeader::_content_location( char* data )
{
    data = data;
}

void HttpHeader::_content_md5( char* data )
{
    data = data;
}

void HttpHeader::_content_range( char* data )
{

    //
    data = data;

}

void HttpHeader::_content_type( char* data )
{

//  multipart/form-data; boundary=----WebKitFormBoundaryX60OvhSiaJfOxSrf

    // Убираем пробелы из начала строки

    while(  ( *data == ' ') && ( *data != '\0') )
    {
        data += 1;
    }

    char* formDataString = data;

    while( ( *data != ';') && ( *data != '\0') )
    {
        data += 1;
    }

    *data = '\0';

    contentType = Magic::getContentTypeFromString( formDataString );

    *data = ' ';

    while( (*data == ' ') && (*data != '\0') )
    {
        data += 1;
    }

    while( ( *data != '=' ) && ( *data != '\0' ) )
    {
        data += 1;
    }

    if( *data != '=' )
    {
        return;
    }
    else
    {
        data += 1;
    }

    int i = 0;  // пропуск "boundary="

    while ( data[i] != '\0' ) // POST_BOUNDARY_LENGTH
    {
        if( (i+1) == POST_BOUNDARY_LENGTH )
        {
            break;
        }
        postBoundary[i] = data[i];
        i++;
    }

    postBoundary[i] = '\0';

/*
    if ( 0 == strncmp( &(data[i]), "boundary=", 9 ) ){

        i += 9;

        postBoundaryHash = 0;

        while( data[i] != '\0' ){

            postBoundaryHash = CRC::crc32( postBoundary, (uint8_t*)&(data[i]), 1 );

            i += 1;

        }

    }
*/

}

void HttpHeader::_date( char *data )
{
    data = data;
}

void HttpHeader::_etag( char *data )
{
    data = data;
}

void HttpHeader::_expires(  char *data )
{
    data = data;
}

void HttpHeader::_from(  char *data )
{
    data = data;
}

void HttpHeader::_host( char *data )
{
    data = data;
}

void HttpHeader::_if_modified_since( char *data  )
{
    data = data;
}

void HttpHeader::_if_match(  char *data )
{
    data = data;
}

void HttpHeader::_if_none_match( char *data )
{
    data = data;
}

void HttpHeader::_if_range( char *data  )
{
    data = data;
}

void HttpHeader::_if_unmodified_since( char *data )
{
    data = data;
}

void HttpHeader::_last_modified( char *data )
{
    data = data;
}

void HttpHeader::_location( char *data )
{
    data = data;
}

void HttpHeader::_max_forwards( char *data )
{
    data = data;
}

void HttpHeader::_pragma( char *data )
{
    data = data;
}

void HttpHeader::_proxy_authenticate( char *data )
{
    data = data;
}

void HttpHeader::_proxy_authorization( char *data )
{
    data = data;
}

void HttpHeader::_public( char *data )
{
    data = data;
}

void HttpHeader::_range( char *data )
{

    data = data;

    // Желаемый диапазон данных, которые хочет получить клиент

    // start-end || start-max || min-end

    // Range: bytes=12345-
/*
    chunkStartByte = 0;
    chunkEndByte   = 0;

    info->byte_end = 0;

    // Пропуск всех пробелов

    for(  uint32_t i = 0 ; ( i < len ) && ( data[i] == ' ' ) ; i++ ){;}

    if( ( (i-len) > (sizeof("bytes=")) ) && ( 0 == a_strncmp(&data[i],"bytes=",sizeof("bytes=")-1) ) ) {

        i += sizeof("bytes=")-1;

        if( data[i] == '-' ){

            // Range: bytes=-12345

            for( uint32_t k = 0 ; i < len ; k++, i++ ){

                if( data[i] < '0' || data[i] > '9' ) {

                    info->byte_end = a_atoui(&data[i-k],k);

                    info->byte_range = 1;

                    return;

                }

            }

        } else {

            // Range: bytes=12345-34758
            // Range: bytes=12345-

            for( uint32_t k = 0 ; i < len ; k++, i++ ){

                if( data[i] == '-' ){

                    info->byte_start = a_atoui(&data[i-k],k);

                    info->byte_range = 1;

                    i += 1;

                    break;

                }

            }

            for( k = 0 ; i < len ; k++, i++ ){

                if( data[i] < '0' || data[i] > '9' ){

                    info->byte_end = a_atoui(&data[i-k],k);

                    i += 1;

                    break;

                }


            }

        }

    } else {

        return;

    }
*/

}

// prev - сколько данных было обработано ранее
// now  - сколько данных поступило

void HttpHeader::_referer( char *data )
{
    data = data;
}

void HttpHeader::_retry_after( char *data )
{
    data = data;
}

void HttpHeader::_server( char *data )
{
    data = data;
}

void HttpHeader::_transfer_encoding( char *data )
{
    data = data;
}

void HttpHeader::_upgrade( char *data )
{
    data = data;
}

void HttpHeader::_useragent( char *data )
{
    data = data;
}

void HttpHeader::_vary( char *data )
{
    data = data;
}

void HttpHeader::_via( char *data )
{
    data = data;
}

void HttpHeader::_warning( char *data )
{
    data = data;
}

void HttpHeader::_www_authenticate( char *data )
{
    data = data;
}

void HttpHeader::_dnt( char *data )
{
    data = data;
}

void HttpHeader::_cookie( char *data )
{

    data = data;

    // Cookie: name=value; name1=value1; name2=value2;

    /*

    if( info->lastTagCount == 0 ){

        // Начало обработки значения

        info->cookie_state = 0;
        info->cookie_nowcount = 0;
        info->cookie_count = 0;

    }

    uint32_t i = 0;

    while( i < len ){

        if( info->cookie_state == 0 ) {

            // Пропуск всех пробелов

            for(  ; ( i < len ) && ( data[i] == ' ' ) ; i++ ){;}

            // Записывание имени cookie

            for(  ; ( i < len ) && ( info->cookie_nowcount < HTTP_COOKIE_NAME_LENGTH ) ; i++, info->cookie_nowcount += 1 ){

                if( data[i] == '=' ){

                    info->cookie_state = 1;

                    info->cookies[info->cookie_count].name[info->cookie_nowcount] = '\0';

                    info->cookies[info->cookie_count].value[0] = '\0';

                    info->cookie_nowcount = 0;

                    info->cookie_count += 1;

                    i += 1;

                    break;

                } else {

                    info->cookies[info->cookie_count].name[info->cookie_nowcount] = data[i];

                }

          }

        }

        if( info->cookie_state == 1 ) {

            for( ; ( i < len ) && ( info->cookie_nowcount < HTTP_COOKIE_VALUE_LENGTH ) ; i++, info->cookie_nowcount += 1 ){

                if( data[i] == ';' ){

                    info->cookie_state = 0;

                    info->cookie_nowcount = 0;

                    i += 1;

                    break;

                } else {

                    info->cookies[(info->cookie_count)-1].value[info->cookie_nowcount]   = data[i];
                    info->cookies[(info->cookie_count)-1].value[info->cookie_nowcount+1] = '\0';

                }

            }

        }

    }
    */

}
