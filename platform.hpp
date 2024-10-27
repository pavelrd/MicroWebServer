#ifndef PLATFORM_HPP
#define PLATFORM_HPP

/**

    @file базовые платформо-зависимые константы и функции

*/

#include "stdint.h"

#ifndef NULL
    #define NULL 0
#endif

enum RSTATUS
{
    SUCCESS_STATUS = 0,
    ERROR_STATUS   = -1
};
	
#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega168A__)

    #define AVR_PLATFORM

    // EEPROM size - 512
    #define SETTINGS_FLASH_BLOCK_SIZE 64
    #define SETTINGS_FLASH_BLOCK_USED 8
    //

    #define BOOTLOADER_NOT_SUPPORTED 1

#elif defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)

    #define AVR_PLATFORM

    // EEPROM size - 1024
    #define SETTINGS_FLASH_BLOCK_SIZE 128
    #define SETTINGS_FLASH_BLOCK_USED 8
    //

    #define BOOTLOADER_NOT_SUPPORTED 1

#elif defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)

    #define AVR_8_BIT_PLATFORM
    #define SETTINGS_FLASH_BLOCK_SIZE 256
    #define SETTINGS_FLASH_BLOCK_USED 16

    #define BOOTLOADER_NOT_SUPPORTED 1

#elif defined(ESP32_PLATFORM)

    #define SETTINGS_FLASH_BLOCK_SIZE 4096
    #define SETTINGS_FLASH_BLOCK_USED 3

    #define BOOTLOADER_FLASH_BLOCK_SIZE         SETTINGS_FLASH_BLOCK_SIZE
    #define BOOTLOADER_PROGRAM_CONTAINER_SIZE    512000
    #define BOOTLOADER_FLAG_ADDRESS           0x00000000
    #define BOOTLOADER_FLAG_BLOCKS                     1
    #define BOOTLOADER_MAXIMUM_PROGRAMS                3

    #include <nvs_flash.h>
    #include <esp_wifi.h>

    #define PLATFORM_LITTLE_ENDIAN

#elif defined(ESP8266_PLATFORM)

    #define SETTINGS_FLASH_BLOCK_SIZE 4096
    #define SETTINGS_FLASH_BLOCK_USED 1

    #define BOOTLOADER_FLASH_BLOCK_SIZE       SETTINGS_FLASH_BLOCK_SIZE
    #define BOOTLOADER_PROGRAM_CONTAINER_SIZE     179200
    #define BOOTLOADER_FLAG_ADDRESS           0x405DFE6E
    #define BOOTLOADER_FLAG_BLOCKS                     2        
    #define BOOTLOADER_MAXIMUM_PROGRAMS 3

    #define PLATFORM_LITTLE_ENDIAN

#elif defined(PC_PLATFORM)

    #define PLATFORM_LITTLE_ENDIAN

    #include <iostream>

    #define SETTINGS_FLASH_BLOCK_SIZE 4096
    #define SETTINGS_FLASH_BLOCK_USED 10

    #define BOOTLOADER_FLASH_BLOCK_SIZE       SETTINGS_FLASH_BLOCK_SIZE
    #define BOOTLOADER_PROGRAM_CONTAINER_SIZE 15000000
    // Конец адресного пространства
    #define BOOTLOADER_FLAG_ADDRESS           0x00000000
    #define BOOTLOADER_FLAG_BLOCKS                     1
    #define BOOTLOADER_MAXIMUM_PROGRAMS                5

    #include <unistd.h>

    #include <winsock2.h>
    #include "windows.h"

    #define _WIN32_WINT    0x0400
    #define _WIN32_WINDOWS 0x0410

    #include <Winbase.h>

    // #pragma comment( lib, "Ws2_32.lib" )

#elif defined(RASBPERRY_PI_PLATFORM)

    #define PLATFORM_LITTLE_ENDIAN

    #include <iostream>

    #define SETTINGS_FLASH_BLOCK_SIZE 4096
    #define SETTINGS_FLASH_BLOCK_USED 10

    #define BOOTLOADER_FLASH_BLOCK_SIZE       SETTINGS_FLASH_BLOCK_SIZE
    #define BOOTLOADER_PROGRAM_CONTAINER_SIZE 15000000
    // Конец адресного пространства
    #define BOOTLOADER_FLAG_ADDRESS           0x00000000
    #define BOOTLOADER_FLAG_BLOCKS                     1
    #define BOOTLOADER_MAXIMUM_PROGRAMS                5

    #include <unistd.h>

#else

    #error "This platform not supported!"

#endif


#ifdef AVR_8_BIT_PLATFORM
    // Так как на 8 битных контроллерах мало RAM, то уменьшаем размеры используемых переменных
    typedef uint8_t  page_index_t;
    typedef uint16_t payload_length_t;
    typedef uint16_t address_t;
#else
    typedef uint32_t  page_index_t;
    typedef uint32_t  payload_length_t;
    typedef uintptr_t address_t;
#endif

typedef uint32_t version_t;

#if (!defined(INET_H)) || (!defined(_WINSOCKAPI_))

    #ifdef PLATFORM_LITTLE_ENDIAN

        #define ntohl(n) (((((unsigned long)(n) & 0xFF)) << 24) | \
                         ((((unsigned long)(n) & 0xFF00)) << 8) | \
                         ((((unsigned long)(n) & 0xFF0000)) >> 8) | \
                         ((((unsigned long)(n) & 0xFF000000)) >> 24))

    #elif defined(PLATFORM_BIG_ENDIAN)

         // #define ntohl(n) (n)

    #else

        #error "Platform endianness not specified!"

    #endif


#endif

extern void setup();
RSTATUS     hardwareInit();

#endif
