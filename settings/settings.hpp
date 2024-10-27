#ifndef SETTINGS_H
#define SETTINGS_H

#include "platform.hpp"
#include "crc.hpp"

/**

    @brief Надежная запись и чтение из/в энергонезависимую память.

    Как использовать:

    #include "Settings.hpp"

    void setup()
    {

        settings.init();

        struct {
            int a;
            char b[10];
            ...
            float c;
        } mySettings;

        // Запись

        settings.write( &mysettings, sizeof(mySettings) );
        settings.commit(); // Записали данные один раз

        settings.write( &mysettings, sizeof(mySettings) );
        settings.commit(); // Второй раз - резервная копия

        // Чтение

        settings.read( &mysettings, sizeof(mySettings) );

    }

    Желательно проверять коды возрата функций, описания возвращаемых кодов в *.cpp файле

*/

class Settings
{

public:

    // ------- Основная функциональность

    RSTATUS init();
    void    deinit();

    payload_length_t read( void* value, payload_length_t len );

    payload_length_t write( void *value, payload_length_t len );
    RSTATUS commit();

    // ------- Дополнительные возможности

    version_t        getVersion();
    payload_length_t getLength();
    payload_length_t getTotalCapacity();

    page_index_t getNecessarySettingsSizeInBlocks( payload_length_t maximumSettingsLength, page_index_t numberOfBackupCopies );

    // ------- Специфические возможности:

    void replaceStorage( address_t _startAddress, payload_length_t _blockSize, page_index_t _blockCount );
    void setDefaultStorage();

    RSTATUS erase( address_t _startAddress, payload_length_t _blockSize, page_index_t _blockCount );

private:

    // ----------- Платформо-зависимые функции

    RSTATUS  _readBlock( address_t address, void* buffer );
    RSTATUS  _writeBlock( address_t address, void* buffer );
    RSTATUS  _eraseBlock( address_t address );

    uintptr_t _getStartAddress();

    uint32_t _getBlockSize();
    uint32_t _GetBlockCount();

    // -----------

    address_t            startAddress;
    payload_length_t     blockSize;
    page_index_t         blockCount;

    page_index_t _roundUp( float value );

    payload_length_t _getServiceDataSize();

    enum BLOCK_TYPE
    {
        LAST_BLOCK      = 1,
        CONTINUED_BLOCK = 0
    };

    enum
    {
        SETTINGS_READ_TRY_MAX  = 3, // Сколько раз пытаться считать заново блок из flash памяти если он не считался правильно или поврежден
        SETTINGS_WRITE_TRY_MAX = 3  // Сколько раз пытаться записать заново блок в flash память если он не записался правильно
    };

    enum STATES
    {
        PAGE_WRITE_COMPLETE = 0
    };

    // Информация о доступных в памяти настройках

    struct Settings_entry_t
    {

        version_t    version;         // Версия последней записи. Если 0 то в памяти не было/нет целых записей.
        page_index_t writeStartIndex; // Индекс элемента, куда можно производить запись
        page_index_t readStartIndex;  // Индекс элемента, откуда можно производить чтение
        page_index_t readIndex;       //
        page_index_t totalPages;      // Количество страниц flash памяти, которые надо пройти чтобы полностью считать
                                      //  доступную для чтения запись(учитывается то, что по пути быть испорченные страницы,
                                      //  в которые уже нельзя ничего записать)
        payload_length_t totalPayloadLength; // Общая длина записи/полезной нагрузки/payload в байтах
     };

     // Буферы для чтения/записи настроек

    payload_length_t writePosition;        // Позиция указателя в текущей записи
    payload_length_t readPosition;         // Позиция чтения
    payload_length_t payloadBlockSize;

    uint8_t readBuffer[4096] __attribute__ ((aligned (8)));      // Текущий буфер для чтения
    uint8_t writeBuffer[4096] __attribute__ ((aligned (8)));     // Текущий буфер для записи

    struct flash_block_t
    {
        uint8_t*         payload;       // Массив с "полезной нагрзукой" блока
        payload_length_t payloadSize;   // Максимальный размер "полезной нагрузки" в блоке
        // Служебная информация
        uint32_t         version;       // версия настроек, начинается с единицы
        uint32_t         pageNumber;    // номер страницы, относящейся к текущей версии настроек
        uint32_t         crc;           // контрольная сумма
        uint32_t         payloadLength; // Сколько реально было записано payload в эту страницу
        uint32_t         next;          //  0 --- продолжение текущих настроек
        uint32_t         checkbyte;
    };

     Settings_entry_t   entry;

     bool              lastVersionFounded;

     RSTATUS   _startRead();
     RSTATUS   _writeCompleteBlock( BLOCK_TYPE type );

     bool _isBlockWriteSuccess( flash_block_t* p );
     bool _isBlockChecksumCorrect( flash_block_t* p );
     bool _isFirstBlock( flash_block_t* p );
     bool _isLastBlock( flash_block_t* p );
     bool _isStartOfRead();
     bool _isEndOfRead();
     bool _isStartOfWrite();
     bool _isBlockWriteSuccessFlagCorrupted( flash_block_t* p );

     RSTATUS _restoreSuccessWriteFlag();
     RSTATUS _findLastSettingsVersion();
     RSTATUS _readNextPage( flash_block_t* p );

     page_index_t     _getPageNumber( flash_block_t* p );
     payload_length_t _getBlockPayloadLength( flash_block_t* p );
     version_t        _getBlockVersion( flash_block_t* p );

     void _setBufferFromBlockDescription( flash_block_t* p, uint8_t* block_buffer );
     void _getBlockDescriptionFromBuffer( flash_block_t* p, uint8_t* block_buffer );
     void _setBlockVersion( flash_block_t* p, version_t version );
     void _setBlockPageNumber( flash_block_t* p, page_index_t page );
     void _setBlockPayloadLength( flash_block_t* p, payload_length_t payload );
     void _setBlockWriteComplete( flash_block_t* p );
     void _setBlockChecksum( flash_block_t* p );
     void _setBlockLast( flash_block_t* p );
     void _unsetBlockLast( flash_block_t* p );

     // Получить адрес из индекса блока
     //
     //  address   - адрес с которого начинаются блоки
     //  blocksize - размер блока в байтах
     //  index     - номер/индекс блока
     //

     inline address_t _getAddressFromIndex( payload_length_t index )
     {

         return  ( ( startAddress ) + ( ( blockSize ) * ( index % blockCount ) ) );

     }

     // Получить размер "полезной нагрузки" в блоке

     inline payload_length_t _getPayloadBlockSize()
     {

        return  ( blockSize - 24 );

     }

#if defined(PC_PLATFORM)
    uint8_t* flash = 0;
#endif

};

extern Settings settings;

#endif
