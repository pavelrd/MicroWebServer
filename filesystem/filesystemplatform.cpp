#include "filesystem.hpp"

/*
 * \brief Считать один байт
 *
 * \param address адрес по которому надо считать байт
 * \return байт
 *
 */

uint8_t Filesystem::_read( uintptr_t address, uint8_t* buffer, uint32_t length )
{

    if( flash )
    {

        flash->read( address + 12, &(buffer[0]), length ); // Адрес плюс 12, так как нужно пропустить
                                                 //  заголовок, длину, контрольную сумму
        return 0;

    }
    else
    {

        uint8_t* arr = (uint8_t*) ( filesAddress + address );

        for( uint32_t i = 0 ; i < length ; i++ )
        {
            buffer[i] = arr[i];
        }

        return 0;

    }

}
