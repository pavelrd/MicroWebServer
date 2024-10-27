#include "mutex.hpp"

#include "serial.hpp"

/**
 *
 * @brief инициализировать блокировку
 *
 */

void Mutex::init()
{

#if defined(PC_PLATFORM)

#elif defined(ESP32_PLATFORM) || defined(ESP8266_PLATFORM)

    mutex = xSemaphoreCreateMutex();

#endif

    isInitialized = true;

}

/**
 *
 * @brief Взять блокировку
 *
 */

void Mutex::lock()
{

    if(!isInitialized)
    {
        init();
    }

    #if defined(PC_PLATFORM)

        mutex.lock();

    #elif defined(ESP32_PLATFORM) || defined(ESP8266_PLATFORM)

        xSemaphoreTake( mutex, portMAX_DELAY );

    #endif

}

/**
 *
 * @brief Отдать блокировку
 *
 */

void Mutex::unlock()
{


    if(!isInitialized)
    {
        init();
    }

    #if defined(PC_PLATFORM)

        mutex.unlock();

    #elif defined(ESP32_PLATFORM) || defined(ESP8266_PLATFORM)

        xSemaphoreGive( mutex );

    #endif

}

