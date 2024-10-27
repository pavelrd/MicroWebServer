#include "settings.hpp"

// Платформо-зависимые части класса настроек

// Пример успешно записанных многостраничных настроек в памяти
//
// |--------------------------------------------------------------------------------------------|
// |                           Страница в flash памяти                                          |
// |--------------------------------------------------------------------------------------------|
// |                            flash_block_t                               |                   |
// |--------------------------------------------------------------------------------------------|
// |   payload  |               struct flash_block_info                     |                   |
// |--------------------------------------------------------------------------------------------|
// | Содержимое |  Номер  | Номер    | Контрольная | Длина     | Признак    | Признак успешной  |
// |  страницы  |  версии | страницы | сумма       | payload   |  конца     |   записи          |
// |  payload   | version |  page    |     crc     | paylength |    next    |                   |
// |------------|---------|----------|-------------|-----------|------------|-------------------|
// |     X      |    1    |    0     |     X       |     X     |     0      |        0          |
// |     X      |    1    |    1     |     X       |     X     |     FF     |        0          |
// |     X      |    2    |    0     |     X       |     X     |     0      |        0          |
// |     X      |    2    |    1     |     X       |     X     |     FF     |        0          |
// |     -      |    -    |    -     |     -       |     -     |     -      |        -          |
// |     -      |    -    |    -     |     -       |     -     |     -      |        -          |
// |     -      |    -    |    -     |     -       |     -     |     -      |        -          |
// |--------------------------------------------------------------------------------------------|
//
//  X - какое то количество байт
//  - - свободное место
//

#if defined(AVR_8_BIT_PLATFORM)

    #include <avr/eeprom.h>

    uint32_t Settings::_getStartAddress()
    {
        return 0;
    }

    uint32_t Settings::_getBlockSize()
    {
        return 0;
    }

    uint32_t Settings::_GetBlockCount()
    {
        return 0;
    }

    STATUS Settings::_readBlock( address_t address, void *buffer )
    {

        eeprom_read_block( buffer, reinterpret_cast<const void*> (address), SETTINGS_FLASH_BLOCK_SIZE );

    }

    STATUS Settings::_writeBlock( address_t address, void *buffer )
    {

        eeprom_write_block( buffer, reinterpret_cast<const void*> (address), SETTINGS_FLASH_BLOCK_SIZE );

    }

    STATUS Settings::_eraseBlock( address_t address )
    {
        return SUCCESS;
    }

#elif defined(ESP32_PLATFORM) || defined(ESP8266_PLATFORM)

    #include "esp_spi_flash.h"

    uint32_t Settings::_getStartAddress()
    {
#if defined(ESP8266_PLATFORM)
        return 0x70000;
#else
        return 0x3DD000;
#endif
    }

    uint32_t Settings::_getBlockSize()
    {
        return SETTINGS_FLASH_BLOCK_SIZE;
    }

    uint32_t Settings::_GetBlockCount()
    {
        return SETTINGS_FLASH_BLOCK_USED;
    }

    RSTATUS Settings::_readBlock( address_t address, void *buffer )
    {
        if( spi_flash_read( address, (address_t*) buffer, 4096 ) == ESP_OK )
        {
            return SUCCESS_STATUS;
        }
        else
        {
            return ERROR_STATUS;
        }
    }

    RSTATUS Settings::_writeBlock( address_t address, void *buffer )
    {
        if( spi_flash_write( address, (address_t*) buffer, 4096 ) == ESP_OK )
        {

            return SUCCESS_STATUS;
        }
        else
        {
            return ERROR_STATUS;
        }
    }

    RSTATUS Settings::_eraseBlock( address_t address )
    {

        int32_t errcode = spi_flash_erase_sector( address / 4096 );

        if( errcode == ESP_OK )
        {
            return SUCCESS_STATUS;
        }
        else
        {
            return ERROR_STATUS;
        }

    }

#elif defined(PC_PLATFORM)

    #include <fstream>
    #include <iostream>

    using namespace std;

    uint8_t* allocate_buffer_for_flash(uint32_t flash_size)
    {
        uint8_t* flash = (uint8_t*) malloc( flash_size );
        for(uint32_t i = 0 ; i < ( flash_size ); i++ )
        {
            flash[i] = 0xFF;
        }
        ifstream file; // открываем файл в конструкторе
        file.open( "settingsClassFileSystemContainer.cfg", ios::binary ); // fstream::in
        if( file.is_open() )
        {
            file.read( (char*) flash, flash_size );
            file.close();
        }
        return flash;
    }

    uintptr_t Settings::_getStartAddress()
    {
        return 0;
    }

    uint32_t Settings::_getBlockSize()
    {
        return SETTINGS_FLASH_BLOCK_SIZE;
    }

    uint32_t Settings::_GetBlockCount()
    {
        return SETTINGS_FLASH_BLOCK_USED;
    }

    RSTATUS Settings::_writeBlock( address_t address, void *buffer )
    {

        if( flash == 0 )
        {
            flash = allocate_buffer_for_flash( SETTINGS_FLASH_BLOCK_SIZE * SETTINGS_FLASH_BLOCK_USED );
        }

        uint8_t* inBlock  = static_cast<uint8_t*> (buffer);
        uint8_t* outBlock = reinterpret_cast<uint8_t*> ( flash + address - startAddress);

        for( address_t i = 0 ; i < blockSize; i++ )
        {

            outBlock[i] = inBlock[i];

        }

        ofstream file; // открываем файл в конструкторе

        file.open( "settingsClassFileSystemContainer.cfg", ios::binary | ios::trunc ); // fstream::in

        printf("Open file\r\n");

        if( file.is_open() )
        {

            printf("Rewrite file\r\n");

            file.write( (char*) flash, blockSize * blockCount );

            file.close();

        }
        else
        {
            printf("Error on open file\r\n");

        }

        return SUCCESS_STATUS;

    }

    RSTATUS Settings::_readBlock( address_t address, void* buffer )
    {

        if( flash == 0 )
        {
            flash = allocate_buffer_for_flash( SETTINGS_FLASH_BLOCK_SIZE * SETTINGS_FLASH_BLOCK_USED );
        }

        // Реализация чтения блока из flash памяти
        //  По-умолчанию память считается внутренней чтение идет как обращение к массиву в памяти программ.

        uint8_t* inBlock;
        uint8_t* outBlock;

        inBlock  = reinterpret_cast<uint8_t*> ( flash + address - startAddress);
        outBlock = static_cast<uint8_t*> (buffer);

        for( address_t i = 0 ; i < blockSize; i++ )
        {

            outBlock[i] = inBlock[i];

        }

        return SUCCESS_STATUS;

    }

    RSTATUS Settings::_eraseBlock( address_t address )
    {

        if( flash == 0 )
        {
            flash = allocate_buffer_for_flash( SETTINGS_FLASH_BLOCK_SIZE * SETTINGS_FLASH_BLOCK_USED );
        }

        // Реализация стирания блока flash памяти, по-умолчанию отсутствует

        uint8_t* outBlock = reinterpret_cast<uint8_t*> ( flash + address - startAddress);

        for( address_t i = 0 ; i < blockSize; i++ )
        {

            outBlock[i] = 0xFF;

        }

        return SUCCESS_STATUS;

    }

#else

    #error "This platform not supported!"

#endif
