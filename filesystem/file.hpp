#ifndef FILE_HPP
#define FILE_HPP

#include "platform.hpp"
#include "magic.hpp"

/**
 *
 * @brief Работа с файлом
 *
 */

class File
{

public:

    File();

    File( uintptr_t _startOffset, uintptr_t _size, Magic::TYPE _type );

    uintptr_t getStartOffset();
    uintptr_t getSize();
    uintptr_t getCurrentOffset();

    bool isOpened();
    bool isEnd();

    void setOpened( bool _opened );
    void setCurrentOffset( uintptr_t _currentOffset );

    Magic::TYPE getContentType();

private:

    bool        opened;
    uintptr_t   startOffset;
    uintptr_t   size;
    Magic::TYPE type;
    uintptr_t   currentOffset;

};

#endif
