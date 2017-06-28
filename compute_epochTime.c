#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>



/*--------------------------------------------------------------------------*/
int compute_epochTime(int yearIn, int monthIn, int dayIn, int hourIn, int minIn, int secIn)
/*--------------------------------------------------------------------------*/
{
  int doy;
  int diffSec_int;
  double diffSec;
  struct tm timeCat; 
  struct tm timeCatRef;

// compute time in seconds since January 1, 1970 00:00:00 (epoch time)

// time struct and epoch time
  timeCat.tm_year=yearIn-1900;
  timeCat.tm_mon=monthIn-1;
  timeCat.tm_mday=dayIn;
  timeCat.tm_hour=hourIn;
  timeCat.tm_min=minIn;
  timeCat.tm_sec=secIn;
  timeCat.tm_isdst=0;

// time struct and epoch time
  timeCatRef.tm_year=1970-1900;
  timeCatRef.tm_mon=1-1;
  timeCatRef.tm_mday=1;
  timeCatRef.tm_hour=0;
  timeCatRef.tm_min=0;
  timeCatRef.tm_sec=0;
  timeCatRef.tm_isdst=0;

//  fprintf(stderr,"Time1 %d %d %d %d %d %d\n", timeCat.tm_year, timeCat.tm_mon, timeCat.tm_mday, timeCat.tm_hour, timeCat.tm_min, timeCat.tm_sec);
//  fprintf(stderr,"TimeRef %d %d %d %d %d %d\n", timeCatRef.tm_year, timeCatRef.tm_mon, timeCatRef.tm_mday, timeCatRef.tm_hour, timeCatRef.tm_min, timeCatRef.tm_sec);
//  fprintf(stderr,"%f seconds \n", (float)mktime(&timeCat));
//  fprintf(stderr,"%f seconds \n", (float)mktime(&timeCatRef));
  diffSec= difftime(mktime(&timeCat), mktime(&timeCatRef));
  diffSec_int= (int)diffSec;

  return diffSec_int;
}


