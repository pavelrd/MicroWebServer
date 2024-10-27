#include "filesystem.hpp"
#include "crc.hpp"
#include <string.h>

/**
 * @brief Filesystem::Filesystem
 */

void Filesystem::init()
{

    flash = 0;

#ifdef PC_PLATFORM

    filesAddress = 0;
    indexAddress = 0;

#else

    // Проходимся по индексу, определяем число файлов

    filesAddress = (uintptr_t) &(files[0]);

    indexAddress = (uintptr_t) &(index[0]);

    for( uint32_t i = 0 ; i < 100000 ; i++ )
    {

        if ( ( index[i].name ) != 0 )
        {

            filesCount += 1;

        }
        else
        {

            return;

        }

    }

#endif

}

/**
 * @brief Filesystem::isFileExist
 * @param filename
 * @return
 */

bool Filesystem::isFileExist( uint32_t filename )
{

    if ( 0 != _bsearch( filename ) )
    {

        return true;

    }
    else
    {

        return false;

    }

}

/**
 * @brief Filesystem::isFileExist
 * @param filename
 * @return
 */

bool Filesystem::isFileExist( const char* filename )
{

    if ( 0 != _bsearch( CRC::crc32( 0, (uint8_t*)filename, strlen(filename) ) ) )
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
 * @brief Открыть файл
 *
 * @param filename имя файла
 *
 * @return открытый файл
 *
 */

File Filesystem::open( const char* filename )
{

    return open( CRC::crc32( 0, (uint8_t*)filename, strlen(filename) ) );

}


/**
 *
 * @brief Открыть файл
 *
 * @param hashname хэш сумма имени файла
 *
 * @return открытый файл
 *
 */

File Filesystem::open( uint32_t hashname )
{

#ifdef PC_PLATFORM

    vector<string> pathHolder;
    vector<string> files;
    vector<string> commonFiles;

    _listdir( string(PC_PLATFORM_PROJECT_PATH) + "/web", &files, &pathHolder );

    pathHolder.clear();

    _listdir( string(PC_PLATFORM_SOURCES_PATH) + "/web", &commonFiles, &pathHolder );

    // Подключение общих файлов

    string      filePath    = "";
    Magic::TYPE fileType    = Magic::PLAIN;
    bool        isFileFound = false;

    for( long long unsigned int i = 0 ; i < files.size(); i++ )
    {
        if( CRC::crc32( 0, (uint8_t*)files.at(i).c_str(), strlen(files.at(i).c_str()) ) == hashname )
        {
            filePath    = string(PC_PLATFORM_PROJECT_PATH) + "/web" + files.at(i);
            fileType    = Magic::getContentTypeFromFilename( (const char*) files.at(i).c_str(), strlen( files.at(i).c_str() ) );
            isFileFound = true;
            break;
        }
    }

    if( !isFileFound )
    {
        for( long long unsigned int i = 0 ; i < commonFiles.size(); i++ )
        {
            if( CRC::crc32( 0, (uint8_t*)commonFiles.at(i).c_str(), strlen(commonFiles.at(i).c_str()) ) == hashname )
            {
                filePath    = string(PC_PLATFORM_SOURCES_PATH) + "/web" + commonFiles.at(i);
                fileType    = Magic::getContentTypeFromFilename( (const char*) commonFiles.at(i).c_str(), strlen( commonFiles.at(i).c_str() ) );
                isFileFound = true;
                break;
            }
        }
    }

    if( isFileFound )
    {

        ifstream file; // открываем файл в конструкторе

        file.open( filePath.c_str(), ios::binary ); // fstream::in

        if( !file.is_open() )
        {
            return File();
        }

        file.seekg( 0, std::ios_base::end );

        int size = file.tellg();

        file.seekg(0, std::ios_base::beg);

        char* buffer = (char*) malloc(size);

        file.read( buffer, size );

        file.close();

        return File( (uintptr_t)buffer, size, fileType );

    }
    else
    {
        return File();
    }

#else


    indexDescriptor* d = _bsearch( hashname );

    if( d == 0 )
    {

        return File();

    }
    else
    {

        return File( d->offset, d->fileSize, d->type );

    }

#endif

}

/**
 * @brief Filesystem::_bsearch
 * @param hashKey
 * @return
 */

Filesystem::indexDescriptor* Filesystem::_bsearch ( const uint32_t hashKey )
{

  uint32_t l = 0;

  uint32_t u = filesCount;

  while ( l < u )
  {

      uint32_t idx = ( l + u ) / 2;

      indexDescriptor* p = (indexDescriptor *) (((const char *) &(index[0])) + (idx * sizeof(indexDescriptor)));

      if( hashKey < p->name )
      {

          u = idx;

      }
      else if( hashKey > p->name )
      {

          l = idx + 1;

      }
      else
      {

          return p;

      }

    }

    return 0;

}


/**

    @brief Изменить смещение чтения файла

    @param descriptor дескриптор файла
    @param offset     величина смещения файла
    @param whence     тип смещения файла

    @return текущая позиция в файле

*/

uint32_t Filesystem::seek( File& file, int offset, SEEK_TYPE whence )
{

    if( file.isOpened() )
    {

        switch(whence)
        {
            case     SET  : file.setCurrentOffset(offset); break;
            case     CUR  : file.setCurrentOffset( file.getCurrentOffset() + offset ); break;
            case     END  : file.setCurrentOffset( file.getCurrentOffset() + offset );  break;
            default       : break;
        }

        return file.getCurrentOffset();

    }
    else
    {

        return 0;

    }

}

/**

    @brief Чтение данных из файла в буфер

    @param descriptor дескриптор файла
    @param buffer приемный буфер для файла
    @param count количество запрашиваемых байт

    @return 0 - файл полностью считан, нет больше данных для чтения
             иначе количество прочитанных байт

*/

uint32_t Filesystem::read( File& file, uint8_t *buffer, uint32_t count )
{

    if( ( file.isOpened() ) && ( buffer != 0 ) && ( count != 0 ) )
    {

        uint32_t readCount = file.getSize() -  ( file.getCurrentOffset() - file.getStartOffset() );

        if( count < readCount )
        {

            readCount = count;

        }

        _read( file.getCurrentOffset(), &(buffer[0]), readCount );

        file.setCurrentOffset( file.getCurrentOffset() + readCount );

        return readCount;

    }
    else
    {

        return 0;

    }

}

/**
 * @brief Filesystem::close
 * @param file
 */

void Filesystem::close( File& file )
{
#ifdef PC_PLATFORM
    if( file.getStartOffset() != 0 )
    {
        free((void*)file.getStartOffset());
    }
#else
    file = file;
#endif
}

/**
 *
 * @brief Возвращает вектор всех файлов
 *
 * @param startDir
 * @param files
 * @param fulldir
 *
 */

#ifdef PC_PLATFORM

void Filesystem::_listdir( string startDir, vector<string>* files, vector<string>* pathHolder )
{

    DIR *dir;

    struct dirent *ent;

    if ( (dir = opendir ( startDir.c_str() )) != NULL )
    {

        while ( (ent = readdir (dir)) != NULL )
        {

            if ( FILE_ATTRIBUTE_DIRECTORY & GetFileAttributesA( ( startDir + string("/") + string(ent->d_name) ).c_str() ) )
            {

                if( ( strcmp( ent->d_name, "." ) != 0 ) && ( strcmp( ent->d_name, ".." ) != 0 )  )
                {

                    pathHolder->push_back( string(ent->d_name) );

                    _listdir( startDir + string("/") + string(ent->d_name), files, pathHolder );

                }

            }
            else
            {

                string filePath = "";

                for( long long unsigned int i = 0 ; i < pathHolder->size(); i++ )
                {
                    filePath += "/" + pathHolder->at(i);
                }

                filePath += "/";

                filePath += ent->d_name;

                files->push_back(filePath);

            }

        }

        closedir (dir);
    }
    else
    {
     /* could not open directory */
        perror ("");
        return;
    }

    if ( !pathHolder->empty() )
    {
        pathHolder->pop_back();
    }

    return;

}

#endif

Filesystem filesystem;

void Filesystem::setExternalStorage( AbstractFlash *_flash )
{
    flash = _flash;
}

uint32_t Filesystem::filesChecksumForExternalStorage()
{
    uint32_t checksum = 0;

    checksum |= files[0];
    checksum |= ((uint32_t)files[1]) << 8;
    checksum |= ((uint32_t)files[2]) << 16;
    checksum |= ((uint32_t)files[3]) << 24;

    return checksum;

}
