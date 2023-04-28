#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"

#include "Detector_Construction.hh"
#include "Primary_Generator.hh"
#include "Physics_List.hh"
#include "Run_Action.hh"
#include "Event_Action.hh"
#include "Tracking_Action.hh"

int main(int argc, char** argv) {
  
  //Construct the run manager
  G4RunManager* runManager = new G4RunManager();
  
  //Set mandatory classes
  runManager->SetUserInitialization(new Detector_Construction());
  runManager->SetUserInitialization(new Physics_List());
  runManager->SetUserAction(new Primary_Generator());

  //Set optional classes
  runManager->SetUserAction(new Run_Action());
  runManager->SetUserAction(new Event_Action());
  runManager->SetUserAction(new Tracking_Action());

  //Initialize
  runManager->Initialize();

  G4UImanager* UI = G4UImanager::GetUIpointer();
  if(argc == 1) { //Interactive Mode

    
    G4VisManager* visManager = new G4VisExecutive();
    visManager->Initialize();
       
    UI->ApplyCommand("/vis/open OGLSX");
    UI->ApplyCommand("/vis/drawVolume");
    UI->ApplyCommand("/vis/scene/add/trajectories");
    UI->ApplyCommand("/vis/viewer/zoom 2");
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
