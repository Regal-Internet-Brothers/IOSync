
// Data type definitions:
#if !defined(QSOCK_DATATYPES)
	#define QSOCK_DATATYPES

	// Character/8-bit types:
	#define QSOCK_CHAR char
	typedef QSOCK_CHAR qchar;

	typedef unsigned QSOCK_CHAR QSOCK_UCHAR;
	typedef unsigned QSOCK_CHAR uqchar;
	typedef uqchar qbyte;

	// 16-bit data types:
	#define QSOCK_INT16 short

	typedef QSOCK_INT16 qshort;
	typedef unsigned QSOCK_INT16 QSOCK_UINT16;
	typedef unsigned QSOCK_INT16 uqshort;
				
	// 32-bit data types:
	#define QSOCK_INT32_LONG long
	#define QSOCK_INT32 int
	#define QSOCK_FLOAT32 float
		
	typedef unsigned QSOCK_INT32 QSOCK_UINT32;
	typedef unsigned QSOCK_INT32_LONG QSOCK_UINT32_LONG;

	//typedef unsigned QSOCK_FLOAT32 QSOCK_UFLOAT32;
	typedef QSOCK_FLOAT32 qfloat;
	//typedef QSOCK_UFLOAT32 QSOCK_UFLOAT;

	typedef QSOCK_INT32 qint;
	typedef QSOCK_UINT32 uqint;

	// 64-bit data types:
	#define QSOCK_INT64 long long
	#define QSOCK_FLOAT64 double

	typedef QSOCK_INT64 qlong;
	typedef QSOCK_FLOAT64 qdouble;
	typedef qdouble QSOCK_DOUBLEX1;

	typedef unsigned QSOCK_INT64 QSOCK_UINT64;
	typedef unsigned QSOCK_INT64 uqlong;
	
	// If only unsigned floats were a thing.
	//typedef unsigned QSOCK_FLOAT64 QSOCK_UDOUBLE;
	//typedef unsigned QSOCK_FLOAT64 QSOCK_UDOUBLEX1;
	//typedef unsigned QSOCK_FLOAT64 QSOCK_UFLOAT64;

	// 128-bit data types:
	#define QSOCK_DOUBLEX2 double double
		
	//typedef QSOCK_DOUBLEX2 QSOCK_DOUBLE_DOUBLE;
	//typedef unsigned QSOCK_DOUBLEX2 QSOCK_UDOUBLE_DOUBLE;
	//typedef unsigned QSOCK_DOUBLEX2 QSOCK_UDOUBLEX2;
#endif
