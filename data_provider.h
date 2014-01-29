#ifndef INC_DATA_PROVIDER_H_
#define INC_DATA_PROVIDER_H_

#include <cstdio>
#include <cassert>
#include "Globals.h"

class CDataProvider {
public:
	virtual ~CDataProvider() { }

	// template to load POD without specifying the size
	template< class POD > 
	void read( POD &pod ) {
		readBytes( &pod, sizeof( POD ) );
	}

	// Derived classes must implement these methods
	virtual void readBytes( void *addr, size_t nbytes ) = 0;
	virtual bool isValid() const = 0;
	virtual const char *getName() const = 0;
};

class CFileDataProvider : public CDataProvider {
	FILE *f;
	const char *filename;
public:
	CFileDataProvider( const char *afilename ) : f( NULL ), filename( afilename ) {
		open( afilename );
	}
	~CFileDataProvider() {
		close();
	}
	const char *getName() const { return filename; }
	void close()  {
		if( f )
			fclose( f ), f = NULL;
	}
	void open( const char *afilename ) {
		close();
		f = fopen( afilename, "rb" );
		assert( isValid() || fatal( "CFileDataProvider: Can't open file %s\n", afilename ) );
		filename = afilename;
	}

	static bool exists( const char *afilename ) {
		// Cambiar por fstat of stats* 
		FILE *mf = fopen( afilename, "rb" );
		if( mf ) {
			fclose( mf );
			return true;
		}
		return false;
	}

	bool isValid() const { return f != NULL; }
	void readBytes( void *addr, size_t nbytes ) {
		assert( isValid() );
		size_t bytes_read = fread( addr, 1, nbytes, f );
		assert( bytes_read == nbytes );
	}
};

#endif
