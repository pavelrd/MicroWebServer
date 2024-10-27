#ifndef CRC_HPP_
#define CRC_HPP_

#include "platform.hpp"

/**
 *
 * @brief Подсчет контрольной суммы crc32 по алгоритму из библиотеки zlib
 *
 *    CRC32 полином -  x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1.
 *
 */

class CRC
{

public:

    static uint32_t crc32(uint32_t crc, const uint8_t *buf, uint32_t len);

private:

    static const uint32_t crc32Table[];

};

#endif
