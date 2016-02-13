#ifndef PHP_LAME_H
#define PHP_LAME_H

#define PHP_LAME_NAME "lame"
#define PHP_LAME_VERSION "0.0.1"

#include <php.h>
#include "BladeMP3EncDLL.h"

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef PHP_WIN32
#define PHP_LAME_API __declspec(dllexport)
#else
#define PHP_LAME_API
#endif

extern zend_module_entry lame_module_entry;
#define phpext_lame__ptr &lame_module_entry
#define DLL_FILE L"lame_enc.dll"

static BEINITSTREAM beInitStream = NULL;
static BEENCODECHUNK beEncodeChunk = NULL;
static BEENCODECHUNKFLOATS16NI beEncodeChunkFloatS16NI = NULL;
static BEDEINITSTREAM beDeinitStream = NULL;
static BECLOSESTREAM beCloseStream = NULL;
static BEVERSION beVersion = NULL;
static BEWRITEVBRHEADER beWriteVBRHeader = NULL;
static BEFLUSHNOGAP beFlushNoGap = NULL;
static BEWRITEINFOTAG beWriteInfoTag = NULL;

static PHP_MINIT_FUNCTION(lame);
static PHP_MSHUTDOWN_FUNCTION(lame);
static PHP_MINFO_FUNCTION(lame);

static void stream_destructor(zend_rsrc_list_entry *rsrc TSRMLS_DC);
static void phpArrayToBeConfig(zval **array, BE_CONFIG *config);
static void phpArrayToPShort(zval **array, PSHORT *pShort);
static void phpArrayToPFloat(zval **array, PFLOAT *pFloat);

zend_function_entry lame_functions[];

PHP_FUNCTION(getLameVersion);
PHP_FUNCTION(newLameConfig);
PHP_FUNCTION(beInitStream);
PHP_FUNCTION(beEncodeChunk);
PHP_FUNCTION(beEncodeChunkFloatS16NI);
PHP_FUNCTION(beDeinitStream);
PHP_FUNCTION(beCloseStream);
PHP_FUNCTION(beWriteVBRHeader);
PHP_FUNCTION(beFlushNoGap);
PHP_FUNCTION(beWriteInfoTag);

#endif
