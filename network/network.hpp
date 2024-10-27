#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "platform.hpp"
#include "connection.hpp"

/**
 *
 * @brief Работа с сетью
 *
 */

class Network
{

public:

    enum ADDRESS
    {
        ANY_ADDRESS
    };

    enum JsonSend
    {
        JsonNext,
        JsonEnd // окончание передачи
    };

    static Connection open( Connection::TYPE type );

    static RSTATUS bind( Connection& connection, char* address,   uint16_t port );
    static RSTATUS bind( Connection& connection, ADDRESS address, uint16_t port );

    static RSTATUS listen( Connection& connection, uint16_t maximumConnections );

    static Connection accept( Connection& connection );

    static int recieve(Connection& connection, uint8_t* buffer, uint32_t length );
    static int send(Connection& connection, uint8_t* buffer, uint32_t length );

    static int send( Connection& connection, const char *str );
    static int send( Connection& connection, uint8_t value );
    static int send( Connection& connection, int8_t value );
    static int send( Connection& connection, uint16_t value );
    static int send( Connection& connection, int16_t value );
    static int send( Connection& connection, float value );
    static int send( Connection& connection, uint32_t value );
    static int send( Connection& connection, int32_t value );

    static void sendJsonHeader( Connection& connection );
    static void sendJsonHeader( Connection& connection, uint32_t dataLength);
    static void sendJson( Connection& connection, const char* name, const char* value, JsonSend next = JsonEnd );
    static void sendJson( Connection& connection, const char* name, const float value, Network::JsonSend next = JsonEnd );
    static void sendJson( Connection& connection, const char* name, const int   value, Network::JsonSend next = JsonEnd );
    static void sendJson( Connection& connection, const char* name, const uint32_t value, Network::JsonSend next = JsonEnd );
    static void sendJsonTail( Connection& connection );
    static void sendSuccessJsonHeader(Connection& connection);
    static void sendErrorJsonHeader(Connection& connection);

    static void    put( Connection& connection, uint8_t byte );
    static void    commit( Connection& connection );
    static uint8_t get( Connection& connection );

    static bool isReadyRead( Connection& connection );

    static bool isError( Connection& connection );

    static void close( Connection& connection );

    static int connect( Connection& connection, const char* server_address, uint16_t port );

private:

    // ----------- Платформо-зависимые функции

    static void*   _socket( Connection::TYPE type );
    static RSTATUS _bind( void* id, const char* address, uint16_t port );
    static RSTATUS _listen( void* id , uint16_t maximumConnections );
    static void*   _accept( void* id );
    static int     _send( void* id,    uint8_t* buffer, uint32_t bufferLength );
    static int     _recieve( void* id, uint8_t* buffer, uint32_t bufferLength );
    static RSTATUS _close( void* id );
    static void    _commit( void* id );
    static int     _connect( void* id, const char* address, uint16_t port );

    // -----------

};

#endif
