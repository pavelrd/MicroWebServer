#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include "file.hpp"
#include "platform.hpp"

#include "abstractflash.hpp"

#ifdef PC_PLATFORM

    #include <dir.h>
    #include <dirent.h>
    #include <vector>
    #include <iostream>
    #include <string>
    #include <fstream> // подключаем библиотеку

    using namespace std;

#endif

/**
 *
 * @brief Работа с файлами
 *
 */

class Filesystem
{

public:

    void init();
    bool isFileExist( const char* filename );
    bool isFileExist( uint32_t hashname );

    File     open( const char* filename );
    File     open( uint32_t hashname );

    uint32_t read( File& file, uint8_t* buffer, uint32_t count );

    enum SEEK_TYPE
    {
        SET  = 0,
        CUR  = 1,
        END  = 2,
        BACK = 3
    };

    uint32_t seek( File& file, int offset, SEEK_TYPE whence );

    void     close( File& file );

    uint32_t filesChecksumForExternalStorage();

    void setExternalStorage( AbstractFlash *_flash );

    void replaceSource( uint32_t _indexAddress, uint32_t _filesAddress );

private:

    // ----------- Платформо-зависимые функции

        uint8_t _read( uintptr_t address, uint8_t *buffer, uint32_t length);

    // ----------- Внутренние функции

#ifdef PC_PLATFORM
        void _listdir(string startDir, vector<string>* files, vector<string>* pathHolder );
#endif

    //
    // File* files[10]; // Максимум открытых файлов
    // char buffers[3][PAGE_SIZE];
    //
    //  Буфер + номер страницы
    // при read читаем страницу в буфер полностью
    //   при новом read из другой страницы исползуем второй буфер
    //

    // Запрос поступает на смещение, которое означает страницу...

    //
    // ---> Можно сделать небольшие буферы, например по 1024 байта
    //    === 7168 байт ram

    // Тогда в буфере хранится кусок страницы

    // Если запрос на чтение на уже считанную страницу то считываение данных из буфера
    //  далее если перешли на другую страницу опять ищем её в буферах,
    //  если в буферах нет считываем страницу в новый буфер

    // 3  - [...]
    // 9  - [...]
    // 47 - [...]


    typedef struct
    {
        uint32_t    name;
        Magic::TYPE type;
        uintptr_t   offset;
        uintptr_t   fileSize;
    } indexDescriptor;

    uint32_t filesCount;
    uint32_t openedfiles;

    uintptr_t indexAddress;
    uintptr_t filesAddress;

    indexDescriptor* _bsearch( const uint32_t hashKey );

    // Внутренняя память(по-умолчанию)

    static const indexDescriptor index[]; // Индексы, в конце нулевой индекс
    static const uint8_t         files[]; // Файлы

    AbstractFlash *flash;

};

extern Filesystem filesystem;

#endif
