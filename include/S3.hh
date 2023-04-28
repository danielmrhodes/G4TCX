#ifndef S3_h
#define S3_h 1

#include "G4Material.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4Tubs.hh"
#include "IonSD.hh"

class S3 {

public:

  S3(G4bool make_sensitive);
  ~S3();

  void Placement(G4LogicalVolume* world, G4double USoff, G4double DSoff, G4bool check = false);
  

private:

  int nRings;
  int nSectors;
  
  G4double innerRadius;
  G4double outerRadius;
  G4double thickness;

  IonSD* TrackerIon;
  
};

#endif
