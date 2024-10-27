/**

    @file magic.h
	
    @brief Нахождение MIME типа файла
	
*/ 

#ifndef MAGIC_H_
#define MAGIC_H_

#include "platform.hpp"

class Magic
{

public:

    typedef enum
    {

        HTML = 0,
        CSS,
        JS,
        PNG,
        JPEG,
        JPG,
        GIF,
        BMP,
        SVG,
        TIFF,
        ICO,
        BIN,
        XML,
        CSV,
        PDF,
        DOC,
        RTF,
        DOCX,
        ODT,
        PPT,
        ZIP, // 20
        RAR,
        TAR,
        GZIP,
        MPEG,
        MP4,
        OGG,
        WEBM,
        FLV,
        MP3,
        FLAC,
        WAV,
        PLAIN,
        MULTIPART_FORM_DATA, // 33
        MULTIPART_BYTE_RANGES, // 34

        TYPES_COUNT,
        UNKNOWN_TYPE

    } TYPE;

    static TYPE        getContentTypeFromString(const char *str);
    static const char* getStringFromContentType( const TYPE ct );
    static TYPE        getContentTypeFromFilename( const char *filename, const uint8_t filenamelen );

private:

    static const char * const types[TYPES_COUNT][2];

};

#endif /* MAGIC_H_ */
