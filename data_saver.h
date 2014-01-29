#ifndef INC_DATA_SAVER_H_
#define INC_DATA_SAVER_H_

#include <cstdio>
#include <cassert>
#include <vector>
#include "Globals.h"

class CDataSaver {
public:
	virtual ~CDataSaver() { }

	// template to load POD without specifying the size
	template< class POD > 
	void write( const POD &pod ) {
		writeBytes( &pod, sizeof( POD ) );
	}

	// Derived classes must implement these methods
	virtual void writeBytes( const void *addr, size_t nbytes ) = 0;
	virtual bool isValid() const = 0;
	virtual const char *getName() const = 0;
};

class CMemoryDataSaver : public CDataSaver {
	typedef std::vector< unsigned char > VBytes;
public:
	CMemoryDataSaver( ) {
	}
	~CMemoryDataSaver() {
	}
	const char *getName() const { return "memory_data_saver"; }
	bool isValid() const { return true; }
	void writeBytes( const void *addr, size_t nbytes ) {
		unsigned char *p = (unsigned char *) addr;
		buffer.insert( buffer.end(), p, p + nbytes );
	}
	VBytes buffer;
};


class CFileDataSaver : public CDataSaver {
	const char *out_name;
	FILE *f;
public:
	CFileDataSaver( const char *out_filename ) : out_name ( out_filename ) {
		assert( out_filename );
		f = fopen( out_filename, "wb" );
	}
	~CFileDataSaver() {
		if( f )
			fclose( f ), f = NULL;
	}
	const char *getName() const { return out_name; }
	bool isValid() const { return f != NULL; }
	void writeBytes( const void *addr, size_t nbytes ) {
		assert( isValid( ) );
		size_t bytes_written = fwrite( addr, 1, nbytes, f );
		assert( bytes_written == nbytes );
	}
};


#endif
