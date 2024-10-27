#include "ssi.hpp"

#include <string.h>

/**

    @brief Инициализировать объект/структуру ssi контекста

    @param ct указатель на объект/структуру ssi контекста
    @param file дескриптор открытого shtml файла

*/

SSI::SSI()
{

    for( uint32_t i = 0; i < IF_LEVELS; i++ )
    {
        ifstate[i] = IF_FREE;
    }

    readyRead    = false;
    fileIncluded = false;

    if_level = 0;

    putCounter = 0;

    _unsetFalseCondition();
    _findTagOpen();

}

/**

    @brief Задать базу данных, из которой будут браться значения переменных

    @param _database база данных

*/

void SSI::setDatabase( Database& _database )
{

    database = &(_database);

}

/**

    @brief Поместить следующий символ исходной shtml страницы на распознавание

*/

void SSI::put(uint8_t value )
{

    if( readyRead == true )
    {
        return;
    }

    if ( _isFindTagOpen() )
    {

        if( value == startTag[0] )
        {

            // Возможно начался новый тэг

            _findTagOpenEnd();

            buffer[putCounter] = value;

            putCounter        += 1;

        }
        else
        {

            if( !_isFalseCondition() )
            {

                buffer[0]  = value;

                putCounter = 1;
                getCounter = 0;

                readyRead = true;

            }

        }

    }
    else if ( _isFindTagOpenEnd() )
    {

        buffer[putCounter] = value;
        putCounter += 1;

        if( startTag[putCounter-1] == value )
        {

            if( putCounter >= strlen(startTag) )
            {

                endCounter = 0;

                _findTagClose();

            }

        }
        else
        {

            if( !_isFalseCondition() )
            {

                // Возвращаем символы, занятые в буфер

                readyRead = true;

                getCounter = 0;

            }
            else
            {

                // Т.к находимся в false condition
                //  то выбрасываем занятые в буфер символы

                putCounter = 0;

            }

            _findTagOpen();

        }

    }
    else if ( _isFindTagClose() )
    {

        if( value == endTag[0] )
        {

            buffer[putCounter] = value;

            endCounter += 1;

            putCounter += 1;

            _findTagCloseEnd();

        }
        else
        {

            buffer[putCounter]  = value;
            putCounter         += 1;

        }


    }
    else if( _isFindTagCloseEnd() )
    {

        if( value == endTag[endCounter] )
        {

            buffer[putCounter] = value;
            endCounter        += 1;
            putCounter        += 1;

            if( endCounter < strlen(endTag) )
            {
                return;
            }

            // Конец завершающего тэга найден

            // В буфере теперь: <!--[содержимое тэга]-->

            uint16_t offset = strlen(startTag);

            buffer[ putCounter ]  = '\0';

            switch ( _getCommandFromString( &(buffer[offset])) )
            {
                case ECHO    : offset += strlen(commands[ECHO]);    _echo    ( &(buffer[offset]) ) ; break;
                case IF      : offset += strlen(commands[IF]);      _if      ( &(buffer[offset]) ) ; break;
                case ELSE    : offset += strlen(commands[ELSE]);    _else    ()                    ; break;
                case ELIF    : offset += strlen(commands[ELIF]);    _elif    ( &(buffer[offset]) ) ; break;
                case ENDIF   : offset += strlen(commands[ENDIF]);   _endif   ()                    ; break;
                case INCLUDE : offset += strlen(commands[INCLUDE]); _include ( &(buffer[offset]) ) ; break;
                case SET     : offset += strlen(commands[SET]);     _set     ( &(buffer[offset]) ) ; break;
                default      : break;
            }

            _findTagOpen();

        }

    }
    else
    {

            _findTagClose(); // Опять ищем завершение тэга

    }

}

/**

    @brief Получить следующий символ распознанной shtml страницы

    @return следующий символ

*/

uint8_t SSI::get()
{

    if( readyRead == true )
    {

        putCounter -= 1;
        getCounter += 1;

        if( putCounter == 0 )
        {

            readyRead = false;

        }

        return buffer[getCounter-1];

    }
    else
    {

        return '\0';

    }

}

/**

    @brief Можно ли получить следующий символ распознанной shtml страницы

    @return true - можно, false - нельзя.

*/

bool SSI::isReadyRead()
{

    return readyRead;

}

/**

    @brief Был ли в shtml странице встречен запрос на распознавание внешнего shtml файла.

    @return true - встречен, необходимо перейти к распознаванию внешнего файла, false - нет.

*/

bool SSI::isIncludeFile()
{

    bool oldFileIncluded = fileIncluded;

    fileIncluded = false;

    return oldFileIncluded;

}

/**

    @brief Завершить распознавание shtml страницы.

*/

void SSI::commit()
{

    if( putCounter > 0 )
    {

        readyRead  = true;
        getCounter = 0;

    }

    _findTagOpen();

}

/**

    @brief Получить имя внешней страницы, которую необходимо распознать

*/

char* SSI::getIncludeFilename()
{

    return &(buffer[0]);

}

// -------------------------------------------------------
// ------- Остальные методы, обслуживают работу класса

void SSI::_setFalseCondition()
{
    falseCondition = true;
}

void SSI::_unsetFalseCondition()
{
    falseCondition = false;
}

bool SSI::_isFalseCondition()
{
    return falseCondition;
}

void SSI::_findTagOpen()
{
    state = FIND_TAG_OPEN;
}

void SSI::_findTagOpenEnd()
{
    state = FIND_TAG_OPEN_END;
}

void SSI::_findTagClose()
{
    state = FIND_TAG_CLOSE;
}

void SSI::_findTagCloseEnd()
{
    state = FIND_TAG_CLOSE_END;
}

bool SSI::_isFindTagOpen()
{

    if( state == FIND_TAG_OPEN )
    {
        return true;
    }
    else
    {
        return false;
    }

}

bool SSI::_isFindTagOpenEnd()
{
    if( state == FIND_TAG_OPEN_END )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool SSI::_isFindTagClose()
{
    if( state == FIND_TAG_CLOSE )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool SSI::_isFindTagCloseEnd()
{
    if( state == FIND_TAG_CLOSE_END )
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**

    @brief Подставить значение из базы данных

     Если входной буфер полностью заполнен, вернет outstrlen байт. Значит значение
      переменной возможно было скопированно не до конца(есть ещё данные). Чтобы их получить
      нужно повторно вызвать эту функцию, передав в параметре str нулевой указатель а stlen - 0.


    @param  str строка с содержимым тэга. Образец: <!--#echo var="[содержимое_тэга]"-->

*/

void SSI::_echo( char *str )
{

    if( _isFalseCondition() )
    {
        putCounter = 0;
        return;
    }

    for( uint16_t i = 0 ; str[i] != '\0' ; i++ )
    {

        if( str[i] == '\"' )
        {
            str[i] = '\0';
            break;
        }

    }

    database->getStringValue( str, &(buffer[0]) );

    putCounter = strlen( &(buffer[0]) );

    if( putCounter > 0 )
    {

       getCounter = 0;
       readyRead  = true;

    }

    return;

}

/**

    @brief Возвращает имя файла, который нужно подключить

    @param str строка с содержимым тэга. Образец: <!--#include file="[содержимое_тэга]"-->

*/

void SSI::_include( char *str )
{

    if( _isFalseCondition() )
    {
        putCounter = 0;
        return;
    }

    uint16_t i;

    for( i = 0 ; ( str[i] != '\0' ) && ( i < MAX_TAGBODY_LEN ) ; i++ )
    {

        if( str[i] == '\"' )
        {

            break;

        }
        else
        {

            buffer[i] = str[i];

        }

    }

    buffer[i] = '\0';

    if( i > 0 )
    {
        putCounter = 0;
        fileIncluded = true;
    }

    return;

}

/**

    @brief Тэг проверки условия, вычиление истинности/ложности выражения и установка внутреннего флага ssi_now_state->ifstate
      в истинное/ложное состояние.
       В выходной буфер ничего не записывается.

    @param str строка с содержимым тэга. Образец: <!--#if expr="[содержимое_тэга]"-->

*/

void SSI::_if( char *str )
{

    putCounter = 0;

    if( _isFalseCondition() )
    {

        // Проброс значения skipdata во вложенный if

        ifstate[if_level] = LAST_IF_SKIP;

        if_level += 1;

        return;

    }

    // Находим тип сравнения

    char* rightValue = 0;

    CONDITION condition = NOT_EQUALS;

    for( uint16_t i = 0 ; str[i] != '\0'; i++ )
    {

        if( str[i] == '!' )
        {

            str[i] = '\0';

            rightValue = &(str[i+2]);
            condition  = NOT_EQUALS;
            break;

        }
        else if( str[i] == '=' )
        {

             str[i] = '\0';

             rightValue = &(str[i+1]);
             condition  = EQUALS;
             break;

        } else if( str[i] == '<' )
        {

            str[i] = '\0';

            if( str[i+1] == '=' )
            {

                rightValue = &(str[i+2]);
                condition  = SMALLER_OR_EQUALS;

            }
            else
            {

                rightValue = &(str[i+1]);
                condition  = SMALLER;

            }

            break;

        }
        else if( str[i] == '>' )
        {

            str[i] = '\0';

            if( str[i+1] == '=' )
            {

                rightValue = &(str[i+2]);
                condition  = LONGER_OR_EQUALS;

            }
            else
            {

                rightValue = &(str[i+1]);
                condition  = LONGER;

            }

            break;

        }

    }

    if( rightValue == 0 )
    {
        return;
    }

    for( uint16_t i = 0 ; ( rightValue[i] != '\0') ; i++ )
    {

        if( rightValue[i] == '\"' )
        {

            rightValue[i] = '\0';

            break;
        }

    }

    // В condition тип сравнения

    char* leftValue = &(str[0]);

    if( ( leftValue[0] == '$' ) && ( rightValue[0] == '$' ) )
    {

        // Левое и правое значение выражения - переменные

         leftValue  = &(leftValue[1]);
         rightValue = &(rightValue[1]);

         Database::COMPARE_RESULT compareResult = database->compare( leftValue, rightValue );

         if( compareResult == Database::EQUALS )
         {

             switch(condition)
             {
                 case EQUALS            : ifstate[if_level] = LAST_IF_TRUE;  break;
                 case NOT_EQUALS        : ifstate[if_level] = LAST_IF_FALSE; break;
                 case LONGER            : ifstate[if_level] = LAST_IF_FALSE; break;
                 case SMALLER           : ifstate[if_level] = LAST_IF_FALSE; break;
                 case LONGER_OR_EQUALS  : ifstate[if_level] = LAST_IF_TRUE;  break;
                 case SMALLER_OR_EQUALS : ifstate[if_level] = LAST_IF_TRUE;  break;
             }

         }
         else if( compareResult == Database::SMALLER )
         {

             switch(condition)
             {
                 case EQUALS            : ifstate[if_level] = LAST_IF_FALSE; break;
                 case NOT_EQUALS        : ifstate[if_level] = LAST_IF_TRUE;  break;
                 case LONGER            : ifstate[if_level] = LAST_IF_FALSE; break;
                 case SMALLER           : ifstate[if_level] = LAST_IF_TRUE;  break;
                 case LONGER_OR_EQUALS  : ifstate[if_level] = LAST_IF_FALSE; break;
                 case SMALLER_OR_EQUALS : ifstate[if_level] = LAST_IF_TRUE;  break;
             }

         }
         else
         {

             switch(condition)
             {
                 case EQUALS            : ifstate[if_level] = LAST_IF_FALSE; break;
                 case NOT_EQUALS        : ifstate[if_level] = LAST_IF_TRUE;  break;
                 case LONGER            : ifstate[if_level] = LAST_IF_TRUE;  break;
                 case SMALLER           : ifstate[if_level] = LAST_IF_FALSE; break;
                 case LONGER_OR_EQUALS  : ifstate[if_level] = LAST_IF_TRUE;  break;
                 case SMALLER_OR_EQUALS : ifstate[if_level] = LAST_IF_FALSE; break;
             }

         }

    }
    else if( ( leftValue[0] != '$' ) && ( rightValue[0] != '$' ) )
    {

        // Левое и правое значение выражения - строки

        int sstate = strcmp( leftValue, rightValue );

        if( sstate == 0 )
        {

            switch(condition)
            {
                case EQUALS            : ifstate[if_level] = LAST_IF_TRUE;  break;
                case NOT_EQUALS        : ifstate[if_level] = LAST_IF_FALSE; break;
                case LONGER            : ifstate[if_level] = LAST_IF_FALSE; break;
                case SMALLER           : ifstate[if_level] = LAST_IF_FALSE; break;
                case LONGER_OR_EQUALS  : ifstate[if_level] = LAST_IF_TRUE;  break;
                case SMALLER_OR_EQUALS : ifstate[if_level] = LAST_IF_TRUE;  break;
            }

        } else if( sstate < 0 )
        {

            switch(condition)
            {
                case EQUALS            : ifstate[if_level] = LAST_IF_FALSE; break;
                case NOT_EQUALS        : ifstate[if_level] = LAST_IF_TRUE;  break;
                case LONGER            : ifstate[if_level] = LAST_IF_FALSE; break;
                case SMALLER           : ifstate[if_level] = LAST_IF_TRUE;  break;
                case LONGER_OR_EQUALS  : ifstate[if_level] = LAST_IF_FALSE; break;
                case SMALLER_OR_EQUALS : ifstate[if_level] = LAST_IF_TRUE;  break;
            }

        }
        else
        {

            switch(condition)
            {
                case EQUALS            : ifstate[if_level] = LAST_IF_FALSE; break;
                case NOT_EQUALS        : ifstate[if_level] = LAST_IF_TRUE;  break;
                case LONGER            : ifstate[if_level] = LAST_IF_TRUE;  break;
                case SMALLER           : ifstate[if_level] = LAST_IF_FALSE; break;
                case LONGER_OR_EQUALS  : ifstate[if_level] = LAST_IF_TRUE;  break;
                case SMALLER_OR_EQUALS : ifstate[if_level] = LAST_IF_FALSE; break;
            }

        }


    }
    else if( ( leftValue[0] == '$' ) && ( rightValue[0] != '$' ) )
    {

        // Левое значение переменная, правое значение строка

        leftValue  = &(leftValue[1]);

        Database::COMPARE_RESULT compareResult = database->compareWithString( leftValue, rightValue );

        if( compareResult == Database::EQUALS )
        {

            switch(condition)
            {
                case EQUALS            : ifstate[if_level] = LAST_IF_TRUE;  break;
                case NOT_EQUALS        : ifstate[if_level] = LAST_IF_FALSE; break;
                case LONGER            : ifstate[if_level] = LAST_IF_FALSE; break;
                case SMALLER           : ifstate[if_level] = LAST_IF_FALSE; break;
                case LONGER_OR_EQUALS  : ifstate[if_level] = LAST_IF_TRUE;  break;
                case SMALLER_OR_EQUALS : ifstate[if_level] = LAST_IF_TRUE;  break;
            }

        }
        else if( compareResult == Database::SMALLER )
        {

            switch(condition)
            {
                case EQUALS            : ifstate[if_level] = LAST_IF_FALSE; break;
                case NOT_EQUALS        : ifstate[if_level] = LAST_IF_TRUE;  break;
                case LONGER            : ifstate[if_level] = LAST_IF_FALSE; break;
                case SMALLER           : ifstate[if_level] = LAST_IF_TRUE;  break;
                case LONGER_OR_EQUALS  : ifstate[if_level] = LAST_IF_FALSE; break;
                case SMALLER_OR_EQUALS : ifstate[if_level] = LAST_IF_TRUE;  break;
            }

        }
        else
        {

            switch(condition)
            {
                case EQUALS            : ifstate[if_level] = LAST_IF_FALSE; break;
                case NOT_EQUALS        : ifstate[if_level] = LAST_IF_TRUE;  break;
                case LONGER            : ifstate[if_level] = LAST_IF_TRUE;  break;
                case SMALLER           : ifstate[if_level] = LAST_IF_FALSE; break;
                case LONGER_OR_EQUALS  : ifstate[if_level] = LAST_IF_TRUE;  break;
                case SMALLER_OR_EQUALS : ifstate[if_level] = LAST_IF_FALSE; break;
            }

        }

    }
    else if( ( leftValue[0] != '$' ) && ( rightValue[0] == '$' ) )
    {

        // Левое значение строка, правое значение переменная

        rightValue = &(rightValue[1]);

        Database::COMPARE_RESULT compareResult = database->compareWithString( rightValue, leftValue );

        if( compareResult == Database::EQUALS )
        {

            switch(condition)
            {
                case EQUALS            : ifstate[if_level] = LAST_IF_TRUE;  break;
                case NOT_EQUALS        : ifstate[if_level] = LAST_IF_FALSE; break;
                case LONGER            : ifstate[if_level] = LAST_IF_FALSE; break;
                case SMALLER           : ifstate[if_level] = LAST_IF_FALSE; break;
                case LONGER_OR_EQUALS  : ifstate[if_level] = LAST_IF_TRUE;  break;
                case SMALLER_OR_EQUALS : ifstate[if_level] = LAST_IF_TRUE;  break;
            }

        }
        else if( compareResult == Database::SMALLER )
        {

            switch(condition)
            {
                case EQUALS            : ifstate[if_level] = LAST_IF_FALSE; break;
                case NOT_EQUALS        : ifstate[if_level] = LAST_IF_TRUE;  break;
                case LONGER            : ifstate[if_level] = LAST_IF_TRUE;  break;
                case SMALLER           : ifstate[if_level] = LAST_IF_FALSE; break;
                case LONGER_OR_EQUALS  : ifstate[if_level] = LAST_IF_TRUE;  break;
                case SMALLER_OR_EQUALS : ifstate[if_level] = LAST_IF_FALSE; break;
            }

        }
        else
        {

            switch(condition)
            {
                case EQUALS            : ifstate[if_level] = LAST_IF_FALSE; break;
                case NOT_EQUALS        : ifstate[if_level] = LAST_IF_TRUE;  break;
                case LONGER            : ifstate[if_level] = LAST_IF_FALSE; break;
                case SMALLER           : ifstate[if_level] = LAST_IF_TRUE;  break;
                case LONGER_OR_EQUALS  : ifstate[if_level] = LAST_IF_FALSE; break;
                case SMALLER_OR_EQUALS : ifstate[if_level] = LAST_IF_TRUE;  break;
            }

        }

    }

    if( ifstate[if_level] == LAST_IF_FALSE )
    {

        _setFalseCondition();

    }

    if_level += 1;

    return; // Эта команда не предусматривает копирование в буфер

}

/**

    @brief Обработка условия else

      В выходной буфер ничего не записывается.

*/

void SSI::_else()
{

    putCounter = 0;

    if( ifstate[ if_level - 1 ] == LAST_IF_TRUE )
    {

        // Пропустить все содержимое до/включая endif

        ifstate[ if_level - 1 ] = LAST_IF_ELSE;

        _setFalseCondition();

    }
    else if( ifstate[ if_level - 1 ] == LAST_IF_FALSE )
    {

        // Без пропуска

        ifstate[ if_level - 1 ] = LAST_IF_TRUE;

       _unsetFalseCondition();

    }
    else if(  ifstate[ if_level - 1 ] == LAST_IF_SKIP )
    {

        // skip, nothing

    }

    return;

}

/**

    @brief Обработка условия elif

    @param  str строка с содержимым тэга. Образец: <!--#elif expr="[содержимое тэга]"-->

*/

void SSI::_elif( char *str )
{

    _endif();

    _if(str);

    return;

}

/**

    @brief Обработка условия endif

    @param str строка с содержимым тэга. Образец: <!--#endif[содержимое тэга отсутствует]-->

*/

void SSI::_endif()
{

    putCounter = 0;

    if_level -= 1;

    if( ifstate[if_level] != LAST_IF_SKIP )
    {

        _unsetFalseCondition();

    }

    ifstate[if_level] = IF_FREE;

    return;

}

/**

    @brief Изменяет значение переменной, в том числе можно присвоить другую переменую

    @param str строка с содержимым тэга. Образец: <!--#set var="[содержимое_тэга]" value="$VARIABLE"-->

*/

void SSI::_set( char *str )
{

    putCounter = 0;

    if( _isFalseCondition() )
    {

        return;

    }

    char* value = 0;

    for( uint16_t i = 0 ; str[i] != '\0' ; i++ )
    {

        if( str[i] == '\"' ){

            str[i] = '\0';
            value  = &(str[i+1]);

            break;

        }

    }

    if( value == 0 )
    {
        return;
    }

    for( uint16_t i = 0 ; value[i] != '\0' ; i++ )
    {

        if ( 0 == strncmp( &(value[i]), "value=\"", 7 ) )
        {

            // Нашли устанавливаемое значение переменной

            i += 7;

            value = &(value[i]);

            break;

        }

    }

    for( uint16_t i = 0; value[i] != '\0' ; i++ )
    {

        if( value[i] == '\"' )
        {
            value[i] = '\0';
            break;
        }

    }

    database->change( str, value );

    return;

}

/**
 *
 * @brief Получить команду из строки с ssi тэгом
 *
 * @param строка с тэгом, например echo var="...
 *
 * @return найденная команда
 *
 */

SSI::COMMAND SSI::_getCommandFromString( char *str )
{

    for( uint8_t i = 0 ; i < COMMANDS_LENGTH; i++ )
    {

        if( ( strlen(commands[i]) != '\0' ) && ( 0 == strncmp( commands[i], str, strlen(commands[i]) ) ) )
        {

            return (COMMAND) i;

        }

    }

    return UNKNOWN_COMMAND;

}

const char* SSI::commands[ SSI::COMMANDS_LENGTH ] =
{
     "echo var=\""     ,
     "if expr=\""      ,
     "else"            ,
     "elif expr=\""    ,
     "endif"           ,
     "include file=\"" ,
     "exec"            ,
     "set var=\""      ,
};

const char SSI::startTag[] = { "<!--#" };
const char SSI::endTag[]   = { "-->" };
