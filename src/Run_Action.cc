#include "Run_Action.hh"
#include "Primary_Generator.hh"
#include "Event_Action.hh"
#include "Tracking_Action.hh"
#include "IonSD.hh"
#include "GammaSD.hh"

#include "G4Run.hh"
#include "G4MTRunManager.hh"
#include "G4WorkerRunManager.hh"
#include "G4SDManager.hh"
#include "G4Threading.hh"

Run_Action::Run_Action() {

  messenger = new Run_Action_Messenger(this);

  fname = "output.dat";
  dname = "info.dat";

  owc = false;
}

Run_Action::~Run_Action() {

  delete messenger;
  
}

void Run_Action::BeginOfRunAction(const G4Run* aRun) {
  
  if(IsMaster())
    return;
  
  G4WorkerRunManager* Rman = (G4WorkerRunManager*)G4MTRunManager::GetRunManager();

  Primary_Generator* gen = (Primary_Generator*)Rman->GetUserPrimaryGeneratorAction();
  gen->Update();
  Primary_Generator::MODE mode = gen->GetMode();
  
  Tracking_Action* trkAct = (Tracking_Action*)Rman->GetUserTrackingAction();
  trkAct->SetMode(mode);
  if(gen->IsSimpleSource())
    trkAct->SetIsSimpleSource();

  Run* run = static_cast<Run*>(Rman->GetNonConstCurrentRun());
  run->SetPrimaryGenerator(gen);

  if(owc)
    run->OWC();

  G4int threadID = G4Threading::G4GetThreadId();
  
  //Must have three letter extension for this to work (i.e. output.dat)
  G4String name = fname.substr(0,fname.length()-4) + std::to_string(threadID)
    + fname.substr(fname.length()-4,fname.length());

  //run->SetOutputFileName(name);
  run->SetOutputFile(fopen(name.c_str(),"wb"));

  //Must have three letter extension for this to work (i.e. output.dat)
  name = dname.substr(0,dname.length()-4) + std::to_string(threadID)
    + dname.substr(dname.length()-4,dname.length());
  
  //run->SetDiagnosticsFileName(name);
  run->SetDiagnosticsFile(fopen(name.c_str(),"wb"));

  G4int nEvents = aRun->GetNumberOfEventToBeProcessed();
  
  Event_Action* evtAct = (Event_Action*)Rman->GetUserEventAction();
  evtAct->SetNEvents(nEvents);
  evtAct->SetPerEvent(nEvents);

  G4SDManager* SDman = G4SDManager::GetSDMpointer();

  G4String message = "";
  switch(mode) {
    case Primary_Generator::MODE::Scattering: {

      IonSD* iSD = (IonSD*)SDman->FindSensitiveDetector("IonTracker");
      iSD->SetProjectileName(gen->GetProjectileName());
      iSD->SetRecoilName(gen->GetRecoilName());

      message = "Simulating " + std::to_string(nEvents) + " two-body scattering events";
      
      break;
  
    }
    case Primary_Generator::MODE::Source: {

      GammaSD* gSD = (GammaSD*)SDman->FindSensitiveDetector("GammaTracker");
      gSD->SetTrackingAction(trkAct);
      
      message = "Simulating " + std::to_string(nEvents) + " source gamma-ray events";
      
      break;

    }
    case Primary_Generator::MODE::Full: {

      IonSD* iSD = (IonSD*)SDman->FindSensitiveDetector("IonTracker");
      iSD->SetProjectileName(gen->GetProjectileName());
      iSD->SetRecoilName(gen->GetRecoilName());
      
      GammaSD* gSD = (GammaSD*)SDman->FindSensitiveDetector("GammaTracker");
      gSD->SetTrackingAction(trkAct);

      trkAct->SetProjectileName(gen->GetProjectileName());
      
      message = "Simulating " + std::to_string(nEvents) + " full CoulEx events";
      
      break;

    }
  } 
  
  if(!threadID)
    std::cout << "\nStarting run!\n" << message << std::endl;
  else
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
  return;
}

void Run_Action::EndOfRunAction(const G4Run* aRun) {

  if(!IsMaster())
    return;

  G4int nEvents = aRun->GetNumberOfEventToBeProcessed();
  G4cout << "Event " << nEvents << " (100%)\nRun Complete!\n\nMerging output files... " << G4endl;
  
  G4int num = ((G4MTRunManager*)G4MTRunManager::GetRunManager())->GetNumberOfThreads();
  
  std::ofstream dataFile(fname.c_str(),std::ios::out);
  std::ofstream infoFile(dname.c_str(),std::ios::out);
  
  for(G4int i=0;i<num;i++) {

    G4String name = fname.substr(0,fname.length()-4) + std::to_string(i)
      + fname.substr(fname.length()-4,fname.length());
    
    std::ifstream inFileF(name.c_str(),std::ios::in);

    std::copy((std::istreambuf_iterator<char>(inFileF)),std::istreambuf_iterator<char>(),
	      std::ostreambuf_iterator<char>(dataFile));

    inFileF.close();
    std::remove(name.c_str());

    name = dname.substr(0,dname.length()-4) + std::to_string(i)
      + dname.substr(dname.length()-4,dname.length());

    std::ifstream inFileD(name.c_str(),std::ios::in);

    std::copy((std::istreambuf_iterator<char>(inFileD)),std::istreambuf_iterator<char>(),
	      std::ostreambuf_iterator<char>(infoFile));

    inFileD.close();
    std::remove(name.c_str());
    
    G4cout << "\r File " << i+1 << "/" << num << " merged" << std::flush;
    
  }

  dataFile.close();
  infoFile.close();
  
  G4cout << "\nDone!" << G4endl;
  
  return;
}
