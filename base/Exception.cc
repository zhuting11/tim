#include <tim/base/Exception.h>

//#include <cxxabi.h>
//#include <execinfo.h>
#include <stdlib.h>
#include <Windows.h>
#include <DbgHelp.h>
//#include <WinBase.h>

using namespace tim;

Exception::Exception(const char* msg)
  : message_(msg)
{
  fillStackTrace();
}

Exception::Exception(const string& msg)
  : message_(msg)
{
  fillStackTrace();
}

Exception::~Exception() throw ()
{
}

const char* Exception::what() const throw()
{
  return message_.c_str();
}

const char* Exception::stackTrace() const throw()
{
  return stack_.c_str();
}

void Exception::fillStackTrace()
{
  //const int len = 200;
  //void* buffer[len];
  //int nptrs = ::backtrace(buffer, len);
  //char** strings = ::backtrace_symbols(buffer, nptrs);
  //if (strings)
  //{
  //  for (int i = 0; i < nptrs; ++i)
  //  {
  //    // TODO demangle funcion name with abi::__cxa_demangle
  //    stack_.append(strings[i]);
  //    stack_.push_back('\n');
  //  }
  //  free(strings);
  //}

	unsigned int   i;
	void         * stack[ 100 ];
	unsigned short frames;
	SYMBOL_INFO  * symbol;
	HANDLE         process;
	char		   buf[512] = {0};

	process = GetCurrentProcess();
	SymInitialize( process, NULL, TRUE );
	frames               = CaptureStackBackTrace( 0, 100, stack, NULL );
	symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
	symbol->MaxNameLen   = 255;
	symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

	for( i = 0; i < frames; i++ )
	{
		SymFromAddr( process, ( DWORD64 )( stack[ i ] ), 0, symbol );

		//printf( "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address );

		_snprintf_s(buf, sizeof(buf)-1, "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address);
		stack_.append(buf);
		stack_.push_back('\n');
		memset(buf, 0, sizeof(buf));
	}

	free( symbol );

}

