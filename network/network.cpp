#include "network.hpp"
#include "convert.hpp"
#include "debug.hpp"

#include <string.h>

#include <stdlib.h>

/**
 *
 * @brief Открыть сетевое соединение
 *
 * @param type тип соединения, tcp или udp
 *
 * @return соединение
 *
 */

Connection Network::open( Connection::TYPE type )
{

    Connection connection;

    connection.id               = _socket( type );

    connection.isGetByte        = false;

    return connection;

}

/**
 *
 * @brief Присвоить соединению адрес и порт
 *
 * @param connection соединение
 * @param address строка с адресом
 * @param port порт от 0 до 65535
 *
 * @return статус присваивания
 *
 */

RSTATUS Network::bind( Connection& connection, char* address, uint16_t port )
{
    return _bind( connection.id, address, port );
}

/**
 *
 * @brief Присвоить соединению адрес и порт
 *
 * @param connection соединение
 * @param address специальное значение адреса
 * @param port порт от 0 до 65535
 *
 * @return статус присваивания
 *
 */

RSTATUS Network::bind( Connection& connection, ADDRESS address, uint16_t port )
{
    // ------- Код-заглушка для подавления предупреждений
    address = address;
    // -------

    return _bind( connection.id, "0.0.0.0", port );
}

/**
 *
 * @brief Начать принимать другие соединения
 *
 * @param maximumConnections максимальное количество принятых соединений
 *
 * @return статус
 *
 */

RSTATUS Network::listen( Connection& connection, uint16_t maximumConnections )
{
    return _listen( connection.id, maximumConnections );
}

/**
 *
 * @brief Принять входящее соединение
 *
 * @param connection
 *
 * @return принятое, новое соединение
 *
 */

Connection Network::accept( Connection& connection )
{

    Connection nConnection;

    nConnection.id = _accept( connection.id );
    nConnection.isGetByte = false;

    return nConnection;

}

/**
 * @brief Network::recieve
 * @param connection
 * @param buffer
 * @param length
 * @return
 */

int Network::recieve( Connection& connection, uint8_t* buffer, uint32_t length )
{
    return _recieve( connection.id, buffer, length );
}

/**
 * @brief Network::send
 * @param connection
 * @param buffer
 * @param length
 * @return
 */

int Network::send( Connection& connection, uint8_t* buffer, uint32_t length )
{
    return _send( connection.id, buffer, length );
}

/**
 * @brief Network::send
 * @param connection
 * @param str
 * @return
 */

int Network::send( Connection& connection, const char* str )
{    
    return _send( connection.id, (uint8_t*)str, strlen(str) );
}

/**
 * @brief Network::send
 * @param connection
 * @param value
 * @return
 */

int Network::send( Connection& connection, uint8_t value )
{

    char    str[8];

    uint8_t index = Convert::unsignedCharToString( &(str[0]), value, 7 );

    return _send( connection.id, (uint8_t*) &(str[0]), index );


}

/**
 * @brief Network::send
 * @param connection
 * @param value
 * @return
 */

int Network::send( Connection& connection, int8_t value )
{

    char    str[8];

    uint8_t index = Convert::charToString( &(str[0]), value, 7 );

    return _send( connection.id, (uint8_t*) &(str[0]), index );

}

/**
 * @brief Network::send
 * @param connection
 * @param value
 * @return
 */

int Network::send( Connection& connection, uint16_t value )
{

   char    str[12];

   uint8_t index = Convert::unsignedShortToString( &(str[0]), value, 11 );

   return _send( connection.id, (uint8_t*) &(str[0]), index );

}

/**
 * @brief Network::send
 * @param connection
 * @param value
 * @return
 */

int Network::send( Connection& connection, int16_t value )
{

    char    str[12];

    uint8_t index = Convert::shortToString( &(str[0]), value, 11 );

    return _send( connection.id, (uint8_t*) &(str[0]), index );

}

/**
 * @brief Network::send
 * @param connection
 * @param value
 * @return
 */

int Network::send( Connection& connection, float value )
{

    char str[12];

    uint8_t index = Convert::floatToString( str, value, 11 );

    return _send( connection.id, (uint8_t*) &(str[0]), index );

}

/**
 * @brief Network::send
 * @param connection
 * @param value
 * @return
 */

int Network::send( Connection& connection, uint32_t value )
{

    char    str[12];

    uint8_t index = Convert::unsignedIntegerToString( &(str[0]), value, 11 );

    return _send( connection.id, (uint8_t*) &(str[0]), index );

}

/**
 * @brief Network::send
 * @param connection
 * @param value
 * @return
 */

int Network::send( Connection& connection, int32_t value )
{

    char stringValue[12];

    uint8_t index = Convert::integerToString( &(stringValue[0]), value, 11 );

    stringValue[index] = '\0';

    return _send( connection.id, (uint8_t*) &(stringValue[0]), index );

}

/**
 * @brief Network::put
 * @param connection
 * @param byte
 */

void Network::put( Connection& connection, uint8_t byte )
{

    _send( connection.id, (uint8_t*)&byte, 1 );

}

/**
 * @brief Network::get
 * @param connection
 * @return
 */

uint8_t Network::get( Connection& connection )
{

    if( connection.isGetByte )
    {

        connection.isGetByte = false;

        return connection.getByte;

    }
    else
    {

        int st  = _recieve( connection.id, &(connection.getByte), 1 );

        if ( 1 == st )
        {

            return connection.getByte;

        }
        else
        {

            return 0;

        }

    }

}

/**
 * @brief Network::isReadyRead
 * @param connection
 * @return
 */

bool Network::isReadyRead( Connection& connection )
{

    if( connection.isGetByte )
    {

        return true;

    }
    else
    {

        int st = _recieve( connection.id, &(connection.getByte), 1 );

        if( 1 == st )
        {

            connection.isGetByte = true;

            return true;

        }
        else
        {

            return false;

        }

    }

}

/**
 * @brief Network::sendJsonHeader
 * @param connection
 */

void Network::sendJsonHeader( Connection& connection )
{

    send( connection, "HTTP/1.1 200 OK\r\nContent-Language: ru\r\nContent-type: application/json\r\n\r\n{" );

}

/**
 * @brief Network::sendJsonHeader
 * @param connection
 */

void Network::sendJsonHeader(Connection& connection, uint32_t dataLength )
{

    char buf[128] = "HTTP/1.1 200 OK\r\nContent-Language: ru\r\nContent-type: application/json\r\nContent-Length: ";

    itoa( dataLength+1, &(buf[strlen(buf)]), 10 );

    strcat( buf, "\r\n\r\n{" );

    send( connection, buf );

}

/**
 * @brief Network::sendJson
 * @param connection
 * @param name
 * @param value
 * @param next
 */

void Network::sendJson( Connection& connection, const char* name, const char* value, Network::JsonSend next )
{

    send( connection, "\"");
    send( connection, name );
    send( connection, "\":\"" );
    send( connection, value );

    if( next == JsonNext )
    {

        send( connection, "\"," );

    }
    else
    {
        // последний элемент
        send( connection, "\"}" );

    }

}

void Network::sendJson( Connection& connection, const char* name, const float value, Network::JsonSend next )
{

    send( connection, "\"");
    send( connection, name );
    send( connection, "\":\"" );
    send( connection, value );

    if( next == JsonNext )
    {

        send( connection, "\"," );

    }
    else
    {
        // последний элемент
        send( connection, "\"}" );

    }

}

void Network::sendJson( Connection& connection, const char* name, const uint32_t value, Network::JsonSend next )
{
    sendJson(connection, name, (int) value, next );
}

void Network::sendJson( Connection& connection, const char* name, const int value, Network::JsonSend next )
{

    send( connection, "\"");
    send( connection, name );
    send( connection, "\":\"" );
    send( connection, (int32_t) value );

    if( next == JsonNext )
    {

        send( connection, "\"," );

    }
    else
    {
        // последний элемент
        send( connection, "\"}" );

    }

}

/**
 * @brief Network::sendJsonTail
 * @param connection
 */

void Network::sendJsonTail( Connection& connection )
{
    send( connection, "}" );
}

/**
 * @brief Network::sendSuccessJsonHeader
 * @param connection
 */

void Network::sendSuccessJsonHeader(Connection& connection)
{

    const char buf[] = "HTTP/1.1 200 OK\r\nContent-Language: ru\r\nContent-type: application/json\r\nContent-Length: 20\r\n\r\n{\"status\":\"success\"}";

    send( connection, (uint8_t*) buf, sizeof(buf) - 1 );

}

void Network::sendErrorJsonHeader(Connection& connection)
{

    const char buf[] = "HTTP/1.1 200 OK\r\nContent-Language: ru\r\nContent-type: application/json\r\nContent-Length: 18\r\n\r\n{\"status\":\"error\"}";

    send( connection, (uint8_t*) buf, sizeof(buf) - 1 );

}


/**
 * @brief Network::isError
 * @param connection
 * @return
 */

bool Network::isError( Connection& connection )
{

    if( connection.id == 0 )
    {

        return true;

    }
    else
    {

        return false;

    }

}

/**
 * @brief Network::commit
 * @param connection
 */

void Network::commit( Connection& connection )
{

    _commit( connection.id );

}

/**
 * @brief Network::close
 * @param connection
 */

void Network::close( Connection& connection )
{

    _close( connection.id );

}

/**
 * @brief connect
 * @param connection
 * @param server_address
 * @param port
 */

int Network::connect( Connection& connection, const char* server_address, uint16_t port )
{
    return _connect( connection.id, server_address, port );
}
