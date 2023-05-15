#include "Worker_Initialization.hh"
#include "globals.hh"
#include "G4UImanager.hh"

void Worker_Initialization::WorkerStart() const {

  if(filename == "")
    return;
  
  G4UImanager* UI = G4UImanager::GetUIpointer();

  //Manually parse macro file for worker thread commands
  std::ifstream file(filename.c_str());
  G4String line;
  while(std::getline(file,line)) {

    if(G4StrUtil::contains(line,"/Mode") || G4StrUtil::contains(line,"/Source")
       || G4StrUtil::contains(line,"/Reaction") || G4StrUtil::contains(line,"/Beam")
       || G4StrUtil::contains(line,"/Excitation") || G4StrUtil::contains(line,"/DeorientationEffect"))
      
      UI->ApplyCommand(line);
    
  }
  
  return;
}
