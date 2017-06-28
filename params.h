#define R 6371
#define BUFFLEN 7000
#define NEVENTS 5000

// structures
struct catDetails {
  int epochTime;
  int locationHypo, locationHD; 
  float mag, lon, lat, dep;
  int year, mon, day, hour, min, doy;
  float sec;
  float lonHD, latHD, depHD, uncertEpiHD, uncertDepthHD;
  float latHypo, lonHypo, depHypo, northingErr, eastingErr, depthErr;
  char freeDep[5];
  char catCM[50];
  char comcatID[50];
//  int cnt;
} ;
