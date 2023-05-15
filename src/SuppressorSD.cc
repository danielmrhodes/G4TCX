#include "SuppressorSD.hh"
#include "G4SystemOfUnits.hh"

SuppressorSD::SuppressorSD(G4String name) : G4VSensitiveDetector(name) {
  collectionName.insert("suppressorCollection");
}

SuppressorSD::~SuppressorSD() {}

void SuppressorSD::Initialize(G4HCofThisEvent*) {
  
  HC = new Suppressor_Hit_Collection(SensitiveDetectorName,collectionName[0]);
  
  return;
}

G4bool SuppressorSD::ProcessHits(G4Step* step, G4TouchableHistory*) {
  
  if(step->GetTotalEnergyDeposit() > 0.0) {
    
    Suppressor_Hit* hit = new Suppressor_Hit();
    
    //hit->SetPos(step->GetPreStepPoint()->GetPosition());
    hit->SetDetSeg(step->GetPreStepPoint()->GetPhysicalVolume()->GetCopyNo());
    //hit->SetEdep(step->GetTotalEnergyDeposit());
    HC->insert(hit);
    
  }
 
  return true;
}

void SuppressorSD::EndOfEvent(G4HCofThisEvent* HCE) {
  
 label:

  for(unsigned int i=0;i<HC->entries();i++) {
    for(unsigned int j=i+1;j<HC->entries();j++) {

      Suppressor_Hit* hit1 = (Suppressor_Hit*)HC->GetHit(i);
      Suppressor_Hit* hit2 = (Suppressor_Hit*)HC->GetHit(j);

      if(hit1->GetSegment() == hit2->GetSegment() && hit1->GetDetector() == hit2->GetDetector()) {
	
	//hit1->SetEdep(hit1->GetEdep()+hit2->GetEdep());
	
	std::vector<Suppressor_Hit*>* vec = HC->GetVector();
	vec->erase(vec->begin()+j);

	goto label;
	
      }
    }
  }
  
  HCE->AddHitsCollection(HCE->GetNumberOfCollections(),HC);
  
  return;
  
}
