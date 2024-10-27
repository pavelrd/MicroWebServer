#ifndef THREAD_HPP
#define THREAD_HPP

#include "platform.hpp"

typedef void (*threadHandler)( void* argument );

/**
 *
 * @brief Реализация потоков
 *
 *  Позволяет легко создавать один или несколько потоков, пример:
 *
 *  #include "thread.hpp"
 *
 *  void mythread(void* argument)
 *  {
 *      while(1)
 *      {
 *          // Какие-то действия
 *      }
 *  }
 *
 *  void setup(void)
 *  {
 *
 *      // Запустить поток mythread
 *
 *      Thread::run(mythread);
 *
 *  }
 *
 */

class Thread
{

public:

    typedef enum
    {
        BAD  = 0
    } STATE;

    static uint32_t run( threadHandler handler );
    static uint32_t run( threadHandler handler, uint32_t stackSize );
    static uint32_t run( threadHandler handler, void* argument );
    static uint32_t run( threadHandler handler, void* argument, uint32_t stackSize );
    static uint32_t run( threadHandler handler, const char* name, void* argument, uint32_t stackSize );
    static uint32_t run( threadHandler handler, const char* name, void* argument, uint32_t stackSize, uint32_t priority );

    static uint32_t getFreeStackSize();
    static uint32_t getFreeHeapSize();
    static void     printStats();

    static void yield();

    static void goToInfinityCycle();
    static void exit();

private:

    // ----------- Платформо-зависимые функции

    static void     _run( threadHandler handler, void* argument, uint32_t stackSize );
    static void     _run( threadHandler handler, const char* name, void* argument, uint32_t stackSize );
    static void     _run( threadHandler handler, const char* name, void* argument, uint32_t stackSize, uint32_t priority );

    static void     _exit();
    static void     _infinityCycle();
    static void     _yield();
    static uint32_t _getFreeStackSize();
    static uint32_t _getDefaultStackSize();
    static uint32_t _getFreeHeapSize();

    // -----------

};


#endif
