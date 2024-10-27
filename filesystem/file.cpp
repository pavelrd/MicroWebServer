#include "file.hpp"

/**
 * @brief File::File
 */

File::File()
{
    opened = false;
    startOffset = 0;
    size = 0;
}

/**
 * @brief File::File
 * @param _startOffset
 * @param _size
 * @param _type
 */

File::File( uintptr_t _startOffset, uintptr_t _size, Magic::TYPE _type )
{
    opened        = true;
    startOffset   = _startOffset;
    currentOffset = _startOffset;
    size          = _size;
    type          = _type;
}

/**
 * @brief File::getContentType
 * @return
 */

Magic::TYPE File::getContentType()
{
    return type;
}

/**
 * @brief File::getStartOffset
 * @return
 */

uintptr_t File::getStartOffset()
{
    return startOffset;
}

/**
 * @brief File::getSize
 * @return
 */

uintptr_t File::getSize()
{
    return size;
}

/**
 * @brief File::getCurrentOffset
 * @return
 */

uintptr_t File::getCurrentOffset()
{
    return currentOffset;
}

/**
 * @brief File::isOpened
 * @return
 */

bool File::isOpened()
{
    return opened;
}

/**
 * @brief File::setOpened
 * @param _opened
 */

void File::setOpened( bool _opened )
{
    opened = _opened;
}

/**
 * @brief File::setCurrentOffset
 * @param _currentOffset
 */

void File::setCurrentOffset( uintptr_t _currentOffset )
{

    currentOffset = _currentOffset;

}

/**
 * @brief File::isEnd
 * @return
 */

bool File::isEnd()
{

    if( currentOffset >= ( startOffset + size )  )
    {
        return true;
    }
    else
    {
        return false;
    }

}
