#ifndef Reaction_h
#define Reaction_h 1
 
#include "globals.hh"
#include "Randomize.hh"
#include "G4SystemOfUnits.hh"
#include "Reaction_Messenger.hh"

class Reaction_Messenger;
class Reaction {
  
public:

  Reaction();
  Reaction(G4int bZ, G4int bA, G4double bM, G4int tZ, G4int tA, G4double tM);
  ~Reaction();

  //////////Kinematic Calclations//////////
  G4double Theta_LAB(G4double thetaCM, G4double Ep, G4double Ex=0.0*MeV);
  G4double Recoil_Theta_LAB(G4double thetaCM, G4double Ep, G4double Ex=0.0*MeV);
  
  G4double KE_LAB(G4double thetaCM, G4double Ep, G4double Ex=0.0*MeV);
  G4double Recoil_KE_LAB(G4double thetaCM, G4double Ep, G4double Ex=0.0*MeV);

  //G4double Theta_LAB_Max(G4double Ep, G4double Ex0.0*MeV);
  //G4double Recoil_Theta_LAB_Max(G4double Ep, G4double Ex=0.0.*MeV);

  //G4double Theta_CM_FP(G4double ThetaLAB, G4double Ep, G4bool sol2=false, G4double Ex=0.0*MeV);
  //G4double Theta_CM_FR(G4double ThetaLAB, G4double Ep, G4bool sol2=false, G4double Ex=0.0*MeV);

  G4double RutherfordCM(G4double thetaCM, G4double Ep, G4double Ex=0.0*MeV);
  ////////////////////////////////////////

  ///These build the desired scattering angle distribution///
  void ConstructRutherfordCM(G4double Ep, G4double Ex=0.0*MeV);
  void FullRutherfordCM(G4double Ep, G4double Ex=0.0*MeV);
  void TruncatedRutherfordCM(G4double Ep, G4double Ex=0.0*MeV);
  ///////////////////////////////////////////////////////////

  //////////These define desired angles//////////
  //void AddThetaLABRange(G4double Tmin, G4double Tmax);
  void AddThetaLAB(G4double T);

  //void S3Thetas();
  //void UpstreamThetas();
  //void DownstreamThetas();
  
  void SetOnlyP();
  void SetOnlyR();
  //void SetRecDS_ProjUS() {rDSpUS = true;}
  //////////////////////////////////////////////
  
  G4double SampleRutherfordCM();
  //G4double SampleRutherfordCM_2();

  ////////Getters and setters////////
  void SetBeamZ(G4int Z) {beamZ = Z;}
  void SetBeamA(G4int A) {beamA = A;}
  void SetBeamMass(G4double M) {beam_mass = M;}

  void SetRecoilZ(G4int Z) {targZ = Z;}
  void SetRecoilA(G4int A) {targA = A;}
  void SetRecoilMass(G4double M) {targ_mass = M;}
  void SetRecoilThreshold(G4double thresh) {recThresh = thresh;}

  G4int GetBeamZ() {return beamZ;}
  G4int GetBeamA() {return beamA;}
  G4double GetBeamMass() {return beam_mass;}

  G4int GetRecoilZ() {return targZ;}
  G4int GetRecoilA() {return targA;}
  G4double GetRecoilMass() {return targ_mass;}
  ///////////////////////////////////

  ////////Static Kinematic Functions////////
  static G4double KE_LAB(G4double thetaCM, G4double Ep, G4double bM, G4double rM, G4double Ex=0.0*MeV);
  static G4double Recoil_KE_LAB(G4double thetaCM, G4double Ep, G4double bM, G4double rM,
				G4double Ex=0.0*MeV);
  static G4double Beta_LAB(G4double thetaCM, G4double Ep, G4double bM, G4double rM, G4double Ex=0.0*MeV);
  static G4double Recoil_Beta_LAB(G4double thetaCM, G4double Ep, G4double bM, G4double rM,
				  G4double Ex=0.0*MeV);
  //////////////////////////////////////////
  
private:

  G4bool KeepThetaCM(G4double thetaCM, G4double Ep, G4double Ex=0.0*MeV);

  Reaction_Messenger* messenger;
  
  G4int beamZ;
  G4int beamA;
  G4double beam_mass;
  
  G4int targZ;
  G4int targA;
  G4double targ_mass;

  G4bool onlyP;
  G4bool onlyR;
  //G4bool rDSpUS;

  G4double recThresh;

  std::vector<G4double> good_LAB_thetas;
  
  //Random generator for CM scattering angle
  CLHEP::RandGeneral* AngleGenerator;
  

};       

#endif
