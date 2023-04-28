#include "IonSD.hh"
#include "G4SystemOfUnits.hh"
#include "vector"

IonSD::IonSD(G4String name) : G4VSensitiveDetector(name) {
  collectionName.insert("ionCollection");
}

IonSD::~IonSD() {}

void IonSD::Initialize(G4HCofThisEvent*) {
  
  HC = new Ion_Hit_Collection(SensitiveDetectorName,collectionName[0]);
  
  return;
}

G4bool IonSD::ProcessHits(G4Step* step, G4TouchableHistory*) {

  G4String name = step->GetTrack()->GetParticleDefinition()->GetParticleName();
  if(!G4StrUtil::contains(name,proj_name) && !G4StrUtil::contains(name,recoil_name))
    return true;
  
  Ion_Hit* rHit = new Ion_Hit();
  rHit->SetPos(step->GetPreStepPoint()->GetPosition());
  rHit->SetEdep(step->GetTotalEnergyDeposit());
  rHit->SetSeg(step->GetPreStepPoint()->GetPhysicalVolume()->GetCopyNo());

  if(G4StrUtil::contains(name,proj_name)) {
    rHit->SetIsProjectile();
    rHit->SetIsNotRecoil();
  }
  else {
    rHit->SetIsNotProjectile();
    rHit->SetIsRecoil();
  }
  
  Ion_Hit* sHit = new Ion_Hit(rHit);

  sHit->SetRing(0);
  rHit->SetSector(0);
  
  HC->insert(rHit);
  HC->insert(sHit);
  
  return true;
}

void IonSD::EndOfEvent(G4HCofThisEvent* HCE) {
  
  if(HC->entries() > 2) {
    ConsolidateHits();
  }
  
  if(HC->entries() > 3) {
    CombineRings();
  }
  
  HCE->AddHitsCollection(HCE->GetNumberOfCollections(),HC);
  
  return;
  
}

void IonSD::ConsolidateHits() {

 label:
  
  for(unsigned int i=0;i<HC->entries();i++) {
    for(unsigned int j=i+1;j<HC->entries();j++) {

      Ion_Hit* hit1 = (Ion_Hit*)HC->GetHit(i);
      Ion_Hit* hit2 = (Ion_Hit*)HC->GetHit(j);

      if(
	 ((hit1->IsProjectile() && hit2->IsProjectile()) || (hit1->IsRecoil() && hit2->IsRecoil()))
	 
	 &&
	 
	 ((hit1->IsRing() && hit2->IsRing()) || (hit1->IsSector() && hit2->IsSector()))
	 ) {

	hit1->SetEdep(hit1->GetEdep()+hit2->GetEdep());
	
	std::vector<Ion_Hit*>* vec = HC->GetVector();
	vec->erase(vec->begin()+j);

	goto label;

      }

    }
  }
  
  return;
  
}

void IonSD::CombineRings() {

  for(unsigned int i=0;i<HC->entries();i++) {
    for(unsigned int j=i+1;j<HC->entries();j++) {

      Ion_Hit* hit1 = (Ion_Hit*)HC->GetHit(i);
      Ion_Hit* hit2 = (Ion_Hit*)HC->GetHit(j);

      if(hit1->IsRing() && hit2->IsRing() && hit1->GetRing() == hit2->GetRing()
	 && hit1->GetDetector() == hit2->GetDetector()) {
	
	hit1->SetEdep(hit1->GetEdep()+hit2->GetEdep());
	
	hit1->SetIsProjectile();
	hit1->SetIsRecoil();
	
	std::vector<Ion_Hit*>* vec = HC->GetVector();
	vec->erase(vec->begin()+j);
	
      }

    }
  }
  
  return;
}
