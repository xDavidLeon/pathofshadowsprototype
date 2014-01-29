#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <windows.h>
#include "globals.h"

void dbg( const char *fmt, ... ) {
	char buf[ 4096*4 ];
	va_list ap;
	va_start( ap, fmt );
	int n = _vsnprintf_s( buf, sizeof( buf )-1, fmt, ap );
	assert( n < sizeof( buf ) );
	OutputDebugString( buf );
}

int fatal( const char *fmt, ... ) {
	char buf[ 4096 ];
	va_list ap;
	va_start( ap, fmt );
	int n = _vsnprintf_s( buf, sizeof( buf )-1, fmt, ap );
	assert( n < sizeof( buf ) );
	OutputDebugString( buf );
	return 0;
}

void fatalErrorWindow(const char* err_message)
{
	MessageBox(NULL, err_message, "Error  :-(", MB_ICONEXCLAMATION | MB_OK);
	exit(1);
}
