#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define R 6371
#define BUFFLEN 200


/* Calculate distance and azimuth/back-azimuth between two (lon,lat) coordinates 
 * using delaz.f subroutine
 */

void delaz_(float *lat1, float *lon1, float *lat2, float *lon2, float *dist, float *az, float *baz);


/*--------------------------------------------------------------------------*/
int main (int argc, char *argv[])
/*--------------------------------------------------------------------------*/
{
  FILE *fpcat, *fpfault, *fploc, *fpcatmod;
  int cnt1, cnt2, nf=0, neqs=0, hlines;
  int eqv, eqy, eqmo, eqd, eqh, eqm; 
  float mag, eqlon, eqlat, eqs, eq_radius; 
  float lon1, lat1, dist, az, baz, mindist;
  float lonf[100], latf[100];
  char catf[200], faultsf[200];
  char eqnote[10], buff[BUFFLEN];

/* CHECK INPUT ARGUMENTS */
  if ( argc != 5 ) {
    fprintf(stderr,"USAGE: %s [EQ catalog] [catalog header lines] [faults file] [EQ radius (km)]\n", argv[0]);
    fprintf(stderr,"Full catalog file, e.g., wmm_cat3.export.txt. Fault file formatted as lon-lat.\n");
    fprintf(stderr,"Code writes out a the local catalog, loc.cc, and modified catalog, mod.cc, with minimum distance to each earthquake.\n");
    exit(1);
  }
  sscanf(argv[1],"%s", catf);
  sscanf(argv[2],"%d", &hlines);
  sscanf(argv[3],"%s", faultsf);
  sscanf(argv[4],"%f", &eq_radius);

// open EQ catalog, faults file, and output files
  if ((fpcat = fopen(catf, "r")) == NULL) {
    fprintf(stderr,"Could not open EQ catalog, %s\n", catf);
    exit(0);
  }
  if ((fpfault = fopen(faultsf, "r")) == NULL) {
    fprintf(stderr,"Could not open EQ fault points, %s\n", faultsf);
    exit(0);
  }
  fploc = fopen("loc.cc", "w");
  fpcatmod = fopen("mod.cc", "w");

// write fault points to array
  while( fgets(buff,BUFFLEN,fpfault) ) {
    sscanf(buff,"%f %f", &lonf[nf], &latf[nf]);
    nf++;
  }
  fclose(fpfault);

// loop through all files in the complete catalog, calculating minimum distance to each
// write out catalog with minimum distance appended and the "local" catalog
// first, remove header lines from catalog
  for (cnt1=0; cnt1<hlines; cnt1++) {
    fgets(buff,BUFFLEN,fpcat);
    fprintf(fpcatmod,"%s", buff);
  }
  while( fgets(buff,BUFFLEN,fpcat) ) {
    mindist=999;
    sscanf(buff,"%f %f %f %d %d %d %d %d %d %f %s", &mag, &eqlon, &eqlat, &eqv, &eqy, &eqmo, &eqd, &eqh, &eqm, &eqs, eqnote); 
//    fprintf(stdout,"%.2f %.3f %.3f %3d %4d %02d %02d %02d %02d %04.1f %s\n", mag, eqlon, eqlat, eqv, eqy, eqmo, eqd, eqh, eqm, eqs, eqnote);
    for(cnt1=0; cnt1<nf; cnt1++) {
      lat1=latf[cnt1];
      lon1=lonf[cnt1];
      delaz_(&eqlat,&eqlon,&lat1,&lon1,&dist,&az,&baz);
      if ( dist < mindist ) mindist=dist;
    }
// write to modified catalog and local catalog, if appropriated
    fprintf(fpcatmod,"%.2f %.3f %.3f %3d %4d %02d %02d %02d %02d %04.1f %s %.3f\n", mag, eqlon, eqlat, eqv, eqy, eqmo, eqd, eqh, eqm, eqs, eqnote, mindist);
    if ( mindist <= eq_radius ) fprintf(fploc,"%.2f %.3f %.3f %3d %4d %02d %02d %02d %02d %04.1f %s\n", mag, eqlon, eqlat, eqv, eqy, eqmo, eqd, eqh, eqm, eqs, eqnote);
  }

// close files
  fclose(fpcat);
  fclose(fploc);
  fclose(fpcatmod);

  return 0;
}
