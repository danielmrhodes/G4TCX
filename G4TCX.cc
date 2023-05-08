#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"
#include "Action_Initialization.hh"
#include "Detector_Construction.hh"
#include "Physics_List.hh"

int main(int argc, char** argv) {
  
  //Construct the run manager
  G4MTRunManager* runManager = new G4MTRunManager();
  runManager->SetNumberOfThreads(1);
  
  //Set mandatory classes
  runManager->SetUserInitialization(new Detector_Construction());
  runManager->SetUserInitialization(new Physics_List());

  runManager->SetUserInitialization(new Action_Initialization());

  //Initialize
  runManager->Initialize();

  G4UImanager* UI = G4UImanager::GetUIpointer();
  if(argc == 1) { //Interactive Mode

    
    G4VisManager* visManager = new G4VisExecutive();
    visManager->Initialize();
       
    UI->ApplyCommand("/vis/open OGLSX");
    UI->ApplyCommand("/vis/drawVolume");
    //UI->ApplyCommand("/vis/scene/add/trajectories");
    //UI->ApplyCommand("/vis/viewer/zoom 2");
    //UI->ApplyCommand("/vis/viewer/set/viewpointThetaPhi 70 20");
    UI->ApplyCommand("/Geometry/S3/Construct");
    UI->ApplyCommand("/Geometry/Target/Construct");
    UI->ApplyCommand("/vis/viewer/zoom 4");
    UI->ApplyCommand("/Geometry/Update");
    
    G4UIsession* session = new G4UIterminal(new G4UItcsh());
    session->SessionStart();
    
    delete session;
    delete visManager;
    
  }
  else { //Batch Mode

    G4String command = "/control/execute ";
    G4String filename = argv[1];
   
    UI->ApplyCommand(command+filename);
    
  }

  delete runManager;
  
  return 0;
}
