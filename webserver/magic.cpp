#include "magic.hpp"

#include <string.h>


/**

    @brief Получить идентификатор content-type по mimestring
	
    @param str   mimestring с названием content-type
	
    @return Идентификатор content-type строки
	
*/

Magic::TYPE Magic::getContentTypeFromString( const char *str )
{

    for( uint8_t i = 0 ; i < TYPES_COUNT ; i++ )
    {
		
        if ( 0 == strcmp( str, types[i][1] ) )
        {
			
            return (TYPE)i;
		}
		
	}

    return UNKNOWN_TYPE;

}

/**

    @brief По расширению имени файла определяет его content-type и возвращает content-type id
	
    @param строка с именем файла, например "index.html"
    @param длина имени файла, например 11

    @return идентификатор content-type

*/

Magic::TYPE Magic::getContentTypeFromFilename( const char *filename, const uint8_t filenamelen )
{
	
    if( filename == 0 ) return UNKNOWN_TYPE;
	
    if( filenamelen == 0 )
    {
        return PLAIN;
	}
	
	int j,k,fl;
	
    int i;
	
    for(i=0; i < TYPES_COUNT ; i++ )
    {

        if( filenamelen < strlen(types[i][0]) ) continue;
		
		fl = 0;
			
        for( j=(strlen(types[i][0])-1), k = (filenamelen-1) ; ((j  >= 0) && (k >= 0)) ; j--, k--)
        {
			
            if( filename[k] != types[i][0][j] )
            {
				fl = 1;
				break;
			}

		}
		
        if( fl == 0 )
        {
            return (TYPE) i ;
		}

		// continue

	}

    return UNKNOWN_TYPE;
	
}

/**

    @brief По идентификатору content-type возвращает строку с описанием content-type
	
    @param ct идентификатор content-type
	
    @return строка с описанием типа ресурса(для поля content-type). Например для файла с названием
	  index.html это будет строка "text/html"

*/

const char* Magic::getStringFromContentType( const TYPE ct )
{
	
    return types[ct][1];
	
}


const char * const Magic::types[TYPES_COUNT][2] =
{
    {".html","text/html"},
    {".css","text/css"},
    {".js","application/javascript"},
    {".png","image/png"},
    {".jpeg","image/jpeg"},
    {".jpg","image/jpeg"},
    {".gif","image/gif"},
    {".bmp","image/bmp"},
    {".svg","image/svg+xml"},
    {".tiff","image/tiff"},
    {".ico","image/vnd.microsoft.icon"},
    {".bin","application/octet-stream"},
    {".xml","text/xml"},
    {".csv","text/csv"},
    {".pdf","application/pdf"},
    {".doc","application/msword"},
    {".rtf","application/x-rtf"},
    {".docx","application/msword"},
    {".odt","application/vnd.oasis.opendocument.text"},
    {".ppt","application/vnd.ms-powerpoint"},
    {".zip","application/zip"},
    {".rar","application/x-rar-compressed"},
    {".tar","application/x-tar"},
    {".gz","application/gzip"},
    {".mpeg","video/mpeg"},
    {".mp4","video/mp4"},
    {".ogg","video/ogg"},
    {".webm","video/webm"},
    {".flv","video/x-flv"},
    {".mp3","audio/mpeg"},
    {".flac","audio/ogg"},
    {".wav","audio/vnd.wave"},
    {"","text/plain"},
    {"","multipart/form-data"},
    {"","multipart/byteranges"}
};
