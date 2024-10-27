#ifndef THREAD_PLATFORM
#define THREAD_PLATROFM

#include "thread.hpp"

#include "serial.hpp"

#if defined(PC_PLATFORM)

    #include <thread>
    #include <unistd.h>

    void Thread::_run( threadHandler handler, const char* name, void* argument, uint32_t stackSize, uint32_t priority )
    {

        name = name;
        stackSize = stackSize;
        priority = priority;

        if ( argument == 0 )
        {

            std::thread thr( handler, (void*)0 );

            thr.detach();

        } else {

            std::thread thr( handler, argument );

            thr.detach();

        }

    }

    void Thread::_run( threadHandler handler, const char* name, void* argument, uint32_t stackSize )
    {

        name = name;

        stackSize = stackSize;

        if ( argument == 0 ) {

            std::thread thr( handler, (void*)0 );

            thr.detach();

        } else {

            std::thread thr( handler, argument );

            thr.detach();

        }

    }

    void Thread::_run( threadHandler handler, void* argument, uint32_t stackSize )
    {

        stackSize = stackSize;

        if ( argument == 0 ) {

            std::thread thr( handler, (void*)0 );

            thr.detach();

        } else {

            std::thread thr( handler, argument );

            thr.detach();

        }


    }

    void Thread::_infinityCycle()
    {

        while(1){

            sleep(1000);

        }

    }

    void Thread::_exit()
    {



    }

    void Thread::_yield()
    {
    }

    uint32_t Thread::_getFreeStackSize()
    {
        return 0;
    }

    uint32_t Thread::_getDefaultStackSize()
    {
        return 16384;
    }

    uint32_t Thread::_getFreeHeapSize()
    {
        return 1000000;
    }

#elif defined(ESP32_PLATFORM) || defined(ESP8266_PLATFORM)

    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"

    #define FREERTOS_DEFAULT_STACK_SIZE 2048

    #if defined(ESP8266_PLATFORM)

        #include "esp_system.h"

    #endif

#endif

#if defined(ESP32_PLATFORM) || defined(ESP8266_PLATFORM)

    uint32_t Thread::_getFreeHeapSize()
    {
        #if !defined(ESP8266_PLATFORM)
            return xPortGetFreeHeapSize();
        #else
            return esp_get_free_heap_size();
        #endif
    }

    void Thread::_run( threadHandler handler, const char* name, void* argument, uint32_t stackSize )
    {
        xTaskCreate( handler, name, stackSize, argument, 1, NULL );
    }

    void Thread::_run( threadHandler handler, const char* name, void* argument, uint32_t stackSize, uint32_t priority )
    {
        xTaskCreate( handler, name, stackSize, argument, priority, NULL );
    }

    void Thread::_run( threadHandler handler, void* argument, uint32_t stackSize )
    {

        xTaskCreate( handler, "Thread::", stackSize, argument, 1, NULL );

    }

    void Thread::_exit()
    {

            vTaskDelete(NULL);

    }

    void Thread::_yield()
    {

            taskYIELD();

    }

    uint32_t Thread::_getFreeStackSize()
    {

            return uxTaskGetStackHighWaterMark(NULL);
    }

    uint32_t Thread::_getDefaultStackSize()
    {

            return FREERTOS_DEFAULT_STACK_SIZE;

    }

    void Thread::_infinityCycle()
    {

            while(1)
            {
                vTaskDelay( 1000 / portTICK_PERIOD_MS );
            }
    }

#endif

#endif
