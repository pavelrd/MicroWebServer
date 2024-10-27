#ifndef HTTPHEADER_HPP
#define HTTPHEADER_HPP

#include "platform.hpp"
#include "crc.hpp"

#include "magic.hpp"
#include "convert.hpp"

/**
 *
 * @brief Обработка заголовка http запроса
 *
 *    На входе заголовок - на выходе данные заголовка
 *
*/

class HttpHeader
{

public:

    HttpHeader();

    void clear();

    void put( uint8_t symbol );

    uint8_t get();
    bool    isReadyRead();

    bool isParameter();
    bool isCookie();

    bool isFilled();
    bool isError();

    uint32_t getParameterNameHash();
    char*    getParameterValue();

    uint32_t getCookieNameHash();
    char*    getCookieValue();

    enum LENGTHS
    {
        STRING_BUFFER_LENGTH = 2048,
        POST_BOUNDARY_LENGTH = 128
    };

    enum METHOD
    {
        GET_METHOD,
        POST_METHOD,
        OPTIONS_METHOD,
        HEAD_METHOD,
        PUT_METHOD,
        DELETE_METHOD,
        TRACE_METHOD,
        UNKNOWN_METHOD
    } ;

    enum VERSION
    {
        HTTP_1_0,
        HTTP_1_1,
        HTTP_2_0,
        UNKNOWN_VERSION
    };

    METHOD       type;              // Метод http
    VERSION      protocolVersion;   // Версия протокола
    uint32_t     resourceNameHash;  // Имя запрашиваемого ресурса(Хэш от имени)
    bool         isChunked;         // Ресурс запрашивается частично
    uint32_t     chunkStartByte;    // Начальный байт частично запрашиваемого ресурса
    uint32_t     chunkEndByte;      // Конечный байт частично запрашиваемого ресурса
    bool         isCloseConnection; // Закрыть соединение после ответа на запрос
    bool         isCache;           // Ресурс кэшируемый.
    uint32_t     contentLength;     // Длина передаваемых данных(POST запрос)
    Magic::TYPE  contentType;       // Тип файла
    char         postBoundary[POST_BOUNDARY_LENGTH]; // Разделитель для POST запроса

    typedef enum  {
        GZIP    = 0x01,
        DEFLATE = 0x02
    } ENCODING;

 private:

    enum STATE
    {

        FIND_START, // Поиск начала запроса, пропуск всех \r\n и пробелов
        PARSE_METHOD,      // Распознавание запроса
        PARSE_NAME,         // Запрос найден и распознан, распознавание имени запрашиваемого ресурса
        PARSE_CGI_NAME,
        PARSE_CGI_VALUE,
        PARSE_CGI_TOKEN_COMPLETED,
        PARSE_CGI_COMPLETED,
        PARSE_PROTO,        // Имя найдено и распознано, распознавание протокола
        PARSE_ENDLINE,      // Обработка конца строки/запроса

        // -------
        PARSE_TAGNAME,
        PARSE_TAGVALUE,
        PARSE_COOKIE_TOKEN_COMPLETED,
        HEADER_FILLED,

        // ------- Ошибки

        ERROR_VALUE_BUFFER_OVERFLOW,
        ERROR_BAD_ENDLINE

    };

    enum SEND_MODE
    {
        SEND_MODE_CHUNK,
        SEND_MODE_FILE,
        SEND_MODE_SSI
    };

    // ------- Параметры для распознавания запроса

    STATE currentParseState;

    uint32_t hashBuffer;
    char     stringBuffer[STRING_BUFFER_LENGTH];
    uint16_t stringBufferIndex;

    bool cookieExpire;

    // -------

    void _accept              ( char *value );
    void _accept_charset      ( char *value );
    void _accept_encoding     ( char *value );
    void _accept_language     ( char *value );
    void _accept_ranges       ( char *value );
    void _age                 ( char *value );
    void _allow               ( char *value );
    void _authorization       ( char *value );
    void _cache_control       ( char *value );
    void _connection          ( char *value );
    void _content_base        ( char *value );
    void _content_encoding    ( char *value );
    void _content_language    ( char *value );
    void _content_length      ( char *value );
    void _content_location    ( char *value );
    void _content_md5         ( char *value );
    void _content_range       ( char *value );
    void _content_type        ( char *value );
    void _date                ( char *value );
    void _etag                ( char *value );
    void _expires             ( char *value );
    void _from                ( char *value );
    void _host                ( char *value );
    void _if_modified_since   ( char *value );
    void _if_match            ( char *value );
    void _if_none_match       ( char *value );
    void _if_range            ( char *value );
    void _if_unmodified_since ( char *value );
    void _last_modified       ( char *value );
    void _location            ( char *value );
    void _max_forwards        ( char *value );
    void _pragma              ( char *value );
    void _proxy_authenticate  ( char *value );
    void _proxy_authorization ( char *value );
    void _public              ( char *value );
    void _range               ( char *value );
    void _referer             ( char *value );
    void _retry_after         ( char *value );
    void _server              ( char *value );
    void _transfer_encoding   ( char *value );
    void _upgrade             ( char *value );
    void _useragent           ( char *value );
    void _vary                ( char *value );
    void _via                 ( char *value );
    void _warning             ( char *value );
    void _www_authenticate    ( char *value );
    void _dnt                 ( char *value );
    void _cookie              ( char *value );

};
#endif // HTTPREQUESTHEADER_HPP
