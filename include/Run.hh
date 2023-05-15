#ifndef Run_h
#define Run_h 1

#include "Data_Format.hh"
#include "Primary_Generator.hh"

#include "G4Run.hh"
#include "G4Event.hh"

class Run : public G4Run {
  
public:

  Run();
  virtual ~Run() {;}

  virtual void RecordEvent(const G4Event* evt);
  virtual void Merge(const G4Run* run);
  
  FILE* GetOutputFile() const {return output;}
  FILE* GetDiagnosticsFile() const {return diagnostics;}
  
  void SetOutputFile(FILE* f) {output = f;}
  void SetDiagnosticsFile(FILE* f) {diagnostics = f;}

  void SetPrimaryGenerator(Primary_Generator* gn) {gen = gn;}
  void OWC() {owc = true;}

private:
  
  Primary_Generator* gen;
  G4bool owc;

  FILE* output;
  FILE* diagnostics;

};
  
#endif
