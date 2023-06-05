#include "Gamma_Decay.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleMomentum.hh"
#include "G4DynamicParticle.hh"
#include "G4DecayProducts.hh"
#include "G4PhysicalConstants.hh"
#include "Randomize.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4HadronicException.hh"

#include "G4RandomDirection.hh"

Gamma_Decay::Gamma_Decay(G4ParticleDefinition* Parent, G4ParticleDefinition* daughter, G4double BR,
			 G4double Ei, G4double Ef, G4int twoJP, G4int twoJD, G4int L0, G4int Lp,
			 G4double del, G4double cc, G4bool emit, G4bool prj) : Gamma_Decay(Parent,daughter,BR) {

  //trans = new G4PolarizationTransition();
  trans = new Polarization_Transition();
  
  twoJi = twoJP;
  twoJf = twoJD;
  
  transL = L0;
  transLp = Lp;
  delta = del;

  EnI = Ei;
  EnF = Ef;
  convCoef = cc;

  proj = prj;
  emit_gamma = emit;
  
}

Gamma_Decay::Gamma_Decay(G4ParticleDefinition* Parent, G4ParticleDefinition* daughter,
			 G4double BR) : G4VDecayChannel("GammaDecay",Parent->GetParticleName(),
							BR,2,daughter->GetParticleName(),"gamma"),
					theDaughterMasses(0) {

  SetParent(Parent);
  parentmass = Parent->GetPDGMass();

  SetDaughter(0,daughter);
  SetDaughter(1,"gamma");
  
}

Gamma_Decay::~Gamma_Decay() {}

G4DecayProducts* Gamma_Decay::DecayIt(G4double) {

  if (GetVerboseLevel()>1) G4cout << "G4GeneralPhaseSpaceDecay::TwoBodyDecayIt()"<<G4endl;
  
  //daughters'mass
  G4double daughtermass[2]; 
  G4double daughtermomentum;
  if ( theDaughterMasses )
    { 
      daughtermass[0]= *(theDaughterMasses);
      daughtermass[1] = *(theDaughterMasses+1);
    } else {   
    daughtermass[0] = G4MT_daughters[0]->GetPDGMass();
    daughtermass[1] = G4MT_daughters[1]->GetPDGMass();
  }

  //create parent G4DynamicParticle at rest
  G4ParticleMomentum dummy;
  G4DynamicParticle * parentparticle = new G4DynamicParticle( G4MT_parent, dummy, 0.0);

  //create G4Decayproducts  @@GF why dummy parentparticle?
  G4DecayProducts *products = new G4DecayProducts(*parentparticle);
  delete parentparticle;

  //calculate daughter momentum
  daughtermomentum = Pmx(parentmass,daughtermass[0],daughtermass[1]);

  G4double costheta;
  G4double phi;
  trans->SampleGammaTransition(proj,twoJi,twoJf,transL,transLp,delta,costheta,phi);
  
  G4double sintheta = std::sqrt((1.0 - costheta)*(1.0 + costheta));
  G4ParticleMomentum direction(sintheta*std::cos(phi),sintheta*std::sin(phi),costheta);

  //create daughter G4DynamicParticle
  G4double Etotal= std::sqrt(daughtermass[0]*daughtermass[0] + daughtermomentum*daughtermomentum); 
  G4DynamicParticle * daughterparticle = new G4DynamicParticle( G4MT_daughters[0],Etotal, direction*daughtermomentum);
  products->PushProducts(daughterparticle);

  if(emit_gamma) {
    G4double prob = 1.0/(convCoef + 1.0);
    if(G4UniformRand() < prob) {
      Etotal= std::sqrt(daughtermass[1]*daughtermass[1] + daughtermomentum*daughtermomentum);
      daughterparticle = new G4DynamicParticle( G4MT_daughters[1],Etotal, direction*(-1.0*daughtermomentum));
      products->PushProducts(daughterparticle);
    }
  }

  if (GetVerboseLevel()>1) 
    {
      G4cout << "G4GeneralPhaseSpaceDecay::TwoBodyDecayIt ";
      G4cout << "  create decay products in rest frame " <<G4endl;
      products->DumpInfo();
    }
  return products;

}

inline G4double Gamma_Decay::Pmx(G4double e, G4double p1, G4double p2) {

  // calculate momentum of daughter particles in two-body decay
   if (e-p1-p2 < 0 ) {
     throw G4HadronicException(__FILE__, __LINE__,
			       "G4GeneralPhaseSpaceDecay::Pmx energy in cms > mass1+mass2");
   }
   
   G4double ppp = (e+p1+p2)*(e+p1-p2)*(e-p1+p2)*(e-p1-p2)/(4.0*e*e);
   if (ppp>0) return std::sqrt(ppp);
   else       return -1.;
  
}

void Gamma_Decay::SetProjectilePolarization(const std::vector< std::vector<G4complex> >& polar) {

  CleanProjectile(); 
  for(std::vector<G4complex> pol : polar)
    polarizationP.push_back(pol);
  
  return;
}

void Gamma_Decay::UnpolarizeProjectile() {

  CleanProjectile();
  polarizationP.resize(1);
  polarizationP[0].push_back(1.0);
  
  return;
}

void Gamma_Decay::CleanProjectile() {

  if(!polarizationP.empty()) {
    for(std::vector<G4complex> pol : polarizationP)
      pol.clear();
    polarizationP.clear();
  }
  
  return;
}

void Gamma_Decay::SetRecoilPolarization(const std::vector< std::vector<G4complex> >& polar) {

  CleanRecoil(); 
  for(std::vector<G4complex> pol : polar)
    polarizationR.push_back(pol);
  
  return;
}

void Gamma_Decay::UnpolarizeRecoil() {

  CleanRecoil();
  polarizationR.resize(1);
  polarizationR[0].push_back(1.0);
  
  return;
}

void Gamma_Decay::CleanRecoil() {

  if(!polarizationR.empty()) {
    for(std::vector<G4complex> pol : polarizationR)
      pol.clear();
    polarizationR.clear();
  }
  
  return;
}
