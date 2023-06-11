#ifndef APPARATUSTIGRESSSTRUCTURE_HH
#define APPARATUSTIGRESSSTRUCTURE_HH

#include "G4SystemOfUnits.hh" // new version geant4.10 requires units
#include "G4IntersectionSolid.hh"
#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4SubtractionSolid.hh"
#include "G4Tubs.hh"

class G4AssemblyVolume;

///////////////////////////////////////////////////////////////////////
// ApparatusTigressStructure
///////////////////////////////////////////////////////////////////////
class ApparatusTigressStructure
{
public:
    ApparatusTigressStructure(G4bool chk);
    ~ApparatusTigressStructure();

    G4int Build();
    G4int Place(G4LogicalVolume* expHallLog, G4int selector);

private:
    G4LogicalVolume* fSquarePieceLog;
    G4LogicalVolume* fTrianglePieceLog;
    G4LogicalVolume* fRingPieceLog;
    G4LogicalVolume* fRodPieceLog;


    G4AssemblyVolume* fAssemblySquare;
    G4AssemblyVolume* fAssemblyTriangle;
    G4AssemblyVolume* fAssemblyRing;
    G4AssemblyVolume* fAssemblyRod;

private:
    // Materials
    G4String fStructureMaterial;
    G4String fRodMaterial;

    // Dimensions
    G4double fInnerDistanceToSquareFace; // inner radius to the centre of square plate
    G4double fSquareAndTriangleFaceThickness; // thickness of the square and triangle plates
    G4double fSquareHoleLength; // total legth of square hole, from inside structure.
    G4double fSquareHoleRoundedEdgeRadius; // the roundness of the square holes.
    G4double fSquareRodHoleRadius; // the radius of the holes for the steel rods
    G4double fSquareRodHoleInnerRadius; // the radius from the origin to the steel rod
    G4double fSquareRodPositionFromCenterOfSquareFace; // the position the holes are from the origin. This is NOT a radius, this is along x or y
    G4double fSquareRodTotalLength; // the total length of the rods
    G4double fSquareBgoCutLength; // the length of the bgo slot
    G4double fSquareBgoCutWidth; // the width of the bgo slot
    G4double fSquareSquareAngle; // the angle between square faces in a rhombicuboctahedron
    G4double fSquareTriangleAngle; // the angle between a square face and a triangle face in a rhombicuboctahedron
    G4double fTriangleOuterHoleRadius; // a hole radius in the triangle face. Radius to the rounded edge, this is the larger radius.
    G4double fTriangleInnerHoleRadius; // a hole radius in the triangle face. Radius to the square edge, this is the smaller radius.
    G4double fLargeRingFrameInnerRadius; // the inner radius of one of the the large rings that holds up the structure.
    G4double fLargeRingFrameOuterRadius; // the outer radius of one of the the large rings that holds up the structure.
    G4double fLargeRingFrameThickness; // the thickness of one of the the large rings that holds up the structure.
    G4double fLargeRingFrameZPosition; // the positive z distance the ring is from the origin.

    G4double fTriangleThetaAngle;

    G4double fTigressCoords[16][5];
    G4double fAncillaryCoords[8][5];

    G4bool fSurfCheck;

    // Methods
    G4int BuildSquarePiece();
    G4int BuildTrianglePiece();
    G4int BuildLargeRingFrame();
    G4int BuildRodPiece();


    // Shapes
    G4SubtractionSolid* SquarePiece();
    G4SubtractionSolid* TrianglePiece();
    G4Tubs*             LargeRingFrame();
    G4Tubs*             RodPiece();


    // Hole Shapes
    G4SubtractionSolid* SquareWithRoundedCorners();
    G4SubtractionSolid* TruncatedThreeSidedCylinder();
};

#endif
