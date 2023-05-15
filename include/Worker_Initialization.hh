#ifndef Worker_Initialization_h
#define Worker_Initialization_h 1

#include "G4UserWorkerInitialization.hh"
#include "G4String.hh"

class Worker_Initialization : public G4UserWorkerInitialization {
  
public:

  Worker_Initialization(G4String n) {filename = n;}
  ~Worker_Initialization() {;}

private:

  void WorkerStart() const; 

  G4String filename;
  
};
  
#endif
