#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "platform.hpp"
#include "filesystem.hpp"
#include "ssi.hpp"
#include "magic.hpp"
#include "network.hpp"
#include "httpheader.hpp"
#include <string.h>

struct WebParameter
{
    uint32_t hash;
    char*    value;
};

struct WebParametersNode
{
    WebParameter data;
    WebParametersNode *next;
};

class WebParameters
{

protected:

    int _size;

    WebParametersNode *root;
    WebParametersNode *last;

    // Helps "get" method, by saving last position
    WebParametersNode *lastNodeGot;
    int lastIndexGot;
    // isCached should be set to FALSE
    // everytime the list suffer changes
    bool isCached;

    WebParametersNode* getNode(int index);

public:

    WebParameters();

    ~WebParameters();

    int size();

    bool add(WebParameter);

    WebParameter get(int index);

    const char* operator[](const char* str)
    {


        uint32_t hash = CRC::crc32( 0, (uint8_t*)str, strlen(str) );

        //

        for( int index = 0; index < _size; index++ )
        {

            WebParameter temp = get(index);

            if( temp.hash == hash )
            {
                return temp.value;
            }

        }

        return 0;

    }

};

/**
 *
 * @brief Реализация http web сервера
 *
 */

class WebServer
{

public:

    typedef bool (*pageHandler)( Connection& connection, WebParameters& value );

    WebServer();

    void setDatabase(  Database&   _database  );
    void setFilesytem( Filesystem& _filesytem );
    void setPort( uint16_t _port );
    void setStackSize( uint32_t _stackSize );

    RSTATUS addHandler( const char* pagename, const char* request, pageHandler handler );
    RSTATUS addPostHandler( const char* pagename, const char* request, pageHandler handler );

    void start();
    void stop();

    void showHandlers();

    enum HANDLERS_NAMES
    {
        HANDLER_ACCEPT_ALL_PARAMETERS = 0
    };

private:

    Database*   database;
    Filesystem* filesystem;
    uint16_t    port;
    uint32_t    stackSize;
    Database    handlers;
    Database    postHandlers;

    uint32_t mainHandlerThreadId;

    static void _mainThread( void* server );
    static void _connectionHandler( void* _connection );
    static bool _isHasGetParametersHandlers(Database* database, WebParameters *parameters );
    static bool _processGetParameters( Database* database, Connection* connection, WebParameters* parameters );
    static void _clearParameters(  WebParameters* parameters );
    static bool _sendPageOrFile( WebServer* server, HttpHeader* header, Connection* connection );

    static void _processSSIPage( Database* database, Filesystem* filesystem, File& file, Connection& connection );

    static uint32_t _getFileBufferSize( uint32_t fileSize );
    static uint32_t _getDynamicPageBufferSize();

    enum
    {
        WAIT_MILLISECONDS_FOR_NEW_DATA = 300,
        SHTML_PAGE_BUFFER_SIZE         = 256,
        MAXIMUM_CONNECTIONS            = 8,
        FILE_BUFFER_FOR_SSI_SIZE       = 64,
        DEFAULT_THREAD_STACK_SIZE      = 5000 // 2500 минимум!
    };

    enum PAGE_STATE
    {
        RELOAD_PAGE = 0,
        NO_REQUEST
    };

    enum PAGE_TYPE
    {
        INDEX_PAGE,
        OTHER_PAGE,
        ALL_DATA
    };

    enum SEND_STATE
    {
        SEND_HEADER,
        SEND_FILE,
        SEND_AJAX,
        SEND_SSI_HEADER,
        SEND_SSI_DATA,
        SEND_SSI_PROCESS,
        END_SEND,
        SEND_END_ZERO_CHUNK
    };

    enum FILENAME_STATE
    {
        FILENAME_SEARCH_START,
        FILENAME_SEARCH_END
    };

    enum CONNECTION_STATE
    {
        CONNECTION_OPENED,
        CONNECTION_CLOSED
    };

    enum QUEUE_SEND_MODE
    {
        QUEUE_SEND_DATA,       /// передача файла
        QUEUE_SEND_RANGE_DATA, /// передача части файла
        QUEUE_FREE             /// передача завершена
    };

};

extern WebServer webServer;

#endif /* __HTTPD_H__ */
