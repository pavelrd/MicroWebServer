#ifndef MUTEX_HPP
#define MUTEX_HPP

#include "platform.hpp"

#if defined(ESP8266_PLATFORM)

    #include "FreeRTOS.h"
    #include "semphr.h"

#elif defined(PC_PLATFORM)

    #include <thread>
    #include <mutex>

#endif

/**
 *
 * @brief Реализация взаимных блокировок mutex(mutual exclusion).
 *         Нужна для для синхронизации одновременно выполняющихся потоков.
 *         Если два потока одновременно захотят получить доступ к одному ресурсу, например на запись.
 *
 */

class Mutex
{

public:

    void init();
    void lock();
    void unlock();

private:

    bool isInitialized;

    #if defined(PC_PLATFORM)

        std::mutex mutex;

    #elif defined(ESP32_PLATFORM) || defined(ESP8266_PLATFORM)

        SemaphoreHandle_t mutex;

    #endif

};

#endif
