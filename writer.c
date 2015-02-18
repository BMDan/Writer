#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#define BLOCKSIZE ((long) (1*1024*1024)) //in bytes
#define BENCH_ROUNDS (100) // nuber of times BLOCKSIZE bytes will be written (note: int)
#define FLUSHIT // fflush() after each write?
#define SLEEP_MILLIS (100) // Easier-to-read SLEEP_NANOS
#define SLEEP_NANOS (SLEEP_MILLIS*1000*1000) // This is what is actually used.
#define DEFAULT_TO_SENDFILE 1 // Use sendfile(2) by default?
#define OUTFILE "deletemetempfile.file" //This should probably be made less silly.

int main( int argc, char **argv ) {
/*  struct timespec {
    time_t tv_sec;        // seconds
    long   tv_nsec;       // nanoseconds
  }; */
  const struct timespec sleep_interval = { 0, SLEEP_NANOS };
  int i;
  ssize_t sfrv;
  FILE *fp;
  FILE *rand;
  struct timespec now, then;
  char use_sendfile = DEFAULT_TO_SENDFILE;
  unsigned char buf[BLOCKSIZE];

  fp = fopen( OUTFILE, "w" );

  if ( !fp ) {
    fprintf( stderr, "Unable to open my temp file: %s.  Dying.\n", strerror(errno) );
    exit( 1 );
  }

  rand = fopen( "/dev/urandom", "r" );

  if ( !rand ) {
    fprintf( stderr, "Unable to open urandom: %s.  Dying.\n", strerror(errno) );
    exit( 1 );
  }

  for ( i = 0; i < BENCH_ROUNDS; i++ ) {
    clock_gettime( CLOCK_MONOTONIC, &then );
    if ( use_sendfile ) {
      sfrv = sendfile( fileno(fp), fileno(rand), NULL, BLOCKSIZE );
    } else {
      sfrv = read( fileno(rand), buf, BLOCKSIZE );
      if ( sfrv != BLOCKSIZE ) {
	// No real way to recover from this.
	fprintf( stderr, "Failed read() on rand: %s", strerror(errno) );
	exit( 1 );
      }
      sfrv = write( fileno(fp), buf, BLOCKSIZE );
    }
#ifdef FLUSHIT
    fflush( fp );
#endif
    clock_gettime( CLOCK_MONOTONIC, &now );

    printf( "%3ld.%09ldms", (now.tv_sec - then.tv_sec - ( now.tv_nsec < then.tv_nsec ? 1 : 0 ) ), (now.tv_nsec + ( now.tv_nsec < then.tv_nsec ? 1*1000*1000*1000 : 0 ) - then.tv_nsec) );

    if ( sfrv == -1 ) {
      printf( " [Failed write: %d returned: %s]\n", errno, strerror(errno) );
      if ( use_sendfile ) {
	fprintf( stderr, "Falling back to read(2) and write(2) due to failed sendfile(2).\n" );
	use_sendfile = 0;
      }
    }
    else if ( sfrv != BLOCKSIZE ) {
      printf( " [Short write: only %ld/%ld written!?]\n", sfrv, BLOCKSIZE );
    }
    else {
      printf( "\n" );
    }

    nanosleep( &sleep_interval, NULL );
  }

  return 0;
}
