#include "php_lame.h"
#include <windows.h>
#include <ext/standard/info.h>
#include <zend_exceptions.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define concat(strFirst, strSecond) strFirst strSecond
#define STRLEN(s) (sizeof(s)/sizeof(s[0]))

#define le_stream_name "lame_stream"
#define le_pshort_name "lame_pshort"
#define le_pbyte_name "lame_pbyte"

static int le_stream;
static int le_pshort;
static int le_pbyte;

static HINSTANCE hDLL = NULL;
static char strVersion[32];
static BE_VERSION Version = {0,};

zend_module_entry lame_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_LAME_NAME,
	lame_functions,
	PHP_MINIT(lame),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(lame),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_LAME_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};

PHP_MINIT_FUNCTION(lame)
{
	hDLL = LoadLibrary(DLL_FILE);

	if (hDLL == NULL){
		fprintf(stderr, "Failed to load lame mp3 library: \"%s\" - Error: %d\r\n", DLL_FILE, GetLastError());
		return ERROR;
	}
	else{
		beInitStream = (BEINITSTREAM) GetProcAddress(hDLL, TEXT_BEINITSTREAM);
		beEncodeChunk = (BEENCODECHUNK) GetProcAddress(hDLL, TEXT_BEENCODECHUNK);
		beEncodeChunkFloatS16NI = (BEENCODECHUNKFLOATS16NI) GetProcAddress(hDLL, TEXT_BEENCODECHUNKFLOATS16NI);
		beDeinitStream = (BEDEINITSTREAM) GetProcAddress(hDLL, TEXT_BEDEINITSTREAM);
		beCloseStream = (BECLOSESTREAM) GetProcAddress(hDLL, TEXT_BECLOSESTREAM);
		beVersion = (BEVERSION) GetProcAddress(hDLL, TEXT_BEVERSION);
		beWriteVBRHeader = (BEWRITEVBRHEADER) GetProcAddress(hDLL,TEXT_BEWRITEVBRHEADER);
		beFlushNoGap = (BEFLUSHNOGAP) GetProcAddress(hDLL,TEXT_BEFLUSHNOGAP);
		beWriteInfoTag = (BEWRITEINFOTAG) GetProcAddress(hDLL,TEXT_BEWRITEINFOTAG);
	}

	if(!beInitStream){
		fprintf(stderr, "Failed to load lame function from %s: \"%s\" - Error: %d\r\n", DLL_FILE, TEXT_BEINITSTREAM, GetLastError());
		return ERROR;
	}

	if(!beEncodeChunk){
		fprintf(stderr, "Failed to load lame function from %s: \"%s\" - Error: %d\r\n", DLL_FILE, TEXT_BEENCODECHUNK, GetLastError());
		return ERROR;
	}

	if(!beDeinitStream){
		fprintf(stderr, "Failed to load lame function from %s: \"%s\" - Error: %d\r\n", DLL_FILE, TEXT_BEDEINITSTREAM, GetLastError());
		return ERROR;
	}

	if(!beCloseStream){
		fprintf(stderr, "Failed to load lame function from %s: \"%s\" - Error: %d\r\n", DLL_FILE, TEXT_BECLOSESTREAM, GetLastError());
		return ERROR;
	}

	if(!beVersion){
		fprintf(stderr, "Failed to load lame function from %s: \"%s\" - Error: %d\r\n", DLL_FILE, TEXT_BEVERSION, GetLastError());
		return ERROR;
	}

	if(!beWriteVBRHeader){
		fprintf(stderr, "Failed to load lame function from %s: \"%s\" - Error: %d\r\n", DLL_FILE, TEXT_BEWRITEVBRHEADER, GetLastError());
		return ERROR;
	}

	if(!beFlushNoGap){
		fprintf(stderr, "Failed to load lame function from %s: \"%s\" - Error: %d\r\n", DLL_FILE, TEXT_BEFLUSHNOGAP, GetLastError());
		return ERROR;
	}

	if(!beWriteInfoTag){
		fprintf(stderr, "Failed to load lame function from %s: \"%s\" - Error: %d\r\n", DLL_FILE, TEXT_BEWRITEINFOTAG, GetLastError());
		return ERROR;
	}

	beVersion(&Version);
	
	sprintf(strVersion, "%ls v%u.%02u (%u/%u/%u)", 
		DLL_FILE, Version.byMajorVersion, Version.byMinorVersion, 
		Version.byDay, Version.byMonth, Version.wYear);

	le_stream = zend_register_list_destructors_ex(stream_destructor, NULL, le_stream_name, module_number);

	REGISTER_LONG_CONSTANT("BE_CONFIG_MP3", BE_CONFIG_MP3, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BE_CONFIG_LAME", BE_CONFIG_LAME, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("BE_ERR_SUCCESSFUL", BE_ERR_SUCCESSFUL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BE_ERR_INVALID_FORMAT", BE_ERR_INVALID_FORMAT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BE_ERR_INVALID_FORMAT_PARAMETERS", BE_ERR_INVALID_FORMAT_PARAMETERS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BE_ERR_NO_MORE_HANDLES", BE_ERR_NO_MORE_HANDLES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BE_ERR_INVALID_HANDLE", BE_ERR_INVALID_HANDLE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BE_ERR_BUFFER_TOO_SMALL", BE_ERR_BUFFER_TOO_SMALL, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("BE_MAX_HOMEPAGE", BE_MAX_HOMEPAGE, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("BE_MP3_MODE_STEREO", BE_MP3_MODE_STEREO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BE_MP3_MODE_JSTEREO", BE_MP3_MODE_JSTEREO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BE_MP3_MODE_DUALCHANNEL", BE_MP3_MODE_DUALCHANNEL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("BE_MP3_MODE_MONO", BE_MP3_MODE_MONO, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("MPEG1", MPEG1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("MPEG2", MPEG2, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("VBR_METHOD_NONE", VBR_METHOD_NONE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VBR_METHOD_DEFAULT", VBR_METHOD_DEFAULT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VBR_METHOD_OLD", VBR_METHOD_OLD, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VBR_METHOD_NEW", VBR_METHOD_NEW, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VBR_METHOD_MTRH", VBR_METHOD_MTRH, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VBR_METHOD_ABR", VBR_METHOD_ABR, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("LQP_NOPRESET", LQP_NOPRESET, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_NORMAL_QUALITY", LQP_NORMAL_QUALITY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_LOW_QUALITY", LQP_LOW_QUALITY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_HIGH_QUALITY", LQP_HIGH_QUALITY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_VOICE_QUALITY", LQP_VOICE_QUALITY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_R3MIX", LQP_R3MIX, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_VERYHIGH_QUALITY", LQP_VERYHIGH_QUALITY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_STANDARD", LQP_STANDARD, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_FAST_STANDARD", LQP_FAST_STANDARD, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_EXTREME", LQP_EXTREME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_FAST_EXTREME", LQP_FAST_EXTREME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_INSANE", LQP_INSANE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_ABR", LQP_ABR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_CBR", LQP_CBR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_MEDIUM", LQP_MEDIUM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_FAST_MEDIUM", LQP_FAST_MEDIUM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_PHONE", LQP_PHONE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_SW", LQP_SW, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_AM", LQP_AM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_FM", LQP_FM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_VOICE", LQP_VOICE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_RADIO", LQP_RADIO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_TAPE", LQP_TAPE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_HIFI", LQP_HIFI, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_CD", LQP_CD, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LQP_STUDIO", LQP_STUDIO, CONST_CS | CONST_PERSISTENT);

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(lame){
	if(hDLL != NULL){
		FreeLibrary(hDLL);
		hDLL = NULL;
	}
}

PHP_MINFO_FUNCTION(lame)
{
	php_info_print_table_start();
	php_info_print_table_row(2, concat(PHP_LAME_NAME, " support"), "enabled");
	php_info_print_table_row(2, "Version", PHP_LAME_VERSION);
	php_info_print_table_row(2, "Library", &strVersion[0]);
	php_info_print_table_row(2, "Author", "Sektor");
	php_info_print_table_end();
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_getLameVersion, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_newLameConfig, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_beInitStream, 0, 0, 4)
	ZEND_ARG_INFO(0, config)
	ZEND_ARG_INFO(1, samples)
	ZEND_ARG_INFO(1, buffer)
	ZEND_ARG_INFO(1, stream)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_beEncodeChunk, 0, 0, 4)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, wavBuffer)
	ZEND_ARG_INFO(0, dwMp3BufferSize)
	ZEND_ARG_INFO(1, error)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_beEncodeChunkFloatS16NI, 0, 0, 5)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, buffer_l)
	ZEND_ARG_INFO(0, buffer_r)
	ZEND_ARG_INFO(0, dwMp3BufferSize)
	ZEND_ARG_INFO(1, error)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_beDeinitStream, 0, 0, 3)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, dwMP3Buffer)
	ZEND_ARG_INFO(1, error)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_beCloseStream, 0, 0, 1)
	ZEND_ARG_INFO(0, stream)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_beWriteVBRHeader, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_beFlushNoGap, 0, 0, 3)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, dwMP3Buffer)
	ZEND_ARG_INFO(1, error)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_beWriteInfoTag, 0, 0, 2)
	ZEND_ARG_INFO(0, stream)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

zend_function_entry lame_functions[] =
{
	PHP_FE(getLameVersion, arginfo_getLameVersion)
	PHP_FE(newLameConfig, arginfo_newLameConfig)
	PHP_FE(beInitStream, arginfo_beInitStream)
	PHP_FE(beEncodeChunk, arginfo_beEncodeChunk)
	PHP_FE(beEncodeChunkFloatS16NI, arginfo_beEncodeChunkFloatS16NI)
	PHP_FE(beDeinitStream, arginfo_beDeinitStream)
	PHP_FE(beCloseStream, arginfo_beCloseStream)
	PHP_FE(beWriteVBRHeader, arginfo_beWriteVBRHeader)
	PHP_FE(beFlushNoGap, arginfo_beFlushNoGap)
	PHP_FE(beWriteInfoTag, arginfo_beWriteInfoTag)
	{NULL, NULL, NULL}
};

PHP_FUNCTION(getLameVersion){
	array_init(return_value);
	
	add_assoc_long(return_value, "DLLMajorVersion", Version.byDLLMajorVersion);
	add_assoc_long(return_value, "DLLMinorVersion", Version.byDLLMinorVersion);
	add_assoc_long(return_value, "MajorVersion", Version.byMajorVersion);
	add_assoc_long(return_value, "MinorVersion", Version.byMinorVersion);
	add_assoc_long(return_value, "BetaLevel", Version.byBetaLevel);
	add_assoc_long(return_value, "AlphaLevel", Version.byAlphaLevel);
	add_assoc_long(return_value, "MMXEnabled", Version.byMMXEnabled);
	add_assoc_long(return_value, "Day", Version.byDay);
	add_assoc_long(return_value, "Month", Version.byMonth);
	add_assoc_long(return_value, "Month", Version.byMonth);
	add_assoc_long(return_value, "Month", Version.byMonth);
	add_assoc_long(return_value, "Year", Version.wYear);
}

PHP_FUNCTION(newLameConfig){
	zval *format, *mp3, *LHV1, *aac;
	
	//mp3 array
	ALLOC_INIT_ZVAL(mp3);
	array_init(mp3);

	add_assoc_null(mp3,"iSampleRate");
	add_assoc_null(mp3,"iMode");
	add_assoc_null(mp3,"iBitrate");
	add_assoc_null(mp3,"bPrivate");
	add_assoc_null(mp3,"bCRC");
	add_assoc_null(mp3,"bCopyright");
	add_assoc_null(mp3,"bOriginal");


	//LHV1 array
	ALLOC_INIT_ZVAL(LHV1);
	array_init(LHV1);

	add_assoc_null(LHV1,"iSampleRate");
	add_assoc_null(LHV1,"iReSampleRate");
	add_assoc_null(LHV1,"iMode");
	add_assoc_null(LHV1,"iBitrate");
	add_assoc_null(LHV1,"iMaxBitrate");
	add_assoc_null(LHV1,"iPreset");
	add_assoc_null(LHV1,"iMpegVersion");
	add_assoc_null(LHV1,"iPsyModel");
	add_assoc_null(LHV1,"iEmphasis");
	add_assoc_null(LHV1,"bPrivate");
	add_assoc_null(LHV1,"bCRC");
	add_assoc_null(LHV1,"bCopyright");
	add_assoc_null(LHV1,"bOriginal");
	add_assoc_null(LHV1,"bWriteVBRHeader");
	add_assoc_null(LHV1,"bEnableVBR");
	add_assoc_null(LHV1,"iVBRQuality");
	add_assoc_null(LHV1,"iVbrAbr_bps");
	add_assoc_null(LHV1,"iVbrMethod");
	add_assoc_null(LHV1,"bNoRes");
	add_assoc_null(LHV1,"bStrictIso");
	add_assoc_null(LHV1,"iQuality");
	add_assoc_null(LHV1,"aReserved");


	//aac array
	ALLOC_INIT_ZVAL(aac);
	array_init(aac);

	add_assoc_null(aac,"iSampleRate");
	add_assoc_null(aac,"iMode");
	add_assoc_null(aac,"iBitrate");
	add_assoc_null(aac,"iEncodingMethod");


	//format array
	ALLOC_INIT_ZVAL(format);
	array_init(format);

	add_assoc_zval(format, "mp3", mp3);
	add_assoc_zval(format, "LHV1", LHV1);
	add_assoc_zval(format, "aac", aac);

	//return value
	array_init(return_value);

	add_assoc_null(return_value, "iConfig");
	add_assoc_zval(return_value, "aFormat", format);
}

PHP_FUNCTION(beInitStream){
	BE_ERR err = 0;
	zval *config, *samples, *MP3Buffer, *errorCode;
	HBE_STREAM hbeStream = 0;
	BE_CONFIG beConfig = {0,};
	DWORD dwSamples = 0, dwMP3Buffer = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "azz|z", &config, &samples, &MP3Buffer, &errorCode) == FAILURE) {
		RETURN_FALSE;
	}

	if(Z_TYPE_P(config) == IS_NULL){
		php_error(E_WARNING, "%s() missin config parameter",
			get_active_function_name(TSRMLS_C));
		RETURN_FALSE;
	}

	memset(&beConfig,0,sizeof(beConfig));
	beConfig.format.LHV1.dwStructVersion = 1;
	beConfig.format.LHV1.dwStructSize = sizeof(beConfig);	

	phpArrayToBeConfig(&config, &beConfig);

	err = beInitStream(&beConfig, &dwSamples, &dwMP3Buffer, &hbeStream);
	
	if (errorCode){
		zval_dtor(errorCode);
		ZVAL_LONG(errorCode, err);
	}

	if(err != BE_ERR_SUCCESSFUL){
		RETURN_FALSE;
	}

	if (samples){
		zval_dtor(samples);
		ZVAL_LONG(samples, dwSamples);
	}

	if (MP3Buffer){
		zval_dtor(MP3Buffer);
		ZVAL_LONG(MP3Buffer, dwMP3Buffer);
	}

	ZEND_REGISTER_RESOURCE(return_value, hbeStream, le_stream);
}


PHP_FUNCTION(beEncodeChunk){
	HBE_STREAM hbeStream = NULL;
	zval *stream, *wavBuffer, *error;
	BE_ERR err;
	DWORD dwRead, dwWrite = 0, dwMP3Buffer, i;
	PSHORT pWAVBuffer = NULL;
	PBYTE pMP3Buffer = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ralz", &stream, &wavBuffer, &dwMP3Buffer, &error) == FAILURE) {
		RETURN_FALSE;
	}

	ZEND_FETCH_RESOURCE(hbeStream, HBE_STREAM, &stream, -1, le_stream_name, le_stream);

	if(Z_TYPE_P(wavBuffer) != IS_ARRAY){
		RETURN_FALSE;
	}

	phpArrayToPShort(&wavBuffer, &pWAVBuffer);

	dwRead = _msize(pWAVBuffer) / sizeof(*pWAVBuffer);
	
	pMP3Buffer = malloc(dwMP3Buffer * sizeof(BYTE));

	err = beEncodeChunk(hbeStream, dwRead, pWAVBuffer, pMP3Buffer, &dwWrite);

	if(error){
		zval_dtor(error);
	}

	ZVAL_LONG(error, err);

	array_init(return_value);

	for(i = 0; i < dwWrite; i++){
		add_index_long(return_value, i, pMP3Buffer[i]);
	}
}

PHP_FUNCTION(beEncodeChunkFloatS16NI){
	HBE_STREAM hbeStream = NULL;
	zval *stream, *buffer_l, *buffer_r, *error;
	BE_ERR err;
	DWORD dwRead, dwWrite = 0, dwMP3Buffer, i;
	PFLOAT fbuffer_l = NULL, fbuffer_r = NULL;
	PBYTE pMP3Buffer = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "raalz", &stream, &buffer_l, &buffer_r, &dwMP3Buffer, &error) == FAILURE) {
		RETURN_FALSE;
	}

	ZEND_FETCH_RESOURCE(hbeStream, HBE_STREAM, &stream, -1, le_stream_name, le_stream);

	if(Z_TYPE_P(buffer_l) != IS_ARRAY || Z_TYPE_P(buffer_r) != IS_ARRAY ||
		zend_hash_num_elements(Z_ARRVAL_P(buffer_l)) != zend_hash_num_elements(Z_ARRVAL_P(buffer_r))){
		RETURN_FALSE;
	}

	phpArrayToPFloat(&buffer_l, &fbuffer_l);
	phpArrayToPFloat(&buffer_r, &fbuffer_r);

	dwRead = zend_hash_num_elements(Z_ARRVAL_P(buffer_l));

	pMP3Buffer = malloc(dwMP3Buffer * sizeof(BYTE));

	err = beEncodeChunkFloatS16NI(hbeStream, dwRead, fbuffer_l, fbuffer_r, pMP3Buffer, &dwWrite);

	if(error){
		zval_dtor(error);
	}

	ZVAL_LONG(error, err);

	array_init(return_value);

	for(i = 0; i < dwWrite; i++){
		add_index_long(return_value, i, pMP3Buffer[i]);
	}
}

PHP_FUNCTION(beDeinitStream){
	HBE_STREAM hbeStream = NULL;
	zval *stream, *mp3Buffer, *error;
	BE_ERR err;
	PBYTE pMP3Buffer = NULL;
	DWORD dwWrite = 0, i, dwMP3Buffer;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlz", &stream, &dwMP3Buffer, &error) == FAILURE) {
		RETURN_FALSE;
	}

	ZEND_FETCH_RESOURCE(hbeStream, HBE_STREAM, &stream, -1, le_stream_name, le_stream);

	pMP3Buffer = malloc(dwMP3Buffer * sizeof(BYTE));
	err = beDeinitStream(hbeStream, pMP3Buffer, &dwWrite);

	if(error){
		zval_dtor(error);
	}

	ZVAL_LONG(error, err);

	array_init(return_value);

	for(i = 0; i < dwWrite; i++){
		add_index_long(return_value, i, pMP3Buffer[i]);
	}
}

PHP_FUNCTION(beCloseStream){
	HBE_STREAM hbeStream = NULL;
	zval *stream;
	BE_ERR err;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &stream) == FAILURE) {
		RETURN_FALSE;
	}

	ZEND_FETCH_RESOURCE(hbeStream, HBE_STREAM, &stream, -1, le_stream_name, le_stream);

	err = beCloseStream(hbeStream);

	RETURN_LONG(err);
}

PHP_FUNCTION(beWriteVBRHeader){
	char *data;
	int data_len;
	BE_ERR err;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &data, &data_len) == FAILURE) {
		RETURN_FALSE;
	}

	err = beWriteVBRHeader(data);

	RETURN_LONG(err);
}

PHP_FUNCTION(beFlushNoGap){
	HBE_STREAM hbeStream = NULL;
	zval *stream, *mp3Buffer, *error;
	BE_ERR err;
	PBYTE pMP3Buffer = NULL;
	DWORD dwWrite = 0, i, dwMP3Buffer;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlz", &stream, &dwMP3Buffer, &error) == FAILURE) {
		RETURN_FALSE;
	}

	ZEND_FETCH_RESOURCE(hbeStream, HBE_STREAM, &stream, -1, le_stream_name, le_stream);

	pMP3Buffer = malloc(dwMP3Buffer * sizeof(BYTE));
	err = beFlushNoGap(hbeStream, pMP3Buffer, &dwWrite);

	if(error){
		zval_dtor(error);
	}

	ZVAL_LONG(error, err);

	array_init(return_value);

	for(i = 0; i < dwWrite; i++){
		add_index_long(return_value, i, pMP3Buffer[i]);
	}
}

PHP_FUNCTION(beWriteInfoTag){
	HBE_STREAM hbeStream = NULL;
	zval *stream;
	CHAR *data;
	int data_len;
	BE_ERR err;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &stream, &data, &data_len) == FAILURE) {
		RETURN_FALSE;
	}

	ZEND_FETCH_RESOURCE(hbeStream, HBE_STREAM, &stream, -1, le_stream_name, le_stream);

	err = beWriteInfoTag(hbeStream, data);

	RETURN_LONG(err);
}

ZEND_GET_MODULE(lame)

static void stream_destructor(zend_rsrc_list_entry *rsrc TSRMLS_DC){
	
}

static void phpArrayToBeConfig(zval **array, BE_CONFIG *config){
	zval **array_value = NULL, **aFormat = NULL, **subFormat = NULL, *entry;
	Bucket *arrReserved;
	size_t i, arrLen;


	//dwConfig
	if(zend_hash_find(Z_ARRVAL_PP(array), "iConfig", sizeof("iConfig"), (void **)&array_value) != FAILURE) {
		convert_to_long_ex(array_value);

		if(Z_LVAL_PP(array_value) != IS_NULL){
			config->dwConfig = (DWORD)Z_LVAL_PP(array_value);
		}
	}

	//format
	if(zend_hash_find(Z_ARRVAL_PP(array), "aFormat", sizeof("aFormat"), (void **)&aFormat) != FAILURE) {
		convert_to_array_ex(aFormat);

		//mp3
		if(zend_hash_find(Z_ARRVAL_PP(aFormat), "mp3", sizeof("mp3"), (void **)&subFormat) != FAILURE) {
			convert_to_array_ex(subFormat);
			
			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iSampleRate", sizeof("iSampleRate"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.mp3.dwSampleRate = (DWORD)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iMode", sizeof("iMode"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.mp3.byMode = (BYTE)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iBitrate", sizeof("iBitrate"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.mp3.wBitrate = (WORD)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "bPrivate", sizeof("bPrivate"), (void **)&array_value) != FAILURE) {
				convert_to_boolean_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.mp3.bPrivate = (BOOL)Z_BVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "bCRC", sizeof("bCRC"), (void **)&array_value) != FAILURE) {
				convert_to_boolean_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.mp3.bCRC = (BOOL)Z_BVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "bCopyright", sizeof("bCopyright"), (void **)&array_value) != FAILURE) {
				convert_to_boolean_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.mp3.bCopyright = (BOOL)Z_BVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "bOriginal", sizeof("bOriginal"), (void **)&array_value) != FAILURE) {
				convert_to_boolean_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.mp3.bOriginal = (BOOL)Z_BVAL_PP(array_value);
				}
			}
		}

		//LHV1
		if(zend_hash_find(Z_ARRVAL_PP(aFormat), "LHV1", sizeof("LHV1"), (void **)&subFormat) != FAILURE) {
			convert_to_array_ex(subFormat);

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iSampleRate", sizeof("iSampleRate"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.dwSampleRate = (DWORD)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iReSampleRate", sizeof("iReSampleRate"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.dwReSampleRate = (DWORD)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iMode", sizeof("iMode"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.nMode = (LONG)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iBitrate", sizeof("iBitrate"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.dwBitrate = (DWORD)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iMaxBitrate", sizeof("iMaxBitrate"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.dwMaxBitrate = (DWORD)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iPreset", sizeof("iPreset"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.nPreset = (LONG)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iMpegVersion", sizeof("iMpegVersion"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.dwMpegVersion = (DWORD)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iPsyModel", sizeof("iPsyModel"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.dwPsyModel = (DWORD)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iEmphasis", sizeof("iEmphasis"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.dwEmphasis = (DWORD)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "bPrivate", sizeof("bPrivate"), (void **)&array_value) != FAILURE) {
				convert_to_boolean_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.bPrivate = (BOOL)Z_BVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "bCRC", sizeof("bCRC"), (void **)&array_value) != FAILURE) {
				convert_to_boolean_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.bCRC = (BOOL)Z_BVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "bCopyright", sizeof("bCopyright"), (void **)&array_value) != FAILURE) {
				convert_to_boolean_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.bCopyright = (BOOL)Z_BVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "bOriginal", sizeof("bOriginal"), (void **)&array_value) != FAILURE) {
				convert_to_boolean_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.bOriginal = (BOOL)Z_BVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "bWriteVBRHeader", sizeof("bWriteVBRHeader"), (void **)&array_value) != FAILURE) {
				convert_to_boolean_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.bWriteVBRHeader = (BOOL)Z_BVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "bEnableVBR", sizeof("bEnableVBR"), (void **)&array_value) != FAILURE) {
				convert_to_boolean_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.bEnableVBR = (BOOL)Z_BVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iVBRQuality", sizeof("iVBRQuality"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.nVBRQuality = (INT)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iVbrAbr_bps", sizeof("iVbrAbr_bps"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.dwVbrAbr_bps = (DWORD)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iVbrMethod", sizeof("iVbrMethod"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.nVbrMethod = (WORD)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "bNoRes", sizeof("bNoRes"), (void **)&array_value) != FAILURE) {
				convert_to_boolean_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.bNoRes = (BOOL)Z_BVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "bStrictIso", sizeof("bStrictIso"), (void **)&array_value) != FAILURE) {
				convert_to_boolean_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.bStrictIso = (BOOL)Z_BVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iQuality", sizeof("iQuality"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.LHV1.nQuality = (WORD)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "aReserved", sizeof("aReserved"), (void **)&array_value) != FAILURE) {
				convert_to_array_ex(array_value);

				if(Z_TYPE_PP(array_value) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_PP(array_value)) > 0){
					arrLen = sizeof(config->format.LHV1.btReserved);
					
					for (i = 0, arrReserved = Z_ARRVAL_PP(array_value)->pListHead; i < arrLen && arrReserved; i++, arrReserved = arrReserved->pListNext) {
						entry = *((zval **)arrReserved->pData);
						config->format.LHV1.btReserved[i] = (BYTE)Z_LVAL_P(entry);
					}
				}
			}
		}

		//aac
		if(zend_hash_find(Z_ARRVAL_PP(subFormat), "aac", sizeof("aac"), (void **)&subFormat) != FAILURE) {
			convert_to_array_ex(subFormat);
			
			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iSampleRate", sizeof("iSampleRate"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.aac.dwSampleRate = (DWORD)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iMode", sizeof("iMode"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.aac.byMode = (BYTE)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iBitrate", sizeof("iBitrate"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.aac.wBitrate = (WORD)Z_LVAL_PP(array_value);
				}
			}

			if(zend_hash_find(Z_ARRVAL_PP(subFormat), "iEncodingMethod", sizeof("iEncodingMethod"), (void **)&array_value) != FAILURE) {
				convert_to_long_ex(array_value);

				if(Z_LVAL_PP(array_value) != IS_NULL){
					config->format.aac.byEncodingMethod = (BYTE)Z_LVAL_PP(array_value);
				}
			}
		}
	}
}

static void phpArrayToPShort(zval **array, PSHORT *pShort){
	const int arrLength = zend_hash_num_elements(Z_ARRVAL_PP(array));
	Bucket *item;
	zval *entry;
	int i;
	
	*pShort = malloc(arrLength * sizeof(SHORT)); //don't forget free allocated memory!!! (memory leak)
	
	for (i = 0, item = Z_ARRVAL_PP(array)->pListHead; i < arrLength && item; i++, item = item->pListNext) {
		entry = *((zval **)item->pData);
		(*pShort)[i] = (SHORT)Z_LVAL_P(entry);
	}
}

static void phpArrayToPFloat(zval **array, PFLOAT *pFloat){
	const int arrLength = zend_hash_num_elements(Z_ARRVAL_PP(array));
	Bucket *item;
	zval *entry;
	int i;

	*pFloat = malloc(arrLength * sizeof(FLOAT)); //don't forget free allocated memory!!! (memory leak)

	for (i = 0, item = Z_ARRVAL_PP(array)->pListHead; i < arrLength && item; i++, item = item->pListNext) {
		entry = *((zval **)item->pData);
		(*pFloat)[i] = (FLOAT)Z_LVAL_P(entry);
	}
}
