#include "Primary_Generator.hh"
#include "Run_Action.hh"
#include "Event_Action.hh"
#include "Tracking_Action.hh"
#include "IonSD.hh"
#include "GammaSD.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
#include "G4String.hh"

Run_Action::Run_Action() {}
Run_Action::~Run_Action() {}

void Run_Action::BeginOfRunAction(const G4Run* run) {
 
  nEvents = run->GetNumberOfEventToBeProcessed();
  G4RunManager* Rman = G4RunManager::GetRunManager();

  Primary_Generator* gen = (Primary_Generator*)Rman->GetUserPrimaryGeneratorAction();
  gen->Update();
  Primary_Generator::MODE mode = gen->GetMode();
  
  Tracking_Action* trkAct = (Tracking_Action*)Rman->GetUserTrackingAction();
  trkAct->SetMode(mode);
  if(gen->IsSimpleSource())
    trkAct->SetIsSimpleSource();

  Event_Action* evtAct = (Event_Action*)Rman->GetUserEventAction();
  evtAct->SetPrimaryGenerator(gen);

  G4String name = evtAct->GetOutputFileName();
  evtAct->SetOutputFile(fopen(name.c_str(),"wb"));

  //Must have .dat extension fot this to work
  G4String dname = name.substr(0,name.length()-4) + "-info.dat";
  evtAct->SetDiagnosticsFileName(dname);
  evtAct->SetDiagnosticsFile(fopen(evtAct->GetDiagnosticsFileName().c_str(),"wb"));

  evtAct->SetNEvents(nEvents);
  evtAct->SetPerEvent(nEvents);

  G4SDManager* SDman = G4SDManager::GetSDMpointer();

  G4cout << "\nStarting run!" << G4endl; 
  switch(mode) {
    case Primary_Generator::MODE::Scattering: {

      IonSD* iSD = (IonSD*)SDman->FindSensitiveDetector("IonTracker");
      iSD->SetProjectileName(gen->GetProjectileName());
      iSD->SetRecoilName(gen->GetRecoilName());

      G4cout << "Simulating " << nEvents << " two-body scattering events" << G4endl;
      break;
  
    }
    case Primary_Generator::MODE::Source: {

      GammaSD* gSD = (GammaSD*)SDman->FindSensitiveDetector("GammaTracker");
      gSD->SetTrackingAction(trkAct);
      
      G4cout << "Simulating " << nEvents << " source gamma-ray events" << G4endl;
      break;

    }
    case Primary_Generator::MODE::Full: {

      IonSD* iSD = (IonSD*)SDman->FindSensitiveDetector("IonTracker");
      iSD->SetProjectileName(gen->GetProjectileName());
      iSD->SetRecoilName(gen->GetRecoilName());
      
      GammaSD* gSD = (GammaSD*)SDman->FindSensitiveDetector("GammaTracker");
      gSD->SetTrackingAction(trkAct);

      trkAct->SetProjectileName(gen->GetProjectileName());
      
      G4cout << "Simulating " << nEvents << " full CoulEx events" << G4endl;
      break;

    }
  }

  return;
}

void Run_Action::EndOfRunAction(const G4Run*) {

  G4RunManager* Rman = G4RunManager::GetRunManager();
  Event_Action* evtAct = (Event_Action*)Rman->GetUserEventAction();
  fclose(evtAct->GetOutputFile());
  fclose(evtAct->GetDiagnosticsFile());

  G4cout << "Event " << nEvents << " (100%)\nRun Complete!" << G4endl;

  return;
}
