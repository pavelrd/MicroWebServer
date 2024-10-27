#include "webserver.hpp"
#include "convert.hpp"
#include "delay.hpp"
#include "thread.hpp"
#include "debug.hpp"

#include <cstring>
#include <stdlib.h>

/**
 *
 * @brief По-умолчанию нет базы данных и файловой системы, принимает соединения на 80 порту
 *
 * @param database база данных
 *
 */

WebServer::WebServer()
{

    database       = 0;
    filesystem     = 0;
    port           = 80;
    stackSize      = DEFAULT_THREAD_STACK_SIZE;

}

/**
 *
 * @brief Задать базу данных, из которой сервер будет брать значения переменных при формировании shtml страниц
 *
 * @param database база данных
 *
 */

void WebServer::setDatabase( Database& _database )
{

    database = &(_database);

}

/**
 *
 * @brief Задать файловую систему, из которой сервер будет брать запрашиваемые файлы
 *
 * @param database база данных
 *
 */

void WebServer::setFilesytem( Filesystem &_filesytem )
{

    filesystem = &(_filesytem);

}

/**
 *
 * @brief Задать порт, который будет слушать сервер
 *
 * @param database база данных
 *
 */

void WebServer::setPort( uint16_t _port )
{

    port = _port;

}

/**
 *
 * @brief Задать размер стека для потоков-обработчиков соединений. Чем больше, тем лучше, главное чтобы
 *         хватило памяти с запасом.
 *
 * @param _stackSize размер стека
 *
 */

void WebServer::setStackSize( uint32_t _stackSize )
{
    stackSize = _stackSize;
}

/**
 *
 * @brief Добавить обработчик GET запроса
 *
 *  Пример:
 *
 *  \code
 *
 *      bool toggleLedHandler( Connection& connection, char* value ){
 *
 *      }
 *
 *      void main(){
 *
 *          httpServer.addHandler( "index.shtml", "toggleLed", toggleLedHandler );
 *
 *      }
 *
 *  \endcode
 *
 *  Когда к серверу придет GET(URL) запрос вида http://[Адрес сервера]/index.shtml?toggleLed
 *      то будет вызвана функция toggleLedHandler
 *
 * @param pagename строка с именем запрашиваемого файла, например "index.shtml"
 * @param request  строка с именем запроса, например "toggleLed"
 * @param handler  функция, которая будет обрабатывать запрос
 *
 * @return SUCCESS - успешно добавлен, ERROR - ошибка
 *
 */

RSTATUS WebServer::addHandler( const char* pagename, const char* request, WebServer::pageHandler handler )
{

    Database* pageHandlers = handlers.getDatabase(pagename);

    if( pageHandlers == 0 )
    {

        pageHandlers = new Database();

        handlers.add( pagename, pageHandlers );

    }

    if( pageHandlers != 0 )
    {

        if( request == 0 ) // == HANDLER_ACCEPT_ALL_PARAMETERS
        {
            pageHandlers->add( "HANDLER_ACCEPT_ALL_PARAMETERS", (void*) handler );
        }
        else
        {
            pageHandlers->add( request, (void*) handler );
        }

        return SUCCESS_STATUS;

    }
    else
    {

        return ERROR_STATUS;

    }

}

RSTATUS WebServer::addPostHandler( const char* pagename, const char* request, WebServer::pageHandler handler )
{

    Database* pageHandlers = postHandlers.getDatabase(pagename);

    if( pageHandlers == 0 )
    {

        pageHandlers = new Database();

        postHandlers.add( pagename, pageHandlers );

    }

    if( pageHandlers != 0 )
    {

        if( request == 0 ) // == HANDLER_ACCEPT_ALL_PARAMETERS
        {
            pageHandlers->add( "HANDLER_ACCEPT_ALL_PARAMETERS", (void*) handler );
        }
        else
        {
            pageHandlers->add( request, (void*) handler );
        }

        return SUCCESS_STATUS;

    }
    else
    {

        return ERROR_STATUS;

    }

}

/**
 *
 * @brief Запустить сервер
 *
 */


void WebServer::start()
{

    mainHandlerThreadId = Thread::run( _mainThread, "webserver" , this, stackSize, 2 );

}

/**
 *
 * @brief Остановить сервер
 *
 */

void WebServer::stop()
{

}

/**
 *
 * \brief Поток в котором работает сервер, принимает входящие соединения и обрабатывает запросы
 *
 * \param httpServer объект httpServer с которым работает поток
 *
 */

void WebServer::_mainThread( void* server )
{

    Connection connection = Network::open( Connection::TCP );

    if( Network::isError(connection) )
    {
    }

    if( ERROR_STATUS == Network::bind( connection, Network::ANY_ADDRESS, ((WebServer*) server)->port ) )
    {
    }

    if( ERROR_STATUS == Network::listen( connection, MAXIMUM_CONNECTIONS ) )
    {
    }

    while(1)
    {

        Connection acceptedConnection = Network::accept( connection );

        if( Network::isError( acceptedConnection ) )
        {
            continue;
        }

        Connection* connection = new Connection;

        *(connection)           = acceptedConnection;
        connection->argument    = server;

        if ( Thread::BAD == Thread::run( _connectionHandler, "http_connection_handler", connection, ((WebServer*) server)->stackSize, 1 ) )
        {

            Network::close(acceptedConnection);

        }

    }

    Network::close( connection );

    Thread::exit();

    return;

}

/**
 *
 * @brief Обрабатывает запрос/ы с одного соединения
 *
 * @param _connection соединение с которого приходит/ят запрос/ы
 *
 */

void WebServer::_connectionHandler( void* _connection )
{

    Connection* connection = ((Connection*)_connection);

    WebServer* server     = (WebServer*)connection->argument;

    HttpHeader* header    = new HttpHeader();

    if( (connection == 0) || (server == 0) || (header == 0) )
    {

        if( header != 0 ) { delete header; }

        if( connection != 0 )
        {
            Network::close( *connection );

            delete ((Connection*)_connection);
        }

        Thread::exit();

        return;

    }

    header->clear();

    WebParameters *parameters = 0;

    while( 1 )
    {

        if( !( Network::isReadyRead(*connection) ) )
        {
            break;
        }

        uint8_t val = Network::get( *connection );

        header->put( val );

        if( header->isFilled() )
        {

            if ( header->type == HttpHeader::GET_METHOD )
            {

                if( parameters != 0 )
                {

                    if ( _processGetParameters( server->handlers.getDatabase( header->resourceNameHash ), connection, parameters  ) )
                    {

                         _clearParameters( parameters );

                         parameters = 0;

                         // Задержка, чтобы данные успели передасться

                         Delay::milliseconds(100);

                        _sendPageOrFile( server, header, connection );

                    }
                    else
                    {

                         // Задержка, чтобы данные успели передасться

                         Delay::milliseconds(100);

                        _clearParameters( parameters );

                        parameters = 0;

                    }

                    break;

                }
                else
                {

                    if ( false == _sendPageOrFile( server, header, connection ) )
                    {
                        break;
                    }

                }

            }
            else if( header->type == HttpHeader::POST_METHOD )
            {

                // Пока не обрабатывается

                char postBuffer[128];
                volatile uint8_t postBoundaryLength = strlen(header->postBoundary);
                uint32_t readBytes = 0;

                Network::recieve( *connection, (uint8_t*) &(postBuffer[0]), postBoundaryLength + 2  );

                readBytes += postBoundaryLength + 2;

                postBuffer[postBoundaryLength + 2] = '\0';

                if( ( 0 != strcmp( &(postBuffer[2]), header->postBoundary ) ) &&
                    ( postBuffer[0] == '-' ) &&
                    ( postBuffer[1] == '-' )
                )
                {
                    // Ошибка в протоколе
                    break;
                }

                // else, всё хорошо

                Network::recieve( *connection, (uint8_t*) &(postBuffer[0]), 2  );

                readBytes += 2;

                char* name = new char[30];

                name[0] = '\0';

                while(1)
                {

                    int index = 0;

                    while (1)
                    {

                        Network::recieve( *connection, (uint8_t*) &(postBuffer[index]), 1  );

                        readBytes += 1;

                        if( postBuffer[index] == '\n' )
                        {
                            break;
                        }

                        index += 1;

                    }

                    if( ( postBuffer[0]  == '\r' ) && (postBuffer[1] == '\n') )
                    {
                        break;
                    }

                    postBuffer[index+1] = '\0';

                    // Content-Disposition: form-data; name="myprogram"; filename="file.bin"; \r\n\0

                    // Accept-charset: UTF-8 ; param=""; name=""

                    const char contentDisposition[] = "Content-Disposition: form-data; name=\"";

                    if ( 0 == strncmp( &(postBuffer[0]), &(contentDisposition[0]), strlen(contentDisposition) ) )
                    {
                        int i = 0; // счетчик
                        int name_start_index = strlen(&(contentDisposition[0]));
                        while(postBuffer[name_start_index+i] != '"')
                        {
                            name[i] = postBuffer[name_start_index+i];
                            i++;
                            if(i == 29)
                            {
                                break;
                            }
                        }
                        name[i] = '\0';
                    }

                }

                uint32_t fileSize = header->contentLength - ( readBytes + postBoundaryLength + 8 );

                char* fileSizeStr   = new char[16];
                char* tailLengthStr = new char[16];

                itoa( fileSize, &(fileSizeStr[0]), 10 );
                itoa( postBoundaryLength + 8, &(tailLengthStr[0]), 10 );

                Database *base = server->postHandlers.getDatabase( header->resourceNameHash );

                pageHandler fhandl = (pageHandler) base->getFunction( "HANDLER_ACCEPT_ALL_PARAMETERS" );

                if ( fhandl != 0 )
                {

                    WebParameters *postParameters = new WebParameters();

                    WebParameter parameter;

                    parameter.hash  = CRC::crc32(0, (const uint8_t*) "name", 4 );
                    parameter.value = name;

                    postParameters->add( parameter );

                    parameter.hash  = CRC::crc32(0, (const uint8_t*) "length", 6);
                    parameter.value = fileSizeStr;

                    postParameters->add( parameter );

                    parameter.hash  = CRC::crc32(0, (const uint8_t*) "tailLength", 10);
                    parameter.value = tailLengthStr;

                    postParameters->add( parameter );

                    fhandl( *connection, *postParameters);

                    delete [] name;

                    delete [] fileSizeStr;

                    delete [] tailLengthStr;

                    delete postParameters;

                    break;

                }
                else
                {

                    delete [] name;

                    delete [] fileSizeStr;

                    delete [] tailLengthStr;

                }


            }

            header->clear();

        }
        else if( header->isParameter() )
        {

            // Запоминание запрошенных get параметров

            if( parameters == 0 )
            {

                parameters = new WebParameters();

            }

            WebParameter nwParameter;

            nwParameter.hash = header->getParameterNameHash();

            nwParameter.value = new char[ strlen(header->getParameterValue()) + 1 ];

            if( nwParameter.value == 0 )
            {
               delete parameters;
               parameters = 0;
               break;
            }

            strcpy( &( nwParameter.value[0] ), &((header->getParameterValue())[0]) );

            nwParameter.value[strlen(header->getParameterValue())] = '\0';

            parameters->add( nwParameter );

        }
        else if( header->isError() )
        {
            break;
        }

    }

    if( parameters != 0 )
    {
        delete parameters;
    }

    delete header;

    Network::close( *connection );

    delete ((Connection*)_connection);

    // printf("Stack size: %d \r\n", Thread::getFreeStackSize() );

    Thread::exit();

}

void WebServer::_processSSIPage( Database* database, Filesystem* filesystem, File& file, Connection& connection ){

    SSI* ssi = new SSI();

    ssi->setDatabase(*(database));

    uint32_t blockLength = SHTML_PAGE_BUFFER_SIZE;

    char* block = new char[ blockLength ];

    // TODO: if block == 0 --- action

    uint32_t i = 0;

    uint8_t* buffer = new uint8_t[FILE_BUFFER_FOR_SSI_SIZE];

    int blockCounter = 0;

    while( ! file.isEnd() )
    {

        blockCounter += 1;

        uint32_t reallyReadCount = filesystem->read( file, &(buffer[0]), FILE_BUFFER_FOR_SSI_SIZE );

        if( reallyReadCount == 0 )
        {
            break;
        }

        for( uint32_t readCounter = 0 ; readCounter < reallyReadCount; readCounter++ )
        {
            /*
            Serial::write("Symbol - ");
            Serial::writeByte( buffer[readCounter] );
            Serial::write("- read counter - ");
            Serial::write( readCounter );
            Serial::write(" / ");
            Serial::write(reallyReadCount);
            Serial::write("\r\n");
            */

            ssi->put( buffer[readCounter] );

            while( ssi->isReadyRead() )
            {

                block[i] = ssi->get();

                i += 1;

                if( i >= blockLength )
                {

                    char str[10];

                    uint8_t num = Convert::unsignedIntegerToHexString( str, blockLength, 10 );

                    Network::send( connection, (uint8_t*) str, num );

                    Network::send( connection, "\r\n" );

                    Network::send( connection, (uint8_t*) block, blockLength ); // Блок

                    Network::send( connection, "\r\n");

                    i = 0;

                }

            }

            if( ssi->isIncludeFile() )
            {

                // Страница подключает другой html файл
                //  отправляем текущий буфер

                if( i > 0 )
                {

                    char str[10];

                    uint8_t num = Convert::unsignedIntegerToHexString( str, i, 10 );

                    Network::send( connection, (uint8_t*) str, num );

                    Network::send( connection, "\r\n" );

                    Network::send( connection, (uint8_t*) block, i );

                    Network::send( connection, "\r\n" );

                    i = 0;

                }

                // Обрабатываем подклчючаемый файл

                File nfile = filesystem->open( ssi->getIncludeFilename() );

                if( nfile.isOpened() )
                {

                    _processSSIPage( database, filesystem, nfile, connection );

                }
                else
                {

                    // Error: file not found!

                }

            }

        }

    }

    delete [] buffer;

    ssi->commit();

    while( ssi->isReadyRead() )
    {

        block[i] = ssi->get();

        i += 1;

        if( i >= blockLength )
        {

            char str[10];

            uint8_t num = Convert::unsignedIntegerToHexString( str, blockLength, 10 );

            Network::send( connection, (uint8_t*) str, num );

            Network::send( connection, (uint8_t*) block, blockLength ); // Блок

            Network::send( connection, "\r\n");

            i = 0;

        }

    }

    if ( i > 0 )
    {

        char str[11];

        uint8_t num = Convert::unsignedIntegerToHexString( str, i, 10 );

        Network::send( connection, (uint8_t*) str, num );

        Network::send( connection, "\r\n" );

        Network::send( connection, (uint8_t*) block, i );

        Network::send( connection, "\r\n" );

    }

    delete[] block;

    Network::commit( connection );

    filesystem->close(file);

    delete ssi;

}

bool WebServer::_isHasGetParametersHandlers( Database* database, WebParameters* parameters )
{

    if( database != 0 )
    {

        for( int i = 0 ; i < parameters->size(); i++ )
        {

            if ( 0 != database->getFunction( parameters->get(i).hash ) )
            {
                return true;
            }

        }

    }

    return false;

}

/**
 * @brief Вызвать обработчики get запросов
 *
 * @param database база данных с обработчиками
 * @param connection текущее соединение
 * @param parameters параметры get запроса
 *
 * @return
 *          true  - нужно перезагрузить страницу
 *          false - не нужно перезагружать страницу
 *
 */

bool WebServer::_processGetParameters( Database* database, Connection* connection, WebParameters* parameters )
{

    if( (database == 0) || (parameters == 0 ) || (connection == 0) )
    {
        return false;
    }

    bool reloadPageFlag = false;

    pageHandler fhandl = (pageHandler) database->getFunction( "HANDLER_ACCEPT_ALL_PARAMETERS" );

    if( fhandl != 0 )
    {

        reloadPageFlag = fhandl( *connection, *parameters );

    }
    else
    {

        for( int i = 0 ; i < parameters->size(); i++ )
        {

            if( database != 0 )
            {

                pageHandler fhandl = (pageHandler) database->getFunction( parameters->get(i).hash );

                if( fhandl != 0 )
                {

                    reloadPageFlag = fhandl( *connection, *parameters ); // parameters->get(i).value

                }
            }
        }

    }

    return reloadPageFlag;

}

void WebServer::_clearParameters(WebParameters *parameters )
{

    for( int i = 0 ; i < parameters->size(); i++ )
    {

        delete[] parameters->get(i).value;

    }

    delete parameters;

}

/**
 * @brief Отправка запрошенной html страницы или файла
 *
 * @param database база данных с обработчиками
 * @param connection текущее соединение
 * @param parameters параметры get запроса
 *
 * @return true - отправлена, false - страница не найдена
 */

bool WebServer::_sendPageOrFile( WebServer* server, HttpHeader* header, Connection* connection )
{

    File file;

    if( header->resourceNameHash == 0x79d3d2d4 )
    { // '/'

        // Запрошена корневая страница

        file = server->filesystem->open("/index.html");

    }
    else
    {

        // Запрошен файл или любая другая страница


        file = server->filesystem->open( header->resourceNameHash );

    }

    if( !file.isOpened() )
    {

        // Файл не найден!

        Network::send( *connection, "HTTP/1.1 404 Not Found\r\nStatus: 404 Not Found\r\n\r\n" );

        header->clear();

        return false;

    }

    if ( header->isChunked )
    {

        // Передача части файла
        // file.seek();
        // file.setMaxSize();
        Network::send( *connection, "HTTP/1.1 206 Partial Content\r\n" );

    }
    else
    {

        // Передача всего файла целиком

        Network::send( *connection, "HTTP/1.1 200 OK\r\n" );

    }

    Network::send( *connection, "Content-Language: ru\r\n");

    // Network::send( connection, "Connection: Keep-Alive\r\n");

    if( Magic::UNKNOWN_TYPE == file.getContentType() )
    {

        Network::send( *connection, "Content-type: text/plain\r\n" );

    }
    else
    {

         Network::send( *connection,"Content-type: ");

         Network::send( *connection, Magic::getStringFromContentType( file.getContentType() ) );

         Network::send( *connection,"\r\n");

    }

    Network::send( *connection, "Cache-Control: no-cache\r\n" );

    if( file.getContentType() == Magic::HTML )
    {

        Network::send( *connection, "Transfer-Encoding: chunked\r\n\r\n" );

        // Файл - file будет закрыт внутри метода

        _processSSIPage( server->database, server->filesystem, file, *connection );

        Network::send( *connection, "0\r\n\r\n" ); // Последний, нулевой блок

        //break;

    }
    else
    {

        Network::send( *connection,"Content-Length: ");

        Network::send( *connection, (uint32_t) file.getSize() );

        Network::send( *connection, "\r\n\r\n" );

        if( !(header->isChunked) )
        {

            // Передача части файла

            // connection->send( file.size );
            // Был запрос части файла

            // Ответ для файла размером 34531 байт, запрошено с 345 до конца файла

            // Content-Length: 34186 \r\n
            // Content-Range: bytes 345-34530/34531\r\n

            //  Content-Length: конечный байт - начальный байт
            //  Content-Range: начальный байт - конечный байт / суммарный объем файла

            //Network::send( connection, filesystem->seek( file, 0, SEEK_END ) - filesystem->seek( file, 0, SEEK_CUR ) );

            //Network::send( connection, "Content-Range: bytes " );

            //Network::send( connection, filesystem->seek( file, 0, SEEK_CUR ) );

            //Network::send( connection,'-');

            //Network::send( connection, filesystem->seek( file, 0, SEEK_END ) );

            //Network::send( connection,'/');

            //Network::send( connection, file.getSize() );

        }

        // Вычисляю размер буфера для файла

        uint32_t fileBufferSize = _getFileBufferSize( file.getSize() );

        char* fileBuffer = 0;

        do
        {

            fileBuffer = new char[fileBufferSize];

            if( fileBuffer != 0 )
            {
                break;
            }

            fileBufferSize = fileBufferSize / 2;

        } while( fileBufferSize != 0 );

        if( fileBuffer == 0 )
        {
            server->filesystem->close(file);
            return true;
        }

        while( !file.isEnd() )
        {

            int rv = server->filesystem->read( file, (uint8_t*) fileBuffer, fileBufferSize );

            if( rv <= 0 )
            {
                break;
            }

            Network::send( *connection, (uint8_t*)fileBuffer, rv );

        }

        delete [] fileBuffer;

        server->filesystem->close(file);

    }

    return true;

}

// break;

/**
 *
 * @brief Узнать размер буфера для пердачи файла
 *
 *   Вычисляет сбалансированный по объему памяти и производительности размер буфера
 *    на текущий момент работы программы.
 *
 *   Если памяти много то размер буфера выделяется по максимуму, иначе
 *    вычисляется какой-то оптимальный размер чтобы не выйти за границы памяти.
 *
 * @param database база данных
 *
 */

uint32_t WebServer::_getFileBufferSize( uint32_t fileSize )
{

    uint32_t bufferSize = Thread::getFreeHeapSize() / 32; // было 32

    if( ( fileSize < 20000 ) && ( bufferSize >= 1024 ) )
    {
        return 1024;
    }
    else if( bufferSize >= 16384 )
    {
        return 16384;
    }
    else if( bufferSize >= 8192 )
    {
        return 8192;
    }
    else if( bufferSize >= 4096 )
    {
        return 4096;
    }
    else if( bufferSize >= 2048 )
    {
        return 2048;
    }
    else if( bufferSize >= 1024 )
    {
        return 1024;
    }
    else if( bufferSize >= 512 )
    {
        return 512;
    }
    else if( bufferSize >= 256 )
    {
        return 256;
    }
    else if( bufferSize >= 128 )
    {
        return 128;
    }
    else if( bufferSize >= 64 )
    {
        return 64;
    }
    else if( bufferSize >= 32 )
    {
        return 32;
    }
    else
    {
        return 0;
    }

}

// Initialize ParametersList with false values

WebParameters::WebParameters()
{
    root=NULL;
    last=NULL;
    _size=0;

    lastNodeGot = root;
    lastIndexGot = 0;
    isCached = false;
}

// Clear Nodes and free Memory

WebParameters::~WebParameters()
{
    WebParametersNode* tmp;
    while(root!=NULL)
    {
        tmp=root;
        root=root->next;
        delete tmp;
    }
    last = NULL;
    _size=0;
    isCached = false;
}

/*
    Actualy "logic" coding
*/

WebParametersNode* WebParameters::getNode(int index)
{

    int _pos = 0;

    WebParametersNode* current = root;

    // Check if the node trying to get is
    // immediatly AFTER the previous got one
    if(isCached && lastIndexGot <= index)
    {
        _pos = lastIndexGot;
        current = lastNodeGot;
    }

    while(_pos < index && current)
    {
        current = current->next;

        _pos++;
    }

    // Check if the object index got is the same as the required
    if(_pos == index)
    {
        isCached = true;
        lastIndexGot = index;
        lastNodeGot = current;

        return current;
    }

    return 0;

}

int WebParameters::size()
{
    return _size;
}

bool WebParameters::add(WebParameter _t)
{

    WebParametersNode *tmp = new WebParametersNode();
    tmp->data = _t;
    tmp->next = NULL;

    if(root)
    {
        // Already have elements inserted
        last->next = tmp;
        last = tmp;
    }
    else
    {
        // First element being inserted
        root = tmp;
        last = tmp;
    }

    _size++;
    isCached = false;

    return true;
}

WebParameter WebParameters::get(int index)
{
    WebParametersNode *tmp = getNode(index);

    return (tmp ? tmp->data : WebParameter());
}

WebServer webServer;

