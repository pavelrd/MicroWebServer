#include "settings.hpp"

/**
 *
 * @brief Инициализация, сделана не в конструкторе, потому что в нем она срабатывает не на каждой платформе
 *
 * @return SUCCESS_STATUS - успешна, ERROR_STATUS - ошибка
 *
 */

RSTATUS Settings::init()
{

    entry.version            = 0;
    entry.writeStartIndex    = 0;
    entry.readStartIndex     = 0;
    entry.totalPages         = 0;
    entry.totalPayloadLength = 0;

    readPosition     = 0;
    writePosition    = 0;

    for( uint32_t i = 0 ; i < _getBlockSize(); i++ )
    {
        writeBuffer[i] = 0xFF;
        readBuffer[i]  = 0xFF;
    }

    if ( ! ( blockSize > _getServiceDataSize() ) )
    {
        return ERROR_STATUS;
    }

    if ( ! ( ( blockCount * blockSize ) >= blockSize ) )
    {
        return ERROR_STATUS;
    }

    if ( blockCount <= 0 )
    {
        return ERROR_STATUS;
    }

    _restoreSuccessWriteFlag();

    lastVersionFounded = false;

    readPosition = 0;

    writePosition = 0;

    return SUCCESS_STATUS;

}

/**
 *
 * @brief Деинициализация, сделана не в деструкторе, потому что в нем она срабатывает не на каждой платформе
 *
 * @return SUCCESS_STATUS - успешна, ERROR_STATUS - ошибка
 *
 */

void Settings::deinit()
{
}

/**

    @brief Считать следующие len байт из flash памяти, по основному адресу или из резервной копии

    @param value буфер для считанных данных
    @param len сколько байт считывать

    @return Количество считанных байт или 0

*/

payload_length_t Settings::read( void *value, payload_length_t len )
{

    if( len == 0 ) 
    {
        // Запрошено 0 байт данных
        return 0;
    }

    if( _isStartOfRead() )
    {

        if ( ERROR_STATUS == _findLastSettingsVersion() ) 
        {
            return 0;
        }

    }

    if( _isEndOfRead() )
    {

        return 0;

    }

    flash_block_t readBlock;

    _getBlockDescriptionFromBuffer( &readBlock, &( readBuffer[0] ) );

    payload_length_t i;

    for( i = 0 ; ( i < len ) && ( !_isEndOfRead() ) ; i++, readPosition++ )
    {

        payload_length_t nowPosition = readPosition % _getPayloadBlockSize();

        if( 0 == nowPosition )
        {

            // Считываем следующую страницу

            if( ERROR_STATUS == _readNextPage( &readBlock ) )
            {

                return 0;

            }

        }

        (static_cast<uint8_t*>(value))[i] = readBuffer[nowPosition];

    }

    return i;

}

/**

    @brief Записать следующую часть настроек

    @param value буфер с данными
    @param len сколько байт записывать

    @return Количество записанных байт

*/

payload_length_t Settings::write( void* value, payload_length_t len )
{

    if( _isStartOfWrite() )
    {

        // Ищем последнюю версию настроек, чтобы произоводить запись
        //  настроек сразу после неё

        _findLastSettingsVersion();


        // Заполняем буфер единицами
        //  Единица означает что flash память стерта

        for( payload_length_t j = 0 ; j < blockSize ; j++ )
        {

            writeBuffer[j] = 0xFF;

        }

    }

    payload_length_t i;

    for( i = 0 ; ( i < len ) && ( writePosition < getTotalCapacity() ) ; i++, writePosition += 1 )
    {

        // Записывается следующий байт

        if( ( writePosition != 0 ) && ( 0 == ( writePosition % _getPayloadBlockSize() ) )  )
        { // ( i < ( len - 1 )

            // Буфер с блоком был полностью заполнен

            if( 0 > _writeCompleteBlock( CONTINUED_BLOCK ) )
            {

                return 0;

            }

            for( payload_length_t j = 0 ; j < blockSize ; j++ )
            {

                writeBuffer[j] = 0xFF;

            }

        }

        writeBuffer[ writePosition % _getPayloadBlockSize() ] = (static_cast<uint8_t*>(value))[i];

    }

    return i;

}

/**
 *
 *  @brief Записать все недозаписанные(если есть) данные в память.
 *
 */

RSTATUS Settings::commit()
{

    // Записываем оставшиеся в буфере данные в память

    if( ERROR_STATUS > _writeCompleteBlock( LAST_BLOCK ) )
    {

        return ERROR_STATUS;

    }

    writePosition = 0;
    readPosition  = 0;

    lastVersionFounded = false;

    return SUCCESS_STATUS;

}

/**
 * @brief Settings::_setBufferFromBlockDescription
 * @param p
 * @param block_buffer
 */

void Settings::_setBufferFromBlockDescription( flash_block_t* p, uint8_t* block_buffer )
{

    uint32_t payloadBlockSize = _getPayloadBlockSize();

    //

    *((uint32_t*)(block_buffer + payloadBlockSize) )      = p->version;       // версия настроек, начинается с единицы
    *((uint32_t*)(block_buffer + payloadBlockSize + 4))   = p->pageNumber;    // номер страницы, относящейся к текущей версии настроек
    *((uint32_t*)(block_buffer + payloadBlockSize + 8))   = p->crc;           // контрольная сумма
    *((uint32_t*)(block_buffer + payloadBlockSize + 12))  = p->payloadLength; // Сколько реально было записано payload в эту страницу
    *((uint32_t*)(block_buffer + payloadBlockSize + 16))  = p->next;          //  0 --- продолжение текущих настроек
    *((uint32_t*)(block_buffer + payloadBlockSize + 20))  = p->checkbyte;     //  успешное окончание записи

}

/**
 *
 * @brief Соединяет структуру которая описывает блок с буфером где он хранится,
 *         после выполнения структура будет ссылаться на буфер.
 *
 * @param p описание блока
 * @param page_buffer буфер, где хранится блок
 *
 */

void Settings::_getBlockDescriptionFromBuffer( flash_block_t* p, uint8_t* block_buffer )
{

    p->payload     = &( block_buffer[0] );
    p->payloadSize = _getPayloadBlockSize();

    uint32_t payloadBlockSize = _getPayloadBlockSize();

    p->version       = *((uint32_t*)(block_buffer + payloadBlockSize));  // версия настроек, начинается с единицы
    p->pageNumber    = *((uint16_t*)(block_buffer + payloadBlockSize + 4)); // номер страницы, относящейся к текущей версии настроек
    p->crc           = *((uint32_t*)(block_buffer + payloadBlockSize + 8)); // контрольная сумма
    p->payloadLength = *((uint32_t*)(block_buffer + payloadBlockSize + 12)); // Сколько реально было записано payload в эту страницу
    p->next          = *((uint32_t*)(block_buffer + payloadBlockSize + 16)); //  0 --- продолжение текущих настроек
    p->checkbyte     = *((uint32_t*)(block_buffer + payloadBlockSize + 20)); //  0 --- продолжение текущих настроек

    // p->info        = reinterpret_cast<flash_block_info_t*> ( & ( block_buffer[ _getPayloadBlockSize() ] ) );

    //Serial::write("Block buffer: ");
    //Serial::write( (uint32_t)&(block_buffer[0]) );
    //Serial::write("\n Payload Size: ");

    //Serial::write( _getPayloadBlockSize() );
    //Serial::write("\n info: ");

    //Serial::write((uint32_t)& ( block_buffer[ _getPayloadBlockSize() ] ));

    // Serial::write("Sizeofs: ");
    // Serial::write( (uint32_t) sizeof(uint8_t) );
    // Serial::write(" ");
    // Serial::write( (uint32_t) sizeof(uint16_t) );
    // Serial::write(" ");
    // Serial::write( (uint32_t) sizeof(uint32_t) );

}

/**
 *
 * @brief Находит последнюю целую версию настроек
 *
 * @return SUCCESS - настройки найдены, ERROR - в памяти нет настроек(неповрежденных или впринципе нет)
 *
 */

RSTATUS Settings::_findLastSettingsVersion()
{

    if( lastVersionFounded )
    {
        return SUCCESS_STATUS;
    }

    entry.version            = 0;
    entry.totalPayloadLength = 0;

    flash_block_t p;

    for( page_index_t i = 0 ; i < blockCount ; i++ )  // Проходимся по всем страницам, которые выделены под настройки
    {

        for( uint8_t readCounter = 0; readCounter < SETTINGS_READ_TRY_MAX; readCounter++ ) // Пытаемся считать следующий блок в буфер
        {

            if ( 0 > _readBlock( _getAddressFromIndex(i), &(readBuffer[0]) ) )
            {

                continue;

            }

            _getBlockDescriptionFromBuffer( &p, &(readBuffer[0]) );

            // Ищем первый неповрежденный блок

            if ( ( !_isBlockWriteSuccess(&p) ) || ( !_isBlockChecksumCorrect(&p) ) )
            {
                continue;
            }

            // Считали успешно записанный блок

            if( ( _isFirstBlock(&p) ) && ( _isLastBlock(&p) ) )
            {

                // Нашли первую и единственную страницу в записи
                if( entry.version < _getBlockVersion(&p) )
                {

                    // Запоминаем текущую версию настроек в дескрипторе

                    entry.version            = _getBlockVersion(&p);
                    entry.readStartIndex     = i ; // индекс только что считанной страницы
                    entry.readIndex          = entry.readStartIndex;
                    entry.writeStartIndex    = ( i + 1 ) % (blockCount);
                    entry.totalPages         = 1 ; // количество страниц - одна
                    entry.totalPayloadLength = _getBlockPayloadLength(&p) ; // сколько байт можно считать функцией u_flash_read()

                    continue;

                }
                else
                {

                    // Уже найдена более поздняя версия настроек чем эта

                    break;

                }

            }
            else if( _isFirstBlock(&p) )
            {

                // Первая и ожидаются ещё другие страницы

                if( entry.version >= _getBlockVersion(&p) )
                {

                    // Уже найдена более поздняя версия настроек чем эта
                    //  дальнейшая проверка страниц не имеет смысла

                    break;

                }

                // Эти настройки пока-что самые поздние

                // Проверяем контрольные суммы остальных страниц, в которых содержатся эти настройки

                page_index_t     currentPage = 1; // Следующая страница этой версии должна быть под номером 1
                payload_length_t paylength   = _getBlockPayloadLength(&p); // Далее находим общий paylength для всех страниц
                version_t        version     = _getBlockVersion(&p);

                // Ищем остальные страницы
                //  Версия и номер должны совпадать с заданными, если это не так(эта запись с настройками повреждена)
                //   то продолжаем поиск в следующих страницах

                for( page_index_t j = ( i + 1 ) ; j < ( i + (blockCount) ) ; j++ )
                {

                    uint8_t readTryCount;

                    for( readTryCount = 0; readTryCount < SETTINGS_READ_TRY_MAX ; readTryCount++ )
                    {

                        // Пытаемся считать следующую страницу

                        if( ERROR_STATUS == _readBlock( _getAddressFromIndex( j % blockCount ), &(readBuffer[0]) ) )
                        {
                            continue;
                        }

                        _getBlockDescriptionFromBuffer( &p, &(readBuffer[0]) );

                        if( (_isBlockWriteSuccess(&p)) && (_isBlockChecksumCorrect(&p) ) )
                        {
                            break;
                        }

                    }

                    if( readTryCount >= SETTINGS_READ_TRY_MAX )
                    {
                        continue;
                    }

                    if( ( currentPage == _getPageNumber(&p) ) &&  ( version == _getBlockVersion(&p) ) )
                    {

                        if( _isLastBlock(&p) )
                        {

                            // Последняя страница

                            // Эта запись с настройками полностью корректна!
                            //
                            //  В дескрипторе запоминаем: версию, начало записи(индекс первой страницы), полную длину payload в байтах
                            //  общее количество страниц, которые нужно обойти, чтобы полностью считать запись

                            paylength += _getBlockPayloadLength(&p);

                            entry.version             = _getBlockVersion(&p);
                            entry.readStartIndex      = i;
                            entry.readIndex           = entry.readStartIndex;
                            entry.totalPages          = j - i + 1;
                            entry.writeStartIndex     = ( i + entry.totalPages ) % ( blockCount );
                            entry.totalPayloadLength  = paylength;

                            break;

                        }
                        else
                        {

                            currentPage += 1;
                            paylength   += _getBlockPayloadLength(&p);

                        }

                    } // else, пропуск этой страницы

                }

            } // else страница не является первой в записи, пропускаем её

        } // else блок был записан не до конца или поврежден

    }

    if( entry.version == 0 )
    {

        return ERROR_STATUS;

    }
    else
    {

        lastVersionFounded = true;

        return SUCCESS_STATUS;

    }

}



/**

    @brief Считывает в буфер для чтения следующую по номеру страницу

    @return 0 - страница найдена, -1 - страница не найдена

*/

RSTATUS Settings::_readNextPage( flash_block_t* p )
{

    // От текущей страницы идем по памяти дальше в поисках следующей

    for(  ; entry.readIndex < ( entry.readStartIndex + blockCount ) ; (entry.readIndex)++ )
    {

        uint8_t readCounter;

        for( readCounter = 0 ; readCounter < SETTINGS_READ_TRY_MAX ; readCounter++ )
        {

            if( ERROR_STATUS == _readBlock( _getAddressFromIndex( entry.readIndex ), &(readBuffer[0]) ) )
            {
                continue;
            }

            _getBlockDescriptionFromBuffer(p, &(readBuffer[0]) );

            if( ( _isBlockWriteSuccess(p) ) && ( _isBlockChecksumCorrect(p) )  )
            {
                break;
            }

        }

        if( readCounter >= SETTINGS_READ_TRY_MAX )
        {
            // С нескольких попыток не удалось считать страницу
            continue;
        }

        if ( ( _getPageNumber(p) == (readPosition / _getPayloadBlockSize()) ) && ( entry.version == _getBlockVersion(p) ) )
        {

            // Нашли страницу

            return SUCCESS_STATUS;

        }

    }

    return ERROR_STATUS;

}

#include "debug.hpp"

/**
*
*   @brief Добавить служебную информацию и записать блок в память
*         записанный блок проверяется, если его не удалось записать делается несколько повторных попыток.
*
*   @param LAST_BLOCK - этот блок будет последним, CONTINUED_BLOCK - будут записаны ещё блоки
*
*   @return 0 - успешно записана, -1 - ошибка
*
*/

RSTATUS Settings::_writeCompleteBlock( BLOCK_TYPE type )
{

    // val --- индекс по которому записывается текущая страница

    flash_block_t writeBlock;

    _getBlockDescriptionFromBuffer( &writeBlock, &(writeBuffer[0]) );

    if( type )
    {

        _setBlockLast( &writeBlock );

    }
    else
    {

       _unsetBlockLast( &writeBlock );

    }

    // Serial::write("Set block ok\n");
    // Serial::write("Try info\n");
    // Serial::write( (uint32_t) ( ((flash_block_t*) writeBlock)->info ) );
    // Serial::write("\nPayload block size\n");
    // Serial::write( _getPayloadBlockSize() );
    // Serial::write("\nBlock size\n");
    // Serial::write( _getBlockSize() );
    // Serial::write("\nFlash block info size:\n");
    // Serial::write ( (uint32_t)sizeof(flash_block_info_t));
    // Serial::write("\nTry version\n");
    // flash_block_info_t* faddr = (flash_block_info_t*) baddr;

     // = 100; // entry.version + 1;

    _setBlockVersion( &writeBlock, entry.version + 1 );

    page_index_t nowWritePageIndex = entry.writeStartIndex;

    if( writePosition != 0 )
    {

        nowWritePageIndex += ( writePosition - 1 ) / _getPayloadBlockSize();

    }

    _setBlockPageNumber( &writeBlock, 0 );

    if( writePosition == 0 )
    {

        _setBlockPayloadLength( &writeBlock , 0 );

    }
    else if( 0 == ( writePosition % _getPayloadBlockSize() ) )
    {

        _setBlockPayloadLength( &writeBlock, _getPayloadBlockSize() );

    }
    else
    {

        _setBlockPayloadLength( &writeBlock, writePosition % _getPayloadBlockSize() );

    }

    _setBlockChecksum( &writeBlock );

    _setBlockWriteComplete( &writeBlock );

    // Пытаемся записать блок в памяти. если запись не удалась то пишем блок в следующую страницу и.т.д
    //  пока блок не будет записан.

    for( page_index_t browsedPages = 0; browsedPages < blockCount ; nowWritePageIndex++, browsedPages++ )
    {

        // Идем вперед по памяти пока

        for( uint8_t writeCounter = 0 ; writeCounter < SETTINGS_WRITE_TRY_MAX ; writeCounter++ )
        {

            if( ERROR_STATUS == _eraseBlock( _getAddressFromIndex( nowWritePageIndex ) ) )
            {
                continue;
            }

            _setBufferFromBlockDescription( &writeBlock, &(writeBuffer[0]) );

            if( ERROR_STATUS == _writeBlock( _getAddressFromIndex( nowWritePageIndex ), &(writeBuffer[0]) ) )
            {
                continue;
            }

            //uint8_t* readBlockBuffer = static_cast<uint8_t*> ( malloc( blockSize ) );

            //if( 0 == readBlockBuffer )
            //{

                // Нет памяти для считывания записанного блока в отдельный буфер
                //  просто проверяем правильно ли записали блок, без возможности
                //  попробовать ещё раз записать блок

            flash_block_t block; // = static_cast<flash_block_t*> ( malloc( sizeof(flash_block_t) ) );

            for( uint8_t readCounter = 0 ; readCounter < SETTINGS_READ_TRY_MAX ; readCounter++ )
            {

                if( ERROR_STATUS == _readBlock( _getAddressFromIndex( nowWritePageIndex ), &(writeBuffer[0]) ) )
                {
                    continue;
                }

                _getBlockDescriptionFromBuffer( &block, &(writeBuffer[0]) );

                if( ( _isBlockChecksumCorrect(&block) ) && ( _getBlockVersion(&block) == ( entry.version + 1 ) )  )
                {
                    return SUCCESS_STATUS; // Блок был успешно записан
                }

            }

            return ERROR_STATUS;
            /*
            }
            else

            {

                flash_block_t readBlock; // = static_cast<flash_block_t*> ( malloc( sizeof(flash_block_t) ) );

                for( uint8_t readCounter = 0; readCounter < SETTINGS_READ_TRY_MAX ; readCounter++ )
                {

                    if( 0 > _readBlock( _getAddressFromIndex( nowWritePageIndex ), &(readBlockBuffer[0]) ) )
                    {
                        continue;
                    }

                    _getBlockDescriptionFromBuffer( &readBlock, &(readBlockBuffer[0]) );

                    if( _isBlockChecksumCorrect(&readBlock) && ( _getBlockVersion(&readBlock) == ( entry.version + 1 ) ) )
                    {

                        free(readBlockBuffer);

                        return SUCCESS_STATUS; // Блок был успешно записан

                    }

                }

                return ERROR_STATUS;

            }
            */

        }

    }

    return ERROR_STATUS;

}

/**

    @brief Получить размер(сколько байт можно считать) записи в байтах

    @param d текущий контекст

    @return размер записи в байтах

*/

payload_length_t Settings::getLength()
{

    if( ERROR_STATUS == _findLastSettingsVersion() )
    {
        return 0;
    }

    return entry.totalPayloadLength;

}

/**

    @brief Сколько байт можно записать во всю выделенную под настройки память

    @param d текущий контекст

    @return общее количество байт, доступных для записи "полезной нагрузки"

*/

payload_length_t Settings::getTotalCapacity()
{

    return blockCount * _getPayloadBlockSize();

}

/**

    @brief Получить текущую версию настроек

    @return 0 - не удалось загрузить настройки, запрещенный номер версии. Иначе номер версии текущих настроек

 */

version_t Settings::getVersion()
{

    if( ERROR_STATUS == _findLastSettingsVersion() )
    {
        return 0;
    }

    return entry.version;

}

/**

    @brief Получить количество служебной информации в байтах, добавляемой в кадый блок

    @return количество служебной информации в байтах

*/

payload_length_t Settings::_getServiceDataSize()
{

    return 21;

}

/**
 *
 * @brief Получить версию страницы
 * @param p текущая страница
 * @return номер версии страницы
 *
 */

version_t Settings::_getBlockVersion( flash_block_t* p )
{

    return p->version;

}

/**
 *
 * @brief Установить версию страницы
 * @param p текущая страница
 * @return номер версии страницы
 *
 */

void Settings::_setBlockVersion( flash_block_t* p, version_t version )
{

    // Serial::write("Set block version\n");

    p->version = version;

    // Serial::write("Set block version ----> OK\n");

}

/**

    @brief Проверяет была ли страница записана до конца

    @param page текущая страница

    @return 1 - была записана до конца, 0 - запись страницы была не закончена


*/

bool Settings::_isBlockWriteSuccess( flash_block_t* p )
{


    if( p->checkbyte != 0xFF )
    {
        return true;
    }
    else
    {
        return false;
    }

}

// -------------- Методы, обслуживающие работу класса

/**
 *
 * @brief Settings::_isBlockWriteSuccessFlagCorrupted
 * @param p
 * @return
 *
 */

bool Settings::_isBlockWriteSuccessFlagCorrupted( flash_block_t* p )
{


    if( p->checkbyte != 0x00 )
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
 * @brief Settings::_setBlockChecksum
 * @param p
 *
 */

void Settings::_setBlockChecksum( flash_block_t* p )
{

    uint32_t nowCrc = CRC::crc32( static_cast<uint32_t> (0), p->payload, p->payloadSize );

    nowCrc = CRC::crc32( nowCrc, (uint8_t*) &(p->version),       sizeof(p->version)       );
    nowCrc = CRC::crc32( nowCrc, (uint8_t*) &(p->pageNumber),    sizeof(p->pageNumber)    );
    nowCrc = CRC::crc32( nowCrc, (uint8_t*) &(p->payloadLength), sizeof(p->payloadLength) );
    nowCrc = CRC::crc32( nowCrc, (uint8_t*) &(p->next),          sizeof(p->next)          );

    p->crc = nowCrc;

}

/**

    @brief Проверка контрольной суммы страницы

    @param p страница

    @return true - контрольная сумма правильна, false - контрольная сумма ошибочна

*/

bool Settings::_isBlockChecksumCorrect( flash_block_t* p )
{

    uint32_t nowCrc = CRC::crc32( static_cast<uint32_t> (0), p->payload, p->payloadSize );

    nowCrc = CRC::crc32( nowCrc, (uint8_t*) &(p->version),       sizeof(p->version)       );
    nowCrc = CRC::crc32( nowCrc, (uint8_t*) &(p->pageNumber),    sizeof(p->pageNumber)    );
    nowCrc = CRC::crc32( nowCrc, (uint8_t*) &(p->payloadLength), sizeof(p->payloadLength) );
    nowCrc = CRC::crc32( nowCrc, (uint8_t*) &(p->next),          sizeof(p->next)          );

    if( p->crc == nowCrc )
    {
        return true;
    }
    else
    {
        return false;
    }

}

/**
 * @brief Settings::_getPageNumber
 * @param p
 * @return
 */

page_index_t Settings::_getPageNumber( flash_block_t* p )
{

    return p->pageNumber;

}

/**
 *
 * @brief Установить флаг успешной записи страницы в состояние запись завершена
 * @param p текущая страница
 *
 */

void Settings::_setBlockWriteComplete( flash_block_t* p )
{

    p->checkbyte = 0x00;

}

/**
 *
 * @brief Получить количество "полезной нагрузки" на странице
 * @param p текущий блок
 * @return количество в байтах
 *
 */

payload_length_t Settings::_getBlockPayloadLength( flash_block_t* p )
{


    return p->payloadLength;

}

/**
 *
 * @brief Проверяет является ли блок первым
 * @param p текущий блок
 * @return true - первая, false - не первая
 *
 */

bool Settings::_isFirstBlock( flash_block_t* p )
{


    if( p->pageNumber == 0 )
    {
        return true;
    }
    else
    {
        return true;
    }

}

/**
 *
 * @brief Проверяет является ли блок последним
 * @param p текущий блок
 * @return true - последний, false - не последний
 *
 */

bool Settings::_isLastBlock( flash_block_t* p )
{


    if( p->next == 0xFF ){
        return true;
    } else {
        return false;
    }

}
/**
 *
 * @brief Восстановить признак успешной записи
 *
 * @return статус восстановления
 *
 */

RSTATUS Settings::_restoreSuccessWriteFlag()
{

    // В конце каждой страницы есть 1 байтовый флаг-признак успешной записи
    //  он может быть записан не до конца из за отключения питания, ошибки, и.т.д

    flash_block_t block; //  = static_cast<flash_block_t*> ( malloc(sizeof(flash_block_t)) );

   // uint8_t* buffer;

   // buffer = static_cast<uint8_t*> ( malloc( blockSize ) );

    _getBlockDescriptionFromBuffer( &block, &(readBuffer[0]) );

    for( page_index_t i = 0 ; i < blockCount ; i++ )
    {

        for( uint8_t readCount = 0; readCount < SETTINGS_READ_TRY_MAX; readCount++ )
        {

            if ( 0 > _readBlock( _getAddressFromIndex(i), &(readBuffer[0]) ) )
            {
                continue;
            }

            _getBlockDescriptionFromBuffer( &block, &(readBuffer[0]) );

            if( (!_isBlockWriteSuccess(&block)) || (!_isBlockChecksumCorrect(&block)) )
            {
                continue;
            }

            // Удалось считать блок

            if( _isBlockWriteSuccessFlagCorrupted(&block) )
            {

                // Флаг того что страница была усешно записана повержден, восстановление флага

                _setBlockWriteComplete(&block);

                _setBufferFromBlockDescription(&block, &(readBuffer[0]));

                for( uint8_t readCount = 0; readCount < SETTINGS_READ_TRY_MAX; readCount++ )
                {

                    if ( 0 > _writeBlock( _getAddressFromIndex(i), &(readBuffer[0]) ) )
                    {
                        continue;
                    }

                    if( 0 > _readBlock( _getAddressFromIndex(i), &(readBuffer[0])  ) )
                    {
                        continue;
                    }

                    _getBlockDescriptionFromBuffer( &block, &(readBuffer[0]) );

                    if( _isBlockWriteSuccess(&block) )
                    {
                        // Флаг успшености записи был восстановлен
                        break;
                    }

                }

            } // Флаг успешности записи цел, нет необходимости его перезаписывать
        }

    }

    return SUCCESS_STATUS;

    // -------

}

/**
 *
 * @brief Проверяет закончено ли чтение настроек
 *
 * \return true - закончено, false - продолжается
 *
 */

bool Settings::_isEndOfRead()
{

    if ( ( readPosition >= entry.totalPayloadLength ) )
    {
        return true;
    }
    else
    {
        return false;
    }

}

/**
 * @brief Settings::_isStartOfRead
 * @return
 */

bool Settings::_isStartOfRead()
{

    if( readPosition <= 0 )
    {
        return true;
    }
    else
    {
        return false;
    }

}

/**
 * @brief Settings::_isStartOfWrite
 * @return
 */

bool Settings::_isStartOfWrite()
{
    if( writePosition == 0 )
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
 * @brief Установить номер страницы для блока
 * @param p текущий блок
 * @param page номер страницы
 *
 */

void Settings::_setBlockPageNumber( flash_block_t* p, page_index_t page )
{

    p->pageNumber = page;

}

/**
 *
 * @brief Задать сколько байт занимает "полезная нагрузка" блока
 * @param p текущий блок
 * @param payload количество байт "полезной нагрузки" которая есть в этом блоке
 * @return
 *
 */

void Settings::_setBlockPayloadLength( flash_block_t* p, payload_length_t payload )
{

    p->payloadLength = payload;

}


/**

    @brief Делает блок последним
    @param p

*/

void Settings::_setBlockLast( flash_block_t* p )
{

    p->next = 0xFF;

}

/**

    @brief Указывает что за этим блоком будут следующие
    @param p

*/

void Settings::_unsetBlockLast( flash_block_t* p )
{

    p->next = 0x00;

}

/**

    @brief Округляет число вверх до целого

    @param value число которое нужно округлить

    @return округленное целое число

*/

page_index_t Settings::_roundUp( float value )
{

    if( value - ( (page_index_t) value ) > 0 )
    {

        return ( ( static_cast<page_index_t> (value) ) + 1 );

    }
    else
    {

        return ( static_cast<page_index_t> (value) );

    }

}

/**

    @brief Получить размер памяти, который нужно зарезервировать под настройки

    @param maximumSettingsLength максимальный размер настроек в байтах который планируется хранить
    @param numberOfBackupCopies количество копий настроек, которые могут одновременно находиться в памяти
    @param blocksize размер страницы/блока flash памяти

    @return ERROR - ошибка, иначе количество памяти

*/

page_index_t Settings::getNecessarySettingsSizeInBlocks(payload_length_t maximumSettingsLength, page_index_t numberOfBackupCopies )
{

    /*

        Формула выделения памяти под настройки:

            T - Общее количество блоков, которые необходимо выделить под настройки
            R - Размер настроек в байтах
            B - Размер блока
            M - Запас - количество версий настроек, которые могут одновременно хранится в памяти

            B > getServiceDataSize()

        T = B * ( ( roundUp( R / (B - Settings::getServiceDataSize()) ) * M ) + 1 );

        roundUp() - округляет значение вверх до ближайшего целого

    */

    if( blockSize < _getServiceDataSize() ){
        return ERROR_STATUS;
    }

    return blockSize * ( ( _roundUp( ((float)maximumSettingsLength) / ( (float)( blockSize - _getServiceDataSize() ) ) ) * numberOfBackupCopies ) + 1 );

}

/**
 *
 * @brief Заменить место куда записываются опции
 * @param _startAddress начальный адрес
 * @param _blockSize размер блока
 * @param _blockCount количество блоков
 *
 */

void Settings::replaceStorage( address_t _startAddress, payload_length_t _blockSize, page_index_t _blockCount )
{
    startAddress = _startAddress;
    blockSize    = _blockSize;
    blockCount   = _blockCount;
}

/**
 *
 * @brief Установить место для записи по-умолчанию
 *
 */

void Settings::setDefaultStorage()
{
    startAddress = _getStartAddress();
    blockSize    = _getBlockSize();
    blockCount   = _GetBlockCount();
}

RSTATUS Settings::erase( address_t _startAddress, payload_length_t _blockSize, page_index_t _blockCount )
{

    for( uint32_t index = _startAddress; index < ( _startAddress + (_blockSize*_blockCount))  ; index += _blockSize )
    {

         if( ERROR_STATUS == _eraseBlock( index ) )
         {
             return ERROR_STATUS;
         }

    }

    return SUCCESS_STATUS;

}

Settings settings;

//uint8_t Settings::readBuffer[4096] ;      // Текущий буфер для чтения
//uint8_t Settings::writeBuffer[4096] __attribute__ ((aligned (8)));     // Текущий буфер для записи
