#include "Reaction.hh"
#include "G4Threading.hh"

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

  G4int threadID = G4Threading::G4GetThreadId();
  
  if(!good_LAB_thetas.size()) {
    
    if(!threadID)
      std::cout << "Desired LAB angle ranges were not defined!"
		<< " Defaulting large LAB angle range of (13,180) deg!" << std::endl;

    good_LAB_thetas.push_back(13*deg);
    good_LAB_thetas.push_back(180*deg);
    SetOnlyP();
  }
  else if(good_LAB_thetas.size()%2) {
    
    if(!threadID)
      std::cout << "There must be an even number desired LAB scattering angles!"
		<< " Defaulting large LAB angle range of (13,180) deg!" << std::endl;

    good_LAB_thetas.push_back(13*deg);
    good_LAB_thetas.push_back(180*deg);
    SetOnlyP();
  }
  else {
    for(unsigned int i=0;i<good_LAB_thetas.size();i+=2) {
      if(good_LAB_thetas.at(i) > good_LAB_thetas.at(i+1)) {

	if(!threadID)
	  std::cout << "Desired LAB angle ranges were improperly defined!"
		    << " Defaulting large LAB angle range of (13,180) deg!" << std::endl;

	good_LAB_thetas.clear();
	good_LAB_thetas.push_back(13*deg);
	good_LAB_thetas.push_back(180*deg);
	SetOnlyP();
	
	break;
      }
    }
  }
  
  const G4int nBins = 1800;
  G4double probDist[nBins];
  
  for(G4int i=0;i<nBins;i++) {

    probDist[i] = 0.0;
    if(i < 50) //mandatory CM angle cut of 5 deg to avoid divergence
      continue;
    
    G4double thetaCM = (pi/(double)nBins)*i;
    if(KeepThetaCM(thetaCM,Ep,Ex))
      probDist[i] = 2.0*pi*std::sin(thetaCM)*RutherfordCM(thetaCM,Ep,Ex);    

  }
  
  AngleGenerator = new CLHEP::RandGeneral(probDist,nBins);

  return;
}

G4bool Reaction::KeepThetaCM(G4double thetaCM, G4double Ep, G4double Ex) {

  if(onlyP) { //Only consider projectiles
    
    for(unsigned int i=0;i<good_LAB_thetas.size();i+=2) {
  
      if(
	 //Check if projectile will scatter into desired  range
         ((Theta_LAB(thetaCM,Ep,Ex) > good_LAB_thetas.at(i)) &&
          (Theta_LAB(thetaCM,Ep,Ex) < good_LAB_thetas.at(i+1)))
        ) {
      
        return true;
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
      
        return true;
      }
    }

  }
  
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
      
	return true;
      }
    }
    
  }

  return false;
}

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

  if(tanTheta > 0)
    return std::atan(tanTheta);
  
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
