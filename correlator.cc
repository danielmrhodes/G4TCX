#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <vector>

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TRandom.h"
#include "TVector3.h"
#include "TMath.h"

//#include "/path/to/G4TCX/include/Data_Format.hh"
#include "/opt/G4TCX/include/Data_Format.hh"

////These should match the parameters defined in the simulation input////
//These correspond to the input file Examples/Macros/full.mac
//Masses in MeV/c^2

const int beamZ = 48;
const double beam_mass = 98626.9;

//You should reduce this value by the energy loss in the target
//MeV
const double beam_en = 265;
  
const int targZ = 22;
const double targ_mass = 44652.0;

//Silicon detector z-offsets (downstream and upstream) (cm)
double DS_Offset = 3.0;
double US_Offset = 3.0;

//Beam spot position (cm)
const double beam_X = 0.0;
const double beam_Y = 0.0;

//Tigress configuration. 0 for high-efficiency (detectors forward), 1 for figh Peak-to-Total (detectors back)
const int tigConfig = 1;

//Tigress Z-offset (cm)
double Tigress_Offset = 0.0; 
/////////////////////////////////////////////////////////////////////////

//Gate on S3 rings (inclusive)
const int Ri = 1;
const int Rf = 24;

////Tigress Resolution////
double Sigma(double en) {
  return 0.7088 + en*0.00034535; //S1893
}
///////////////////////

////Kinematics////
double Theta_CM_FP(double ThetaLAB, double Ep, bool sol2=false, double Ex=0.) { //From projectile

  double tau = (beam_mass/targ_mass)/std::sqrt(1 - (Ex/Ep)*(1 + beam_mass/targ_mass));
  
  if(std::sin(ThetaLAB) > 1.0/tau) {

    ThetaLAB = std::asin(1.0/tau);
    if(ThetaLAB < 0)
      ThetaLAB += TMath::Pi();

    return std::asin(tau*std::sin(ThetaLAB)) + ThetaLAB;
  }

  if(!sol2)
    return std::asin(tau*std::sin(ThetaLAB)) + ThetaLAB;
 
  return std::asin(tau*std::sin(-ThetaLAB)) + ThetaLAB + TMath::Pi();

}

double Theta_CM_FR(double ThetaLAB, double Ep, bool sol2=false, double Ex=0.) { //From recoil

  double tau = 1.0/std::sqrt(1 - (Ex/Ep)*(1 + beam_mass/targ_mass));
  
  if(std::sin(ThetaLAB) > 1.0/tau) {

    ThetaLAB = std::asin(1.0/tau);
    if(ThetaLAB < 0)
      ThetaLAB += TMath::Pi();

    return std::asin(tau*std::sin(ThetaLAB)) + ThetaLAB;
  }

  if(!sol2)
    return TMath::Pi() - (std::asin(tau*std::sin(ThetaLAB)) + ThetaLAB);
  
  return -std::asin(tau*std::sin(-ThetaLAB)) - ThetaLAB;

}

double Theta_LAB_Max(double Ep, double Ex=0.0) {

  double tau = (beam_mass/targ_mass)/std::sqrt(1 - (Ex/Ep)*(1 + beam_mass/targ_mass));
  if(tau < 1.0)
    return TMath::Pi();
  
  return std::asin(1.0/tau);
  
}

double Theta_LAB(double thetaCM, double Ep, double Ex=0.) {

  double tau = (beam_mass/targ_mass)/std::sqrt(1 - (Ex/Ep)*(1 + beam_mass/targ_mass));
  double tanTheta = std::sin(thetaCM)/(std::cos(thetaCM) + tau);

  if(tanTheta > 0)
    return std::atan(tanTheta);
  
  return std::atan(tanTheta) + TMath::Pi();
  
}

double Recoil_Theta_LAB(double thetaCM, double Ep, double Ex=0.) {

  double tau = 1.0/std::sqrt(1 - (Ex/Ep)*(1 + beam_mass/targ_mass));
  double tanTheta = std::sin(TMath::Pi() - thetaCM)/(std::cos(TMath::Pi() - thetaCM) + tau);
  
  return std::atan(tanTheta);
  
}

double KE_LAB(double thetaCM, double Ep, double Ex=0.) {

  double tau = (beam_mass/targ_mass)/std::sqrt(1 - (Ex/Ep)*(1 + beam_mass/targ_mass));

  double term1 = std::pow(targ_mass/(beam_mass + targ_mass),2);
  double term2 = 1 + tau*tau + 2*tau*std::cos(thetaCM);
  double term3 = Ep - Ex*(1 + beam_mass/targ_mass);
  
  return term1*term2*term3;
}

double Recoil_KE_LAB(double thetaCM, double Ep, double Ex=0.) {

  double tau = 1.0/std::sqrt(1 - (Ex/Ep)*(1 + beam_mass/targ_mass));

  double term1 = beam_mass*targ_mass/std::pow(beam_mass + targ_mass,2);
  double term2 = 1 + tau*tau + 2*tau*std::cos(TMath::Pi() - thetaCM);
  double term3 = Ep - Ex*(1 + beam_mass/targ_mass);
  
  return term1*term2*term3;
}
//////////////////

////Positions////
//S3 segment position
TVector3 GetPos(const int det, const int ring, const int sec) {

   if(det < 0 || det > 1 || ring < 1 || ring > 24 || sec < 1 || sec > 32) {
     std::cout << "Bad det, ring, sec (" << det << "," << ring << "," << sec << ")"<< std::endl;
     return TVector3(std::sqrt(-1),std::sqrt(-1),std::sqrt(-1));
  }

  const double PI = TMath::Pi();
  
  double phi_offset = 0.5*PI; // Phi of sector 1 of downstream detector 
  bool clockwise; // Winding direction of sectors.
  if(det==0)
    clockwise = false;
  else
    clockwise = true;

  double s3_outer_radius = 3.5;
  double s3_inner_radius = 1.1;

  TVector3 pos(1.,0,0);
  double rad_slope = (s3_outer_radius - s3_inner_radius)/24.;
  double rad_offset = s3_inner_radius;
   
  pos.SetPerp((ring - 0.5)*rad_slope + rad_offset);
  
  double phi = phi_offset + (clockwise ? -1 : 1) * 2.*PI/32. * (sec - 1);
  pos.SetPhi(phi);

  double zoff;
  if(det == 1)
    zoff = DS_Offset;
  else
    zoff = US_Offset;
  
  pos.SetZ(zoff);
  if(det == 0)
    pos.RotateY(PI);

  return pos;
  
}

//Tigress positions
//forward (0) or back (1), det, seg
TVector3 tigPosVec[2][64][9];
void FillPositonVectors() {

  std::ifstream file;
  file.open("/opt/G4TCX/positions.txt",std::ios::in);

  std::string line;
  double x,y,z;
  
  for(int i=0;i<64;i++) {
    for(int j=0;j<9;j++) {

      std::getline(file,line);
      std::stringstream ss(line);
      ss >> x >> y >> z;
	
      TVector3 vec(x/10.,y/10.,z/10.);
      tigPosVec[0][i][j] = vec;

    }
  }

  std::ifstream file1;
  file1.open("/opt/G4TCX/positionsBack.txt",std::ios::in);
  
  for(int i=0;i<64;i++) {
    for(int j=0;j<9;j++) {

      std::getline(file1,line);
      std::stringstream ss(line);
      ss >> x >> y >> z;
	
      TVector3 vec(x/10.,y/10.,z/10.);
      tigPosVec[1][i][j] = vec;

    }
  }

  return;
  
}

//Tigress positions
TVector3 GetPos(const int det, const int seg) {

  if(det < 1 || det > 64 || seg < 0 || seg > 8)
    return TVector3(0,0,0);
  
  return tigPosVec[tigConfig][det-1][seg];

}
/////////////////

////////////Build data format////////////
struct S3 {

  //realistic info
  int det, ring, sector;
  double rEn, sEn;

  //perfect info
  TVector3 rPos, sPos;
  bool rP, rR, sP, sR;
  
};

struct TIGRESS {

  //realistic info
  int det, nsegs, segs[8];
  double cEn, sEn[8];
  bool sup;
  
  //perfect info
  double x[8], y[8], z[8];
  bool fep, pfep;

  int MainSeg() {
    
    int index=0;
    for(int i=1;i<nsegs;i++) {
      if(sEn[i] > sEn[index]) {
	index=i;
      }
    }
    
    return segs[index];
  }

  int LastSeg() {
    
    int index=0;
    for(int i=1;i<nsegs;i++) {
      if(sEn[i] < sEn[index]) {
	index=i;
      }
    }
    
    return segs[index];
  }

  TVector3 MainPos() {
    return GetPos(det,MainSeg());
  }

  TVector3 LastPos() {
    return GetPos(det,LastSeg());
  }

  void AddHit(const TIGRESS& hit) {

    cEn += hit.cEn;
    if(!sup)
      sup = hit.sup;
    
    return;
  }

  bool operator>(const TIGRESS& hit) const {
    return cEn > hit.cEn;
  }
  
};

bool AddbackCriterion(TIGRESS& hit1,TIGRESS& hit2) {

  double res = (hit1.LastPos() - hit2.MainPos()).Mag();

  // In clover core separation 54.2564, 76.7367
  // Between clovers core separation 74.2400 91.9550 (high-eff mode)
  double seperation_limit = 93.0/10.0;

  int one_seg = hit1.LastSeg();
  int two_seg = hit2.MainSeg();

  // front segment to front segment OR back segment to back segment
  if((one_seg < 5 && two_seg < 5) || (one_seg > 4 && two_seg > 4))
    seperation_limit = 54.0/10.0;

  // front to back
  else if((one_seg < 5 && two_seg > 4) || (one_seg > 4 && two_seg < 5))
    seperation_limit = 105.0/10.0;
  
  if(res < seperation_limit)
    return true;

  return false;
}

struct BuiltData {

  int evt;
  int nS3;
  int nTi;
  
  S3 s3[5];
  std::vector<TIGRESS> tigress;
  std::vector<TIGRESS> tigressAB;

  void MakeS3Hit(const S3Data ringHit, const S3Data secHit) {

    s3[nS3].det = ringHit.det; //either hit works
    s3[nS3].ring = ringHit.ring;
    s3[nS3].sector = secHit.sector;
    
    s3[nS3].rEn = ringHit.en;
    s3[nS3].sEn = secHit.en;

    s3[nS3].rPos = TVector3(ringHit.x,ringHit.y,ringHit.z);
    s3[nS3].sPos = TVector3(secHit.x,secHit.y,secHit.z);
    
    s3[nS3].rP = ringHit.proj;
    s3[nS3].rR = ringHit.rec;

    s3[nS3].sP = secHit.proj;
    s3[nS3].sR = secHit.rec;

    nS3++;
  }

  void BuildAddbackHits() {
    
    if(nTi == 0)
      return;

    tigressAB.push_back(tigress[0]);
    if(nTi == 1)
      return;
      
    int i;
    unsigned int j;
    for(i=1;i<nTi;i++) {
      
      for(j=0;j<tigressAB.size();j++) {
	if(AddbackCriterion(tigressAB[j],tigress[i])) {

	  tigressAB[j].AddHit(tigress[i]);
	  break;
	}
      }
      
      // if hit[i] was not added to a higher energy hit, create its own addback hit
      if(j == tigressAB.size())
	tigressAB.push_back(tigress[i]);
      
    }

    return;
  }
  
};
/////////////////////////////////////////

//Unpack raw data into correlated data
BuiltData BuildData(const int nE, const int nSch, const int nTch, const RawData& raw_dat) {
  
  BuiltData data;
  data.evt = nE;
  data.nS3 = 0;
  data.nTi = 0;

  //Correlate S3 Data
  std::vector<S3Data> rings;
  std::vector<S3Data> sectors;
  for(int i=0;i<nSch;i++) {

    S3Data sChan(raw_dat.sData[i]);
    
    if(sChan.IsRing())
      rings.push_back(sChan);
    else
      sectors.push_back(sChan);
    
  }

  std::sort(rings.begin(),rings.end(),std::greater<S3Data>());
  std::sort(sectors.begin(),sectors.end(),std::greater<S3Data>());

  std::vector<bool> used_rings;
  std::vector<bool> used_sectors;
  
  used_rings.resize(rings.size());
  std::fill(used_rings.begin(),used_rings.end(),false);

  used_sectors.resize(sectors.size());
  std::fill(used_sectors.begin(),used_sectors.end(),false);

  for(unsigned int i=0;i<sectors.size();i++) {
    if(used_sectors.at(i))
      continue;
    
    for(unsigned int j=0;j<rings.size();j++) {
      if(used_rings.at(j))
        continue;

      //Same detector
      if((sectors.at(i).det == rings.at(j).det) &&

	 //Same energy
	 (TMath::Abs(sectors.at(i).en - rings.at(j).en) < 1.)) {

	data.MakeS3Hit(rings.at(j),sectors.at(i));

	used_sectors.at(i) = true;
	used_rings.at(j) = true;
	break;

      }
    }
  }

  bool broken = false;
  for(unsigned int i=0;i<sectors.size();i++) {
    broken=false;
    if(used_sectors.at(i))
      continue;
    
    for(unsigned int j=0;j<rings.size();j++) {
      if(used_rings.at(j))
        continue;

      for(unsigned int k=0;k<sectors.size();k++) {
	if(used_sectors.at(k))
	  continue;

	//Same detector
	if((sectors.at(i).det == rings.at(j).det) && (sectors.at(k).det == rings.at(j).det) &&

	   //Sector energies add to ring energy
	   (TMath::Abs(sectors.at(i).en + sectors.at(k).en - rings.at(j).en) < 1.)) {

	  data.MakeS3Hit(rings.at(j),sectors.at(i));
	  data.MakeS3Hit(rings.at(j),sectors.at(k));
	  
	  used_sectors.at(i) = true;
	  used_rings.at(j) = true;
	  used_sectors.at(k) = true;
	  broken = true;
	  break;

	}
      }
      if(broken)
	break;
    }
  }

  //Organize Tigress data
  std::vector<bool> exists;
  exists.resize(64);
  std::fill(exists.begin(),exists.end(),false);
  
  int nT = 0;
  for(int i=0;i<nTch;i++) {

    int detect = raw_dat.tData[i].det;
    int segment = raw_dat.tData[i].seg;
    
    double energy = raw_dat.tData[i].en;
    double x = raw_dat.tData[i].x;
    double y = raw_dat.tData[i].y;
    double z = raw_dat.tData[i].z;
    
    bool FEP = raw_dat.tData[i].fep;
    bool PFEP = raw_dat.tData[i].pfep;
    bool SUP = raw_dat.tData[i].sup;
    
    if(!exists.at(detect-1)) {
      data.tigress.emplace_back();
      data.tigress[nT].det = detect;
      data.tigress[nT].sup = SUP;
      
      if((bool)segment) {
        data.tigress[nT].nsegs = 1;
	data.tigress[nT].segs[0] = segment;
	data.tigress[nT].sEn[0] = energy;

	data.tigress[nT].x[0] = x;
        data.tigress[nT].y[0] = y;
        data.tigress[nT].z[0] = z;
      }
      else {
	data.tigress[nT].nsegs = 0;
	data.tigress[nT].cEn = energy;
	data.tigress[nT].fep = FEP;
	data.tigress[nT].pfep = PFEP;
      }

      nT++;
      exists.at(detect-1) = true;
    }
    else {

      int index = 0;
      for(int j=0;j<nT;j++) {
	if(data.tigress[j].det == detect) {
	  index = j;
	  break;
	}
      }
      data.tigress[index].sup = SUP;
      
      int Nsegs = data.tigress[index].nsegs;
      
      if((bool)segment) {
	data.tigress[index].segs[Nsegs] = segment;
	data.tigress[index].sEn[Nsegs] = energy;

	data.tigress[index].x[Nsegs] = x;
        data.tigress[index].y[Nsegs] = y;
        data.tigress[index].z[Nsegs] = z;
	
	data.tigress[index].nsegs++;
      }
      else {
	data.tigress[index].cEn = energy;
	data.tigress[index].fep = FEP;
	data.tigress[index].pfep = PFEP;
      }
      
    }
    
  }
  data.nTi = nT;
  
  std::sort(data.tigress.begin(),data.tigress.end(),std::greater<TIGRESS>());
  data.BuildAddbackHits();
  
  return data;
}
int main(int argc, char** argv) {
  
  if(argc < 3) {
    std::cerr << "Usage: correlator INPUT_FILE OUTPUT_FILE" << std::endl;
    return 1;
  }

  const char* input_filename = argv[1];
  const char* output_filename = argv[2];
  if(!strcmp(input_filename,output_filename)) {
    std::cout << "Give your input and output files different names" << std::endl;
    return 1;
  }

  FILE* input_file = fopen(input_filename,"rb");
  if(input_file == NULL) {
    std::cout << "Could not open file " << input_filename << std::endl;
    return 1;
  }
  
  //S3 singles
  TH2* bSum = new TH2D("Summary","S3 Summary",120,1,121,500,0,500);
  TH2* pSum = new TH2D("pSummary","S3 Projectile Summary",120,1,121,500,0,500);
  TH2* rSum = new TH2D("rSummary","S3 Recoil Summary",120,1,121,500,0,500);

  TH2* pSvRDS = new TH2D("pSvRDS","Downstream projectile Sector vs Ring",24,1,25,32,1,33);
  TH2* pThvPhDS = new TH2D("pThvPhDS","Projectile #phi-#theta surface",1000,0,90,1000,-200,200);
  TH2* pThvPhDS1 = new TH2D("pThvPhDS1","Projectile #phi-#theta surface 1",1000,0,90,1000,-200,200);
  TH2* pThvPhDS2 = new TH2D("pThvPhDS2","Projectile #phi-#theta surface 2",1000,0,90,1000,-200,200);

  TH2* rThvPh = new TH2D("rThvPh","Recoil #phi-#theta surface",1000,0,90,1000,-200,200);
  
  TH1* secD0 = new TH1D("Sectors_Det0","Upstream Sectors",32,1,33);
  TH1* secD1 = new TH1D("Sectors_Det1","Downstream Sectors",32,1,33);

  TH1* pSecDS = new TH1D("pSecDS","DS Projectile Sectors",32,1,33);
  TH1* rSec = new TH1D("rSec","Recoil Sectors",32,1,33);
  
  TH2* rPid0 = new TH2D("RingPID_Det0","Upstream Ring Energy PID",24,1,25,500,0,500);
  TH2* rPid1 = new TH2D("RingPID_Det1","Downstream Ring Energy PID",24,1,25,500,0,500);

  TH2* sPid0 = new TH2D("SecPID_Det0","Upstream Sector Energy PID",24,1,25,500,0,500);
  TH2* sPid1 = new TH2D("SecPID_Det1","Downstream Sector Energy PID",24,1,25,500,0,500);

  TH2* sPid1_p = new TH2D("SecPID_Det1_proj","Downstream Sector Energy Projectile PID",24,1,25,500,0,500);
  TH2* sPid1_r = new TH2D("SecPID_Det1_rec","Downsream Sector Energy Recoil PID",24,1,25,500,0,500);
  
  //Tigress singles
  TH1* coreEnergy = new TH1D("Core_Energy","Tigress Core Energy",12000,0,4000);
  TH2* coreSum = new TH2D("Core_Summary","Core Energy Summary",64,1,65,6000,0,4000);

  TH1D* addEnergy = new TH1D("Add_Energy","Tigress Addback Energy",12000,0,4000);
  TH2D* addSum = new TH2D("Add_Summary","Addback Energy Summary",64,1,65,6000,0,3000);

  TH1D* supEnergy = new TH1D("Sup_Energy","Tigress Suppressed Energy",12000,0,4000);
  TH2D* supSum = new TH2D("Sup_Summary","Suppressed Energy Summary",64,1,65,6000,0,3000);

  TH1D* addSupEn = new TH1D("AddSup_En","Tigress Suppressed Addback Energy",12000,0,4000);
  TH2D* addSupSum = new TH2D("AddSup_Sum","Suppressed Addback Energy Summary",64,1,65,6000,0,3000);
  
  TH1* segEnergy = new TH1D("Seg_Energy","Tigress Segment Energy",12000,0,4000);
  TH2* segSum = new TH2D("Seg_Summary","Segment Energy Summary",512,1,513,3000,0,3000);

  TH1* coreEn_Fep = new TH1D("FEP","Tigress FEP",12000,0,4000);
  TH1* coreEn_NotFep = new TH1D("nFEP","Tigress Not FEP",12000,0,4000);

  TH2* sPosTP = new TH2D("sPosPT","Tigress Phi-Theta Surface",360,0,180,760,-10,370);

  //Coincidences
  //Projectile DS
  TH2* sPidDS = new TH2D("SecPID_DS","DS SectorEn PID",24,1,25,500,0,500);
  TH2* rPidDS = new TH2D("RingPID_DS","DS RingEn PID",24,1,25,500,0,500);
  
  TH1* pCoreEnergyDS = new TH1D("Core_EnergyDS","Tigress Core Energy",12000,0,4000);
  TH2* pCoreSumDS = new TH2D("Core_SummaryDS","Core Energy Summary",16,1,17,3000,0,3000);

  TH1* pDopEnergyDS = new TH1D("Dop_EnergyDS","Doppler Energy",12000,0,4000);
  TH2* pDopSumDS = new TH2D("Dop_SummaryDS","Doppler Energy Summary",16,1,17,6000,0,3000);

  TH1D* pAddDS = new TH1D("Add_EnergyDS","Addback Energy",12000,0,4000);
  TH2D* pAddSumDS = new TH2D("Add_SummaryDS","Addback Energy Summary",64,1,65,6000,0,3000);

  TH1D* pAddDopDS = new TH1D("AddDop_EnergyDS","Addback Doppler Energy",12000,0,4000);
  TH2D* pAddDopSumDS = new TH2D("AddDop_SummaryDS","Addback Doppler Energy Summary",64,1,65,6000,0,3000);

  TH1D* pSupDopDS = new TH1D("SupDop_EnDS","Suppressed Doppler Energy",12000,0,4000);
  TH1D* pAddSupDopDS = new TH1D("AddSupDop_EnDS","Suppressed Addback Doppler Energy",12000,0,4000);

  TH1* pCoreEnergyDS_fep = new TH1D("Core_EnergyDS_fep","Tigress Core Energy FEP",12000,0,4000);
  TH1* pCoreEnergyDS_pfep = new TH1D("Core_EnergyDS_pfep","Tigress Core Energy Projectile FEP",12000,0,4000);
  TH1* pCoreEnergyDS_rfep = new TH1D("Core_EnergyDS_rfep","Tigress Core Energy Recoil FEP",12000,0,4000);
  TH1* pCoreEnergyDS_nfep = new TH1D("Core_EnergyDS_nfep","Tigress Core Energy Not FEP",12000,0,4000);

  TH1* pDopEnergyDS_fep = new TH1D("Dop_EnergyDS_fep","Doppler Energy FEP",12000,0,4000);
  TH1* pDopEnergyDS_pfep = new TH1D("Dop_EnergyDS_pfep","Doppler Energy Projectile FEP",12000,0,4000);
  TH1* pDopEnergyDS_rfep = new TH1D("Dop_EnergyDS_rfep","Doppler Energy Recoil FEP",12000,0,4000);
  TH1* pDopEnergyDS_nfep = new TH1D("Dop_EnergyDS_nfep","Doppler Energy Not FEP",12000,0,4000);

  TH2* pDopvPartDS = new TH2D("DopEn_v_PartEn_DS","Doppler Energy vs Particle Energy",
			      3000,0,3000,500,0,500);

  TH2* pThCorDS = new TH2D("Theta_CorrDS","Theta Correlation",3000,0,3000,90,0,180);
  TH2* pThCrtDS = new TH2D("Theta_CrctDS","Theta Correction",6000,0,3000,90,0,180);

  double thing1 = 65.*180./32.0;
  TH2* pPhCorDS = new TH2D("Phi_CorrDS","Phi Correlation",3000,0,3000,32,0,thing1);
  TH2* pPhCrtDS = new TH2D("Phi_CrctDS","Phi Correction",6000,0,3000,32,0,thing1);

  TH1* pReconEnergyDS = new TH1D("Recon_EnergyDS","Recon Energy",12000,0,4000);
  TH2* pReconSumDS = new TH2D("Recon_SummaryDS","Recon Energy Summary",16,1,17,6000,0,3000);

  TH1D* pSupReconEnDS = new TH1D("SupRecon_EnDS","Suppresed Recon Energy",12000,0,4000);
  TH1D* pAddReconEnDS = new TH1D("AddRecon_EnDS","Addback Recon Energy",12000,0,4000);
  TH1D* pAddSupReconEnDS = new TH1D("AddSupRecon_EnDS","Addback Suppresed Recon Energy",12000,0,4000);

  TH1* pReconEnergyDS_fep = new TH1D("Recon_EnergyDS_fep","Recon Energy FEP",12000,0,4000);
  TH1* pReconEnergyDS_pfep = new TH1D("Recon_EnergyDS_pfep","Recon Energy Projectile FEP",12000,0,4000);
  TH1* pReconEnergyDS_rfep = new TH1D("Recon_EnergyDS_rfep","Recon Energy Recoil FEP",12000,0,4000);
  TH1* pReconEnergyDS_nfep = new TH1D("Recon_EnergyDS_nfep","Recon Energy Not FEP",12000,0,4000);

  TH2* pReconvPartDS = new TH2D("ReconEn_v_partEn_DS","Recon Energy vs Particle Energy",
				3000,0,3000,500,0,500);

  TH2* pReconThCorDS = new TH2D("ReconTheta_CorrDS","Recon Theta Correlation",3000,0,3000,90,0,180);
  TH2* pReconThCrtDS = new TH2D("ReconTheta_CrctDS","Recon Theta Correction",6000,0,3000,90,0,180);

  TH2* pReconPhCorDS = new TH2D("ReconPhi_CorrDS","Recon Phi Correlation",3000,0,3000,32,0,thing1);
  TH2* pReconPhCrtDS = new TH2D("ReconPhi_CrctDS","Recon Phi Correction",6000,0,3000,32,0,thing1);

  //Projectile US
  TH2* sPidUS = new TH2D("SecPID_US","DS SectorEn PID",24,1,25,500,0,500);
  TH2* rPidUS = new TH2D("RingPID_US","DS RingEn PID",24,1,25,500,0,500);
  
  TH1* pCoreEnergyUS = new TH1D("Core_EnergyUS","Tigress Core Energy",12000,0,4000);
  TH2* pCoreSumUS = new TH2D("Core_SummaryUS","Core Energy Summary",16,1,17,3000,0,3000);
  
  TH1* pDopEnergyUS = new TH1D("Dop_EnergyUS","Doppler Energy",12000,0,4000);
  TH2* pDopSumUS = new TH2D("Dop_SummaryUS","Doppler Energy Summary",16,1,17,6000,0,3000);

  TH1D* pAddDopUS = new TH1D("AddDop_EnUS","Addback Doppler Energy",12000,0,4000);
  TH1D* pSupDopUS = new TH1D("SupDop_EnUS","Suppressed Doppler Energy",12000,0,4000);
  TH1D* pAddSupDopUS = new TH1D("AddSupDop_EnUS","Suppressed Addback Doppler Energy",12000,0,4000);

  TH1* pCoreEnergyUS_fep = new TH1D("Core_EnergyUS_fep","Tigress Core Energy FEP",12000,0,4000);
  TH1* pCoreEnergyUS_pfep = new TH1D("Core_EnergyUS_pfep","Tigress Core Energy Projectile FEP",12000,0,4000);
  TH1* pCoreEnergyUS_rfep = new TH1D("Core_EnergyUS_rfep","Tigress Core Energy Recoil FEP",12000,0,4000);
  TH1* pCoreEnergyUS_nfep = new TH1D("Core_EnergyUS_nfep","Tigress Core Energy Not FEP",12000,0,4000);

  TH1* pDopEnergyUS_fep = new TH1D("Dop_EnergyUS_fep","Doppler Energy FEP",12000,0,4000);
  TH1* pDopEnergyUS_pfep = new TH1D("Dop_EnergyUS_pfep","Doppler Energy Projectile FEP",12000,0,4000);
  TH1* pDopEnergyUS_rfep = new TH1D("Dop_EnergyUS_rfep","Doppler Energy Recoil FEP",12000,0,4000);
  TH1* pDopEnergyUS_nfep = new TH1D("Dop_EnergyUS_nfep","Doppler Energy Not FEP",12000,0,4000);

  TH2* pDopvPartUS = new TH2D("Dop_PartEn_US","Doppler Energy vs Particle Energy",3000,0,3000,500,0,500);

  TH2* pThCorUS = new TH2D("Theta_CorrUS","Theta Correlation",3000,0,3000,90,0,180);
  TH2* pThCrtUS = new TH2D("Theta_CrctUS","Theta Correction",6000,0,3000,90,0,180);
  
  TH2* pPhCorUS = new TH2D("Phi_CorrUS","Phi Correlation",3000,0,3000,32,0,thing1);
  TH2* pPhCrtUS = new TH2D("Phi_CrctUS","Phi Correction",6000,0,3000,32,0,thing1);

  TH1* pReconEnergyUS = new TH1D("Recon_EnergyUS","Recon Energy",12000,0,4000);
  TH2* pReconSumUS = new TH2D("Recon_SummaryUS","Recon Energy Summary",16,1,17,6000,0,3000);

  TH1D* pAddReconUS = new TH1D("AddRecon_EnUS","Addback Recon Energy",12000,0,4000);
  TH1D* pSupReconUS = new TH1D("SupRecon_EnUS","Suppressed Recon Energy",12000,0,4000);
  TH1D* pAddSupReconUS = new TH1D("AddSupRecon_EnUS","Suppressed Addback Recon Energy",12000,0,4000);

  TH1* pReconEnergyUS_fep = new TH1D("Recon_EnergyUS_fep","Recon Energy FEP",12000,0,4000);
  TH1* pReconEnergyUS_pfep = new TH1D("Recon_EnergyUS_pfep","Recon Energy Projectile FEP",12000,0,4000);
  TH1* pReconEnergyUS_rfep = new TH1D("Recon_EnergyUS_rfep","Recon Energy Recoil FEP",12000,0,4000);
  TH1* pReconEnergyUS_nfep = new TH1D("Recon_EnergyUS_nfep","Recon Energy Not FEP",12000,0,4000);

  TH2* pReconvPartUS = new TH2D("ReconEn_v_partEn_US","Recon Energy vs Particle Energy",
				3000,0,3000,500,0,500);

  TH2* pReconThCorUS = new TH2D("ReconTheta_CorrUS","Recon Theta Correlation",3000,0,3000,90,0,180);
  TH2* pReconThCrtUS = new TH2D("ReconTheta_CrctUS","Recon Theta Correction",6000,0,3000,90,0,180);

  TH2* pReconPhCorUS = new TH2D("ReconPhi_CorrUS","Recon Phi Correlation",3000,0,3000,32,0,thing1);
  TH2* pReconPhCrtUS = new TH2D("ReconPhi_CrctUS","Recon Phi Correction",6000,0,3000,32,0,thing1);

  //Recoil
  TH2* sPidRec = new TH2D("SecPID_Rec","DS SectorEn PID",24,1,25,500,0,500);
  TH2* rPidRec = new TH2D("RingPID_Rec","DS RingEn PID",24,1,25,500,0,500);
  
  TH1* rCoreEnergy = new TH1D("Core_EnergyRec","Tigress Core Energy",12000,0,4000);
  TH2* rCoreSum = new TH2D("Core_SummaryRec","Core Energy Summary",16,1,17,3000,0,3000);
  
  TH1* rDopEnergy = new TH1D("Dop_EnergyRec","Doppler Energy",12000,0,4000);
  TH2* rDopSum = new TH2D("Dop_SummaryRec","Doppler Energy Summary",16,1,17,6000,0,3000);

  TH1D* rAddDopEn = new TH1D("AddDop_EnRec","Addback Doppler Energy",12000,0,4000);
  TH1D* rSupDopEn = new TH1D("SupDop_EnRec","Suppressed Doppler Energy",12000,0,4000);
  TH1D* rAddSupDopEn = new TH1D("AddSupDop_EnRec","Suppressed Addback Doppler Energy",12000,0,4000);

  TH1* rCoreEnergy_fep = new TH1D("Core_EnergyRec_fep","Tigress Core Energy FEP",12000,0,4000);
  TH1* rCoreEnergy_pfep = new TH1D("Core_EnergyRec_pfep","Tigress Core Energy Projectile FEP",12000,0,4000);
  TH1* rCoreEnergy_rfep = new TH1D("Core_EnergyRec_rfep","Tigress Core Energy Recoil FEP",12000,0,4000);
  TH1* rCoreEnergy_nfep = new TH1D("Core_EnergRec_nfep","Tigress Core Energy Not FEP",12000,0,4000);

  TH1* rDopEnergy_fep = new TH1D("Dop_EnergyRec_fep","Doppler Energy FEP",12000,0,4000);
  TH1* rDopEnergy_pfep = new TH1D("Dop_EnergyRec_pfep","Doppler Energy Projectile FEP",12000,0,4000);
  TH1* rDopEnergy_rfep = new TH1D("Dop_EnergyRec_rfep","Doppler Energy Recoil FEP",12000,0,4000);
  TH1* rDopEnergy_nfep = new TH1D("Dop_EnergyRec_nfep","Doppler Energy Not FEP",12000,0,4000);

  TH2* rDopvPart = new TH2D("DopEv_v_PartEn_Rec","Doppler Energy vs Particle Energy",3000,0,3000,500,0,500);

  TH2* rThCor = new TH2D("Theta_CorrRec","Theta Correlation",3000,0,3000,90,0,180);
  TH2* rThCrt = new TH2D("Theta_CrctRec","Theta Correction",6000,0,3000,90,0,180);;
  TH2* rPhCor = new TH2D("Phi_CorrRec","Phi Correlation",3000,0,3000,32,0,thing1);
  TH2* rPhCrt = new TH2D("Phi_CrctRec","Phi Correction",6000,0,3000,32,0,thing1);

  TH1* rReconEnergy = new TH1D("Recon_EnergyRec","Recon Energy",12000,0,4000);
  TH2* rReconSum = new TH2D("Recon_SummaryRec","Recon Energy Summary",16,1,17,6000,0,3000);

  TH1D* rRecAddEn = new TH1D("AddRecon_EnRec","Addback Recon Energy",12000,0,4000);
  TH1D* rRecSupEn = new TH1D("SupRecon_EnRec","Suppressed Recon Energy",12000,0,4000);
  TH1D* rRecAddSupEn = new TH1D("AddSupRecon_EnRec","Suppressed Addback Recon Energy",12000,0,4000);

  TH1* rReconEnergy_fep = new TH1D("Recon_EnergyRec_fep","Recon Energy FEP",12000,0,4000);
  TH1* rReconEnergy_pfep = new TH1D("Recon_EnergyRec_pfep","Recon Energy Projectile FEP",12000,0,4000);
  TH1* rReconEnergy_rfep = new TH1D("Recon_EnergyRec_rfep","Recon Energy Recoil FEP",12000,0,4000);
  TH1* rReconEnergy_nfep = new TH1D("Recon_EnergyRec_nfep","Recon Energy Not FEP",12000,0,4000);

  TH2* rReconvPart = new TH2D("ReconEn_v_PartEn_Rec","Recon Energy vs Particle Energy",3000,0,3000,500,0,500);

  TH2* rReconThCor = new TH2D("ReconTheta_CorrRec","Recon Theta Correlation",3000,0,3000,90,0,180);
  TH2* rReconThCrt = new TH2D("ReconTheta_CrctRec","Recon Theta Correction",6000,0,3000,90,0,180);

  TH2* rReconPhCor = new TH2D("ReconPhi_CorrRec","Recon Phi Correlation",3000,0,3000,32,0,thing1);
  TH2* rReconPhCrt = new TH2D("ReconPhi_CrctRec","Recon Phi Correction",6000,0,3000,32,0,thing1);

  FillPositonVectors();
  const TVector3 incBeam = TVector3(0.0,0.0,1.0);
  const double Sol2_En = KE_LAB(Theta_CM_FP(Theta_LAB_Max(beam_en),beam_en),beam_en);
  const double r2d = TMath::RadToDeg();

  TRandom* rand = new TRandom(50747227);

  std::cout << "Correlating and histograming data..." << std::endl;
  
  Header header;
  RawData raw_data;
  while(fread(&header,header.bytes(),1,input_file)) {

    const int nE = header.evtNum;
    const int nS = header.nSdata;
    const int nT = header.nTdata;
  
    fread(&raw_data.sData,sizeof(S3Data),nS,input_file);
    fread(&raw_data.tData,sizeof(TigressData),nT,input_file);
    BuiltData data = BuildData(nE,nS,nT,raw_data);
    
    //S3 singles
    for(int i=0;i<data.nS3;i++) {

      int det = data.s3[i].det;
      int ring = data.s3[i].ring;
      int sec = data.s3[i].sector;

      if(ring < Ri || ring > Rf)
	continue;
      
      double ring_en = data.s3[i].rEn;
      double sec_en = data.s3[i].sEn;

      TVector3 segPos = GetPos(det,ring,sec);
      TVector3 pos = data.s3[i].sPos;

      segPos.SetX(segPos.X() - beam_X);
      segPos.SetY(segPos.Y() - beam_Y);
      pos.SetX(pos.X() - beam_X);
      pos.SetY(pos.Y() - beam_Y);
      
      if(!det) { //Upstream

	bSum->Fill(sec,sec_en);
	bSum->Fill(ring+32,ring_en);
	
	rPid0->Fill(ring,ring_en);
	sPid0->Fill(ring,sec_en);
	secD0->Fill(sec);

	if(data.s3[i].rP && data.s3[i].sP) {
	  pSum->Fill(sec,sec_en);
	  pSum->Fill(ring+32,ring_en);
	}
	if(data.s3[i].rR && data.s3[i].sR) {
	  rSum->Fill(sec,sec_en);
	  rSum->Fill(ring+32,ring_en);
	}
      }
      else { //Downstream
	
	rPid1->Fill(ring,ring_en);
	sPid1->Fill(ring,sec_en);
	secD1->Fill(sec);

	bSum->Fill(sec+64,sec_en);
	bSum->Fill(ring+96,ring_en);

	if(data.s3[i].rP && data.s3[i].sP) { //Projectile
	  sPid1_p->Fill(ring,sec_en);

	  pSum->Fill(sec+64,sec_en);
	  pSum->Fill(ring+96,ring_en);

	  pSecDS->Fill(sec);
	  pSvRDS->Fill(ring,sec);
	  pThvPhDS->Fill(pos.Theta()*r2d,pos.Phi()*r2d);
	  
	  if(ring%2) {
	    if(sec%2) {
	      pThvPhDS1->Fill(pos.Theta()*r2d,pos.Phi()*r2d);
	    }
	    else {
	      pThvPhDS2->Fill(pos.Theta()*r2d,pos.Phi()*r2d);
	    }
	  }
	  else {
	    if(sec%2) {
	      pThvPhDS2->Fill(pos.Theta()*r2d,pos.Phi()*r2d);
	    }
	    else {
	      pThvPhDS1->Fill(pos.Theta()*r2d,pos.Phi()*r2d);
	    }
	  }  
 
	} //End projectile gate

	if(data.s3[i].rR && data.s3[i].sR) { //Recoil
	  sPid1_r->Fill(ring,sec_en);

	  rSum->Fill(sec+64,sec_en);
	  rSum->Fill(ring+96,ring_en);

	  rSec->Fill(sec);
	  rThvPh->Fill(pos.Theta()*r2d,pos.Phi()*r2d);
	  
	} //End recoil gate

      } //End downstream gate
      
    } //End S3 singles

    //Tigress singles
    for(int i=0;i<data.nTi;i++) {

      int det = data.tigress[i].det;
      double en = data.tigress[i].cEn;
      double core_en = rand->Gaus(en,Sigma(en));
      bool fep = data.tigress[i].fep;
      bool sup = data.tigress[i].sup;
      
      coreEnergy->Fill(core_en);
      coreSum->Fill(det,core_en);
      
      for(int j=0;j<data.tigress[i].nsegs;j++) {

	double seg_en = data.tigress[i].sEn[j];
	int num = data.tigress[i].segs[j] + 32*(det-1);

	double exactX = data.tigress[i].x[j];
	double exactY = data.tigress[i].y[j];
	double exactZ = data.tigress[i].z[j];
	TVector3 exact_pos(exactX,exactY,exactZ);

	double theta = exact_pos.Theta()*r2d;
	double phi = exact_pos.Phi();
	if(phi < 0) {
	  phi += TMath::TwoPi();
	}
	phi *= r2d;	
	
	segEnergy->Fill(seg_en);
	segSum->Fill(num,seg_en);
	sPosTP->Fill(theta,phi);
	
      }
      
      if(fep) {
	coreEn_Fep->Fill(core_en);
      }
      else {
	coreEn_NotFep->Fill(core_en);
      }

      if(!sup) {
	supEnergy->Fill(core_en);
	supSum->Fill(det,core_en);
      }
      
    } //End Tigress singles

    //Tigress Addback
    for(unsigned int i=0;i<data.tigressAB.size();i++) {

      int det = data.tigressAB[i].det;
      //int seg = data.tigressAB[i].MainSeg();
      bool sup = data.tigressAB[i].sup;
      
      double en = data.tigressAB[i].cEn;
      double coreEn = rand->Gaus(en,Sigma(en));
      
      addEnergy->Fill(coreEn);
      addSum->Fill(det,coreEn);

      if(!sup) {
	addSupEn->Fill(coreEn);
	addSupSum->Fill(det,coreEn);
      }
      
    } //end Addback Tigress loop

    //Coincidences
    if(data.nS3 > 0 && data.nTi > 0) {
      
      for(int i=0;i<data.nS3;i++) {

	int bDet = data.s3[i].det;
	int ring = data.s3[i].ring;
	int sector = data.s3[i].sector;

	if(ring < Ri || ring > Rf)
	  continue;

	double ring_en = data.s3[i].rEn;
        double sec_en = data.s3[i].sEn;

	TVector3 bPos = GetPos(bDet,ring,sector);
	bPos.SetX(bPos.X() - beam_X);
	bPos.SetY(bPos.Y() - beam_Y);
	
	if(bDet && data.s3[i].rP && data.s3[i].sP) { //Projectile DS gate

	  sPidDS->Fill(ring,sec_en);
	  rPidDS->Fill(ring,ring_en);

	  bool sol2 = false;
	  if(sec_en < Sol2_En)
	    sol2 = true;
	  
	  double thetaCM = Theta_CM_FP(bPos.Theta(),beam_en,sol2);
	  
	  double energy = KE_LAB(thetaCM,beam_en);
	  double gam = (energy/beam_mass) + 1.0;
	  double beta = TMath::Sqrt(1.0 - 1.0/(gam*gam));

	  double recon_energy = Recoil_KE_LAB(thetaCM,beam_en);
	  double recon_gam = (recon_energy)/targ_mass + 1.0;
	  double recon_beta = TMath::Sqrt(1.0 - 1.0/(recon_gam*recon_gam));
	  
	  TVector3 rPos(0,0,1); //Reconstructed position of the recoil
	  rPos.SetTheta(Recoil_Theta_LAB(thetaCM,beam_en));
	  rPos.SetPhi(bPos.Phi() - TMath::Pi());
	  
	  for(int j=0;j<data.nTi;j++) {

	    int det = data.tigress[j].det;
	    int seg = data.tigress[j].MainSeg();
	    double en = data.tigress[j].cEn;
	    double coreEn = rand->Gaus(en,Sigma(en));
	    bool FEP = data.tigress[j].fep;
	    bool PFEP = data.tigress[j].pfep;
	    bool SUP = data.tigress[j].sup;
	  
	    TVector3 sPos = GetPos(det,seg);
	    sPos.SetX(sPos.X() - beam_X);
	    sPos.SetY(sPos.Y() - beam_Y);

	    double theta = bPos.Angle(sPos);
	    double dopEn = gam*(1 - beta*TMath::Cos(theta))*coreEn;
	    
	    TVector3 reacPlane = bPos.Cross(incBeam);
	    TVector3 detPlane = sPos.Cross(incBeam);

	    double reac_phi = reacPlane.Phi();
	    if(reac_phi < 0)
	      reac_phi += TMath::TwoPi();

	    double det_phi = detPlane.Phi();
	    if(det_phi < 0)
	      det_phi += TMath::TwoPi();

	    double planeAng = reac_phi - det_phi;
	    if(planeAng < 0)
	      planeAng += TMath::TwoPi();

	    double recon_theta = rPos.Angle(sPos);
	    double recon_en = recon_gam*(1 - recon_beta*TMath::Cos(recon_theta))*coreEn;
	    
	    TVector3 reconPlane = rPos.Cross(incBeam);

	    double recon_phi = reconPlane.Phi();
	    if(recon_phi < 0)
	      recon_phi += TMath::TwoPi();

	    double reconAng = recon_phi - det_phi;
	    if(reconAng < 0)
	      reconAng += TMath::TwoPi();
	    
	    pCoreEnergyDS->Fill(coreEn);
	    pCoreSumDS->Fill(det,coreEn);

	    if(FEP) {
	      pCoreEnergyDS_fep->Fill(coreEn);
	      pDopEnergyDS_fep->Fill(dopEn);
	      pReconEnergyDS_fep->Fill(recon_en);

	      if(PFEP) {
		pCoreEnergyDS_pfep->Fill(coreEn);
		pDopEnergyDS_pfep->Fill(dopEn);
		pReconEnergyDS_pfep->Fill(recon_en);
	      }
	      else {
		pCoreEnergyDS_rfep->Fill(coreEn);
		pDopEnergyDS_rfep->Fill(dopEn);
		pReconEnergyDS_rfep->Fill(recon_en);
	      }
	    }
	    else {
	      pCoreEnergyDS_nfep->Fill(coreEn);
	      pDopEnergyDS_nfep->Fill(dopEn);
	      pReconEnergyDS_nfep->Fill(recon_en);
	    }

	    if(!SUP) {
	      pSupDopDS->Fill(dopEn);
	      pSupReconEnDS->Fill(recon_en);
	    }
	    
	    pDopEnergyDS->Fill(dopEn);
	    pDopSumDS->Fill(det,dopEn);

	    pDopvPartDS->Fill(dopEn,sec_en);
	    
	    pThCorDS->Fill(coreEn,theta*r2d);
	    pThCrtDS->Fill(dopEn,theta*r2d);

	    pPhCorDS->Fill(coreEn,planeAng*r2d);
	    pPhCrtDS->Fill(dopEn,planeAng*r2d);

	    pReconEnergyDS->Fill(recon_en);
	    pReconSumDS->Fill(det,recon_en);

	    pReconvPartDS->Fill(recon_en,sec_en);

	    pReconThCorDS->Fill(coreEn,recon_theta*r2d);
	    pReconThCrtDS->Fill(recon_en,recon_theta*r2d);

	    pReconPhCorDS->Fill(coreEn,reconAng*r2d);
	    pReconPhCrtDS->Fill(recon_en,reconAng*r2d);
	    
	  } //End Tigress loop

	  for(unsigned int j=0;j<data.tigressAB.size();j++) {

	    int det = data.tigressAB[j].det;
	    int seg = data.tigressAB[j].MainSeg();
	    
	    double en = data.tigressAB[j].cEn;
	    double coreEn = rand->Gaus(en,Sigma(en));
	    bool SUP = data.tigressAB[j].sup;
	 
	    TVector3 sPos = GetPos(det,seg);
	    sPos.SetX(sPos.X() - beam_X);
	    sPos.SetY(sPos.Y() - beam_Y);
	    
	    double theta = bPos.Angle(sPos);
	    double dopEn = gam*(1 - beta*TMath::Cos(theta))*coreEn;

	    double recon_theta = rPos.Angle(sPos);
	    double recon_en = recon_gam*(1 - recon_beta*TMath::Cos(recon_theta))*coreEn;
	    
	    pAddDS->Fill(coreEn);
	    pAddSumDS->Fill(det,coreEn);

	    pAddDopDS->Fill(dopEn);
	    pAddDopSumDS->Fill(det,dopEn);

	    pAddReconEnDS->Fill(recon_en);

	    if(!SUP) {
	      pAddSupDopDS->Fill(dopEn);
	      pAddSupReconEnDS->Fill(recon_en);
	    }
	    
	  } //end Addback Tigress loop
	  
	} //End DS projectile gate

	else if(!bDet && data.s3[i].rP && data.s3[i].sP) { //Projectile US gate

	  sPidUS->Fill(ring,sec_en);
	  rPidUS->Fill(ring,ring_en);
	  
	  double thetaCM = Theta_CM_FP(bPos.Theta(),beam_en,false);
	  
	  double energy = KE_LAB(thetaCM,beam_en);
	  double gam = (energy)/beam_mass + 1.0;
	  double beta = TMath::Sqrt(1.0 - 1.0/(gam*gam));

	  double recon_energy = Recoil_KE_LAB(thetaCM,beam_en);
	  double recon_gam = (recon_energy)/targ_mass + 1.0;
	  double recon_beta = TMath::Sqrt(1.0 - 1.0/(recon_gam*recon_gam));

	  TVector3 rPos(0,0,1); //Reconstructed position of the recoil
	  rPos.SetTheta(Recoil_Theta_LAB(thetaCM,beam_en));
	  rPos.SetPhi(bPos.Phi() - TMath::Pi());
	  
	  for(int j=0;j<data.nTi;j++) {

	    int det = data.tigress[j].det;
	    int seg = data.tigress[j].MainSeg();
	    double en = data.tigress[j].cEn;
	    double coreEn = rand->Gaus(en,Sigma(en));
	    bool FEP = data.tigress[j].fep;
	    bool PFEP = data.tigress[j].pfep;
	    bool SUP = data.tigress[j].sup;
	  
	    TVector3 sPos = GetPos(det,seg);
	    sPos.SetX(sPos.X() - beam_X);
	    sPos.SetY(sPos.Y() - beam_Y);

	    double theta = bPos.Angle(sPos);
	    double dopEn = gam*(1 - beta*TMath::Cos(theta))*coreEn;
	  
	    TVector3 reacPlane = bPos.Cross(incBeam);
	    TVector3 detPlane = sPos.Cross(incBeam);

	    double reac_phi = reacPlane.Phi();
	    if(reac_phi < 0)
	      reac_phi += TMath::TwoPi();

	    double det_phi = detPlane.Phi();
	    if(det_phi < 0)
	      det_phi += TMath::TwoPi();

	    double planeAng = reac_phi - det_phi;
	    if(planeAng < 0)
	      planeAng += TMath::TwoPi();

	    double recon_theta = rPos.Angle(sPos);
	    double recon_en = recon_gam*(1 - recon_beta*TMath::Cos(recon_theta))*coreEn;

	    TVector3 reconPlane = rPos.Cross(incBeam);

	    double recon_phi = reconPlane.Phi();
	    if(recon_phi < 0)
	      recon_phi += TMath::TwoPi();

	    double reconAng = recon_phi - det_phi;
	    if(reconAng < 0)
	      reconAng += TMath::TwoPi();

	    pCoreEnergyUS->Fill(coreEn);
	    pCoreSumUS->Fill(det,coreEn);
	    
	    pDopEnergyUS->Fill(dopEn);
	    pDopSumUS->Fill(det,dopEn);

	    if(FEP) {
	      pCoreEnergyUS_fep->Fill(coreEn);
	      pDopEnergyUS_fep->Fill(dopEn);
	      pReconEnergyUS_fep->Fill(recon_en);

	      if(PFEP) {
		pCoreEnergyUS_pfep->Fill(coreEn);
		pDopEnergyUS_pfep->Fill(dopEn);
		pReconEnergyUS_pfep->Fill(recon_en);
	      }
	      else {
		pCoreEnergyUS_rfep->Fill(coreEn);
		pDopEnergyUS_rfep->Fill(dopEn);
		pReconEnergyUS_rfep->Fill(recon_en);
	      }
	    }
	    else {
	      pCoreEnergyUS_nfep->Fill(coreEn);
	      pDopEnergyUS_nfep->Fill(dopEn);
	      pReconEnergyUS_nfep->Fill(recon_en);
	    }

	    if(!SUP) {
	      pSupDopUS->Fill(dopEn);
	      pSupReconUS->Fill(recon_en);
	    }
	    
	    pDopvPartUS->Fill(dopEn,sec_en);

	    pThCorUS->Fill(coreEn,theta*r2d);
	    pThCrtUS->Fill(dopEn,theta*r2d);

	    pPhCorUS->Fill(coreEn,planeAng*r2d);
	    pPhCrtUS->Fill(dopEn,planeAng*r2d);

	    pReconEnergyUS->Fill(recon_en);
	    pReconSumUS->Fill(det,recon_en);

	    pReconvPartUS->Fill(recon_en,sec_en);

	    pReconThCorUS->Fill(coreEn,recon_theta*r2d);
	    pReconThCrtUS->Fill(recon_en,recon_theta*r2d);

	    pReconPhCorUS->Fill(coreEn,reconAng*r2d);
	    pReconPhCrtUS->Fill(recon_en,reconAng*r2d);
	  
	  } //End Tigress loop

	  for(unsigned int j=0;j<data.tigressAB.size();j++) {

	    int det = data.tigressAB[j].det;
	    int seg = data.tigressAB[j].MainSeg();
	    
	    double en = data.tigressAB[j].cEn;
	    double coreEn = rand->Gaus(en,Sigma(en));
	    //bool FEP = data.tigressAB[j].fep;
	    //bool PFEP = data.tigressAB[j].pfep;
	    bool SUP = data.tigressAB[j].sup;
	 
	    TVector3 sPos = GetPos(det,seg);
	    sPos.SetX(sPos.X() - beam_X);
	    sPos.SetY(sPos.Y() - beam_Y);
	    
	    double theta = bPos.Angle(sPos);
	    double dopEn = gam*(1 - beta*TMath::Cos(theta))*coreEn;

	    double recon_theta = rPos.Angle(sPos);
	    double recon_en = recon_gam*(1 - recon_beta*TMath::Cos(recon_theta))*coreEn;

	    pAddDopUS->Fill(dopEn);
	    pAddReconUS->Fill(recon_en);
	    if(!SUP) {
	      pAddSupDopUS->Fill(dopEn);
	      pAddSupReconUS->Fill(recon_en);
	    }
	    
	  } //end Addback Tigress loop
	  
	} ////End US projectile gate

	else if(data.s3[i].rR && data.s3[i].sR) { //Recoil gate

	  sPidRec->Fill(ring,sec_en);
	  rPidRec->Fill(ring,ring_en);

	  double thetaCM = Theta_CM_FR(bPos.Theta(),beam_en);
	  
	  double energy = Recoil_KE_LAB(thetaCM,beam_en);
	  double gam = (energy)/targ_mass + 1.0;
	  double beta = TMath::Sqrt(1.0 - 1.0/(gam*gam));

	  double recon_energy = KE_LAB(thetaCM,beam_en);
	  double recon_gam = (recon_energy)/beam_mass + 1.0;
	  double recon_beta = TMath::Sqrt(1.0 - 1.0/(recon_gam*recon_gam));

	  TVector3 rPos(0,0,1); //Reconstructed position of the projectile
	  rPos.SetTheta(Theta_LAB(thetaCM,beam_en));
	  rPos.SetPhi(bPos.Phi() - TMath::Pi());

	  for(int j=0;j<data.nTi;j++) {

	    int det = data.tigress[j].det;
	    int seg = data.tigress[j].MainSeg();
	    double en = data.tigress[j].cEn;
	    double coreEn = rand->Gaus(en,Sigma(en));
	    bool FEP = data.tigress[j].fep;
	    bool PFEP = data.tigress[j].pfep;
	    bool SUP = data.tigress[j].sup;
	  
	    TVector3 sPos = GetPos(det,seg);

	    double theta = bPos.Angle(sPos);
	    double dopEn = gam*(1 - beta*TMath::Cos(theta))*coreEn;
	  
	    TVector3 reacPlane = bPos.Cross(incBeam);
	    TVector3 detPlane = sPos.Cross(incBeam);

	    double reac_phi = reacPlane.Phi();
	    if(reac_phi < 0)
	      reac_phi += TMath::TwoPi();

	    double det_phi = detPlane.Phi();
	    if(det_phi < 0)
	      det_phi += TMath::TwoPi();

	    double planeAng = reac_phi - det_phi;
	    if(planeAng < 0)
	      planeAng += TMath::TwoPi();

	    double recon_theta = rPos.Angle(sPos);
	    double recon_en = recon_gam*(1 - recon_beta*TMath::Cos(recon_theta))*coreEn;

	    TVector3 reconPlane = rPos.Cross(incBeam);

	    double recon_phi = reconPlane.Phi();
	    if(recon_phi < 0)
	      recon_phi += TMath::TwoPi();

	    double reconAng = recon_phi - det_phi;
	    if(reconAng < 0)
	      reconAng += TMath::TwoPi();
	    
	    rCoreEnergy->Fill(coreEn);
	    rCoreSum->Fill(det,coreEn);
	    
	    rDopEnergy->Fill(dopEn);
	    rDopSum->Fill(det,dopEn);

	    if(FEP) {
	      rCoreEnergy_fep->Fill(coreEn);
	      rDopEnergy_fep->Fill(dopEn);
	      rReconEnergy_fep->Fill(recon_en);

	      if(PFEP) {
		rCoreEnergy_pfep->Fill(coreEn);
		rDopEnergy_pfep->Fill(dopEn);
		rReconEnergy_pfep->Fill(recon_en);
	      }
	      else {
		rCoreEnergy_rfep->Fill(coreEn);
		rDopEnergy_rfep->Fill(dopEn);
		rReconEnergy_rfep->Fill(recon_en);
	      }
	    }
	    else {
	      rCoreEnergy_nfep->Fill(coreEn);
	      rDopEnergy_nfep->Fill(dopEn);
	      rReconEnergy_nfep->Fill(recon_en);
	    }

	    if(!SUP) {
	      rSupDopEn->Fill(dopEn);
	      rRecSupEn->Fill(recon_en);
	    }

	    rDopvPart->Fill(dopEn,sec_en);

	    rThCor->Fill(coreEn,theta*r2d);
	    rThCrt->Fill(dopEn,theta*r2d);
	    
	    rPhCor->Fill(coreEn,planeAng*r2d);
	    rPhCrt->Fill(dopEn,planeAng*r2d);

	    rReconEnergy->Fill(recon_en);
	    rReconSum->Fill(det,recon_en);

	    rReconvPart->Fill(recon_en,sec_en);

	    rReconThCor->Fill(coreEn,recon_theta*r2d);
	    rReconThCrt->Fill(recon_en,recon_theta*r2d);

	    rReconPhCor->Fill(coreEn,reconAng*r2d);
	    rReconPhCrt->Fill(recon_en,reconAng*r2d);
	    
	  } //End Tigress loop

	  for(unsigned int j=0;j<data.tigressAB.size();j++) {

	    int det = data.tigressAB[j].det;
	    int seg = data.tigressAB[j].MainSeg();
	    
	    double en = data.tigressAB[j].cEn;
	    double coreEn = rand->Gaus(en,Sigma(en));
	    //bool FEP = data.tigressAB[j].fep;
	    //bool PFEP = data.tigressAB[j].pfep;
	    bool SUP = data.tigressAB[j].sup;
	 
	    TVector3 sPos = GetPos(det,seg);
	    sPos.SetX(sPos.X() - beam_X);
	    sPos.SetY(sPos.Y() - beam_Y);
	    
	    double theta = bPos.Angle(sPos);
	    double dopEn = gam*(1 - beta*TMath::Cos(theta))*coreEn;

	    double recon_theta = rPos.Angle(sPos);
	    double recon_en = recon_gam*(1 - recon_beta*TMath::Cos(recon_theta))*coreEn;

	    rAddDopEn->Fill(dopEn);
	    rRecAddEn->Fill(recon_en);

	    if(!SUP) {
	      rAddSupDopEn->Fill(dopEn);
	      rRecAddSupEn->Fill(recon_en);
	    }
	    
	  } //end Addback Tigress loop
	  
	} //End recoil gate
	
      } //End S3 loop
    } //End coincidences

    
  } //End while loop
  fclose(input_file);

  std::cout << "Writing histograms to file..." << std::endl;

  TFile* outFile = new TFile(output_filename,"RECREATE");
  
  outFile->mkdir("Tigress");
  outFile->mkdir("S3");
  
  outFile->mkdir("Coincidence/ProjectileDS");
  outFile->mkdir("Coincidence/ProjectileDS/Doppler");
  outFile->mkdir("Coincidence/ProjectileDS/Recon");
  
  outFile->mkdir("Coincidence/ProjectileUS");
  outFile->mkdir("Coincidence/ProjectileUS/Doppler");
  outFile->mkdir("Coincidence/ProjectileUS/Recon");
  
  outFile->mkdir("Coincidence/Recoil");
  outFile->mkdir("Coincidence/Recoil/Doppler");
  outFile->mkdir("Coincidence/Recoil/Recon");

  outFile->cd("S3");

  bSum->Write();
  pSum->Write();
  rSum->Write();
  
  pThvPhDS->Write();
  pThvPhDS1->Write();
  pThvPhDS2->Write();

  rThvPh->Write();
     
  rPid0->Write();
  sPid0->Write();
  secD0->Write();
  
  rPid1->Write();
  sPid1->Write();
  secD1->Write();

  pSecDS->Write();
  rSec->Write();
  pSvRDS->Write();

  sPid1_p->Write();
  sPid1_r->Write();
  
  outFile->cd("Tigress");

  coreEnergy->Write();
  coreSum->Write();
  
  segEnergy->Write();
  segSum->Write();

  addEnergy->Write();
  addSum->Write();

  supEnergy->Write();
  supSum->Write();

  addSupEn->Write();
  addSupSum->Write();

  coreEn_Fep->Write();
  coreEn_NotFep->Write();

  sPosTP->Write();

  outFile->cd("Coincidence/ProjectileDS");

  sPidDS->Write();
  rPidDS->Write();

  pCoreEnergyDS->Write();
  pCoreSumDS->Write();

  pAddDS->Write();
  pAddSumDS->Write();
  
  pCoreEnergyDS_fep->Write();
  pCoreEnergyDS_pfep->Write();
  pCoreEnergyDS_rfep->Write();
  pCoreEnergyDS_nfep->Write();

  outFile->cd("Coincidence/ProjectileDS/Doppler");
  
  pDopEnergyDS->Write();
  pDopSumDS->Write();

  pAddDopDS->Write();
  pAddDopSumDS->Write();

  pSupDopDS->Write();
  pAddSupDopDS->Write();

  pDopEnergyDS_fep->Write();
  pDopEnergyDS_pfep->Write();
  pDopEnergyDS_rfep->Write();
  pDopEnergyDS_nfep->Write();

  pDopvPartDS->Write();

  pThCorDS->Write();
  pThCrtDS->Write();

  pPhCorDS->Write();
  pPhCrtDS->Write();

  outFile->cd("Coincidence/ProjectileDS/Recon");

  pReconEnergyDS->Write();
  pReconSumDS->Write();

  pSupReconEnDS->Write();
  pAddReconEnDS->Write();
  pAddSupReconEnDS->Write();

  pReconEnergyDS_fep->Write();
  pReconEnergyDS_pfep->Write();
  pReconEnergyDS_rfep->Write();
  pReconEnergyDS_nfep->Write();

  pReconvPartDS->Write();

  pReconThCorDS->Write();
  pReconThCrtDS->Write();

  pReconPhCorDS->Write();
  pReconPhCrtDS->Write();

  outFile->cd("Coincidence/ProjectileUS");

  sPidUS->Write();
  rPidUS->Write();

  pCoreEnergyUS->Write();
  pCoreSumUS->Write();

  pCoreEnergyUS_fep->Write();
  pCoreEnergyUS_pfep->Write();
  pCoreEnergyUS_rfep->Write();
  pCoreEnergyUS_nfep->Write();

  outFile->cd("Coincidence/ProjectileUS/Doppler");
  
  pDopEnergyUS->Write();
  pDopSumUS->Write();

  pSupDopUS->Write();
  pAddDopUS->Write();
  pAddSupDopUS->Write();

  pDopEnergyUS_fep->Write();
  pDopEnergyUS_pfep->Write();
  pDopEnergyUS_rfep->Write();
  pDopEnergyUS_nfep->Write();

  pDopvPartUS->Write();

  pThCorUS->Write();
  pThCrtUS->Write();

  pPhCorUS->Write();
  pPhCrtUS->Write();

  outFile->cd("Coincidence/ProjectileUS/Recon");

  pReconEnergyUS->Write();
  pReconSumUS->Write();

  pSupReconUS->Write();
  pAddReconUS->Write();
  pAddSupReconUS->Write();

  pReconEnergyUS_fep->Write();
  pReconEnergyUS_pfep->Write();
  pReconEnergyUS_rfep->Write();
  pReconEnergyUS_nfep->Write();

  pReconvPartUS->Write();

  pReconThCorUS->Write();
  pReconThCrtUS->Write();

  pReconPhCorUS->Write();
  pReconPhCrtUS->Write();
  
  outFile->cd("Coincidence/Recoil");

  sPidRec->Write();
  rPidRec->Write();

  rCoreEnergy->Write();
  rCoreSum->Write();

  rCoreEnergy_fep->Write();
  rCoreEnergy_pfep->Write();
  rCoreEnergy_rfep->Write();
  rCoreEnergy_nfep->Write();

  outFile->cd("Coincidence/Recoil/Doppler");
  
  rDopEnergy->Write();
  rDopSum->Write();

  rSupDopEn->Write();
  rAddDopEn->Write();
  rAddSupDopEn->Write();

  rDopEnergy_fep->Write();
  rDopEnergy_pfep->Write();
  rDopEnergy_rfep->Write();
  rDopEnergy_nfep->Write();

  rDopvPart->Write();

  rThCor->Write();
  rThCrt->Write();

  rPhCor->Write();
  rPhCrt->Write();

  outFile->cd("Coincidence/Recoil/Recon");

  rReconEnergy->Write();
  rReconSum->Write();

  rRecSupEn->Write();
  rRecAddEn->Write();
  rRecAddSupEn->Write();
  
  rReconEnergy_fep->Write();
  rReconEnergy_pfep->Write();
  rReconEnergy_rfep->Write();
  rReconEnergy_nfep->Write();

  rReconvPart->Write();

  rReconThCor->Write();
  rReconThCrt->Write();

  rReconPhCor->Write();
  rReconPhCrt->Write();

  outFile->Close();
  std::cout << "Done!" << std::endl;

  return 0;
}
