#ifndef Physics_List_h
#define Physics_List_h 1

#include "G4VUserPhysicsList.hh"

class Physics_List : public G4VUserPhysicsList {

public:
   
  Physics_List();
  ~Physics_List();

  void ConstructProcess();
  void ConstructParticle();

};

#endif
