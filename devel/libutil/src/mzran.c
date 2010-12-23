#include <stdlib.h>
#include <string.h>
#include <util.h>
#include <mzran.h>

/*****************************************************************/
/*
  This file implements the mz random number generator. Historically
  the rng has been translated from Fortran code found on the internet,
  used in the context of SSE QMC simulations by Anders Sandvik.

  The state of the random number generator is based on 4 unsigned
  integers.
*/



#define MZRAN_TYPE_ID  77156432

struct mzran_struct {
  UTIL_TYPE_ID_DECLARATION;
  unsigned int x;
  unsigned int y;
  unsigned int z;
  unsigned int c;
  unsigned int n;
};


#define DEFAULT_S0  99
#define DEFAULT_S1 199
#define DEFAULT_S2  13
#define DEFAULT_S3  77


/*****************************************************************/


/**
   This function will return and unsigned int. This is the fundamental
   low level function which drives the random number generator state
   forward. The returned value will be in the interval [0,MZRAN_MAX).
*/

unsigned int mzran_forward(void * __rng) {
  //mzran_type * rng = (mzran_type *) __rng;
  mzran_type * rng = mzran_safe_cast( __rng );
  {
    unsigned int s;
  
    if (rng->y > (rng->x + rng->c)) { 
      s = rng->y - rng->x - rng->c;    
      rng->c=0; 
    } else  { 
      s = rng->y - rng->x - rng->c - 18; 
      rng->c=1; 
    }
    
    rng->x = rng->y; 
    rng->y = rng->z; 
    rng->z = s; 
    rng->n = 69069*rng->n + 1013904243;
    return rng->z + rng->n;
  }
}






/**
  This function will set the state of the rng, based on four input
  seeds.
*/ 
static void mzran_set_state4(mzran_type * rng , 
                             unsigned int s0 , unsigned int s1,
                             unsigned int s2 , unsigned int s3) {
  
  rng->x = s0;
  rng->y = s1;
  rng->z = s2;
  rng->n = s3;
  rng->c = 1;  
  
}


static unsigned int fscanf_4bytes( FILE * stream ) {
  unsigned int s;
  char * char_ptr = (char *) &s;
  for (int i=0; i < 4; i++) {
    int c;
    fscanf(stream , "%d" , &c);
    char_ptr[i] = c;
  }
  return s;
}


/**
   Will load four unsigned integers from the open FILE * and call
   mzran_set_state4(). The integers will be loaded with fscanf(),
   i.e. this is formatted file. Will crash and burn if the reading fails. 
*/

void mzran_fscanf_state( void * __rng , FILE * stream ) {
  unsigned int s0 = fscanf_4bytes( stream );
  unsigned int s1 = fscanf_4bytes( stream );
  unsigned int s2 = fscanf_4bytes( stream );
  unsigned int s3 = fscanf_4bytes( stream );

  mzran_type * rng = (mzran_type *) __rng;
  mzran_set_state4( rng , s0 , s1 , s2 , s3);
}



static void fprintf_4bytes( unsigned int s , FILE * stream) {
  char * char_ptr = (char *) &s;
  for (int i=0; i < 4; i++)
    fprintf(stream , "%d " , (int) char_ptr[i]);
}


void mzran_fprintf_state( const void * __rng , FILE * stream) {
  mzran_type * rng = (mzran_type *) __rng;
  fprintf_4bytes( rng->x , stream );
  fprintf_4bytes( rng->y , stream );
  fprintf_4bytes( rng->z , stream );
  fprintf_4bytes( rng->n , stream );
}






/**
   This function will set the state of the rng, based on a buffer of
   length buffer size. 16 bytes will be read from the seed buffer.
*/

void mzran_set_state(void * __rng , const char * seed_buffer) {
  mzran_type * rng = mzran_safe_cast( __rng );
  const unsigned int * seed = (const unsigned int *) seed_buffer;
  mzran_set_state4( rng , seed[0] , seed[1] , seed[2] , seed[3]);
}




UTIL_SAFE_CAST_FUNCTION( mzran , MZRAN_TYPE_ID)


/**
   Creates (and initializes) a new rng instance. To recover a known
   state you must subsequently call one of the mzran_set_state()
   functions.
*/

void * mzran_alloc( ) {
  mzran_type * rng = util_malloc( sizeof * rng , __func__);
  UTIL_TYPE_ID_INIT( rng , MZRAN_TYPE_ID );
  mzran_set_state4( rng , DEFAULT_S0 , DEFAULT_S1 , DEFAULT_S2 , DEFAULT_S3);
  return rng;
}


void mzran_free( void * __rng ) {
  mzran_type * rng = mzran_safe_cast( __rng );
  free( rng );
}

