#include "Run.hh"
#include "Ion_Hit.hh"
#include "Gamma_Hit.hh"
#include "Suppressor_Hit.hh"
#include "Data_Format.hh"
#include "G4SystemOfUnits.hh"

Run::Run() {

  gen = NULL;
  output = NULL;
  diagnostics = NULL;

  owc = false;
  
}

void Run::RecordEvent(const G4Event* evt) {

  RawData data;
  G4int nS = 0;
  G4int nT = 0;

  G4bool pFlagDS = false;
  G4bool pFlagUS = false;
  G4bool rFlag = false;

  G4bool sup[64] = {false,false,false,false,false,false,false,false,false,false,false,false,false,
		    false,false,false,false,false,false,false,false,false,false,false,false,false,
		    false,false,false,false,false,false,false,false,false,false,false,false,false,
		    false,false,false,false,false,false,false,false,false,false,false,false,false,
		    false,false,false,false,false,false,false,false,false,false,false,false};
  
  G4HCofThisEvent* HCE = evt->GetHCofThisEvent();
  for(G4int i=0;i<HCE->GetNumberOfCollections();i++) {

    G4String name = HCE->GetHC(i)->GetName();
    if(name == "ionCollection") {

      Ion_Hit_Collection* iHC = (Ion_Hit_Collection*)HCE->GetHC(i);  
      for(unsigned int j=0;j<iHC->entries();j++) {

	if(nS > 4) {
	  G4cout << "Too many ion hits!" << G4endl;
	  break;
	}
    
        Ion_Hit* hit = (Ion_Hit*)iHC->GetHit(j);
	G4int det = hit->GetDetector();
	
	if(hit->IsProjectile()) {
	  if(det)
	    pFlagDS = true;
	  else
	    pFlagUS = true;
	}
	if(hit->IsRecoil())
	  rFlag = true;
	
        G4ThreeVector pos = hit->GetPos();
	data.sData[nS]= {det,hit->GetRing(),hit->GetSector(), hit->GetEdep()/MeV,
			 pos.x()/cm,pos.y()/cm,pos.z()/cm,
	                 hit->IsProjectile(),hit->IsRecoil()};
	 
	nS++;

      }
    }
    else if(name == "gammaCollection") {

      Gamma_Hit_Collection* gHC = (Gamma_Hit_Collection*)HCE->GetHC(i);
      for(unsigned int j=0;j<gHC->entries();j++) {

	if(nT > 99) {
	  G4cout << "Too many gamma hits!" << G4endl;
	  break;
	}
    
        Gamma_Hit* hit = (Gamma_Hit*)gHC->GetHit(j);
        G4ThreeVector pos = hit->GetPos();

	data.tData[nT] = {hit->GetDetector(),hit->GetSegment(),hit->GetEdep()/keV,
			  pos.x()/cm,pos.y()/cm,pos.z()/cm,hit->IsFEP(),hit->IsProjFEP(),false};

	nT++;

      }
    }
    else if(name == "suppressorCollection") {

      Suppressor_Hit_Collection* sHC = (Suppressor_Hit_Collection*)HCE->GetHC(i);
      for(unsigned int j=0;j<sHC->entries();j++) {
    
        Suppressor_Hit* hit = (Suppressor_Hit*)sHC->GetHit(j);
	G4int det = hit->GetDetector();

	sup[det-1] = true;

      }
    }
  }
 
  for(G4int i=0;i<nT;i++)
    data.tData[i].sup = sup[data.tData[i].det-1];
  
  G4int num = evt->GetEventID();

  INFO info;
  info.evtNum = num;
  info.indexP = gen->GetProjectileIndex();
  info.indexR = gen->GetRecoilIndex();

  info.beamEn = gen->GetBeamEnergy();
  info.thetaCM = gen->GetThetaCM();
  
  info.projDS = pFlagDS;
  info.projUS = pFlagUS;
  info.rec = rFlag;
  
  Header header;
  header.evtNum = num;
  header.nSdata = nS;
  header.nTdata = nT;

  fwrite(&info,info.bytes(),1,diagnostics);
  G4Run::RecordEvent(evt);
  
  if(nS == 0 && nT == 0)
    return;

  if(owc && (nS == 0 || nT == 0))
    return;
  
  fwrite(&header,header.bytes(),1,output);
  fwrite(&data.sData,sizeof(S3Data),nS,output);
  fwrite(&data.tData,sizeof(TigressData),nT,output);
  
  return;
}

void Run::Merge(const G4Run* aRun) {

  const Run* run = static_cast<const Run*>(aRun);
  fclose(run->GetOutputFile());
  fclose(run->GetDiagnosticsFile());
  
  G4Run::Merge(aRun);
  
  return;
}
