#include "network.hpp"
#include "debug.hpp"

#if defined(PC_PLATFORM)

    void* Network::_socket( Connection::TYPE type )
    {

        SOCKET* sock = (SOCKET*) malloc(sizeof(SOCKET));

        *(sock) = INVALID_SOCKET;

        if ( type == Connection::UDP )
        {

           *(sock) = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

        }
        else
        {

           *(sock) = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

        }

        if( *(sock) == (long long unsigned int)SOCKET_ERROR )
        {

            free(sock);
            printf("Error - %d \r\n", WSAGetLastError() );
            return 0;

        }
        else
        {

            return sock;

        }

    }

    RSTATUS Network::_bind( void* id,  const char* address, uint16_t port )
    {

        sockaddr_in naddress;

        naddress.sin_family = AF_INET;

        if( strcmp(address, "0.0.0.0") == 0 )
        {

            naddress.sin_addr.s_addr = INADDR_ANY;

        }
        else
        {

            naddress.sin_addr.s_addr = inet_addr(address);

        }

        naddress.sin_port = htons( (unsigned short) port );

        if ( ::bind( *((SOCKET*)id), (const sockaddr*) &naddress, sizeof(sockaddr_in) ) < 0 )
        {

            return ERROR_STATUS;

        }
        else
        {

            return SUCCESS_STATUS;

        }

    }

    RSTATUS Network::_listen( void* id, uint16_t maximumConnections )
    {

        if ( SOCKET_ERROR == ::listen( *((SOCKET*)id), maximumConnections ) )
        {

            return ERROR_STATUS;

        }
        else
        {

            return SUCCESS_STATUS;

        }

    }

    void* Network::_accept( void* id )
    {

        SOCKET* socket;

        socket = (SOCKET*) malloc(sizeof(SOCKET));

        *(socket) = ::accept( *((SOCKET*)id), NULL, NULL );
        if( *(socket) == (long long unsigned int)SOCKET_ERROR )
        {
            return NULL;
        }
        else
        {
            return socket;
        }

    }

    int Network::_send( void* id, uint8_t* buffer, uint32_t bufferLength )
    {

        return ::send( *((SOCKET*)id), (char*) buffer, bufferLength, 0 );

    }

    int Network::_recieve( void* id, uint8_t* buffer, uint32_t bufferLength )
    {

        return ::recv( *((SOCKET*)id), (char*) buffer, bufferLength, 0 );

    }

    RSTATUS Network::_close( void* id )
    {

        closesocket( *((SOCKET*)id) );

        free(id);

        return SUCCESS_STATUS;

    }

    void Network::_commit( void* id )
    {
        id = id;
    }

    int Network::_connect(void* id, const char* address, uint16_t port )
    {

        sockaddr_in naddress;

        naddress.sin_family = AF_INET;

        naddress.sin_addr.s_addr = inet_addr(address);

        naddress.sin_port = htons( (unsigned short) port );

        return ::connect( *((int*)id), (sockaddr*)&naddress, sizeof(sockaddr_in) );

    }

#elif defined(ESP32_PLATFORM) || defined(ESP8266_PLATFORM)

    #include <lwip/sockets.h>
    #include <stdlib.h>
    #include <string.h>

    void* Network::_socket( Connection::TYPE type )
    {

        int* sock = (int*) malloc(sizeof(int));

        if ( type == Connection::UDP )
        {

            *(sock) = ::socket( AF_INET, SOCK_DGRAM, 0 );

        }
        else
        {

            *(sock) = ::socket( AF_INET, SOCK_STREAM, 0 );

        }

        if( *(sock) == -1 )
        {

            free(sock);

            return 0;

        }
        else
        {
            #if defined(ESP8266_PLATFORM)

                uint32_t opt = 0;

                ioctl( *sock, FIONBIO, (void*)&opt );

            #endif

            return sock;

        }

    }

    RSTATUS Network::_bind( void* id, const char* address, uint16_t port )
    {

        sockaddr_in naddress;

        naddress.sin_family = AF_INET;

        if( strcmp(address, "0.0.0.0") == 0 )
        {

            naddress.sin_addr.s_addr = INADDR_ANY;

        }
        else
        {

            naddress.sin_addr.s_addr = inet_addr(address);

        }

        naddress.sin_port = htons( (unsigned short) port );

        if ( ::bind( *((int*)id), (const sockaddr*) &naddress, sizeof(sockaddr_in) ) == -1 )
        {

            return ERROR_STATUS;

        }
        else
        {

            return SUCCESS_STATUS;

        }

        return  SUCCESS_STATUS;

    }

    RSTATUS Network::_listen( void* id, uint16_t maximumConnections )
    {

        if ( -1 == ::listen( *((int*)id), maximumConnections ) )
        {

            return ERROR_STATUS;

        }
        else
        {

            return SUCCESS_STATUS;

        }

        return SUCCESS_STATUS;

    }

    void* Network::_accept( void* id )
    {

        int* socket;

        socket = (int*) malloc(sizeof(int));

        if( socket == 0 )
        {
            Debug::print("Error on get memory for socket");
            return 0;
        }

        *(socket) = ::accept( *((int*)id), NULL, NULL );

        if( *(socket) == -1 )
        {
            return NULL;
        }
        else
        {
            #if defined(ESP8266_PLATFORM)

                uint32_t opt = 0;

                ioctl( *socket, FIONBIO, (void*)&opt );

            #endif

            return socket;
        }

        return 0;

    }

    int Network::_send( void* id, uint8_t* buffer, uint32_t bufferLength )
    {

        #ifdef ESP8266_PLATFORM

            int descriptor = *((int*)id);

            fd_set sendSet;

            FD_ZERO( &sendSet );

            FD_SET( descriptor, &sendSet );

            select( descriptor + 1, 0, &sendSet, 0, 0 );

        #endif

        int rv = ::send( *((int*)id), (char*) buffer, bufferLength, 0 );

        if( rv == -1 )
        {
            Debug::print("Network send error");
        }

        return rv;

    }

    int Network::_recieve( void* id, uint8_t* buffer, uint32_t bufferLength )
    {

        return ::recv( *((int*)id), (char*) buffer, bufferLength, 0 );

    }

    RSTATUS Network::_close( void* id )
    {

        closesocket( *((int*)id) );

        free(id);

        return SUCCESS_STATUS;

    }

    void Network::_commit( void* id )
    {

        // Посылка нулевых значений через send
        //   и select(writefs) не работают!!!
        // Найти какой-то другой статус

        // Delay::milliseconds(500);

        // Код-заглушка для подавления предупреждений

        id = id;

    }

    int Network::_connect(void* id, const char* address, uint16_t port )
    {

        sockaddr_in naddress;

        naddress.sin_family = AF_INET;

        naddress.sin_addr.s_addr = inet_addr(address);

        naddress.sin_port = htons( (unsigned short) port );

        return ::connect( *((int*)id), (sockaddr*)&naddress, sizeof(sockaddr_in) );

    }

#else

    #error "Unsupported platform!"

#endif
