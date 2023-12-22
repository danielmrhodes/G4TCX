#include "Action_Initialization.hh"
#include "Worker_Initialization.hh"
#include "Detector_Construction.hh"
#include "Physics_List.hh"

#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"
#include "G4VisManager.hh"
#include "G4VisExecutive.hh"

int main(int argc, char** argv) {
  
  //Construct the run manager
  G4MTRunManager* runManager = new G4MTRunManager();
  runManager->SetNumberOfThreads(4);

  //Provide mandatory classes
  runManager->SetUserInitialization(new Detector_Construction());
  runManager->SetUserInitialization(new Physics_List());
  runManager->SetUserInitialization(new Action_Initialization());

  G4UImanager* UI = G4UImanager::GetUIpointer();
  if(argc == 1) { //Interactive Mode
    
    UI->ApplyCommand("/Geometry/Tigress/Construct");
    UI->ApplyCommand("/Geometry/S3/Construct");
    UI->ApplyCommand("/Geometry/Target/Construct");
    UI->ApplyCommand("/Geometry/CheckOverlaps");
    
    runManager->SetNumberOfThreads(1);
    runManager->Initialize();
    
    G4VisManager* visManager = new G4VisExecutive();
    visManager->Initialize();
    
    UI->ApplyCommand("/vis/open OGLSX");
    UI->ApplyCommand("/vis/drawVolume");
    UI->ApplyCommand("/vis/scene/add/trajectories");
    UI->ApplyCommand("/vis/viewer/zoom 6");
    UI->ApplyCommand("/vis/viewer/zoom 2");
    UI->ApplyCommand("/vis/viewer/set/viewpointThetaPhi 90 -22.50");
    
    
    G4UIsession* session = new G4UIterminal(new G4UItcsh());
    session->SessionStart();
    
    delete session;
    delete visManager;
    
  }
  else { //Batch Mode
    
    G4String filename = argv[1];

    //Worker threads also need macro commands
    runManager->SetUserInitialization(new Worker_Initialization(filename));

    //Manually parse macro file for master thread commands
    std::ifstream file(filename.c_str());
    G4String line;
    while(std::getline(file,line)) {

      if(G4StrUtil::contains(line,"/run/beamOn"))
	break;

      if(G4StrUtil::contains(line,"/Geometry") || G4StrUtil::contains(line,"/Output")
	 || G4StrUtil::contains(line,"/run"))
	UI->ApplyCommand(line);
      
    }

    runManager->Initialize();
    UI->ApplyCommand(line); //beamOn command
    
  }
  
  delete runManager;
  
  return 0;
  
}
