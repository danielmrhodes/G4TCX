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
  dname = "";

  gammaTrigger = 0;
  
  owc = false;
  write_diag = false;
}

Run_Action::~Run_Action() {

  delete messenger;
  
}

void Run_Action::BeginOfRunAction(const G4Run* aRun) {
  
  if(IsMaster()) {

    G4int num = ((G4MTRunManager*)G4MTRunManager::GetRunManager())->GetNumberOfThreads();
    
    std::cout << "\nUsing " << num << " thread";
    if(num > 1)
      std::cout << "s";
    std::cout << " for the simulation" << std::endl;
    
    return;
  }
  
  G4int threadID = G4Threading::G4GetThreadId(); 
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
  run->SetGammaTrigger(gammaTrigger);
  
  //Must have three letter extension for this to work (i.e. xxxx.dat)
  G4String name = fname.substr(0,fname.length()-4) + "-" + std::to_string(threadID)
    + fname.substr(fname.length()-4,fname.length());
  
  run->SetOutputFile(fopen(name.c_str(),"wb"));
  if(owc)
    run->OnlyWriteCoincidences();
  
  if(write_diag) {
    run->WriteDiagnostics();
    
    //Must have three letter extension for this to work (i.e. xxxx.dat)
    if(dname == "")
      name = fname.substr(0,fname.length()-4) + "-info-" + std::to_string(threadID)
	+ fname.substr(fname.length()-4,fname.length());
    else
      name = dname.substr(0,dname.length()-4) + "-" + std::to_string(threadID)
	+ dname.substr(dname.length()-4,dname.length());
    
    run->SetDiagnosticsFile(fopen(name.c_str(),"wb"));

  }
  
  G4int nEvents = aRun->GetNumberOfEventToBeProcessed();
  
  Event_Action* evtAct = (Event_Action*)Rman->GetUserEventAction();
  evtAct->SetNEvents(nEvents);
  evtAct->SetPerEvent(nEvents);

  G4SDManager* SDman = G4SDManager::GetSDMpointer();

  G4String description = "";
  switch(mode) {
    case Primary_Generator::MODE::Scattering: {

      IonSD* iSD = (IonSD*)SDman->FindSensitiveDetector("IonTracker");
      iSD->SetProjectileName(gen->GetProjectileName());
      iSD->SetRecoilName(gen->GetRecoilName());

      description = " two-body scattering events";
      
      break;
  
    }
    case Primary_Generator::MODE::Source: {

      GammaSD* gSD = (GammaSD*)SDman->FindSensitiveDetector("GammaTracker");
      gSD->SetTrackingAction(trkAct);
      
      description = " source gamma-ray events";
      
      break;

    }
    case Primary_Generator::MODE::Full: {

      IonSD* iSD = (IonSD*)SDman->FindSensitiveDetector("IonTracker");
      iSD->SetProjectileName(gen->GetProjectileName());
      iSD->SetRecoilName(gen->GetRecoilName());
      
      GammaSD* gSD = (GammaSD*)SDman->FindSensitiveDetector("GammaTracker");
      gSD->SetTrackingAction(trkAct);

      trkAct->SetProjectileName(gen->GetProjectileName());
      
      description = " full CoulEx events";
      
      break;

    }
  } 
  
  if(!threadID)
    std::cout << "\nStarting run!\nSimulating " << nEvents << description << std::endl;
  else
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
  return;
}

void Run_Action::EndOfRunAction(const G4Run* aRun) {

  if(!IsMaster())
    return;
  
  G4int nEvents = aRun->GetNumberOfEventToBeProcessed();
  std::cout << "Event " << nEvents << " (100%)\nRun Complete!\n\nMerging output files... " << std::endl;

  G4int num = ((G4MTRunManager*)G4MTRunManager::GetRunManager())->GetNumberOfThreads();
  
  std::ofstream dataFile(fname.c_str(),std::ios::out);
  for(G4int i=0;i<num;i++) {

    G4String name = fname.substr(0,fname.length()-4) + "-" + std::to_string(i)
      + fname.substr(fname.length()-4,fname.length());
    
    std::ifstream inFileF(name.c_str(),std::ios::in);

    std::copy((std::istreambuf_iterator<char>(inFileF)),std::istreambuf_iterator<char>(),
	      std::ostreambuf_iterator<char>(dataFile));

    inFileF.close();
    std::remove(name.c_str());
    
    std::cout << "\r Data file " << i+1 << "/" << num << " merged" << std::flush;
    
  }
  dataFile.close();
  
  if(!write_diag) {
    std::cout << "\nDone!" << std::endl;
    return;
  }
  std::cout << "\n";

  if(dname == "")
    dname = fname.substr(0,fname.length()-4) + "-info"
      + fname.substr(fname.length()-4,fname.length());
  
  std::ofstream infoFile(dname.c_str(),std::ios::out);
  for(G4int i=0;i<num;i++) {

    std::string name = dname.substr(0,dname.length()-4) + "-" + std::to_string(i)
      + dname.substr(dname.length()-4,dname.length());

    std::ifstream inFileD(name.c_str(),std::ios::in);
    
    std::copy((std::istreambuf_iterator<char>(inFileD)),std::istreambuf_iterator<char>(),
	      std::ostreambuf_iterator<char>(infoFile));
    
    inFileD.close();
    std::remove(name.c_str());

    std::cout << "\r Diagnostics file " << i+1 << "/" << num << " merged" << std::flush;
  }
  
  infoFile.close();
  
  std::cout << "\nDone!" << std::endl;
  
  return;
}
