#ifndef SSI_H_
#define SSI_H_

#include "platform.hpp"

#include "database.hpp"

/**

    @brief Cоздание html страниц из shtml шаблонов.

    На вход передается база данных, содержащая переменные(пары ключ-значение)
     и содержимое shtml файла.

    На выход поступает содержимое сформированного html файла и при необходимости
     запрос на включение вложенного файла.

    SSI(Server side includes) - SSI это несложный язык для динамической «сборки» веб-страниц на сервере из отдельных составных частей и выдачи клиенту полученного HTML-документа.
       Файлы, использующие этот язык имеют расширение .shtml.

     Синтаксис SSI позволяет включать в текст страницы другие SSI-страницы, реализовывать условные операции (if/else), работать с переменными.

     Основные, реализованные в этом классе команды:

        echo                  — вставляет значение переменной
        set                   — устанавливает значение переменной
        include               — вставляет содержимое другого(shtml,html) файла
        if, else, endif       — условные операторы

     Как использовать команды:

        <!--#echo var="[ИМЯ_ВСТАВЛЯЕМОЙ_ПЕРЕМЕННОЙ]"-->

        <!--#set var="[ИМЯ_УСТАНАВЛИВАЕМОЙ_ПЕРЕМЕННОЙ]" value="$[НАЗВАНИЕ ДРУГОЙ ПЕРЕМЕННОЙ]" -->
        <!--#set var="[ИМЯ_УСТАНАВЛИВАЕМОЙ_ПЕРЕМЕННОЙ]" value="[ЛЮБАЯ_СТРОКА]" -->

        <!--#include file="[ПУТЬ_ДО_ПОДКЛЮЧАЕМОГО_ФАЙЛА]"-->

        <!--#if expr="[ЛОГИЧЕСКОЕ_ВЫРАЖЕНИЕ]"-->

            Условие истинно!

        <!--#else-->

            Условие ложно!

        <!--#endif-->

        ЛОГИЧЕСКОЕ_ВЫРАЖЕНИЕ - могут быть две строки, или две переменные, строка или переменная

        Поддерживаются следующие операторы сравнения:

            =
            <
            >
            >=
            <=

        Чтобы использовать переменную надо перед её именем писать $[НАЗВАНИЕ ПЕРЕМЕННОЙ]

      Пример функции, обработки shtml файла и вывода результата работы на экран
       подключаемые файлы обрабатываются рекурсивно.

    \code

        // @brief Обработка shtml файла и вывод результата работы на экран
        //
        // @param pagename путь до страницы в файловой системе
        // @param database база данных

        void processSSIPage( const char* pagename, Database& database ){

            SSI* ssi = new SSI();

            ssi->setDatabase(database);

            FILE* file = fopen( pagename, "r" );

            while(1){

                uint8_t symbol;

                fread( &symbol, 1, 1, file );

                if( feof(file) ){
                    break;
             }

                ssi->put( symbol );

                while( ssi->isReadyRead() ){

                    printf( "%c", ssi->get() );

                }

                if( ssi->isIncludeFile() ){

                    processSSIPage( ssi->getIncludeFilename(), database );

                }

            }

            ssi->commit();

            while( ssi->isReadyRead() ){

                printf( "%c", ssi->get() );

            }

            fclose(file);

            delete ssi;

        }

    \endcode

*/

class SSI
{

public:

    SSI();

    void    setDatabase( Database& _database );

    void    put( uint8_t value );
    bool    isReadyRead();
    bool    isIncludeFile();
    uint8_t get();

    void    commit();

    char*   getIncludeFilename();

private:

    enum {
        MAX_TAGBODY_LEN   = 128,
        COMMAND_LENGTH    = 9,
        IF_LEVELS         = 5,
    };

    enum CONDITION {
        EQUALS,
        NOT_EQUALS,
        SMALLER,
        LONGER,
        LONGER_OR_EQUALS,
        SMALLER_OR_EQUALS
    };

    typedef enum {
        IF_FREE,
        LAST_IF_TRUE,
        LAST_IF_FALSE,
        LAST_IF_ELSE,
        LAST_IF_SKIP
    } IFSTATE;

    typedef enum {
        FIND_TAG_OPEN,
        FIND_TAG_OPEN_END,
        FIND_TAG_CLOSE,
        FIND_TAG_CLOSE_END
    } STATE;

    typedef enum {
        ECHO = 0,
        IF,
        ELSE,
        ELIF,
        ENDIF,
        INCLUDE,
        SET,
        UNKNOWN_COMMAND,
        COMMANDS_LENGTH
    } COMMAND;

    Database* database;

    char     buffer[MAX_TAGBODY_LEN];

    STATE    state;
    bool     falseCondition;

    bool     fileIncluded;
    bool     readyRead;

    uint16_t putCounter;
    uint16_t getCounter;
    uint16_t endCounter;

    IFSTATE ifstate[IF_LEVELS]; // Состояние предыдущего условного перехода
    uint8_t if_level;           // Текущий уровень вложенности в условных переходах

    static const char startTag[];

    static const char endTag[];

    static const char* commands[];

    void _setFalseCondition();
    void _unsetFalseCondition();

    bool _isFalseCondition();

    void _findTagOpen();
    void _findTagOpenEnd();
    void _findTagClose();
    void _findTagCloseEnd();

    bool _isFindTagOpen();
    bool _isFindTagOpenEnd();
    bool _isFindTagClose();
    bool _isFindTagCloseEnd();

    void _echo    ( char* );
    void _include ( char* );

    void _if      ( char* );
    void _else    ();
    void _endif   ();
    void _set     ( char* );

    void _elif    ( char*  );

    COMMAND _getCommandFromString( char *str );

};


#endif /* SSI_H_ */
