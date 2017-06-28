#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "params.h"


/* Calculate distance and azimuth/back-azimuth between two (lon,lat) coordinates 
 * using delaz.f subroutine
 */

// function declaration
void delaz_(float *lat1, float *lon1, float *lat2, float *lon2, float *dist, float *az, float *baz);
int getcols( const char * const line, const char * const delim, char ***out_storage);
void strip(char *s);
void assign_cols_flatfile(char **columns, float *stLat, float *stLon, float *evMag, float *evLon, float *evLat, float *evDep, int *evYear, int *evMon, int *evDay, int *evHour, int *evMin, float *evSec, char *network, char * stationNm);
char * replace(char const * const original, char const * const pattern, char const * const replacement );
int compute_epochTime(int yearIn, int monthIn, int dayIn, int hourIn, int minIn, int secIn);

/*--------------------------------------------------------------------------*/
void assign_cols_flatfile(char **columns, float *stLat, float *stLon, float *evMag, float *evLon, float *evLat, float *evDep, int *evYear, int *evMon, int *evDay, int *evHour, int *evMin, float *evSec, char *network, char *stationNm)
/*--------------------------------------------------------------------------*/
{
//
  *stLat=atof(columns[5]);
  *stLon=atof(columns[6]);
  *evMag=atof(columns[25]);
  *evLon=atof(columns[27]);
  *evLat=atof(columns[26]);
  *evDep=atof(columns[28]);
  *evYear=atoi(columns[17]);
  *evMon=atoi(columns[18]);
  *evDay=atoi(columns[19]);
  *evHour=atoi(columns[20]);
  *evMin=atoi(columns[21]);
  *evSec=atof(columns[22]);
  network=strcpy(network,columns[1]);
  stationNm=strcpy(stationNm,columns[2]);
//  fprintf(stderr,"assign_cols_flatfile, evMag/stLat: %f %f\n", *evMag, *stLat);
//  fprintf(stderr,"assign_cols_flatfile, network/stationNm: %s %s\n", network, stationNm);

}

/*--------------------------------------------------------------------------*/
void strip(char *s)
/*--------------------------------------------------------------------------*/
{
    char *p2 = s;
    while(*s != '\0') {
        if(*s != '\t' && *s != '\n') {
            *p2++ = *s++;
        } else {
            ++s;
        }
    }
    *p2 = '\0';
}

/*--------------------------------------------------------------------------*/
int main (int argc, char *argv[])
/*--------------------------------------------------------------------------*/
{
  FILE *fpFlatFileMod, *fpFlatFile, *fpAddToFlatFile;
  int cnt, cntMod=0, cntOrig=0, cnt1, cnt2, cnt3;
  int hlines;
  int cols_found, diffSec, minDiff;
  int evYear, evMon, evDay, evHour, evMin;
  int evYear2, evMon2, evDay2, evHour2, evMin2;
  int epochTimeFlatFile, epochTimeFlatFile2; 
  float objMisfit, objMtmp, diffMag;
  float stLon, stLat, evMag, evLon, evLat, evDep, evSec;
  float stLon2, stLat2, evMag2, evLon2, evLat2, evDep2, evSec2;
  float dist, dist2, az, baz, mindist;
  char addToFlatFile[200], flatFile[200], flatFileMod[200];
  char buff[BUFFLEN], buff2[BUFFLEN], buffFin[BUFFLEN];
  char network[5], stationNm[10], network2[5], stationNm2[10];
  char **columns;
  char **columns2;
  char **columnsOrig;
  char **columnsFinal;
  char delim[] = ",";

/* CHECK INPUT ARGUMENTS */
  if ( argc != 4 ) {
    fprintf(stderr,"USAGE: %s [flatfile] [additional flatfile] [output flatfile]\n", argv[0]);
    fprintf(stderr,"Relocated catalog file, e.g., emm_c2_OK_KS_201702_add_all2.csv\n");
    exit(1);
  }
  sscanf(argv[1],"%s", flatFile);
  sscanf(argv[2],"%s", addToFlatFile);
  sscanf(argv[3],"%s", flatFileMod);

// Open flatfiles
  fprintf(stderr,"\nINPUT/OUTPUT FILES:\n");
  if ((fpFlatFile = fopen(flatFile, "r")) == NULL) {
    fprintf(stderr,"Could not open flatfile, %s\n", flatFile);
    exit(0);
  }
  else {
    fprintf(stderr,"Opened flatfile, %s\n", flatFile);
  }
  if ((fpAddToFlatFile = fopen(addToFlatFile, "r")) == NULL) {
    fprintf(stderr,"Could not open flatfile, %s\n", addToFlatFile);
    exit(0);
  }
  else {
    fprintf(stderr,"Opened add to flatfile, %s\n", addToFlatFile);
  }
//  fclose(fpAddToFlatFile);
  fpFlatFileMod = fopen(flatFileMod, "w");
  fprintf(stderr,"Writing to flatfile, %s\n\n", flatFileMod);

// READ/APPEND FLATFILE
// header lines
  hlines=2;
  for (cnt1=0; cnt1<hlines; cnt1++) {
    fgets(buff,BUFFLEN,fpFlatFile);
//    strip(buff);
    fprintf(fpFlatFileMod,"%s",buff);
  }
  cnt1=0;

// LOOP OVER FLATFILE LINES 
  while( fgets(buff,BUFFLEN,fpFlatFile) ) {
    if ( strlen(buff) > BUFFLEN ) {
      fprintf(stderr,"Increase BUFFLEN from %d.\n", (int)BUFFLEN);
      exit(1);
    }
//    strip(buff);
    buff[strcspn(buff, "\r\n")] = 0;
//    fprintf(fpFlatFileMod,"%s,",buff);
    columns = NULL;
    cols_found = getcols(buff, delim, &columns);
//fprintf(stderr,"cols_found: %d\n",cols_found);
//fprintf(stderr,"columns: %d %s %s\n",atoi(columns[0]),columns[1], columns[2]);
    assign_cols_flatfile(columns, &stLat, &stLon, &evMag, &evLon, &evLat, &evDep, &evYear, &evMon, &evDay, &evHour, &evMin, &evSec, network, stationNm);
    epochTimeFlatFile=compute_epochTime(evYear,evMon,evDay,evHour,evMin,(int)evSec);

// LOOP THROUGH ALL LINES IN MODIFIED FLATFILE
    objMisfit=5;
// header lines
    fgets(buff2,BUFFLEN,fpAddToFlatFile);
    fgets(buff2,BUFFLEN,fpAddToFlatFile);
    while( fgets(buff2,BUFFLEN,fpAddToFlatFile) ) {
//      strip(buff2);
      buff2[strcspn(buff2, "\r\n")] = 0;
      columns2 = NULL;
      cols_found = getcols(buff2, delim, &columns2);
//fprintf(stderr,"from %s",addToFlatFile);
//fprintf(stderr,"%s\n",buff2);
      assign_cols_flatfile(columns2, &stLat2, &stLon2, &evMag2, &evLon2, &evLat2, &evDep2, &evYear2, &evMon2, &evDay2, &evHour2, &evMin2, &evSec2, network2, stationNm2);
      free(columns2);
      delaz_(&stLat,&stLon,&stLat2,&stLon2,&dist,&az,&baz);
      delaz_(&evLat,&evLon,&evLat2,&evLon2,&dist2,&az,&baz);
//fprintf(stderr,"buff2: %s\n", buff2);
      epochTimeFlatFile2=compute_epochTime(evYear2,evMon2,evDay2,evHour2,evMin2,(int)evSec2);
      diffSec=abs(epochTimeFlatFile-epochTimeFlatFile2);
      diffMag=fabsf(evMag-evMag2);
      objMtmp=dist+dist2+(float)diffSec+diffMag*10;
      if ( objMtmp < objMisfit ) {
        objMisfit=objMtmp;
        strcpy(buffFin,buff2);
        fprintf(stderr,"objMisfit/dist/dist2/diffSec/diffMag: %.1f %.1f %.1f %.1f\n", objMisfit, dist, dist2, diffSec, diffMag);
//        fprintf(stderr,"mag: %f %f\n", evMag, evMag2);
//          fprintf(stderr,"cnt1/cnt2: %d %d\n", cnt1, cnt2);
//          fprintf(stderr,"%s\n%s\n", buff, buff2);
      }
      if ( objMisfit < 1 ) break;
//      else {
//        fprintf(stderr,"NO MATCH: objMisfit/dist/dist2/diffSec/diffMag: %.1f %.1f %.1f %.1f %s-%s (%s-%s)\n", objMisfit, dist, dist2, diffSec, diffMag, network2,stationNm2,network,stationNm);
//      }
    }
// rewind to beginning of file for next iteration through original flatfile
    rewind(fpAddToFlatFile);

// WRITE MODIFIED FLATFILE
    if ( objMisfit < 1 ) {
      cntMod++;
//      fprintf(stderr,"%.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.2f %.2f\n", stLat, stLat2, stLon, stLon2, evLat, evLat2, evLon, evLon2,evMag, evMag2);
//      fprintf(stderr,"flatfile: %s\n", buff);
//      fprintf(stderr,"modified flatfile %s\n", buffFin);
      columnsOrig = NULL;
      columnsFinal = NULL;
      cols_found = getcols(buff, delim, &columnsOrig);
      cols_found = getcols(buffFin, delim, &columnsFinal);
      fprintf(fpFlatFileMod,"%s,",columnsOrig[0]);
// replace flatfile entries with reprocessed values
      for(cnt3=1; cnt3<57; cnt3++) {
        fprintf(fpFlatFileMod,"%s,",columnsOrig[cnt3]);
      }
      for(cnt3=57; cnt3<233; cnt3++) {
        fprintf(fpFlatFileMod,"%s,",columnsFinal[cnt3]);
      }
      for(cnt3=233; cnt3<260; cnt3++) {
        fprintf(fpFlatFileMod,"%s,",columnsOrig[cnt3]);
      }
      free(columnsOrig);
      free(columnsFinal);
    }
    else {
      cntOrig++;
      fprintf(fpFlatFileMod,"%s,",buff);
    }
    fprintf(fpFlatFileMod,"\n");
    if ( cnt1%100 == 0 ) {
      fprintf(stderr,"cnt1=%d\n", cnt1); 
    }
//    if ( cnt1 > 10 ) exit(1);
    free(columns);
    cnt1++;
  }
  fprintf(stderr,"Wrote modified flatfile, %s\n%d original lines\n%d modified lines\n", flatFileMod, cntOrig, cntMod);


// close files
  fclose(fpFlatFile);
  fclose(fpAddToFlatFile);
  fclose(fpFlatFileMod);

  return 0;
}
