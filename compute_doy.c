#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*--------------------------------------------------------------------------*/
int compute_doy(int year, int month, int day)
/*--------------------------------------------------------------------------*/
{
  int jday;
  int a, y, m, b, c, d, e, f;
//  int year, month, day;

// DOY from Year/Month/Day
  m=month;
  d=day;
  y=year;
  int jd = 0;
  int i;

  for ( i = 1; i < m; i++ )  {
    if ( (i==1) || (i==3) || (i==5) || (i==7) || (i==8) || (i==10) ) jd += 31;
    else if (i==2) {
      if ( y == 4*(y/4) ) jd += 29;
      else jd += 28;
    }
    else jd += 30;
  }

  jday = jd + d;
  
// print results
//  fprintf(stderr,"Year/Month/Day: %d %d %d\n", year, month, day);
//  fprintf(stderr,"Julian Day: %d \n", jday);

  return jday;
}
