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
//void assign_cols_catalog(char **columns,struct catDetails *cat, int cnt);
void assign_cols_catalog(char **columns, struct catDetails cat[], int cnt);
void assign_cols_flatfile(char **columns, float *stLat, float *stLon, float *evMag, float *evLon, float *evLat, float *evDep, int *evYear, int *evMon, int *evDay, int *evHour, int *evMin, float *evSec);
char * replace(char const * const original, char const * const pattern, char const * const replacement );
int compute_epochTime(int yearIn, int monthIn, int dayIn, int hourIn, int minIn, int secIn);



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
  FILE *fpCat, *fpFlatFileMod, *fpFlatFile, *fpErrorLog, *fpErrorLog2;
  int writeEventOutput=0;
  int cnt, cnt1, cnt2, nf=0, neqs=0, hlines, cols_found;
  int matchCat=0, matchEle;
  int epochTimeFlatFile, epochTimeCatalog;
  int eqv, eqy, eqmo, eqd, eqh, eqm; 
  int cntCat;
  int yearF, monF, dayF;
  float objMisfit, objMtmp;
  float distMisfit, timeMisfit, magMisfit;
  float mag, eqlon, eqlat, eqs, eq_radius; 
  float lon1, lat1, dist, az, baz, mindist;
  float lonF, latF, depF, epiF, hypoDistF, magF;
  float lonHD, latHD, depHD, epiHD, hypoDistHD, uncertEpiHD, uncertDepthHD; 
  float  lonHypoDD, latHypoDD, depHypoDD, epiHypoDD, hypoDistHypoDD, northingErr, eastingErr, depthErr;
  float lonf[100], latf[100];
// events from Flatfile
  float stLon, stLat;
  float evMag, evLon, evLat, evDep, evSec;
  int evYear, evMon, evDay, evHour, evMin;
  int diffSec;
  float diffMag;
//
  char catalogFile[200], flatFile[200], flatFileMod[200];
  char catf[200], faultsf[200];
  char eqnote[10], buff[BUFFLEN];
  char **columns;
  char delim[] = ",";
  int i;
//
  struct catDetails catalogStruct[NEVENTS];
  struct tm timeFlatFile;

/* CHECK INPUT ARGUMENTS */
  if ( argc != 4 ) {
    fprintf(stderr,"USAGE: %s [relocated EQ catalog] [flatfile] [output flatfile]\n", argv[0]);
    fprintf(stderr,"Relocated catalog file, e.g., emm_c2_OK_KS_201702_add_all2.csv\n");
    exit(1);
  }
  sscanf(argv[1],"%s", catalogFile);
  sscanf(argv[2],"%s", flatFile);
  sscanf(argv[3],"%s", flatFileMod);

// open EQ catalog, faults file, and output files
  if ((fpCat = fopen(catalogFile, "r")) == NULL) {
    fprintf(stderr,"Could not open EQ catalog, %s\n", catalogFile);
    exit(0);
  }
  if ((fpFlatFile = fopen(flatFile, "r")) == NULL) {
    fprintf(stderr,"Could not open flatfile, %s\n", flatFile);
    exit(0);
  }
  fpFlatFileMod = fopen(flatFileMod, "w");
  fpErrorLog = fopen("event_relocErrors.txt", "w");
  fprintf(fpErrorLog,"d-Mag d-Time(s) d-Dist(km) sum-errors\n");
  fpErrorLog2 = fopen("event_relocErrors_junk.txt", "w");
  fprintf(fpErrorLog2,"d-Mag d-Time(s) d-Dist(km) sum-errors\n");

// READ CATALOG to structure
// first, remove header lines from catalog
  hlines=1;
  for (cnt1=0; cnt1<hlines; cnt1++) {
    fgets(buff,BUFFLEN,fpCat);
  }
  cnt=0;
  while( fgets(buff,BUFFLEN,fpCat) ) {
    if ( strlen(buff) > BUFFLEN ) {
      fprintf(stderr,"Increase BUFFLEN from %d.\n", (int)BUFFLEN);
      exit(1);
    }
    columns = NULL;
    cols_found = getcols(buff, delim, &columns);
    if ( cols_found ) {
//      for ( i = 0; i < cols_found; i++ ) printf("Column[ %d ] = %s\n", i, columns[ i ] ); 
      assign_cols_catalog(columns,catalogStruct,cnt);
//      fprintf(stderr,"%d\n", cols_found);
      free(columns);
      cnt++;
    }
  }
  cntCat=cnt;
  fprintf(stderr,"Read %d events from catalog, %s\n\n", cntCat, catalogFile);

// READ/APPEND FLATFILE
// header lines
  hlines=2;
  for (cnt1=0; cnt1<hlines; cnt1++) {
    fgets(buff,BUFFLEN,fpFlatFile);
    strip(buff);
    fprintf(fpFlatFileMod,"%s",buff);
    if (cnt1==1) {
      strip(buff);
      fprintf(fpFlatFileMod,",Event Longitude haz,Event Latitude haz,Event Depth haz (km),R_{epi} haz (km),R{hypo} haz (km),Event Longitude HD,Event Latitude HD,Event Depth HD (km),Uncert. epi HD,Uncert. Depth HD (km),R_{epi} HD (km),R_{hypo} HD (km),Event Longitude Reloc.,Event Latitude Reloc.,Event Depth Reloc. (km),Uncert. northing Reloc.,Uncert. easting Reloc.,Uncert. Depth Reloc. (km),R_{epi} Reloc. (km),R_{hypo} Reloc. (km),");
    }
    fprintf(fpFlatFileMod,"\n");
//    fprintf(stderr,"%s", buff);
  }
  cnt1=0;
  cnt2=0;
// loop over data values
  while( fgets(buff,BUFFLEN,fpFlatFile) ) {
//    fprintf(stderr,"%s", buff);
//if ( cnt1 > 2) exit(1);
    if ( strlen(buff) > BUFFLEN ) {
      fprintf(stderr,"Increase BUFFLEN from %d.\n", (int)BUFFLEN);
      exit(1);
    }
    strip(buff);
    fprintf(fpFlatFileMod,"%s,",buff);
    columns = NULL;
    cols_found = getcols(buff, delim, &columns);
    if ( cols_found ) {
      assign_cols_flatfile(columns, &stLat, &stLon, &evMag, &evLon, &evLat, &evDep, &evYear, &evMon, &evDay, &evHour, &evMin, &evSec);
//      for ( i = 0; i < cols_found; i++ ) printf("Column[ %d ] = %s\n", i, columns[ i ] ); 
      free(columns);
      epochTimeFlatFile=compute_epochTime(evYear,evMon,evDay,evHour,evMin,(int)evSec);
      objMisfit=999;
      for(cnt=0; cnt<cntCat;cnt++) {
        lon1=catalogStruct[cnt].lon;
        lat1=catalogStruct[cnt].lat;
        epochTimeCatalog=catalogStruct[cnt].epochTime;
        delaz_(&evLat,&evLon,&lat1,&lon1,&dist,&az,&baz);
//        diffSec=fabs(difftime(mktime(&timeFlatFile), catalogStruct[cnt].tmTime));
        diffSec=abs(epochTimeFlatFile-epochTimeCatalog);
        diffMag=fabsf(evMag-catalogStruct[cnt].mag);
        objMtmp=dist+(float)diffSec+diffMag;
        if ( objMtmp < objMisfit ) {
          objMisfit=objMtmp;
          matchEle=cnt;
          distMisfit=dist;
          timeMisfit=(float)diffSec;
          magMisfit=diffMag;
        }
      }
// assign values from min-misfit event
      yearF=catalogStruct[matchEle].year;
      monF=catalogStruct[matchEle].mon;
      dayF=catalogStruct[matchEle].day;
      lonF=catalogStruct[matchEle].lon;
      latF=catalogStruct[matchEle].lat;
      magF=catalogStruct[matchEle].mag;
      depF=catalogStruct[matchEle].dep;
      delaz_(&stLat,&stLon,&latF,&lonF,&epiF,&az,&baz);
      hypoDistF=sqrt(epiF*epiF+depF*depF);
      strip(buff);
      fprintf(fpFlatFileMod,"%.2f,%.3f,%.3f,%.1f,%.2f,%.2f,",magF,lonF,latF,depF,epiF,hypoDistF);
//      fprintf(stderr,"%.1f %.1f %.1f %.1f %.1f\n", stLon, stLat, lonF, latF, epiF);
// HypoDD locations
      if ( catalogStruct[matchEle].locationHypo ) {
        lonHypoDD=catalogStruct[matchEle].lon;
        latHypoDD=catalogStruct[matchEle].lat;
        depHypoDD=catalogStruct[matchEle].dep;
        northingErr=catalogStruct[matchEle].northingErr;
        eastingErr=catalogStruct[matchEle].eastingErr;
        depthErr=catalogStruct[matchEle].depthErr;
        delaz_(&stLat,&stLon,&latHypoDD,&lonHypoDD,&epiHypoDD,&az,&baz);
        hypoDistHypoDD=sqrt(epiHypoDD*epiHypoDD+depHypoDD*depHypoDD);
        fprintf(fpFlatFileMod,"%.3f,%.3f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,", lonHypoDD,latHypoDD,depHypoDD,northingErr,eastingErr,depthErr,epiHypoDD,hypoDistHypoDD);
//Event Longitude Reloc.,Event Latitude Reloc.,Event Depth Reloc. (km),Uncert. northing Reloc.,Uncert. easting Reloc.,Uncert. Depth Reloc. (km),R_{epi} Reloc. (km),R_{hypo} Reloc. (km)\n",buff);
      }
      else {
        fprintf(fpFlatFileMod,",,,,,,,,");
      }
// HD locations
      if ( catalogStruct[matchEle].locationHD ) {
        lonHD=catalogStruct[matchEle].lon;
        latHD=catalogStruct[matchEle].lat;
        depHD=catalogStruct[matchEle].dep;
        uncertEpiHD=catalogStruct[matchEle].uncertEpiHD;
        uncertDepthHD=catalogStruct[matchEle].uncertDepthHD;
        delaz_(&stLat,&stLon,&latHD,&lonHD,&epiHD,&az,&baz);
        hypoDistHD=sqrt(epiHD*epiHD+depHD*depHD);
        fprintf(fpFlatFileMod,"%.3f,%.3f,%.1f,%.1f,%.1f,%.1f,%.1f,\n", lonHD,latHD,depHD,uncertEpiHD,uncertDepthHD,epiHD,hypoDistHD);
      }
      else {
        fprintf(fpFlatFileMod,",,,,,,,\n", lonHD,latHD,depHD,uncertEpiHD,uncertDepthHD,epiHD,hypoDistHD);
      }


// WRITE ALL OUTPUT
      if ( writeEventOutput ) {
        fprintf(stderr,"Flatfile: %.2f %.2f %.2f %d %d %d %d %d %.1f\n", evMag, evLon, evLat, evYear, evMon, evDay, evHour, evMin, evSec); 
        fprintf(stderr,"Catalog: %.2f %.2f %.2f %d %d %d %d %d %.1f\n", catalogStruct[matchEle].mag, catalogStruct[matchEle].lon, catalogStruct[matchEle].lat, catalogStruct[matchEle].year, catalogStruct[matchEle].mon, catalogStruct[matchEle].day, catalogStruct[matchEle].hour, catalogStruct[matchEle].min, catalogStruct[matchEle].sec);
        fprintf(stderr,"dMag/dTime/dDist: %.2f %d %.1f %.1f\n\n", magMisfit, (int)timeMisfit, distMisfit, objMisfit);
      }
// WRITE ERROR LOGS
      if ( (magMisfit<0.2) && (timeMisfit<10) && (distMisfit<10) ) {
        fprintf(fpErrorLog,"%.2f %d %.1f %.1f %.1f %d\n", magMisfit, (int)timeMisfit, distMisfit, objMisfit, matchEle);
        cnt2++;
      }
      else {
        fprintf(fpErrorLog2,"%.2f %d %.1f %.1f %.1f\n", magMisfit, (int)timeMisfit, distMisfit, objMisfit, hypoDistF);
        if ( epiF <= 50 ) {
          fprintf(stderr,"%.1f %.1f %.1f %.1f %.1f %.2f %d %.1f M%.1f %d %d %d\n", stLon, stLat, lonF, latF, epiF, magMisfit, (int)timeMisfit, distMisfit, evMag, yearF, monF, dayF);
          fprintf(stderr,"Flatfile: %.2f %.2f %.2f %d %d %d %d %d %.1f\n", evMag, evLon, evLat, evYear, evMon, evDay, evHour, evMin, evSec); 
          fprintf(stderr,"Catalog: %.2f %.2f %.2f %.1f %d %d %d %d %d %.1f\n", catalogStruct[matchEle].mag, catalogStruct[matchEle].lon, catalogStruct[matchEle].lat, catalogStruct[matchEle].dep, catalogStruct[matchEle].year, catalogStruct[matchEle].mon, catalogStruct[matchEle].day, catalogStruct[matchEle].hour, catalogStruct[matchEle].min, catalogStruct[matchEle].sec);
          fprintf(stderr,"dMag/dTime/dDist: %.2f %d %.1f %.1f\n\n", magMisfit, (int)timeMisfit, distMisfit, objMisfit);
        }
      }
    }
    cnt1++;
//if (cnt1>5) exit(1);
  }
  fprintf(stderr,"Read %d events from flatfile, %s.\n", cnt1, flatFile); 
  fprintf(stderr,"%d/%d events associated with catalog events.\n", cnt2, cnt1); 

// close files
  fclose(fpCat);
  fclose(fpFlatFile);
  fclose(fpFlatFileMod);
  fclose(fpErrorLog);
  fclose(fpErrorLog2);

  return 0;
}


/*--------------------------------------------------------------------------*/
void assign_cols_flatfile(char **columns, float *stLat, float *stLon, float *evMag, float *evLon, float *evLat, float *evDep, int *evYear, int *evMon, int *evDay, int *evHour, int *evMin, float *evSec)
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

}

/*--------------------------------------------------------------------------*/
void assign_cols_catalog(char **columns, struct catDetails cat[], int cnt)
/*--------------------------------------------------------------------------*/
{
  int doy;
  struct tm timeCat;

// column values to structure
  cat[cnt].mag=atof(columns[0]);
  cat[cnt].lon=atof(columns[1]);
  cat[cnt].lat=atof(columns[2]);
  cat[cnt].dep=atof(columns[3]);
  cat[cnt].year=atoi(columns[4]);
  cat[cnt].mon=atoi(columns[5]);
  cat[cnt].day=atoi(columns[6]);
  cat[cnt].hour=atoi(columns[7]);
  cat[cnt].min=atoi(columns[8]);
  cat[cnt].sec=atof(columns[9]);
  doy=compute_doy(cat[cnt].year,cat[cnt].mon,cat[cnt].day);
// time struct and epoch time
  timeCat.tm_year=cat[cnt].year;
  timeCat.tm_mon=cat[cnt].mon;
  timeCat.tm_mday=cat[cnt].day;
  timeCat.tm_hour=cat[cnt].hour;
  timeCat.tm_min=cat[cnt].min;
  timeCat.tm_sec=(int)cat[cnt].sec;
  timeCat.tm_isdst=0;
  cat[cnt].epochTime=compute_epochTime(cat[cnt].year,cat[cnt].mon,cat[cnt].day,cat[cnt].hour,cat[cnt].min,timeCat.tm_sec);

  char * const newstr = replace(columns[13],"\"","");
  char * const newstr2 = replace(columns[14],"\"","");
  sprintf(cat[cnt].catCM,"%s,%s",newstr,newstr2);
  free(newstr);
  free(newstr2);
  sprintf(cat[cnt].comcatID,"%s", columns[16]);
//  fprintf(stderr,"strlen %d\n", strlen(columns[17]));
  if ( strlen(columns[17]) > 0 ) {
    cat[cnt].lonHD=atof(columns[17]);
    cat[cnt].latHD=atof(columns[18]);
    cat[cnt].depHD=atof(columns[19]);
    cat[cnt].uncertEpiHD=atof(columns[21]);
    cat[cnt].uncertDepthHD=atof(columns[22]);
    sprintf(cat[cnt].freeDep,"%s",columns[20]);
    cat[cnt].locationHD=1;
  }
  else {
    cat[cnt].lonHD=-999;
    cat[cnt].latHD=-999;
    cat[cnt].depHD=-999;
    cat[cnt].uncertEpiHD=-999;
    cat[cnt].uncertDepthHD=-999;
    sprintf(cat[cnt].freeDep,"");
    cat[cnt].locationHD=0;
  }
  if ( strlen(columns[23]) > 0 ) {
    cat[cnt].lonHypo=atof(columns[23]);
    cat[cnt].latHypo=atof(columns[24]);
    cat[cnt].depHypo=atof(columns[25]);
    cat[cnt].northingErr=atof(columns[26]);
    cat[cnt].eastingErr=atof(columns[27]);
    cat[cnt].depthErr=atof(columns[28]);
    cat[cnt].locationHypo=1;
  }
  else {
    cat[cnt].lonHypo=-999;
    cat[cnt].latHypo=-999;
    cat[cnt].depHypo=-999;
    cat[cnt].northingErr=-999;
    cat[cnt].eastingErr=-999;
    cat[cnt].depthErr=-999;
    cat[cnt].locationHypo=0;
  }
  
//fprintf(stderr,"%f\n", cat[cnt].mag);
/*
fprintf(stderr,"%f\n", cat[cnt].lon);
fprintf(stderr,"%f\n", cat[cnt].lat);
fprintf(stderr,"%f\n", cat[cnt].dep);
fprintf(stderr,"%d\n", cat[cnt].year);
fprintf(stderr,"%d\n", cat[cnt].mon);
fprintf(stderr,"%d\n", cat[cnt].day);
fprintf(stderr,"%d\n", cat[cnt].hour);
fprintf(stderr,"%d\n", cat[cnt].min);
fprintf(stderr,"%f\n", cat[cnt].sec);
fprintf(stderr,"%s\n", cat[cnt].catCM);
fprintf(stderr,"%s\n", cat[cnt].comcatID);
fprintf(stderr,"lonHD: %f\n", cat[cnt].lonHD);
fprintf(stderr,"lonHypo: %f\n", cat[cnt].lonHypo);
fprintf(stderr,"completed\n");
*/


}


/*--------------------------------------------------------------------------*/
char * replace(
    char const * const original, 
    char const * const pattern, 
    char const * const replacement )
/*--------------------------------------------------------------------------*/
{
  size_t const replen = strlen(replacement);
  size_t const patlen = strlen(pattern);
  size_t const orilen = strlen(original);

  size_t patcnt = 0;
  const char * oriptr;
  const char * patloc;

  // find how many times the pattern occurs in the original string
  for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
  {
    patcnt++;
  }

  {
    // allocate memory for the new string
    size_t const retlen = orilen + patcnt * (replen - patlen);
    char * const returned = (char *) malloc( sizeof(char) * (retlen + 1) );

    if (returned != NULL)
    {
      // copy the original string, 
      // replacing all the instances of the pattern
      char * retptr = returned;
      for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
      {
        size_t const skplen = patloc - oriptr;
        // copy the section until the occurence of the pattern
        strncpy(retptr, oriptr, skplen);
        retptr += skplen;
        // copy the replacement 
        strncpy(retptr, replacement, replen);
        retptr += replen;
      }
      // copy the rest of the string.
      strcpy(retptr, oriptr);
    }
    return returned;
  }
}

/*
 float mag, lon, lat, dep;
  int year, mon, day, hour, min;
  float sec;
  float lonHD, latHD, depHD, uncertEpiHD, uncertDepthHD;
  float latHypo, lonHypo, depHypo, northingErr, eastingErr, depthErr;
  char *comcatID;
  int cnt;

*/
