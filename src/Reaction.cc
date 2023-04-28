#include "Reaction.hh"
#include "G4PhysicalConstants.hh"
#include "G4RunManager.hh"
#include "Detector_Construction.hh"

Reaction::Reaction() : beamZ(48), beamA(106), beam_mass(0.0), targZ(82), targA(208),
		       targ_mass(0.0), onlyP(false), onlyR(false) {

  messenger = new Reaction_Messenger(this);

  recThresh = 0.0*MeV;
  
}

Reaction::Reaction(G4int bZ, G4int bA, G4double bM, G4int tZ, G4int tA, G4double tM)
  : beamZ(bZ), beamA(bA), beam_mass(bM), targZ(tZ), targA(tA), targ_mass(tM),
    onlyP(false), onlyR(false) {

  messenger = new Reaction_Messenger(this);

  recThresh = 0.0*MeV;

}

Reaction::~Reaction() {

  delete messenger;
  
}

void Reaction::ConstructRutherfordCM(G4double Ep, G4double Ex) {
  
  if(good_LAB_thetas.size()) {
    
    if(good_LAB_thetas.size()%2 == 0) {
      TruncatedRutherfordCM(Ep,Ex);
    }
    else {
      
      G4cout << "Desired LAB angle ranges were improperly defined! Resorting to the full scattering angle range!"
	     << G4endl;
      
      //S3Thetas();
      FullRutherfordCM(Ep,Ex);
    }
    
  }
  else {
    FullRutherfordCM(Ep,Ex);
  }

  return;
}

void Reaction::FullRutherfordCM(G4double Ep, G4double Ex) {
  
  const G4int nBins = 1800;
  G4double probDist[nBins];

  for(G4int i=0;i<nBins;i++) {

    G4double thetaCM = (pi/(double)nBins)*i;

    if(i<100)
      probDist[i] = 0.0;
    else
      probDist[i] = 2.0*pi*std::sin(thetaCM)*RutherfordCM(thetaCM,Ep,Ex);

  }

  AngleGenerator = new CLHEP::RandGeneral(probDist,nBins-1);

  return;
  
}

void Reaction::TruncatedRutherfordCM(G4double Ep, G4double Ex) {
  
  const G4int nBins = 1800;
  G4double probDist[nBins-1];
  
  for(G4int i=0;i<nBins-1;i++) {

    G4double thetaCM = (pi/(double)nBins)*(i+1);
    if(KeepThetaCM(thetaCM,Ep,Ex))
      probDist[i] = 2.0*pi*std::sin(thetaCM)*RutherfordCM(thetaCM,Ep,Ex);
    else 
      probDist[i] = 0.0;

  }
  
  AngleGenerator = new CLHEP::RandGeneral(probDist,nBins-1);

  return;
  
}

G4bool Reaction::KeepThetaCM(G4double thetaCM, G4double Ep, G4double Ex) {

  G4bool keep = false;

  if(onlyP) { //Only consider projectiles
    
    for(unsigned int i=0;i<good_LAB_thetas.size();i+=2) {
  
      if(
	 //Check if projectile will scatter into desired  range
         ((Theta_LAB(thetaCM,Ep,Ex) > good_LAB_thetas.at(i)) &&
          (Theta_LAB(thetaCM,Ep,Ex) < good_LAB_thetas.at(i+1)))
        ) {
      
        keep = true;
      } 
    }
    
  }
  else if(onlyR) { //Only consider recoils

    for(unsigned int i=0;i<good_LAB_thetas.size();i+=2) {
  
      if(
	 //Check if recoil will will scatter into desired  range
         ((Recoil_Theta_LAB(thetaCM,Ep,Ex) > good_LAB_thetas.at(i)) &&
          (Recoil_Theta_LAB(thetaCM,Ep,Ex) < good_LAB_thetas.at(i+1)) &&
	  (Recoil_KE_LAB(thetaCM,Ep,Ex) > recThresh))
        ) {
      
        keep = true;
      }
    }

  }
  /*
  else if(rDSpUS) { //Recoil downstream, projectile upstream

    for(unsigned int i=0;i<good_LAB_thetas.size();i+=2) {
  
      if(!i && //DS thetas only for recoil
	 //Check if recoil will will scatter into desired  range
         ((Recoil_Theta_LAB(thetaCM,Ep,Ex) > good_LAB_thetas.at(i)) &&
          (Recoil_Theta_LAB(thetaCM,Ep,Ex) < good_LAB_thetas.at(i+1)) &&
	  (Recoil_KE_LAB(thetaCM,Ep,Ex) > recThresh))
        ) {
      
        keep = true;
      }
      else if(i && //US thetas only for projectile
	      //Check if projectile will scatter into desired  range
	      ((Theta_LAB(thetaCM,Ep,Ex) > good_LAB_thetas.at(i)) &&
	       (Theta_LAB(thetaCM,Ep,Ex) < good_LAB_thetas.at(i+1)))
	     ) {
      
        keep = true;
      }
    }
    
  }
  */
  else { //Consider both
    
    for(unsigned int i=0;i<good_LAB_thetas.size();i+=2) {
  
      if(
	 //Check if projectile will scatter into desired range
         ((Theta_LAB(thetaCM,Ep,Ex) > good_LAB_thetas.at(i)) &&
          (Theta_LAB(thetaCM,Ep,Ex) < good_LAB_thetas.at(i+1)))
       
         ||
	 
         //Check if recoil will will scatter into desired range
         ((Recoil_Theta_LAB(thetaCM,Ep,Ex) > good_LAB_thetas.at(i)) &&
          (Recoil_Theta_LAB(thetaCM,Ep,Ex) < good_LAB_thetas.at(i+1)) &&
	  (Recoil_KE_LAB(thetaCM,Ep,Ex) > recThresh))
       
        ) {	
      
        keep = true;
      }
    }
    
  }

  return keep;
}

/*
void Reaction::S3Thetas() {

  good_LAB_thetas.clear();
  Detector_Construction* con = (Detector_Construction*)G4RunManager::GetRunManager()->GetUserDetectorConstruction();

  //S3 offsets
  G4double uso = con->GetUS_Offset();
  G4double dso = con->GetDS_Offset();

  //S3 thickness
  G4double th = 300.0*um;

  G4ThreeVector v(0,0,1);

  //Downstream inner edge
  v.setRhoPhiZ(1.1*cm,0.0*rad,dso-(th/2.0));
  good_LAB_thetas.push_back(v.theta());

  //Downstream outer edge
  v.setRhoPhiZ(3.5*cm,0.0*rad,dso+(th/2.0));
  good_LAB_thetas.push_back(v.theta());

  //Upstream outer edge
  v.setRhoPhiZ(3.5*cm,0.0*rad,-uso-(th/2.0));
  good_LAB_thetas.push_back(v.theta());
  
  //Upstream inner edge
  v.setRhoPhiZ(1.1*cm,0.0*rad,-uso+(th/2.0));
  good_LAB_thetas.push_back(v.theta());

  return;
}

void Reaction::UpstreamThetas() {

  S3Thetas();
  
  good_LAB_thetas.erase(good_LAB_thetas.begin());
  good_LAB_thetas.erase(good_LAB_thetas.begin());
  
  return;
}

void Reaction::DownstreamThetas() {

  S3Thetas();
  
  good_LAB_thetas.erase(good_LAB_thetas.begin()+2);
  good_LAB_thetas.erase(good_LAB_thetas.begin()+2); 
  
  return;
}
*/

void Reaction::AddThetaLAB(G4double T) {
  
  good_LAB_thetas.push_back(T);

  return;
}

void Reaction::SetOnlyP() {

  onlyR = false;
  onlyP = true;

  return;
}

void Reaction::SetOnlyR() {

  onlyP = false;
  onlyR = true;

  return;
}

G4double Reaction::SampleRutherfordCM() {

  return AngleGenerator->shoot()*pi;
  
}

/*
//this is a standard GEANT4 implementation
G4double Reaction::SampleRutherfordCM_2() {

  //CM Angles
  G4double cosTetMinNuc = 0.939693; //Cos(20 deg)
  G4double cosTetMaxNuc = -0.866025; //Cos(150 deg)
  //G4double screenZ = 2.18921e-06; //80Ge 281.6 MeV on 196Pt
  G4double screenZ = 0.0;
  
  G4double x1 = 1. - cosTetMinNuc + screenZ;
  G4double x2 = 1. - cosTetMaxNuc + screenZ;
  G4double dx = cosTetMinNuc - cosTetMaxNuc;
  G4double z1 = x1*x2/(x1 + G4UniformRand()*dx) - screenZ;

  return std::acos(1.0 - z1);
  
}
*/

//Everything below here come the GOSIA manual, chapter 5
G4double Reaction::RutherfordCM(G4double thetaCM, G4double Ep, G4double Ex) {

  G4double Ecm = Ep/(1.0 + (beam_mass/targ_mass));
  G4double Esym2 = std::pow(Ecm,1.5)*std::sqrt(Ecm-Ex);
  G4double denom = Esym2*std::pow(std::sin(thetaCM/2.0),4);
  G4double factor = 1.29596*MeV*MeV*(millibarn/sr);
  
  return factor*std::pow(beamZ*targZ,2)/denom;
  
}
  
G4double Reaction::Theta_LAB(G4double thetaCM, G4double Ep, G4double Ex) {

  G4double tau = (beam_mass/targ_mass)/std::sqrt(1 - (Ex/Ep)*(1 + beam_mass/targ_mass));
  G4double tanTheta = std::sin(thetaCM)/(std::cos(thetaCM) + tau);

  if(tanTheta > 0) {
    return std::atan(tanTheta);
  }
  
  return std::atan(tanTheta) + pi;
  
}

G4double Reaction::Recoil_Theta_LAB(G4double thetaCM, G4double Ep, G4double Ex) {

  G4double tau = 1.0/std::sqrt(1 - (Ex/Ep)*(1 + beam_mass/targ_mass));
  G4double tanTheta = std::sin(pi - thetaCM)/(std::cos(pi - thetaCM) + tau);
  
  return std::atan(tanTheta);
  
}

G4double Reaction::KE_LAB(G4double thetaCM, G4double Ep, G4double Ex) {

  G4double tau = (beam_mass/targ_mass)/std::sqrt(1 - (Ex/Ep)*(1 + beam_mass/targ_mass));

  G4double term1 = std::pow(targ_mass/(beam_mass + targ_mass),2);
  G4double term2 = 1 + tau*tau + 2*tau*std::cos(thetaCM);
  G4double term3 = Ep - Ex*(1 + beam_mass/targ_mass);
  
  return term1*term2*term3;
  
}

G4double Reaction::Recoil_KE_LAB(G4double thetaCM, G4double Ep, G4double Ex) {

  G4double tau = 1.0/std::sqrt(1 - (Ex/Ep)*(1 + beam_mass/targ_mass));

  G4double term1 = beam_mass*targ_mass/std::pow(beam_mass + targ_mass,2);
  G4double term2 = 1 + tau*tau + 2*tau*std::cos(pi - thetaCM);
  G4double term3 = Ep - Ex*(1 + beam_mass/targ_mass);
  
  return term1*term2*term3;
  
}

///////Static Functions///////
G4double Reaction::KE_LAB(G4double thetaCM, G4double Ep, G4double bM, G4double rM, G4double Ex) {

  G4double tau = (bM/rM)/std::sqrt(1 - (Ex/Ep)*(1 + bM/rM));

  G4double term1 = std::pow(rM/(bM + rM),2);
  G4double term2 = 1 + tau*tau + 2*tau*std::cos(thetaCM);
  G4double term3 = Ep - Ex*(1 + bM/rM);
  
  return term1*term2*term3;
}

G4double Reaction::Recoil_KE_LAB(G4double thetaCM, G4double Ep, G4double bM, G4double rM, G4double Ex) {

  G4double tau = 1.0/std::sqrt(1 - (Ex/Ep)*(1 + bM/rM));

  G4double term1 = bM*rM/std::pow(bM + rM,2);
  G4double term2 = 1 + tau*tau + 2*tau*std::cos(pi - thetaCM);
  G4double term3 = Ep - Ex*(1 + bM/rM);
  
  return term1*term2*term3;
}

G4double Reaction::Beta_LAB(G4double thetaCM, G4double Ep, G4double bM, G4double rM, G4double Ex) {

  G4double energy = KE_LAB(thetaCM,Ep,bM,rM,Ex);
  G4double gam = energy/bM + 1.0;

  return std::sqrt(1.0 - 1.0/(gam*gam));
  
}

G4double Reaction::Recoil_Beta_LAB(G4double thetaCM, G4double Ep, G4double bM, G4double rM, G4double Ex) {

  G4double energy = Recoil_KE_LAB(thetaCM,Ep,bM,rM,Ex);
  G4double gam = energy/rM + 1.0;

  return std::sqrt(1.0 - 1.0/(gam*gam));
  
}
//////////////////////////////
