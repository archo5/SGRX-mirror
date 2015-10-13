

#define RX_NEED_DEFAULT_MEMFUNC

#include <utility>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  undef _WIN32_WINNT
#  define _WIN32_WINNT 0x0600
#  undef WINVER
#  define WINVER 0x0600
#  define NOCOMM
#  include <windows.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  define _stat stat
#else
#  include <sys/time.h>
#  include <sys/stat.h>
#  include <unistd.h>
#  include <pthread.h>
#endif

#include <sgscript/sgs_regex.h>


#include "utils.hpp"


void sgrx_assert_func( const char* code, const char* file, int line )
{
	fprintf( stderr, "\n== Error detected: \"%s\", file: %s, line %d ==\n", code, file, line );
#if defined( _MSC_VER )
	__debugbreak();
#elif defined( __GNUC__ )
#  if defined(i386)
	__asm__( "int $3" );
#  else
	__builtin_trap();
#  endif
#else
	assert( 0 );
#endif
}


#if defined( _WIN32 )
void* sgrx_aligned_malloc( size_t size, size_t align ){ return _aligned_malloc( size, align ); }
void sgrx_aligned_free( void* ptr ){ _aligned_free( ptr ); }
#else
void* sgrx_aligned_malloc( size_t size, size_t align )
{
	void* ptr = NULL;
	ASSERT( posix_memalign( &ptr, align, size ) == 0 );
	return ptr;
}
void sgrx_aligned_free( void* ptr ){ free( ptr ); }
#endif


void NOP( int x ){}



/*------------------------------------------------------------*\
 * part of CRC-32 v2.0.0 by Craig Bruce, 2006-04-29.
 * license: public domain
 * original source at http://www.csbruce.com/software/crc32.c
\*------------------------------------------------------------*/
uint32_t sgrx_crc32( const void* buf, size_t len, uint32_t in_crc )
{
	static const uint32_t crcTable[256] =
	{
		0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,
		0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,
		0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,
		0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,
		0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,
		0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,
		0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,0x26D930AC,
		0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
		0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,
		0xB6662D3D,0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,
		0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,
		0x086D3D2D,0x91646C97,0xE6635C01,0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,
		0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,
		0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,0x4DB26158,0x3AB551CE,
		0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,
		0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
		0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,
		0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,
		0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,
		0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,
		0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,0xF00F9344,0x8708A3D2,0x1E01F268,
		0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,
		0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,0xD6D6A3E8,
		0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
		0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,
		0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,
		0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,
		0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,
		0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,
		0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,0x86D3D2D4,0xF1D4E242,
		0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,
		0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
		0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,
		0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,
		0x47B2CF7F,0x30B5FFE9,0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,
		0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,
		0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
	};
	uint32_t crc32;
	const uint8_t *byteBuf;
	size_t i;
	
	crc32 = in_crc ^ 0xFFFFFFFF;
	byteBuf = (const uint8_t*) buf;
	for( i = 0; i < len; ++i )
		crc32 = ( crc32 >> 8 ) ^ crcTable[ ( crc32 ^ byteBuf[ i ] ) & 0xFF ];
	
	return crc32 ^ 0xFFFFFFFF;
}



/**** BEGIN CUSTOM QSORT CODE ****/

/*******************************************************************************
*
*  Author:  Remi Dufour - remi.dufour@gmail.com
*  ! code is modified !, for original, refer to:
*    http://www.codeproject.com/Articles/426706/A-simple-portable-yet-efficient-Quicksort-implemen
*  Date:    July 23rd, 2012
*
*  Name:        Quicksort
*
*  Description: This is a well-known sorting algorithm developed by C. A. R. 
*               Hoare. It is a comparison sort and in this implementation,
*               is not a stable sort.
*
*  Note:        This is public-domain C implementation written from
*               scratch.  Use it at your own risk.
*
*******************************************************************************/

/* Insertion sort threshold shift
 *
 * This macro defines the threshold shift (power of 2) at which the insertion
 * sort algorithm replaces the Quicksort.  A zero threshold shift disables the
 * insertion sort completely.
 *
 * The value is optimized for Linux and MacOS on the Intel x86 platform.
 */
#ifndef INSERTION_SORT_THRESHOLD_SHIFT
# if defined( __APPLE__ ) && defined( __MACH__ )
#  define INSERTION_SORT_THRESHOLD_SHIFT 0
# else
#  define INSERTION_SORT_THRESHOLD_SHIFT 2
# endif
#endif

/* Macro SWAP
 *
 * Swaps the elements of two arrays.
 *
 * The length of the swap is determined by the value of "SIZE".  While both
 * arrays can't overlap, the case in which both pointers are the same works.
 */
#define SWAP(A,B,SIZE)                               \
	{                                                \
		register char       *a_byte = A;             \
		register char       *b_byte = B;             \
		register const char *a_end  = a_byte + SIZE; \
		while (a_byte < a_end)                       \
		{                                            \
			register const char swap_byte = *b_byte; \
			*b_byte++ = *a_byte;                     \
			*a_byte++ = swap_byte;                   \
		}                                            \
	}

/* Macro SWAP_NEXT
 *
 * Swaps the elements of an array with its next value.
 *
 * The length of the swap is determined by the value of "size".  This macro
 * must be used at the beginning of a scope and "A" shouldn't be an expression.
 */
#define SWAP_NEXT(A,SIZE)                                 \
	register char       *a_byte = A;                      \
	register const char *a_end  = A + SIZE;               \
	while (a_byte < a_end)                                \
	{                                                     \
		register const char swap_byte = *(a_byte + SIZE); \
		*(a_byte + SIZE) = *a_byte;                       \
		*a_byte++ = swap_byte;                            \
	}

void sgrx_quicksort( void* array, size_t length, size_t size,
	int(*compare)(const void *, const void *, void*), void* userdata)
{
	struct stackframe
	{
		void *left;
		void *right;
	} stack[CHAR_BIT * sizeof(void *)];

	/* Recursion level */
	struct stackframe *recursion = stack;

#if INSERTION_SORT_THRESHOLD_SHIFT != 0
	/* Insertion sort threshold */
	const ptrdiff_t threshold = (ptrdiff_t) size << INSERTION_SORT_THRESHOLD_SHIFT;
#endif
	
	if( length <= 1 )
		return;

	/* Assign the first recursion level of the sorting */
	recursion->left = array;
	recursion->right = (char *)array + size * (length - 1);

	do
	{
		/* Partition the array */
		register char *idx = (char*) recursion->left;
		register char *right = (char*) recursion->right;
		char          *left  = idx;

		/* Assigning store to the left */
		register char *store = idx;

		/* Pop the stack */
		--recursion;

		/* Determine a pivot (in the middle) and move it to the end */
		/* @modification@ changed the left address to something that works */
		SWAP(left + ((size_t)((right - left) >> 1) / size * size),right,size)

		/* From left to right */
		while (idx < right)
		{
			/* If item is smaller than pivot */
			if (compare(right, idx, userdata) > 0)
			{
				/* Swap item and store */
				SWAP(idx,store,size)

				/* We increment store */
				store += size;
			}

			idx += size;
		}

	    /* Move the pivot to its final place */
		SWAP(right,store,size)

/* Performs a recursion to the left */
#define RECURSE_LEFT                     \
	if (left < store - size)             \
	{                                    \
		(++recursion)->left = left;      \
		recursion->right = store - size; \
	}

/* Performs a recursion to the right */
#define RECURSE_RIGHT                       \
	if (store + size < right)               \
	{                                       \
		(++recursion)->left = store + size; \
		recursion->right = right;           \
	}

/* Insertion sort inner-loop */
#define INSERTION_SORT_LOOP(LEFT)                                 \
	{                                                             \
		register char *trail = idx - size;                        \
		while (trail >= LEFT && compare(trail, trail + size, userdata) > 0) \
		{                                                         \
			SWAP_NEXT(trail,size)                                 \
			trail -= size;                                        \
		}                                                         \
	}

/* Performs insertion sort left of the pivot */
#define INSERTION_SORT_LEFT                                \
	for (idx = left + size; idx < store; idx +=size)       \
		INSERTION_SORT_LOOP(left)

/* Performs insertion sort right of the pivot */
#define INSERTION_SORT_RIGHT                                        \
	for (idx = store + (size << 1); idx <= right; idx +=size)       \
		INSERTION_SORT_LOOP(store + size)

/* Sorts to the left */
#if INSERTION_SORT_THRESHOLD_SHIFT == 0
# define SORT_LEFT RECURSE_LEFT
#else
# define SORT_LEFT                 \
	if (store - left <= threshold) \
	{                              \
		INSERTION_SORT_LEFT        \
	}                              \
	else                           \
	{                              \
		RECURSE_LEFT               \
	}
#endif

/* Sorts to the right */
#if INSERTION_SORT_THRESHOLD_SHIFT == 0
# define SORT_RIGHT RECURSE_RIGHT
#else
# define SORT_RIGHT                 \
	if (right - store <= threshold) \
	{                               \
		INSERTION_SORT_RIGHT        \
	}                               \
	else                            \
	{                               \
		RECURSE_RIGHT               \
	}
#endif

		/* Recurse into the smaller partition first */
		if (store - left < right - store)
		{
		/* Left side is smaller */
			SORT_RIGHT
			SORT_LEFT

			continue;
		}

		/* Right side is smaller */
		SORT_LEFT
		SORT_RIGHT

#undef RECURSE_LEFT
#undef RECURSE_RIGHT
#undef INSERTION_SORT_LOOP
#undef INSERTION_SORT_LEFT
#undef INSERTION_SORT_RIGHT
#undef SORT_LEFT
#undef SORT_RIGHT
	}
	while (recursion >= stack);
}

#undef INSERTION_SORT_THRESHOLD_SHIFT
#undef SWAP
#undef SWAP_NEXT

/**** END CUSTOM QSORT CODE ****/


void memswap( void* a, void* b, size_t size )
{
	char* ca = (char*) a;
	char* cb = (char*) b;
	char* ea = ca + size;
	for( ; ca != ea; ++ca, ++cb )
	{
		char tmp = *ca;
		*ca = *cb;
		*cb = tmp;
	}
}

void sgrx_combsort( void* array, size_t length, size_t size,
	bool(*compless)(const void*, const void*, void*), void* userdata )
{
	char* first = (char*) array;
	char* end = first + length * size;
	static const double shrink_factor = 1.247330950103979;
	ptrdiff_t dist = length;
	bool swaps = true;
	while( dist > 1 || swaps )
	{
		if( dist > 1 )
			dist = ptrdiff_t( double( dist ) / shrink_factor );
		swaps = false;
		char* left = first;
		char* right = first + dist * size;
		while( right != end )
		{
			if( compless( right, left, userdata ) )
			{
				memswap( left, right, size );
				swaps = true;
			}
			left += size;
			right += size;
		}
	}
}



double sgrx_hqtime()
{
#ifdef _WIN32
	LARGE_INTEGER c, f;
	QueryPerformanceCounter( &c );
	QueryPerformanceFrequency( &f );
	return double(c.QuadPart) / double(f.QuadPart);
#else
	timespec ts;
	clock_gettime( CLOCK_MONOTONIC, &ts );
	static const double S2NS = 1.0 / 1000000000.0;
	return ts.tv_sec + S2NS * ts.tv_nsec;
#endif
}



//
// THREADING
//


#ifdef __GNUC__
#  define atomic_inc32(ptr) __sync_add_and_fetch((ptr),1)
#  define atomic_dec32(ptr) __sync_sub_and_fetch((ptr),1)
#  define atomic_cmpxchg(ptr,test,val) __sync_val_compare_and_swap((ptr),(test),(val))
#elif defined (_WIN32)
#  define atomic_inc32(ptr) InterlockedIncrement((ptr))
#  define atomic_dec32(ptr) InterlockedDecrement((ptr))
#  define atomic_cmpxchg(ptr,test,val) InterlockedCompareExchange((ptr),(val),(test))
SGRX_CASSERT(sizeof(LONG) == sizeof(int32_t), long_equals_int32_t);
#else
#  error "no support for interlocked ops?"
#endif

int32_t sgrx_atomic_inc( volatile int32_t* ptr ){ return atomic_inc32( (volatile LONG*) ptr ); }
int32_t sgrx_atomic_dec( volatile int32_t* ptr ){ return atomic_dec32( (volatile LONG*) ptr ); }
int32_t sgrx_atomic_cmpxchg( volatile int32_t* ptr, int32_t test, int32_t val )
{
	return atomic_cmpxchg( (volatile LONG*) ptr, test, val );
}


struct _ThreadData
{
	SGRX_Thread::Proc fn;
	void* data;
};


#ifdef _WIN32

void sgrx_sleep( uint32_t ms )
{
	Sleep( ms );
}

int sgrx_numcpus()
{
	SYSTEM_INFO sysinfo;
	GetNativeSystemInfo( &sysinfo );
	return sysinfo.dwNumberOfProcessors;
}

SGRX_Thread::SGRX_Thread() : handle(NULL), m_nextproc(NULL), m_nextdata(NULL)
{
}

SGRX_Thread::~SGRX_Thread()
{
	Join();
}

static DWORD __stdcall _ThreadProc( void* data )
{
	SGRX_CAST( SGRX_Thread*, T, data );
	T->m_nextproc( T->m_nextdata );
	return 0;
}

void SGRX_Thread::Start( Proc fn, void* data )
{
	ASSERT( handle == NULL && "cannot start a thread that is already started" );
	m_nextproc = fn;
	m_nextdata = data;
	handle = CreateThread( NULL, 1024, _ThreadProc, this, 0, NULL );
	ASSERT( handle && "failed to create a thread" );
}

void SGRX_Thread::Join()
{
	if( !handle )
		return;
	HANDLE T = (HANDLE) handle;
	WaitForSingleObjectEx( T, INFINITE, FALSE );
	CloseHandle( T );
	handle = NULL;
}


#else

void sgrx_sleep( uint32_t ms )
{
	if( ms >= 1000 )
	{
		sleep( ms / 1000 );
		ms %= 1000;
	}
	if( ms > 0 )
	{
		usleep( ms * 1000 );
	}
}

int sgrx_numcpus()
{
	return sysconf( _SC_NPROCESSORS_ONLN );
}

SGRX_Thread::SGRX_Thread() : handle(NULL), m_nextproc(NULL), m_nextdata(NULL)
{
}

SGRX_Thread::~SGRX_Thread()
{
	Join();
}

static void* _ThreadProc( void* data )
{
	SGRX_CAST( SGRX_Thread*, T, data );
	T->m_nextproc( T->m_nextdata );
	return 0;
}

void SGRX_Thread::Start( Proc fn, void* data )
{
	ASSERT( handle && "cannot start a thread that is already started" );
	pthread_t T;
	m_nextproc = fn;
	m_nextdata = data;
	if( pthread_create( &T, NULL, _ThreadProc, this ) )
		handle = T;
	else
		ASSERT( !"failed to create a thread" );
}

void SGRX_Thread::Join()
{
	if( !handle )
		return;
	pthread_t T = (pthread_t) handle;
	pthread_join( T, NULL );
	handle = NULL;
}


#endif



const Quat Quat::Identity = { 0, 0, 0, 1 };


static Quat CalcNearestQuat( const Quat& root, const Quat& qd )
{
	Quat diff = root - qd;
	Quat sum = root + qd;
	if( QuatDot( diff, diff ) < QuatDot( sum, sum ) )
		return qd;
	return -qd;
}

static void CalcDiffAxisAngleQuaternion( const Quat& orn0, const Quat& orn1a, Vec3& axis, float& angle )
{
	Quat orn1 = CalcNearestQuat( orn0, orn1a );
	Quat dorn = orn1 * orn0.Inverted();
	angle = dorn.GetAngle();
	axis = V3( dorn.x, dorn.y, dorn.z );
	//check for axis length
	float len = axis.LengthSq();
	if( len < SMALL_FLOAT * SMALL_FLOAT )
		axis = V3(1,0,0);
	else
		axis /= sqrtf( len );
}

Vec3 CalcAngularVelocity( const Quat& qa, const Quat& qb )
{
	Vec3 axis;
	float angle;
	CalcDiffAxisAngleQuaternion( qa, qb, axis, angle );
	return axis * angle;
}


Quat Mat4::GetRotationQuaternion() const
{
#if 1
	Quat Q;
	
	Mat4 usm = *this;
	Vec3 scale = GetScale();
	if( scale.x != 0 ) scale.x = 1 / scale.x;
	if( scale.y != 0 ) scale.y = 1 / scale.y;
	if( scale.z != 0 ) scale.z = 1 / scale.z;
	usm = usm * Mat4::CreateScale( scale );
#define r(x,y) usm.m[x][y]
	
	float tr = r(0,0) + r(1,1) + r(2,2);
	
	if( tr > 0 )
	{
		float S = sqrtf( tr + 1.0f ) * 2;
		Q.w = 0.25f * S;
		Q.x = (r(1,2) - r(2,1)) / S;
		Q.y = (r(2,0) - r(0,2)) / S;
		Q.z = (r(0,1) - r(1,0)) / S;
	}
	else if( ( r(0,0) > r(1,1) ) && ( r(0,0) > r(2,2) ) )
	{
		float S = sqrtf( 1.0f + r(0,0) - r(1,1) - r(2,2) ) * 2;
		Q.w = ( r(1,2) - r(2,1) ) / S;
		Q.x = 0.25f * S;
		Q.y = ( r(1,0) + r(0,1) ) / S;
		Q.z = ( r(2,0) + r(0,2) ) / S;
	}
	else if( r(1,1) > r(2,2) )
	{
		float S = sqrtf( 1.0f + r(1,1) - r(0,0) - r(2,2) ) * 2;
		Q.w = ( r(2,0) - r(0,2) ) / S;
		Q.x = ( r(1,0) + r(0,1) ) / S;
		Q.y = 0.25f * S;
		Q.z = ( r(2,1) + r(1,2) ) / S;
	}
	else
	{
		float S = sqrtf( 1.0f + r(2,2) - r(0,0) - r(1,1) ) * 2;
		Q.w = ( r(0,1) - r(1,0) ) / S;
		Q.x = ( r(2,0) + r(0,2) ) / S;
		Q.y = ( r(2,1) + r(1,2) ) / S;
		Q.z = 0.25f * S;
	}
#undef r
	
	return Q.Normalized();
#else
#define r(x,y) m[x][y]
	float trace = r(0,0) + r(1,1) + r(2,2);

	float temp[4];

	if (trace > float(0.0)) 
	{
		float s = sqrtf(trace + float(1.0));
		temp[3]=(s * float(0.5));
		s = float(0.5) / s;

		temp[0]=((r(2,1) - r(1,2)) * s);
		temp[1]=((r(0,2) - r(2,0)) * s);
		temp[2]=((r(1,0) - r(0,1)) * s);
	} 
	else 
	{
		int i = r(0,0) < r(1,1) ? 
			(r(1,1) < r(2,2) ? 2 : 1) :
			(r(0,0) < r(2,2) ? 2 : 0); 
		int j = (i + 1) % 3;  
		int k = (i + 2) % 3;

		float s = sqrtf(r(i,i) - r(j,j) - r(k,k) + float(1.0));
		temp[i] = s * float(0.5);
		s = float(0.5) / s;

		temp[3] = (r(k,j) - r(j,k)) * s;
		temp[j] = (r(j,i) + r(i,j)) * s;
		temp[k] = (r(k,i) + r(i,k)) * s;
	}
	Quat Q = { temp[0], temp[1], temp[2], temp[3] };
	return Q;
#endif
}

bool Mat4::InvertTo( Mat4& out ) const
{
	float inv[16], det;
	int i;
	
	inv[0] = a[5]  * a[10] * a[15] -
			 a[5]  * a[11] * a[14] -
			 a[9]  * a[6]  * a[15] +
			 a[9]  * a[7]  * a[14] +
			 a[13] * a[6]  * a[11] -
			 a[13] * a[7]  * a[10];
	
	inv[4] = -a[4]  * a[10] * a[15] +
			  a[4]  * a[11] * a[14] +
			  a[8]  * a[6]  * a[15] -
			  a[8]  * a[7]  * a[14] -
			  a[12] * a[6]  * a[11] +
			  a[12] * a[7]  * a[10];
	
	inv[8] = a[4]  * a[9] * a[15] -
			 a[4]  * a[11] * a[13] -
			 a[8]  * a[5] * a[15] +
			 a[8]  * a[7] * a[13] +
			 a[12] * a[5] * a[11] -
			 a[12] * a[7] * a[9];
	
	inv[12] = -a[4]  * a[9] * a[14] +
			   a[4]  * a[10] * a[13] +
			   a[8]  * a[5] * a[14] -
			   a[8]  * a[6] * a[13] -
			   a[12] * a[5] * a[10] +
			   a[12] * a[6] * a[9];
	
	inv[1] = -a[1]  * a[10] * a[15] +
			  a[1]  * a[11] * a[14] +
			  a[9]  * a[2] * a[15] -
			  a[9]  * a[3] * a[14] -
			  a[13] * a[2] * a[11] +
			  a[13] * a[3] * a[10];
	
	inv[5] = a[0]  * a[10] * a[15] -
			 a[0]  * a[11] * a[14] -
			 a[8]  * a[2] * a[15] +
			 a[8]  * a[3] * a[14] +
			 a[12] * a[2] * a[11] -
			 a[12] * a[3] * a[10];
	
	inv[9] = -a[0]  * a[9] * a[15] +
			  a[0]  * a[11] * a[13] +
			  a[8]  * a[1] * a[15] -
			  a[8]  * a[3] * a[13] -
			  a[12] * a[1] * a[11] +
			  a[12] * a[3] * a[9];
	
	inv[13] = a[0]  * a[9] * a[14] -
			  a[0]  * a[10] * a[13] -
			  a[8]  * a[1] * a[14] +
			  a[8]  * a[2] * a[13] +
			  a[12] * a[1] * a[10] -
			  a[12] * a[2] * a[9];
	
	inv[2] = a[1]  * a[6] * a[15] -
			 a[1]  * a[7] * a[14] -
			 a[5]  * a[2] * a[15] +
			 a[5]  * a[3] * a[14] +
			 a[13] * a[2] * a[7] -
			 a[13] * a[3] * a[6];
	
	inv[6] = -a[0]  * a[6] * a[15] +
			  a[0]  * a[7] * a[14] +
			  a[4]  * a[2] * a[15] -
			  a[4]  * a[3] * a[14] -
			  a[12] * a[2] * a[7] +
			  a[12] * a[3] * a[6];
	
	inv[10] = a[0]  * a[5] * a[15] -
			  a[0]  * a[7] * a[13] -
			  a[4]  * a[1] * a[15] +
			  a[4]  * a[3] * a[13] +
			  a[12] * a[1] * a[7] -
			  a[12] * a[3] * a[5];
	
	inv[14] = -a[0]  * a[5] * a[14] +
			   a[0]  * a[6] * a[13] +
			   a[4]  * a[1] * a[14] -
			   a[4]  * a[2] * a[13] -
			   a[12] * a[1] * a[6] +
			   a[12] * a[2] * a[5];
	
	inv[3] = -a[1] * a[6] * a[11] +
			  a[1] * a[7] * a[10] +
			  a[5] * a[2] * a[11] -
			  a[5] * a[3] * a[10] -
			  a[9] * a[2] * a[7] +
			  a[9] * a[3] * a[6];
	
	inv[7] = a[0] * a[6] * a[11] -
			 a[0] * a[7] * a[10] -
			 a[4] * a[2] * a[11] +
			 a[4] * a[3] * a[10] +
			 a[8] * a[2] * a[7] -
			 a[8] * a[3] * a[6];
	
	inv[11] = -a[0] * a[5] * a[11] +
			   a[0] * a[7] * a[9] +
			   a[4] * a[1] * a[11] -
			   a[4] * a[3] * a[9] -
			   a[8] * a[1] * a[7] +
			   a[8] * a[3] * a[5];
	
	inv[15] = a[0] * a[5] * a[10] -
			  a[0] * a[6] * a[9] -
			  a[4] * a[1] * a[10] +
			  a[4] * a[2] * a[9] +
			  a[8] * a[1] * a[6] -
			  a[8] * a[2] * a[5];
	
	det = a[0] * inv[0] + a[1] * inv[4] + a[2] * inv[8] + a[3] * inv[12];
	
	if( det == 0 )
		return false;
	
	det = 1.0f / det;
	
	for( i = 0; i < 16; ++i )
		out.a[ i ] = inv[ i ] * det;
	
	return true;
}

Quat TransformQuaternion( const Quat& q, const Mat4& m )
{
	return Quat::CreateAxisAngle( m.TransformNormal( q.GetAxis() ), q.GetAngle() );
}

const Mat4 Mat4::Identity =
{
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1,
};



float PointLineDistance( const Vec3& pt, const Vec3& l0, const Vec3& l1 )
{
	Vec3 lN = ( l1 - l0 ).Normalized();
	float lD = Vec3Dot( lN, pt ); // point projections
	float lD0 = Vec3Dot( lN, l0 );
	float lD1 = Vec3Dot( lN, l1 );
	float dst = lD1 - lD0;
	// convert projection to factor:
	float q = fabsf( dst ) > SMALL_FLOAT ? ( lD - lD0 ) / dst : 0.5f;
	q = clamp( q, 0, 1 ); // limit projection factor within line
	Vec3 cp = TLERP( l0, l1, q ); // closest point
	return ( cp - pt ).Length(); // distance from closest point
}

float PointTriangleDistance( const Vec3& pt, const Vec3& t0, const Vec3& t1, const Vec3& t2 )
{
	// plane
	Vec3 nrm = Vec3Cross( t1 - t0, t2 - t0 ).Normalized();
	float pd = fabsf( Vec3Dot( nrm, pt ) - Vec3Dot( nrm, t0 ) );
	
	// tangents
	Vec3 tan0 = ( t1 - t0 ).Normalized();
	Vec3 tan1 = ( t2 - t1 ).Normalized();
	Vec3 tan2 = ( t0 - t2 ).Normalized();
	
	// bounds
	float t0p = Vec3Dot( tan0, pt ), t0min = Vec3Dot( tan0, t0 ), t0max = Vec3Dot( tan0, t1 );
	float t1p = Vec3Dot( tan1, pt ), t1min = Vec3Dot( tan1, t1 ), t1max = Vec3Dot( tan1, t2 );
	float t2p = Vec3Dot( tan2, pt ), t2min = Vec3Dot( tan2, t2 ), t2max = Vec3Dot( tan2, t0 );
	
	// check corners
	if( t0min >= t0p && t2max <= t2p ) return ( pt - t0 ).Length();
	if( t1min >= t1p && t0max <= t0p ) return ( pt - t1 ).Length();
	if( t2min >= t2p && t1max <= t1p ) return ( pt - t2 ).Length();
	
	// edge normals
	Vec3 en0 = Vec3Cross( t1 - t0, nrm ).Normalized();
	Vec3 en1 = Vec3Cross( t2 - t1, nrm ).Normalized();
	Vec3 en2 = Vec3Cross( t0 - t2, nrm ).Normalized();
	
	// signed distances from edges
	float ptd0 = Vec3Dot( en0, pt ) - Vec3Dot( en0, t0 );
	float ptd1 = Vec3Dot( en1, pt ) - Vec3Dot( en1, t1 );
	float ptd2 = Vec3Dot( en2, pt ) - Vec3Dot( en2, t2 );
	
	// check edges
	if( ptd0 >= 0 && t0p >= t0min && t0p <= t0max ) return V2( pd, ptd0 ).Length();
	if( ptd1 >= 0 && t1p >= t1min && t1p <= t1max ) return V2( pd, ptd1 ).Length();
	if( ptd2 >= 0 && t2p >= t2min && t2p <= t2max ) return V2( pd, ptd2 ).Length();
	
	// inside
	return pd;
}

bool TriangleIntersect( const Vec3& ta0, const Vec3& ta1, const Vec3& ta2, const Vec3& tb0, const Vec3& tb1, const Vec3& tb2 )
{
	Vec3 nrma = Vec3Cross( ta1 - ta0, ta2 - ta0 ).Normalized();
	Vec3 nrmb = Vec3Cross( tb1 - tb0, tb2 - tb0 ).Normalized();
	Vec3 axes[] =
	{
		nrma, Vec3Cross( ta1 - ta0, nrma ).Normalized(), Vec3Cross( ta2 - ta1, nrma ).Normalized(), Vec3Cross( ta0 - ta2, nrma ).Normalized(),
		nrmb, Vec3Cross( tb1 - tb0, nrmb ).Normalized(), Vec3Cross( tb2 - tb1, nrmb ).Normalized(), Vec3Cross( tb0 - tb2, nrmb ).Normalized(),
	};
	// increase plane overlap chance, decrease edge overlap chance
	static const float margins[] =
	{
		SMALL_FLOAT, -SMALL_FLOAT, -SMALL_FLOAT, -SMALL_FLOAT,
		SMALL_FLOAT, -SMALL_FLOAT, -SMALL_FLOAT, -SMALL_FLOAT,
	};
//	float minmargin = 1e8f;
	for( size_t i = 0; i < sizeof(axes)/sizeof(axes[0]); ++i )
	{
		Vec3 axis = axes[ i ];
		float margin = margins[ i ];
		
		float qa0 = Vec3Dot( axis, ta0 );
		float qa1 = Vec3Dot( axis, ta1 );
		float qa2 = Vec3Dot( axis, ta2 );
		float qb0 = Vec3Dot( axis, tb0 );
		float qb1 = Vec3Dot( axis, tb1 );
		float qb2 = Vec3Dot( axis, tb2 );
		
		float mina = TMIN( TMIN( qa0, qa1 ), qa2 ), maxa = TMAX( TMAX( qa0, qa1 ), qa2 );
		float minb = TMIN( TMIN( qb0, qb1 ), qb2 ), maxb = TMAX( TMAX( qb0, qb1 ), qb2 );
		if( mina > maxb + margin || minb > maxa + margin )
			return false;
//		if( i % 4 != 0 )
//			minmargin = TMIN( minmargin, TMIN( maxa - minb, maxb - mina ) );
	}
//	printf("ISECT minmargin=%f\n",minmargin);
//	printf("A p0 %g %g %g\nA p1 %g %g %g\nA p2 %g %g %g\n", ta0.x,ta0.y,ta0.z, ta1.x,ta1.y,ta1.z, ta2.x,ta2.y,ta2.z );
//	printf("B p0 %g %g %g\nB p1 %g %g %g\nB p2 %g %g %g\n", tb0.x,tb0.y,tb0.z, tb1.x,tb1.y,tb1.z, tb2.x,tb2.y,tb2.z );
	return true;
}

bool TriangleAABBIntersect( const Vec3& t0, const Vec3& t1, const Vec3& t2, const Vec3& bbmin, const Vec3& bbmax )
{
	// aabb-aabb test first
	Vec3 tmin = Vec3::Min( t0, Vec3::Min( t1, t2 ) );
	Vec3 tmax = Vec3::Max( t0, Vec3::Max( t1, t2 ) );
	if( tmin.x > bbmax.x || tmin.y > bbmax.y || tmin.z > bbmax.z ||
		tmax.x < bbmin.x || tmax.y < bbmin.y || tmax.z < bbmin.z )
		return false;
	
	// SAT on triangle axes
	Vec3 tN = Vec3Cross( t1 - t0, t2 - t0 ).Normalized();
	Vec3 tEN0 = Vec3Cross( t1 - t0, tN ).Normalized();
	Vec3 tEN1 = Vec3Cross( t2 - t1, tN ).Normalized();
	Vec3 tEN2 = Vec3Cross( t0 - t2, tN ).Normalized();
	Vec3 bpts[8] =
	{
		V3( bbmin.x, bbmin.y, bbmin.z ),
		V3( bbmax.x, bbmin.y, bbmin.z ),
		V3( bbmin.x, bbmax.y, bbmin.z ),
		V3( bbmax.x, bbmax.y, bbmin.z ),
		V3( bbmin.x, bbmin.y, bbmax.z ),
		V3( bbmax.x, bbmin.y, bbmax.z ),
		V3( bbmin.x, bbmax.y, bbmax.z ),
		V3( bbmax.x, bbmax.y, bbmax.z ),
	};
	Vec3 xtdaxes[4] = { tN, tEN0, tEN1, tEN2 };
	
	for( int axis_id = 0; axis_id < 4; ++axis_id )
	{
		Vec3 axis = xtdaxes[ axis_id ];
		float bpmin = FLT_MAX, bpmax = -FLT_MAX;
		for( int bp = 0; bp < 8; ++bp )
		{
			float bpproj = Vec3Dot( bpts[ bp ], axis );
			bpmin = TMIN( bpmin, bpproj );
			bpmax = TMAX( bpmax, bpproj );
		}
		float tp0 = Vec3Dot( t0, axis );
		float tp1 = Vec3Dot( t1, axis );
		float tp2 = Vec3Dot( t2, axis );
		float tpmin = TMIN( tp0, TMIN( tp1, tp2 ) );
		float tpmax = TMAX( tp0, TMAX( tp1, tp2 ) );
		if( tpmin > bpmax || tpmax < bpmin )
			return false;
	}
	return true;
}

float PolyArea( const Vec2* points, int pointcount )
{
	float area = 0;
	if( pointcount < 3 )
		return area;
	
	for( int i = 0; i < pointcount; ++i )
	{
		int i1 = ( i + 1 ) % pointcount;
		area += points[i].x * points[i1].y - points[i1].x * points[i].y;
	}
	
	return area * 0.5f;
}

float TriangleArea( float a, float b, float c )
{
	return 0.25f * sqrtf( ( a + b + c ) * ( b + c - a ) * ( c + a - b ) * ( a + b - c ) );
}

float TriangleArea( const Vec3& p0, const Vec3& p1, const Vec3& p2 )
{
	return TriangleArea( ( p1 - p0 ).Length(), ( p2 - p1 ).Length(), ( p0 - p2 ).Length() );
}

bool RayPlaneIntersect( const Vec3& pos, const Vec3& dir, const Vec4& plane, float dsts[2] )
{
	/* returns <distance to intersection, signed origin distance from plane>
			\ <false> on (near-)parallel */
	float sigdst = Vec3Dot( pos, plane.ToVec3() ) - plane.w;
	float dirdot = Vec3Dot( dir, plane.ToVec3() );
	
	if( fabs( dirdot ) < SMALL_FLOAT )
	{
		return false;
	}
	else
	{
		dsts[0] = -sigdst / dirdot;
		dsts[1] = sigdst;
		return true;
	}
}

bool PolyGetPlane( const Vec3* points, int pointcount, Vec4& plane )
{
	Vec3 dir = {0,0,0};
	for( int i = 2; i < pointcount; ++i )
	{
		Vec3 nrm = Vec3Cross( points[i-1] - points[0], points[i] - points[0] ).Normalized();
		dir += nrm;
	}
	float lendiff = dir.Length() - ( pointcount - 2 );
	if( fabs( lendiff ) > SMALL_FLOAT )
		return false;
	dir = dir.Normalized();
	plane.x = dir.x;
	plane.y = dir.y;
	plane.z = dir.z;
	plane.w = Vec3Dot( dir, points[0] );
	return true;
}

bool RayPolyIntersect( const Vec3& pos, const Vec3& dir, const Vec3* points, int pointcount, float dst[1] )
{
	float dsts[2];
	Vec4 plane;
	if( !PolyGetPlane( points, pointcount, plane ) )
		return false;
	if( !RayPlaneIntersect( pos, dir, plane, dsts ) || dsts[0] < 0 || dsts[1] < 0 )
		return false;
	Vec3 isp = pos + dir * dsts[0];
	Vec3 normal = plane.ToVec3();
	for( int i = 0; i < pointcount; ++i )
	{
		int i1 = ( i + 1 ) % pointcount;
		Vec3 edir = points[ i1 ] - points[ i ];
		Vec3 eout = Vec3Cross( edir, normal ).Normalized();
		if( Vec3Dot( eout, isp ) - Vec3Dot( eout, points[ i ] ) > SMALL_FLOAT )
			return false;
	}
	dst[0] = dsts[0];
	return true;
}

bool RaySphereIntersect( const Vec3& pos, const Vec3& dir, const Vec3& spherePos, float sphereRadius, float dst[1] )
{
	/* vec3 ray_pos, vec3 ray_dir, vec3 sphere_pos, float radius;
		returns <distance to intersection> \ false on no intersection */
	Vec3 r2s = spherePos - pos;
	float a = Vec3Dot( dir, r2s );
	if( a < 0 )
	{
		return false;
	}
	float b = Vec3Dot( r2s, r2s ) - a * a;
	if( b > sphereRadius * sphereRadius )
	{
		return false;
	}
	dst[0] = a - sqrtf( sphereRadius * sphereRadius - b );
	return true;
}

bool SegmentAABBIntersect( const Vec3& p1, const Vec3& p2, const Vec3& bbmin, const Vec3& bbmax )
{
	Vec3 bbcenter = ( bbmin + bbmax ) * 0.5f;
	Vec3 bbext = bbmax - bbcenter;
	
	Vec3 sdir = ( p2 - p1 ) * 0.5f;
	Vec3 sabsdir = sdir.Abs();
	Vec3 diff = ( 0.5f * ( p2 + p1 ) ) - bbcenter;
	
	if( fabsf( diff.x ) > bbext.x + sabsdir.x ) return false;
	if( fabsf( diff.y ) > bbext.y + sabsdir.y ) return false;
	if( fabsf( diff.z ) > bbext.z + sabsdir.z ) return false;
	
	float f;
	f = sdir.y * diff.z - sdir.z * diff.y; if( fabsf( f ) > bbext.y * sabsdir.z + bbext.z * sabsdir.y ) return false;
	f = sdir.z * diff.x - sdir.x * diff.z; if( fabsf( f ) > bbext.x * sabsdir.z + bbext.z * sabsdir.x ) return false;
	f = sdir.x * diff.y - sdir.y * diff.x; if( fabsf( f ) > bbext.x * sabsdir.y + bbext.y * sabsdir.x ) return false;
	
	return true;
}

bool SegmentAABBIntersect2( const Vec3& p1, const Vec3& p2, const Vec3& bbmin, const Vec3& bbmax, float dst[1] )
{
	const Vec3& rorg = p1;
	float rlen = ( p2 - p1 ).Length();
	Vec3 rdir = ( p2 - p1 ).Normalized();
	
	Vec3 dirfrac =
	{
		safe_fdiv( 1.0f, rdir.x ),
		safe_fdiv( 1.0f, rdir.y ),
		safe_fdiv( 1.0f, rdir.z ),
	};
	
	float t1 = ( bbmin.x - rorg.x ) * dirfrac.x;
	float t2 = ( bbmax.x - rorg.x ) * dirfrac.x;
	float t3 = ( bbmin.y - rorg.y ) * dirfrac.y;
	float t4 = ( bbmax.y - rorg.y ) * dirfrac.y;
	float t5 = ( bbmin.z - rorg.z ) * dirfrac.z;
	float t6 = ( bbmax.z - rorg.z ) * dirfrac.z;
	
	float tmin = TMAX( TMAX( TMIN( t1, t2 ), TMIN( t3, t4 ) ), TMIN( t5, t6 ) );
	float tmax = TMIN( TMIN( TMAX( t1, t2 ), TMAX( t3, t4 ) ), TMAX( t5, t6 ) );
	
	// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
	if( tmax < 0 )
		return false;
	
	// if tmin > tmax, ray doesn't intersect AABB
	if( tmin > tmax )
		return false;
	
	dst[0] = safe_fdiv( tmin, rlen );
	return true;
}



FINLINE char sgrx_tolower( char a )
{
	if( a >= 'A' && a <= 'Z' )
		return a + 'a' - 'A';
	return a;
}

bool StringView::match_loose( const StringView& substr )
{
#define MAX_ML_CHARS 256
	if( substr.size() == 0 )
		return true;
	if( substr.size() > MAX_ML_CHARS )
		return false;
	
	size_t matchstack[ MAX_ML_CHARS ];
	for( size_t off = 0; off < m_size; ++off )
	{
		size_t chpos = off;
		int stacksize = 0;
		for(;;)
		{
			if( chpos >= m_size )
			{
				// reached end of string, backtrack
				chpos = matchstack[ --stacksize ];
				if( stacksize <= 0 )
					break; // nowhere to backtrack
				continue;
			}
			if( sgrx_tolower( substr[ stacksize ] ) == sgrx_tolower( m_str[ chpos ] ) )
			{
				// enter the stack
				matchstack[ stacksize++ ] = ++chpos;
				if( stacksize == (int) substr.size() )
					return true; // matched last char, success
				continue;
			}
			chpos++;
		}
	}
	return false;
}

bool StringView::match( const StringView& regex )
{
	char endch = regex.ch();
	char endpat[] = { endch, 0 };
	StringView pattern = regex.part( 1 );
	StringView mods = pattern.after( endpat );
	pattern = pattern.until( endpat );
	srx_Context* R = srx_CreateExt( pattern.data(), pattern.size(),
		StackString<4>(mods), NULL, srx_DefaultMemFunc, NULL );
	if( R == NULL )
		return false;
	bool ret = srx_MatchExt( R, m_str, m_size, 0 ) > 0;
	srx_Destroy( R );
	return ret;
}

SGRX_Regex::SGRX_Regex( const StringView& regex, const char* mods )
{
	m_R = srx_CreateExt( regex.data(), regex.size(), mods, NULL, srx_DefaultMemFunc, NULL );
}

SGRX_Regex::~SGRX_Regex()
{
	srx_Destroy( (srx_Context*) m_R );
}

bool SGRX_Regex::Match( const StringView& str, size_t off )
{
	if( m_R == NULL )
		return false;
	return srx_MatchExt( (srx_Context*) m_R, str.data(), str.size(), off ) > 0;
}

/* string -> number conversion */

typedef const char CCH;

static int strtonum_hex( CCH** at, CCH* end, int64_t* outi )
{
	int64_t val = 0;
	CCH* str = *at + 2;
	while( str < end && hexchar( *str ) )
	{
		val *= 16;
		val += gethex( *str );
		str++;
	}
	*at = str;
	*outi = val;
	return 1;
}

static int strtonum_oct( CCH** at, CCH* end, int64_t* outi )
{
	int64_t val = 0;
	CCH* str = *at + 2;
	while( str < end && octchar( *str ) )
	{
		val *= 8;
		val += getoct( *str );
		str++;
	}
	*at = str;
	*outi = val;
	return 1;
}

static int strtonum_bin( CCH** at, CCH* end, int64_t* outi )
{
	int64_t val = 0;
	CCH* str = *at + 2;
	while( str < end && binchar( *str ) )
	{
		val *= 2;
		val += getbin( *str );
		str++;
	}
	*at = str;
	*outi = val;
	return 1;
}

static int strtonum_real( CCH** at, CCH* end, double* outf )
{
	double val = 0;
	double vsign = 1;
	CCH* str = *at, *teststr;
	if( str == end )
		return 0;
	
	if( *str == '+' ) str++;
	else if( *str == '-' ){ vsign = -1; str++; }
	
	teststr = str;
	while( str < end && decchar( *str ) )
	{
		val *= 10;
		val += getdec( *str );
		str++;
	}
	if( str == teststr )
		return 0;
	if( str >= end )
		goto done;
	if( *str == '.' )
	{
		double mult = 1.0;
		str++;
		while( str < end && decchar( *str ) )
		{
			mult /= 10;
			val += getdec( *str ) * mult;
			str++;
		}
	}
	if( str < end && ( *str == 'e' || *str == 'E' ) )
	{
		double sign, e = 0;
		str++;
		if( str >= end || ( *str != '+' && *str != '-' ) )
			goto done;
		sign = *str++ == '-' ? -1 : 1;
		while( str < end && decchar( *str ) )
		{
			e *= 10;
			e += getdec( *str );
			str++;
		}
		val *= pow( 10, e * sign );
	}
	
done:
	*outf = val * vsign;
	*at = str;
	return 2;
}

static int strtonum_dec( CCH** at, CCH* end, int64_t* outi, double* outf )
{
	CCH* str = *at, *teststr;
	if( *str == '+' || *str == '-' ) str++;
	teststr = str;
	while( str < end && decchar( *str ) )
		str++;
	if( str == teststr )
		return 0;
	if( str < end && ( *str == '.' || *str == 'E' || *str == 'e' ) )
		return strtonum_real( at, end, outf );
	else
	{
		int64_t val = 0;
		int invsign = 0;
		
		str = *at;
		if( *str == '+' ) str++;
		else if( *str == '-' ){ invsign = 1; str++; }
		
		while( str < end && decchar( *str ) )
		{
			val *= 10;
			val += getdec( *str );
			str++;
		}
		if( invsign ) val = -val;
		*outi = val;
		*at = str;
		return 1;
	}
}

int util_strtonum( CCH** at, CCH* end, int64_t* outi, double* outf )
{
	CCH* str = *at;
	if( str >= end )
		return 0;
	if( end - str >= 3 && *str == '0' )
	{
		if( str[1] == 'x' ) return strtonum_hex( at, end, outi );
		else if( str[1] == 'o' ) return strtonum_oct( at, end, outi );
		else if( str[1] == 'b' ) return strtonum_bin( at, end, outi );
	}
	return strtonum_dec( at, end, outi, outf );
}


String String_Concat( const StringView& a, const StringView& b )
{
	String out;
	out.resize( a.size() + b.size() );
	memcpy( out.data(), a.data(), a.size() );
	memcpy( out.data() + a.size(), b.data(), b.size() );
	return out;
}

String String_Replace( const StringView& base, const StringView& sub, const StringView& rep )
{
	String out;
	size_t at, cur = 0;
	while( ( at = base.find_first_at( sub, cur ) ) != NOT_FOUND )
	{
		out.append( &base[ cur ], at - cur );
		out.append( rep.data(), rep.size() );
		cur = at + sub.size();
	}
	if( cur < base.size() )
		out.append( &base[ cur ], base.size() - cur );
	return out;
}


bool String_ParseBool( const StringView& sv )
{
	if( sv == "true" || sv == "TRUE" ) return true;
	if( sv == "false" || sv == "FALSE" ) return false;
	return String_ParseInt( sv ) != 0;
}

int64_t String_ParseInt( const StringView& sv, bool* success )
{
	int64_t val = 0;
	double valdbl = 0;
	const char* begin = sv.begin(), *end = sv.end();
	int suc = util_strtonum( &begin, end, &val, &valdbl );
	if( success )
		*success = !!suc;
	return suc == 2 ? valdbl : val;
}

double String_ParseFloat( const StringView& sv, bool* success )
{
	double val = 0;
	const char* begin = sv.begin(), *end = sv.end();
	bool suc = strtonum_real( &begin, end, &val ) != 0;
	if( success )
		*success = suc;
	return val;
}

Vec2 String_ParseVec2( const StringView& sv, bool* success )
{
	bool suc = true;
	Vec2 out = {0,0};
	StringView substr = ";";
	if( success ) *success = true;
	
	out.x = String_ParseFloat( sv.until( substr ), &suc );
	if( success ) *success = *success && suc;
	
	StringView tmp = sv.after( substr );
	out.y = String_ParseFloat( tmp.until( substr ), &suc );
	if( success ) *success = *success && suc;
	
	return out;
}

Vec3 String_ParseVec3( const StringView& sv, bool* success )
{
	bool suc = true;
	Vec3 out = {0,0,0};
	StringView substr = ";";
	if( success ) *success = true;
	
	out.x = String_ParseFloat( sv.until( substr ), &suc );
	if( success ) *success = *success && suc;
	
	StringView tmp = sv.after( substr );
	out.y = String_ParseFloat( tmp.until( substr ), &suc );
	if( success ) *success = *success && suc;
	
	tmp = tmp.after( substr );
	out.z = String_ParseFloat( tmp.until( substr ), &suc );
	if( success ) *success = *success && suc;
	
	return out;
}

Vec4 String_ParseVec4( const StringView& sv, bool* success )
{
	bool suc = true;
	Vec4 out = {0,0,0,0};
	StringView substr = ";";
	if( success ) *success = true;
	
	out.x = String_ParseFloat( sv.until( substr ), &suc );
	if( success ) *success = *success && suc;
	
	StringView tmp = sv.after( substr );
	out.y = String_ParseFloat( tmp.until( substr ), &suc );
	if( success ) *success = *success && suc;
	
	tmp = tmp.after( substr );
	out.z = String_ParseFloat( tmp.until( substr ), &suc );
	if( success ) *success = *success && suc;
	
	tmp = tmp.after( substr );
	out.w = String_ParseFloat( tmp.until( substr ), &suc );
	if( success ) *success = *success && suc;
	
	return out;
}



#define U8NFL( x ) ((x&0xC0)!=0x80)

int UTF8Decode( const char* buf, size_t size, uint32_t* outchar )
{
	char c;
	if( size == 0 )
		return 0;
	
	c = *buf;
	if( !( c & 0x80 ) )
	{
		*outchar = (uint32_t) c;
		return 1;
	}
	
	if( ( c & 0xE0 ) == 0xC0 )
	{
		if( size < 2 || U8NFL( buf[1] ) )
			return - (int) TMIN( size, (size_t) 2 );
		*outchar = (uint32_t) ( ( ((int)(buf[0]&0x1f)) << 6 ) | ((int)(buf[1]&0x3f)) );
		return 2;
	}
	
	if( ( c & 0xF0 ) == 0xE0 )
	{
		if( size < 3 || U8NFL( buf[1] ) || U8NFL( buf[2] ) )
			return - (int) TMIN( size, (size_t) 3 );
		*outchar = (uint32_t) ( ( ((int)(buf[0]&0x0f)) << 12 ) | ( ((int)(buf[1]&0x3f)) << 6 )
			| ((int)(buf[2]&0x3f)) );
		return 3;
	}
	
	if( ( c & 0xF8 ) == 0xF0 )
	{
		if( size < 4 || U8NFL( buf[1] ) || U8NFL( buf[2] ) || U8NFL( buf[3] ) )
			return - (int) TMIN( size, (size_t) 4 );
		*outchar = (uint32_t) ( ( ((int)(buf[0]&0x07)) << 18 ) | ( ((int)(buf[1]&0x3f)) << 12 )
				| ( ((int)(buf[2]&0x3f)) << 6 ) | ((int)(buf[3]&0x3f)) );
		return 4;
	}
	
	return -1;
}

int UTF8Encode( uint32_t ch, char* out )
{
	if( ch <= 0x7f )
	{
		*out = (char) ch;
		return 1;
	}
	if( ch <= 0x7ff )
	{
		out[ 0 ] = (char)( 0xc0 | ( ( ch >> 6 ) & 0x1f ) );
		out[ 1 ] = (char)( 0x80 | ( ch & 0x3f ) );
		return 2;
	}
	if( ch <= 0xffff )
	{
		out[ 0 ] = (char)( 0xe0 | ( ( ch >> 12 ) & 0x0f ) );
		out[ 1 ] = (char)( 0x80 | ( ( ch >> 6 ) & 0x3f ) );
		out[ 2 ] = (char)( 0x80 | ( ch & 0x3f ) );
		return 3;
	}
	if( ch <= 0x10ffff )
	{
		out[ 0 ] = (char)( 0xf0 | ( ( ch >> 18 ) & 0x07 ) );
		out[ 1 ] = (char)( 0x80 | ( ( ch >> 12 ) & 0x3f ) );
		out[ 2 ] = (char)( 0x80 | ( ( ch >> 6 ) & 0x3f ) );
		out[ 3 ] = (char)( 0x80 | ( ch & 0x3f ) );
		return 4;
	}

	return 0;
}

bool UTF8Iterator::Advance()
{
	offset = m_nextoff;
	StringView curchr = m_text.part( m_nextoff );
	codepoint = 0;
	int ret = UTF8Decode( curchr.data(), curchr.size(), &codepoint );
	ret = abs( ret );
	m_nextoff += ret;
	return ret != 0;
}

void UTF8Iterator::SetOffset( size_t off )
{
	offset = off;
	m_nextoff = off;
}


Hash HashFunc( const char* str, size_t size )
{
	size_t i, adv = size / 127 + 1;
	Hash h = 2166136261u;
	for( i = 0; i < size; i += adv )
	{
		h ^= (Hash) (uint8_t) str[ i ];
		h *= 16777619u;
	}
	return h;
}


#ifdef _WIN32
template< int N >
struct StackWString
{
	WCHAR str[ N + 1 ];
	
	StackWString( const StringView& sv )
	{
		int sz = MultiByteToWideChar( CP_UTF8, 0, sv.data(), sv.size(), str, N );
		str[ sz ] = 0;
	}
	operator const WCHAR* (){ return str; }
};
#endif


struct _IntDirIter
{
	_IntDirIter( const StringView& path ) :
		searched( false ),
		searchdir( path ),
		utf8file( "" ),
		hfind( INVALID_HANDLE_VALUE )
	{
		if( wcslen( searchdir ) <= MAX_PATH - 2 )
			wcscat( searchdir.str, L"\\*" );
	}
	~_IntDirIter()
	{
		if( searched && hfind != INVALID_HANDLE_VALUE )
			FindClose( hfind );
	}
	bool Next()
	{
		BOOL ret;
		if( searched )
			ret = FindNextFileW( hfind, &wfd );
		else
		{
			ret = ( hfind = FindFirstFileExW( searchdir, FindExInfoStandard, &wfd, FindExSearchNameMatch, NULL, 0 ) ) != INVALID_HANDLE_VALUE;
			searched = true;
		}
		
		if( ret )
		{
			int sz = WideCharToMultiByte( CP_UTF8, 0, wfd.cFileName, wcslen( wfd.cFileName ), utf8file.str, MAX_PATH * 4, NULL, NULL );
			utf8file.str[ sz ] = 0;
		}
		return ret != FALSE;
	}
	bool IsDirectory()
	{
		return !!( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY );
	}
	
	bool searched;
	StackWString< MAX_PATH > searchdir;
	StackString< MAX_PATH * 4 > utf8file;
	WIN32_FIND_DATAW wfd;
	HANDLE hfind;
};

DirectoryIterator::DirectoryIterator( const StringView& path )
{
	m_int = new _IntDirIter( path );
}

DirectoryIterator::~DirectoryIterator()
{
	delete m_int;
}

bool DirectoryIterator::Next()
{
	return m_int->Next();
}

StringView DirectoryIterator::Name()
{
	return m_int->utf8file.str;
}

bool DirectoryIterator::IsDirectory()
{
	return m_int->IsDirectory();
}


InLocalStorage::InLocalStorage( const StringView& path )
{
#if WINAPI_FAMILY == WINAPI_FAMILY_PC_APP || WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
	// TODO
#else
	// retrieve current directory
	CWDGet( m_path );
	
	// retrieve application data directory and go to it
#ifdef _WIN32
	WCHAR bfr[ MAX_PATH + 1 ] = {0};
	GetEnvironmentVariableW( L"APPDATA", bfr, MAX_PATH );
	bool ret = SetCurrentDirectoryW( bfr ) != FALSE;
#else
	bool ret = chdir( getenv("HOME") ) == 0;
#endif
	if( !ret )
	{
		LOG << "Failed to move to application data root";
		return;
	}
	
	// generate necessary directories
	StringView it;
	while( it.size() < path.size() )
	{
		it = path.part( 0, path.find_first_at( "/", it.size() + 1, path.size() ) );
	//	LOG << it;
		if( !DirExists( it ) && !DirCreate( it ) )
		{
			LOG << "Failed to create an application data subdirectory";
			CWDSet( m_path );
			return;
		}
	}
	
	// set new directory
	CWDSet( path );
#endif
}

InLocalStorage::~InLocalStorage()
{
#if WINAPI_FAMILY == WINAPI_FAMILY_PC_APP || WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
	// TODO
#else
	CWDSet( m_path );
#endif
}


bool DirExists( const StringView& path )
{
#ifdef _WIN32
	WIN32_FILE_ATTRIBUTE_DATA attribs;
	if( GetFileAttributesExW( StackWString< MAX_PATH >( path ), GetFileExInfoStandard, &attribs ) == FALSE )
		return false;
	return ( attribs.dwFileAttributes != INVALID_FILE_ATTRIBUTES && ( attribs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) );
#else
	struct stat info;
	if( stat( path, &info ) != 0 )
		return false;
	else if( info.st_mode & S_IFDIR )
		return true;
	return false;
#endif
}

bool DirCreate( const StringView& path )
{
#ifdef _WIN32
	return CreateDirectoryW( StackWString< MAX_PATH >( path ), NULL ) != FALSE;
#else
	return mkdir( StackString< 4096 >( path ) ) == 0;
#endif
}

bool CWDGet( String& path )
{
#if WINAPI_FAMILY == WINAPI_FAMILY_PC_APP || WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
	return false;
#elif defined(_WIN32)
	WCHAR bfr[ MAX_PATH ];
	DWORD nchars = GetCurrentDirectoryW( MAX_PATH, bfr );
	if( nchars )
	{
		path.resize( nchars * 4 );
		DWORD cchars = WideCharToMultiByte( CP_UTF8, 0, bfr, nchars, path.data(), path.size(), NULL, NULL );
		if( cchars )
		{
			path.resize( cchars );
			return true;
		}
	}
	return false;
#else
	const char* ret = getcwd( StackString< 4096 >(""), 4096 );
	if( ret )
		path = ret;
	return ret != NULL;
#endif
}

bool CWDSet( const StringView& path )
{
#if WINAPI_FAMILY == WINAPI_FAMILY_PC_APP || WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
	return false;
#elif defined(_WIN32)
	return SetCurrentDirectoryW( StackWString< MAX_PATH >( path ) ) != FALSE;
#else
	return chdir( StackString< 4096 >( path ) ) == 0;
#endif
}

bool LoadBinaryFile( const StringView& path, ByteArray& out )
{
	FILE* fp = fopen( StackPath( path ), "rb" );
	if( !fp )
		return false;
	
	fseek( fp, 0, SEEK_END );
	long len = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	
	if( len > 0x7fffffff )
	{
		fclose( fp );
		return false;
	}
	
	out.resize( len );
	bool ret = fread( out.data(), (size_t) len, 1, fp ) == 1;
	fclose( fp );
	return ret;
}

bool SaveBinaryFile( const StringView& path, const void* data, size_t size )
{
	FILE* fp = fopen( StackPath( path ), "wb" );
	if( !fp )
		return false;
	
	bool ret = fwrite( data, size, 1, fp ) == 1;
	fclose( fp );
	return ret;
}

bool LoadTextFile( const StringView& path, String& out )
{
	FILE* fp = fopen( StackPath( path ), "rb" );
	if( !fp )
		return false;
	
	fseek( fp, 0, SEEK_END );
	long len = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	
	if( len > 0x7fffffff )
	{
		fclose( fp );
		return false;
	}
	
	out.resize( len );
	bool ret = fread( out.data(), (size_t) len, 1, fp ) == 1;
	if( ret )
	{
		char* src = out.data();
		char* dst = src;
		char* end = src + out.size();
		while( src < end )
		{
			if( *src == '\r' )
			{
				if( src + 1 < end && src[1] == '\n' )
					src++;
				else
					*src = '\n';
			}
			if( src != dst )
				*dst = *src;
			src++;
			dst++;
		}
		out.resize( dst - out.data() );
	}
	fclose( fp );
	return ret;
}

bool SaveTextFile( const StringView& path, const StringView& data )
{
	FILE* fp = fopen( StackPath( path ), "wb" );
	if( !fp )
		return false;
	
#define TXT_WRITE_BUF 1024
	char buf[ TXT_WRITE_BUF ];
	char xbuf[ TXT_WRITE_BUF * 2 ];
	int xbufsz;
	
	bool ret = true;
	StringView it = data;
	while( it )
	{
		int readamount = TMIN( (int) it.size(), TXT_WRITE_BUF );
		memcpy( buf, it.data(), readamount );
		it.skip( readamount );
		
		xbufsz = 0;
		for( int i = 0; i < readamount; ++i )
		{
			if( buf[ i ] == '\n' )
				xbuf[ xbufsz++ ] = '\r';
			xbuf[ xbufsz++ ] = buf[ i ];
		}
		
		ret = ret && fwrite( xbuf, xbufsz, 1, fp ) == 1;
	}
	
	fclose( fp );
	return ret;
}

bool FileExists( const StringView& path )
{
	FILE* fp = fopen( StackPath( path ), "rb" );
	if( !fp )
		return false;
	fclose( fp );
	return path;
}

uint32_t FileModTime( const StringView& path )
{
	struct stat info;
	if( stat( StackPath( path ), &info ) == -1 )
		return 0;
	return TMAX( info.st_ctime, info.st_mtime );
}

bool LoadItemListFile( const StringView& path, ItemList& out )
{
	String text;
	if( !LoadTextFile( path, text ) )
		return false;
	
	out.clear();
	
	StringView it = text;
	it = it.after_all( SPACE_CHARS );
	while( it.size() )
	{
		// prepare for in-place editing
		out.push_back( IL_Item() );
		IL_Item& outitem = out.last();
		
		// comment
		if( it.ch() == '#' )
		{
			it = it.from( "\n" ).after_all( SPACE_CHARS );
			continue;
		}
		
		// read name
		StringView value, name = it.until_any( SPACE_CHARS );
		outitem.name = name;
		
		// skip name and following spaces
		it.skip( name.size() );
		it = it.after_all( HSPACE_CHARS );
		
		// while not at end of line
		while( it.size() && it.ch() != '\n' )
		{
			String tmpval;
			
			// read and skip param name
			name = it.until( "=" );
			it.skip( name.size() + 1 );
			
			// read and skip param value
			if( it.ch() == '"' )
			{
				// parse quoted value
				const char* ptr = it.data(), *end = it.data() + it.size();
				while( ptr < end )
				{
					if( *ptr == '\n' )
					{
						// detected newline in parameter value
						return false;
					}
					if( *ptr == '\"' )
					{
						it.skip( ptr + 1 - it.data() );
						break;
					}
					if( ptr > it.m_str && *(ptr-1) == '\\' )
					{
						if( *ptr == 'n' )
							tmpval.push_back( '\n' );
						else if( *ptr == '"' )
							tmpval.push_back( '\"' );
						else
							tmpval.push_back( '\\' );
					}
					else
						tmpval.push_back( *ptr );
					ptr++;
				}
				
				value = tmpval;
			}
			else
				value = it.until_any( SPACE_CHARS );
			it.skip( value.size() );
			
			// add param
			outitem.params.set( name, value );
			
			it = it.after_all( HSPACE_CHARS );
		}
		
		it = it.after_all( SPACE_CHARS );
	}
	
	return true;
}



//
// LOGGING
//

// #define LOG_TO_STDOUT
#define LOG_TO_FILE


#ifdef GATHER_STATS
SGRX_Log::RegFunc::STATS SGRX_Log::RegFunc::stats;

SGRX_Log::RegFunc::STATS::STATS()
{
	f = fopen( "stats.txt", "w" );
}

SGRX_Log::RegFunc::STATS::~STATS()
{
	fclose( f );
}

void SGRX_Log::RegFunc::WriteStat( double dt )
{
	RegFunc* curr = lastfunc;
	while( curr )
	{
		fprintf( stats.f, "%s(", curr->funcname );
		if( curr->arg )
			fprintf( stats.f, "\"%s\"", StackString< 200 >( curr->arg ).str );
		fprintf( stats.f, ")" );
		curr = curr->prev;
		if( curr )
			fprintf( stats.f, "<-" );
	}
	fprintf( stats.f, ": %f\n", dt );
}
#  define GTHST_BEGIN time = sgrx_hqtime();
#  define GTHST_END WriteStat( sgrx_hqtime() - time );
#else
#  define GTHST_BEGIN
#  define GTHST_END
#endif


SGRX_Log::RegFunc::RegFunc( const char* func, const char* file, int ln, StringView a ) :
	funcname( func ), filename( file ), linenum( ln ), arg( a ), prev( lastfunc )
{
	lastfunc = this;
	GTHST_BEGIN;
}

SGRX_Log::RegFunc::~RegFunc()
{
	GTHST_END;
	lastfunc = prev;
}


SGRX_Log::init::init()
{
#ifdef LOG_TO_STDOUT
	out = stdout;
#endif
#ifdef LOG_TO_FILE
	out = fopen( "log.txt", "w" );
	if( out )
	{
		setvbuf( out, NULL, _IONBF, 1024 );
	}
#endif
}

SGRX_Log::init::~init()
{
#ifdef LOG_TO_FILE
	fclose( out );
#endif
}

SGRX_Log::init SGRX_Log::_init;
FILE* SGRX_Log::out = NULL;
THREAD_LOCAL SGRX_Log::RegFunc* SGRX_Log::lastfunc = NULL;

SGRX_Log::SGRX_Log() : end_newline(true), need_sep(false), sep("") {}
SGRX_Log::~SGRX_Log(){ sep = ""; if( end_newline ) *this << "\n"; }

void SGRX_Log::prelog()
{
	if( !out ) return;
	if( need_sep )
		fprintf( out, "%s", sep );
	else
		need_sep = true;
}

SGRX_Log& SGRX_Log::operator << ( EMod_Partial ){ end_newline = false; return *this; }
SGRX_Log& SGRX_Log::operator << ( ESpec_Date )
{
	if( !out ) return *this;
	time_t ttv;
	time( &ttv );
	struct tm T = *localtime( &ttv );
	char pbuf[ 256 ] = {0};
	strftime( pbuf, 255, "%Y-%m-%d %H:%M:%S", &T );
	fprintf( out, "%s", pbuf );
	return *this;
}
SGRX_Log& SGRX_Log::operator << ( ESpec_CallStack )
{
	if( !out ) return *this;
	fprintf( out, "--- CALL STACK ---\n" );
	RegFunc* curr = lastfunc;
	if( curr == NULL )
		fprintf( out, "    === empty ===\n" );
	while( curr )
	{
		fprintf( out, "> %s(", curr->funcname );
		if( curr->arg )
			fprintf( out, "\"%s\"", StackString< 200 >( curr->arg ).str );
		fprintf( out, ") (%s:%d)\n", curr->filename, curr->linenum );
		curr = curr->prev;
	}
	fprintf( out, "--- ---------- ---" );
	return *this;
}
SGRX_Log& SGRX_Log::operator << ( const Separator& s ){ sep = s.sep; return *this; }
SGRX_Log& SGRX_Log::operator << ( bool v ){ if( out ){ prelog(); fprintf( out, "[%s / %02X]", v ? "true" : "false", (int) v ); } return *this; }
SGRX_Log& SGRX_Log::operator << ( int8_t v ){ if( out ){ prelog(); fprintf( out, "%d", (int) v ); } return *this; }
SGRX_Log& SGRX_Log::operator << ( uint8_t v ){ if( out ){ prelog(); fprintf( out, "%d", (int) v ); } return *this; }
SGRX_Log& SGRX_Log::operator << ( int16_t v ){ if( out ){ prelog(); fprintf( out, "%d", (int) v ); } return *this; }
SGRX_Log& SGRX_Log::operator << ( uint16_t v ){ if( out ){ prelog(); fprintf( out, "%d", (int) v ); } return *this; }
SGRX_Log& SGRX_Log::operator << ( int32_t v ){ if( out ){ prelog(); fprintf( out, "%" PRId32, v ); } return *this; }
SGRX_Log& SGRX_Log::operator << ( uint32_t v ){ if( out ){ prelog(); fprintf( out, "%" PRIu32, v ); } return *this; }
SGRX_Log& SGRX_Log::operator << ( int64_t v ){ if( out ){ prelog(); fprintf( out, "%" PRId64, v ); } return *this; }
SGRX_Log& SGRX_Log::operator << ( uint64_t v ){ if( out ){ prelog(); fprintf( out, "%" PRIu64, v ); } return *this; }
SGRX_Log& SGRX_Log::operator << ( float v ){ return *this << (double) v; }
SGRX_Log& SGRX_Log::operator << ( double v ){ if( out ){ prelog(); fprintf( out, "%g", v ); } return *this; }
SGRX_Log& SGRX_Log::operator << ( const void* v ){ if( out ){ prelog(); fprintf( out, "[%p]", v ); } return *this; }
SGRX_Log& SGRX_Log::operator << ( const char* v ){ if( out ){ prelog(); fprintf( out, "%s", v ); } return *this; }
SGRX_Log& SGRX_Log::operator << ( const StringView& sv ){ if( out ){ prelog(); fprintf( out, "[%d]\"", (int) sv.size() ); fwrite( sv.data(), sv.size(), 1, out ); fputc( '\"', out ); } return *this; }
SGRX_Log& SGRX_Log::operator << ( const String& sv ){ return *this << (StringView) sv; }
SGRX_Log& SGRX_Log::operator << ( const Vec2& v )
{
	prelog();
	if( out ) fprintf( out, "Vec2( %g ; %g )", v.x, v.y );
	return *this;
}
SGRX_Log& SGRX_Log::operator << ( const Vec3& v )
{
	prelog();
	if( out ) fprintf( out, "Vec3( %g ; %g ; %g )", v.x, v.y, v.z );
	return *this;
}
SGRX_Log& SGRX_Log::operator << ( const Vec4& v )
{
	prelog();
	if( out ) fprintf( out, "Vec4( %g ; %g ; %g ; %g )", v.x, v.y, v.z, v.w );
	return *this;
}
SGRX_Log& SGRX_Log::operator << ( const Quat& q )
{
	prelog();
	if( out ) fprintf( out, "Quat( %g ; %g ; %g ; w = %g )", q.x, q.y, q.z, q.w );
	return *this;
}
SGRX_Log& SGRX_Log::operator << ( const Mat4& v )
{
	prelog();
	if( out ) fprintf( out, "Mat4(\n" );
	if( out ) fprintf( out, "\t%g\t%g\t%g\t%g\n",  v.m[0][0], v.m[0][1], v.m[0][2], v.m[0][3] );
	if( out ) fprintf( out, "\t%g\t%g\t%g\t%g\n",  v.m[1][0], v.m[1][1], v.m[1][2], v.m[1][3] );
	if( out ) fprintf( out, "\t%g\t%g\t%g\t%g\n",  v.m[2][0], v.m[2][1], v.m[2][2], v.m[2][3] );
	if( out ) fprintf( out, "\t%g\t%g\t%g\t%g\n)", v.m[3][0], v.m[3][1], v.m[3][2], v.m[3][3] );
	return *this;
}



//
// TESTS
//

struct _teststr_ { const void* p; int x; };
inline bool operator == ( const _teststr_& a, const _teststr_& b ){ return a.p == b.p && a.x == b.x; }
inline Hash HashVar( const _teststr_& v ){ return (int)v.p + v.x; }
#define CLOSE(a,b) (fabsf((a)-(b))<SMALL_FLOAT)

#define TESTSER_SIZE (1+8+4+(4+4+4)+(4+15))
struct _testser_
{
	uint8_t a; int64_t b; float c; Vec3 d; String e;
	bool operator != ( const _testser_& o ) const { return a != o.a || b != o.b || c != o.c || d != o.d || e != o.e; }
	template< class T > void Serialize( T& arch ){ if( T::IsText ) arch.marker( "TEST" ); arch << a << b << c << d << e; }
};

bool complessint( const void* a, const void* b, void* )
{
	return *(int*)a < *(int*)b;
}

int TestSystems()
{
	LOG_FUNCTION;
	LOG << LOG_CALLSTACK;
	
	char bfr[ 4 ];
	sgrx_snprintf( bfr, 4, "abcd" );
	if( bfr[ 3 ] != '\0' ) return 101; // sgrx_snprintf not null-terminating
	
	int arr1a[] = { 3, 1, 2 };
	int arr1b[] = { 1, 2, 3 };
	sgrx_combsort( arr1a, 3, sizeof(int), complessint, NULL );
	for( int i = 0; i < 3; ++i ) if( arr1a[i] != arr1b[i] ) return 151;

	HashTable< int, int > ht_ii;
	if( ht_ii.size() != 0 ) return 501; // empty
	if( ht_ii.getcopy( 0, 0 ) ) return 502; // miss
	ht_ii.set( 42, 12345 );
	if( ht_ii.size() == 0 ) return 503; // not empty
	if( ht_ii.item(0).key != 42 || ht_ii.item(0).value != 12345 ) return 504; // created item
	if( !ht_ii.getraw( 42 ) ) return 505; // can find item
	if( ht_ii.getcopy( 42 ) != 12345 ) return 506; // returns right value
	
	HashTable< _teststr_, int > ht_si;
	_teststr_ tskey = { "test", 42 };
	if( ht_si.size() != 0 ) return 551; // empty
	if( ht_si.getcopy( tskey, 0 ) ) return 552; // miss
	ht_si.set( tskey, 12345 );
	if( ht_si.size() == 0 ) return 553; // not empty
	if( ht_si.item(0).key.x != tskey.x || ht_si.item(0).value != 12345 ) return 554; // created item
	if( !ht_si.getraw( tskey ) ) return 555; // can find item
	if( ht_si.getcopy( tskey ) != 12345 ) return 556; // returns right value
	
	_testser_ dst, src = { 1, -2, 3, { 4, 5, 6 }, String("abC40!\x00\x01\x7f \\!end",15) };
	ByteArray barr;
	String carr;
	ByteWriter( &barr ) << src;
	if( barr.size() != TESTSER_SIZE ) return 601;
	TextWriter( &carr ) << src;
	if( !carr.size() ) return 602;
	ByteReader br( &barr );
	br << dst;
	if( br.error ) return 603;
	if( src != dst ) return 604;
	TextReader tr( &carr );
	tr << dst;
	if( tr.error ) return 605;
	if( src != dst ) return 606;
	
	Vec3 rot_angles = DEG2RAD( V3(25,50,75) );
	Mat4 rot_mtx = Mat4::CreateRotationXYZ( rot_angles );
	Vec3 out_rot_angles = rot_mtx.GetXYZAngles();
	if( !( rot_angles - out_rot_angles ).NearZero() ) return 701;
	
	Vec3 tri0[3] = { V3(0,0,0), V3(2,0,0), V3(0,3,0) };
	if( PointTriangleDistance( V3(0.5f,0.5f,0), tri0[0], tri0[1], tri0[2] ) != 0.0f ) return 801;
	if( PointTriangleDistance( V3(0,0,0), tri0[0], tri0[1], tri0[2] ) != 0.0f ) return 802;
	if( !CLOSE( PointTriangleDistance( V3(-1,-1,0), tri0[0], tri0[1], tri0[2] ), V3(-1,-1,0).Length() ) ) return 803;
	if( !CLOSE( PointTriangleDistance( V3(-1,0.5f,0), tri0[0], tri0[1], tri0[2] ), 1 ) ) return 804;
	if( !CLOSE( PointTriangleDistance( V3(0.5f,-1,0), tri0[0], tri0[1], tri0[2] ), 1 ) ) return 805;
	if( !CLOSE( PointTriangleDistance( V3(3,-1,0), tri0[0], tri0[1], tri0[2] ), V3(1,-1,0).Length() ) ) return 806;
	
	return 0;
}


