import sys
import time
import os
import glob
import zlib
import mimetypes
from   struct import *

# Для минимизации размера файлов с интерфейсом

# Зависимости:
# css-html-js-minify
# zlib

mimetypesArray = [ 
"HTML",
"CSS",
"JS",
"PNG",
"JPEG",
"JPG",
"GIF",
"BMP",
"SVG",
"TIFF",
"ICO",
"BIN",
"XML",
"CSV",
"PDF",
"DOC",
"RTF",
"DOCX",
"ODT",
"PPT",
"ZIP",
"RAR",
"TAR",
"GZIP",
"MPEG",
"MP4",
"OGG",
"WEBM",
"FLV",
"MP3",
"FLAC",
"WAV",
"PLAIN",
"MULTIPART_FORM_DATA",
"MULTIPART_BYTE_RANGES",
"TYPES_COUNT",
"UNKNOWN_TYPE"
]

"""
from css_html_js_minify import *


 

    } TYPE;
    
"""

# Необходимые для работы функции

def getMimetypeId( mimeString ):

    if mimeString   == "text/html":
        return 0
    elif mimeString == "text/css":
        return 1
    elif mimeString == "application/javascript":
        return 2
    elif mimeString == "image/png":
        return 3
    elif mimeString == "image/jpeg":
        return 4
    elif mimeString == "image/jpeg":
        return 5
    elif mimeString == "image/gif":
        return 6
    elif mimeString == "image/bmp":
        return 7
    elif mimeString == "image/svg+xml":
        return 8
    elif mimeString == "image/tiff":
        return 9
    elif mimeString == "image/vnd.microsoft.icon":
        return 10
    elif mimeString == "application/octet-stream":
        return 11
    elif mimeString == "text/xml":
        return 12
    elif mimeString == "text/csv":
        return 13
    elif mimeString == "application/pdf":
        return 14
    elif mimeString == "application/msword":
        return 15
    elif mimeString == "application/x-rtf":
        return 16
    elif mimeString == "application/msword":
        return 17
    elif mimeString == "application/vnd.oasis.opendocument.text":
        return 18
    elif mimeString == "application/vnd.ms-powerpoint":
        return 19
    elif mimeString == "application/zip":
        return 20
    elif mimeString == "application/x-rar-compressed":
        return 21
    elif mimeString == "application/x-tar":
        return 22
    elif mimeString == "application/gzip":
        return 23
    elif mimeString == "video/mpeg":
        return 24
    elif mimeString == "video/mp4":
        return 25
    elif mimeString == "video/ogg":
        return 26
    elif mimeString == "video/webm":
        return 27
    elif mimeString == "video/x-flv":
        return 28
    elif mimeString == "audio/mpeg":
        return 29
    elif mimeString == "audio/ogg":
        return 30
    elif mimeString == "audio/vnd.wave":
        return 31

    return 32

# Формирует файл с образом файловой системы из конкетной директории/й

# Аргументы:
#
#   1-n список корневых каталогов, откуда берутся файлы
#
#   -O выходной файл
#
# Пример: makefilesystem.py C:/library/web C:/myproject/web -O C:/myproject/src/files.cpp
#

# Реализация:

# В выходном файле создаются следующие два массива

# ------- Идексы, отсортированные по возрастанию хэш сумм

#    const Filesystem::indexDescriptor index[] = {
#        {[Хэш/контрольная сумма от имени файла], [тип файла], [смещение от начала массива files], [размер файла в байтах]},
#        ...
#        {[Хэш/контрольная сумма от имени файла], [тип файла], [смещение от начала массива files], [размер файла в байтах]},
#    }

# ------- Файлы
	
#    const char Filesystem::files[] = {

#        // file: C:/library/...html

#            0x..,0x.. ...
#            ...

#        // file: C:/library/

#            0x.., 0x.., ...
#            ...

#    };

# Если необходимо сократить размер(css,js) файлов - убрать все пустые(пробелы) места из файлов и.т.п, то True

isMinimizeFiles = False # некорректно сжимается библиотека jquery при True

if len(sys.argv) < 2:
    print("\nSpecify input folder/s")
    print("Specify output file/s -O \n")
    print("Example:\n")
    print("makefilesystem.py C:/library/web/common/script.js C:/web/index.html -O C:\myproject\src\outputFile.c ")
    exit(-1)

i = 1

inputFiles   = []

outputFile   = ""

while i < len(sys.argv):

    if ( sys.argv[i] == "-o" ) or ( sys.argv[i] == "-O" ):

        i += 1

        if( i < len(sys.argv) ):

            outputFile = sys.argv[i]

            break

        else:

            print("Specify output file!")

            break

    inputFiles.append(sys.argv[i])

    i+=1

offset = 0

containerNewLine = 0

indexEntrys = []

containerArray = "#if !defined(USE_EXTERNAL_FLASH_STORAGE)\n";

containerArray += "    const uint8_t Filesystem::files[] = \n    { \n"

binaryArray = bytearray()

for filepath in inputFiles:
	
	filePathOut = filepath.replace('\\','/')[ filepath.find("/web/") + 4 : ] 
	
	file = open(filepath, "rb")

	fileEntry   = "\n{ "

	hash = zlib.crc32( filePathOut.encode("UTF-8") )

	fileEntry += "0x" + format( hash, '08X' )

	fileEntry += ", "

	fileTypeId = getMimetypeId( mimetypes.guess_type(filepath)[0] )

	fileEntry += "Magic::" + mimetypesArray[fileTypeId] # + format( fileTypeId, '08X' )
    
	fileEntry += ", "

	fileEntry += "0x" + format( offset, '08X' )

	fileEntry += ", "

	fileBody = file.read()

	if isMinimizeFiles:

		# Минимизируем файл
		#  убрав из него лишние пробелы, сократив имена переменных и.т.д

		if fileTypeId == 0: # html

		    fileBody = html_minify( fileBody.decode('UTF-8') ).encode('UTF8') # bytearray() #

		if fileTypeId == 1: # css

			fileBody = css_minify( fileBody.decode('UTF-8') ).encode('UTF8')

		elif fileTypeId == 2: # javascirpt

			fileBody = js_minify( fileBody.decode('UTF-8') ).encode('UTF8')

	file.close()
    
	fileEntry += "0x" + format( len(fileBody), '08X' )

	fileEntry += " },"

	fileEntry += "    //   " + filePathOut

	indexEntrys.append( [ hash, fileEntry  ]  )

	# [хэш, тип,смещение,размер]

	offset += len(fileBody)
    
    # Добавление файла к переменной для bin файла

	binaryArray += fileBody
    
	# Заполнение массива-контейнера

	for byte in fileBody:

		containerArray += hex(byte)
		containerArray += ","

		containerNewLine += 1

		if containerNewLine > 100:
			containerNewLine = 0
			containerArray += "\n"

binFileCrc32 = zlib.crc32( binaryArray )

containerArray = containerArray[:-1]
          
containerArray += "};\n";

containerArray += "#else \n";
          
containerArray += "    const uint8_t Filesystem::files[] = { ";

binCrc32Bytes  = binFileCrc32.to_bytes(4, 'little')

containerArray +=  str(binCrc32Bytes[0]) + "," + str(binCrc32Bytes[1]) + "," + str(binCrc32Bytes[2]) + "," + str(binCrc32Bytes[3]);

containerArray += " }; // Контрольная сумма всех файлов во флешке\n";

containerArray += "#endif\n";

indexArray = "\nconst Filesystem::indexDescriptor Filesystem::index[] = \n{\n\n"

indexArray += "//                                                                     \n";
indexArray += "//        |               |           |          |                   | \n";
indexArray += "//   Хэш  | Идентификатор | Смещение  |  Размер  |    Путь до файла  | \n";
indexArray += "//        |      типа     |           |          |                   | \n";
indexArray += "//                                                                   | \n";

for item in sorted( indexEntrys, key=lambda item: item[0] ):
    indexArray += item[1]

indexArray += "\n{ 0x00000000, Magic::UNKNOWN_TYPE, 0x00000000, 0x00000000 }     //   Признак окончания индексного массива"

indexArray += "\n\n};\n\n"

# Создание cpp файла

cppFile = open( outputFile, "wb" )

cppFile.write( "// Автоматически созданный файл \n\n".encode("UTF-8") )

cppFile.write("// ".encode("UTF-8"))

cppFile.write(time.ctime().encode("UTF-8"))

cppFile.write("\n\n#include \"filesystem.hpp\" \n".encode("UTF-8"))

cppFile.write(indexArray.encode("UTF-8"))

cppFile.write(containerArray.encode("UTF-8"))

cppFile.close()

# Создание бинарного файла с образом файлов

binFile = open( outputFile + ".bin", "wb" )

binFileLength = len(binaryArray)

headerValue = 0xAAAAAAAA

# Заголовок 0xAAAAAAAA, +4 байта 

binFile.write( headerValue.to_bytes(4, 'little') ) 

# Длина всех последующих файлов в bin файле, +4 байта

binFile.write( binFileLength.to_bytes(4, 'little') ) 

# Записываем контрольную сумму binFileCrc32 binaryArray, +4 байта

binFile.write( binFileCrc32.to_bytes(4, 'little') )

#

binFile.write(binaryArray)

binFile.close()

exit(0)
