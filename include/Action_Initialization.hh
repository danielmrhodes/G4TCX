#ifndef Action_Initialization_h
#define Action_Initialization_h 1

#include "G4VUserActionInitialization.hh"

class Action_Initialization : public G4VUserActionInitialization {
  
public:
   
  Action_Initialization() {;}
  ~Action_Initialization() {;}
  
private:

  void Build() const;
  void BuildForMaster() const;
  
};

#endif

