#ifndef Run_h
#define Run_h 1

#include "G4Run.hh"
#include "G4Event.hh"
#include "Data_Format.hh"
#include "Primary_Generator.hh"

class Run : public G4Run {
  
public:

  Run();
  ~Run() {;}

  G4String GetOutputFileName() {return fname;}
  FILE* GetOutputFile() {return output;}

  G4String GetDiagnosticsFileName() {return dname;}
  FILE* GetDiagnosticsFile() {return diagnostics;}

  void SetOutputFileName(G4String n) {fname = n;}
  void SetOutputFile(FILE* f) {output = f;}

  void SetDiagnosticsFileName(G4String n) {dname = n;}
  void SetDiagnosticsFile(FILE* f) {diagnostics = f;}

private:

  void RecordEvent(const G4Event* evt);
  void Merge(const G4Run* run);

  Header header;
  RawData data;
  INFO info;

  Primary_Generator* gen;

  G4String fname;
  FILE* output;

  G4String dname;
  FILE* diagnostics;

  G4bool owc;
  
};
  
#endif
