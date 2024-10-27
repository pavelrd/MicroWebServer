#ifndef CONNECTION_HPP
#define CONNECTION_HPP


/**
 *
 * @brief Хранит данные о сетевом соединении
 *
 */

class Connection
{

public:

    Connection()
    {
        id = 0;
    }

    enum TYPE
    {
        TCP,
        UDP,
        UNKNOWN
    };

    bool    isGetByte;
    uint8_t getByte;

    void* argument; // Дополнительный аргумент для передачи какого-либо объекта в поток вместе с соединением


    void* id; // Идентификатор соединения, используется зависимыми от аппаратной части функциями
              //  на разных системах это может быть дескриптор или объект соединения и.т.п

};

#endif
