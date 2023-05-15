#include "Gamma_Source.hh"
#include "Gamma_Decay.hh"

#include "G4ProcessManager.hh"
#include "G4IonTable.hh"
#include "G4Decay.hh"
#include "G4DecayTable.hh"

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4PhysicalConstants.hh"

Gamma_Source::Gamma_Source() {

  messenger = new Gamma_Source_Messenger(this);

  source_energy = -1.0*MeV;
  GSS = 0.0;
  file_name = "";

  threadID = G4Threading::G4GetThreadId();
}

Gamma_Source::~Gamma_Source() {

  delete messenger;
  
}

void Gamma_Source::BuildLevelScheme() {
  
  if(source_energy > 0.0*MeV) {

    if(!threadID)
      std::cout << "Simple isotropic gamma-ray of " << source_energy/keV
	     << " keV will be emitted each event" << std::endl;

    return;
  }

  if(file_name == "") {

    if(!threadID)
      std::cout << "\033[1;31mNeither the source level scheme file nor the source energy is set!"
	     << " The simulation will break now...\033[m" << std::endl;

    return;
  }

  G4IonTable* table = (G4IonTable*)(G4ParticleTable::GetParticleTable()->GetIonTable());

  G4ParticleDefinition* ground_state = table->GetIon(82,208,0.0*MeV);
  ground_state->SetPDGLifeTime(-1.0);
  
  Polarized_Particle* polGS = new Polarized_Particle(ground_state,82,208,GSS,0.0*MeV);
  levels.push_back(polGS);
 
  std::ifstream file;
  file.open(file_name.c_str(),std::ios::in);
  
  if(!file.is_open()) {
    if(!threadID)
      std::cout << "\n\033[1;31mCould not open source level scheme file " << file_name
	     << "! The simulation won't do anything...\033[m" << std::endl;
    return;
  }

  if(!threadID)
    std::cout << "\nBuilding source level scheme from " << file_name << std::endl;
  
  std::string line, word;
  while(std::getline(file,line)) {

    G4int state_index, nbr;
    G4double energy, spin, lifetime, prob;

    std::stringstream ss1(line);
    ss1 >> state_index >> energy >> spin >> lifetime >> prob >> nbr;

    energy *= keV;
    lifetime *= ps;
    probs.push_back(prob);

    if(!threadID)
      std::cout << " " << state_index << " " << energy/keV << " " << spin << " " << lifetime/ps << " "
		<< prob << " " << nbr;

    G4ParticleDefinition* part = table->GetIon(82,208,energy);
    if(nbr) {
      if(!threadID) {
	part->SetPDGLifeTime(lifetime);
	part->SetDecayTable(new G4DecayTable());
      }
      part->GetProcessManager()->SetParticleType(part);
      part->GetProcessManager()->AddProcess(new G4Decay(),0,-1,0);
    }
    else {
      if(!threadID)
	std::cout << " \033[1;36m Warning: State " << state_index << " has no decay branches.\033[m";
      part->SetPDGLifeTime(-1.0);
    }
    if(!threadID)
      std::cout << std::endl;
    
    Polarized_Particle* ppart = new Polarized_Particle(part,82,208,spin,energy);
    for(int i=0;i<nbr;i++) {

      G4int index, L0, Lp;
      G4double BR, del, cc;
      
      std::getline(file,line);
      std::stringstream ss2(line);
      ss2 >> index >> BR >> L0 >> Lp >> del >> cc;

      if(!threadID)
	std::cout << "  " << index << " " << BR << " " << L0 << " " << Lp << " " << del << " " << cc
		  << std::endl;

      if(!threadID)
	part->GetDecayTable()->Insert(new Gamma_Decay(ppart,levels.at(index),BR,L0,Lp,del,cc,true));
      
    }

    if((unsigned int)state_index != levels.size())
      if(!threadID)
	std::cout << "States are out of order in source level scheme file " << file_name << "!" << std::endl;
    
    levels.push_back(ppart);
    
  }

  if(levels.size()-1 == probs.size()) {
    if(!threadID)
      std::cout << levels.size()-1  << " excited states built for the source!" << std::endl;
  }
  else
    if(!threadID)
      std::cout << "There are " << levels.size()-1 << " excited states but " << probs.size()
	     << " population probabilities!" << std::endl;
  
  G4double sum = 0.0;
  for(auto p : probs)
    sum += p;

  for(unsigned int i=0;i<probs.size();i++)
    probs.at(i) /= sum;
  
  return;
  
}

G4int Gamma_Source::ChooseState() {

  G4double sumBR = 0.0;
  G4double num = G4UniformRand();

  for(unsigned int i=0;i<probs.size();i++) {
    sumBR += probs[i];
    if(num < sumBR) {
      return i+1;
    }
  }

  return 0;
}

void Gamma_Source::Unpolarize() {

  for(auto& lvl : levels)
    lvl->Unpolarize();
  
  return;
}
