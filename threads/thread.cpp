#include "thread.hpp"
#include "serial.hpp"

/**
 *
 * @brief Получить размер всей оставшейся глобальной кучи(RAM памяти) в байтах
 *
 * @return сколько места осталось в куче в байтах
 *
 */

uint32_t Thread::getFreeHeapSize()
{
    return _getFreeHeapSize();
}

/**
 *
 * @brief Запустить планировщик задач и перейти в бесконечный цикл исполнения.
 *         Вызов этого метода необходим только на некоторых платфомах
 *
 */

void Thread::goToInfinityCycle()
{

    _infinityCycle();

}

/**
 *
 * @brief Запустить новый поток
 *
 * @param handler название функции, которая будет в нем выполняться
 *
 * @return всегда 1
 *
 */

uint32_t Thread::run( threadHandler handler )
{

    _run( handler, 0, _getDefaultStackSize() );

    return 1;

}

/**
 *
 * @brief Запустить новый поток
 *
 * @param handler название функции, которая будет в нем выполняться
 * @param stackSize использовать следующий размер стека вместо значения по-умолчанию
 *                  Это значение необходимо задать/увеличить если в каком-то потоке начнет происходить
 *                  переполнение стека. Переполнение можно отследить вызвав метод getFreeStackSize().
 *
 * @return всегда 1
 *
 */

uint32_t Thread::run( threadHandler handler, uint32_t stackSize )
{

    _run( handler, 0, stackSize );

    return 1;

}

/**
 *
 * @brief Запустить новый поток и передать в него аргумент
 *
 * @param handler название функции, которая будет в нем выполняться
 * @param argument будет передан в функцию, которая будет выполняться в потоке.
 *
 * @return всегда 1
 *
 */

uint32_t Thread::run( threadHandler handler, void* argument )
{

    _run( handler, argument, _getDefaultStackSize() );

    return 1;

}

/**
 *
 * @brief Запустить новый поток и передать в него аргумент
 *
 * @param handler название функции, которая будет в нем выполняться
 * @param argument будет передан в функцию, которая будет выполняться в потоке.
 * @param stackSize использовать следующий размер стека вместо значения по-умолчанию
 *                  Это значение необходимо задать/увеличить если в каком-то потоке начнет происходить
 *                  переполнение стека. Переполнение можно отследить вызвав метод getFreeStackSize().
 *
 * @return всегда 1
 *
 */

uint32_t Thread::run( threadHandler handler, void* argument, uint32_t stackSize )
{

    _run( handler, argument, stackSize );

    return 1;

}

/**
 *
 * @brief Запустить новый поток и передать в него аргумент
 *
 * @param handler название функции, которая будет в нем выполняться
 * @param argument будет передан в функцию, которая будет выполняться в потоке.
 * @param stackSize использовать следующий размер стека вместо значения по-умолчанию
 *                  Это значение необходимо задать/увеличить если в каком-то потоке начнет происходить
 *                  переполнение стека. Переполнение можно отследить вызвав метод getFreeStackSize().
 *
 * @return всегда 1
 *
 */

uint32_t Thread::run( threadHandler handler, const char* name, void* argument, uint32_t stackSize )
{

    _run( handler, name, argument, stackSize );

    return 1;

}

/**
 * @brief run
 * @param handler
 * @param name
 * @param argument
 * @param stackSize
 * @param priority
 * @return
 */

uint32_t Thread::run( threadHandler handler, const char* name, void* argument, uint32_t stackSize, uint32_t priority )
{
    _run( handler, name, argument, stackSize, priority );
    return 1;
}

/**
 *
 * @brief Завершить текущий поток
 *
 */

void Thread::exit()
{

    _exit();

}

/**
 *
 * @brief Принудительно переключить контекст. Снижает вероятность дальнейшего прерывания текущего
 *         потока. Когда контекст переключится обратно на текущий поток будет меньшая чем до переключения вероятность
 *         его прерывания другим потоком.
 *
 */

void Thread::yield()
{

    _yield();

}

/**
 *
 * @brief Получить размер всей оставшейся глобальной кучи(RAM памяти) в байтах
 *
 * @return сколько места осталось в куче в байтах
 *
 */

uint32_t Thread::getFreeStackSize()
{
    return _getFreeStackSize();
}

/**
 *
 *  @brief Распечатать статистику использования потоков по каждому потоку
 *
 */

void Thread::printStats()
{

}
