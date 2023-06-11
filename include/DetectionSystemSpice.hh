#ifndef DETECTIONSYSTEMSPICE_HH
#define DETECTIONSYSTEMSPICE_HH

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

#ifndef AL_COL
#define AL_COL 0.5, 0.5, 0.5
#endif
#define PEEK_COL 0.5, 0.5, 0.0

class DetectionSystemSpice
{
public:
	DetectionSystemSpice(G4bool check);
	~DetectionSystemSpice();

	//------------------------------------------------//
	// logical and physical volumes
	//------------------------------------------------//
public:
	G4int BuildPlace(G4LogicalVolume* ExpHallLog);


private:

// 	G4ThreeVector GetDirectionXYZ(G4double theta, G4double phi);

	G4LogicalVolume* fDetectorMountLog;
	G4LogicalVolume* fAnnularClampLog;
	std::vector<G4LogicalVolume*> fSiliLog;

	G4VPhysicalVolume* fDetectorMountPhys;
	G4VPhysicalVolume* fAnnularClampPhys;

	//--------------------------------------------------------//
	// SPICE physical properties
	// OBS: crystal properties are public, others are private
	//--------------------------------------------------------//
	G4String fWaferMaterial;
	G4String fDetectorMountMaterial;
	G4String fAnnularClampMaterial;

	// ----------------------------
	// Dimensions of Detector Mount
	// ----------------------------
	G4double fDetectorMountLength;
	G4double fDetectorMountWidth;
	G4double fDetectorMountThickness;
	G4double fDetectorMountInnerRadius;
	G4double fDetectorMountLipRadius;
	G4double fDetectorMountLipThickness;
	G4double fDetectorMountAngularOffset;
	G4double fDetectorToTargetDistance;
	G4double fDetectorThickness;

	// ---------------------------
	// Dimensions of Annular Clamp
	// ---------------------------
	G4double fAnnularClampThickness;
	G4double fAnnularClampLength;
	G4double fAnnularClampWidth;
	G4double fAnnularClampPlaneOffset;

	//-----------------------------
	// copy numbers
	//-----------------------------
	G4int fDetectorMountCopyNumber;
	G4int fAnnularClampCopyNumber;

	//-----------------------------//
	// parameters for the annular  //
	// planar detector crystal     //
	//-----------------------------//
	G4double 	fSiDetCrystalThickness;
	G4double 	fSiDetCrystalOuterDiameter;
	G4double 	fSiDetCrystalInnerDiameter;
	G4double 	fSiDetCrystalActiveInnerDiameter;
	G4double 	fSiDetCrystalActiveOuterDiameter;
	G4double 	fSiDetRingRadii;
	G4int 		fSiDetRadialSegments;
	G4int 		fSiDetPhiSegments;

	//-------------------------------//
	// parameters for the guard ring //
	//-------------------------------//
	G4double 	fSiDetGuardRingInnerDiameter;
	G4double 	fSiDetGuardRingOuterDiameter;

  //Check overlaps
  G4bool check;
	
	//------------------------------------------------//
	// internal methods in Build()
	//------------------------------------------------//
private:
	G4int 	BuildPlaceSiliconWafer(G4LogicalVolume* ExpHallLog);
	void 	BuildDetectorMount();
	void 	BuildAnnularClamps();   
	void PlaceDetectorMount(G4LogicalVolume* ExpHallLog); 
	void PlaceAnnularClamps(G4LogicalVolume* ExpHallLog);
};

#endif
