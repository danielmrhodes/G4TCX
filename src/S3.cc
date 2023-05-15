#include "S3.hh"

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4Color.hh"
#include "G4VisAttributes.hh"
#include "G4SDManager.hh"
#include "G4NistManager.hh"
#include "G4RunManager.hh"
#include "G4Box.hh"
#include "G4SubtractionSolid.hh"

S3::S3() {

  innerRadius = 1.1*cm;
  outerRadius = 3.5*cm;
  thickness = 300.0*um;

  nRings = 24;
  nSectors = 32;
  
}

void S3::Placement(G4LogicalVolume* world, G4double USoff, G4double DSoff, G4bool check) {
  
  //Visualization
  G4VisAttributes* vis1 = new G4VisAttributes(G4Colour::Yellow());
  G4VisAttributes* vis2 = new G4VisAttributes(G4Colour::Red());
  G4VisAttributes* vis3 = new G4VisAttributes(G4Colour::Brown());
  G4VisAttributes* vis4 = new G4VisAttributes(G4Colour::White());
  vis1->SetVisibility(true);
  vis1->SetForceSolid(true);
  vis2->SetVisibility(true);
  vis2->SetForceSolid(true);
  vis3->SetVisibility(true);
  vis3->SetForceSolid(true);
  vis4->SetVisibility(true);
  vis4->SetForceSolid(true);

  G4NistManager* nist = G4NistManager::Instance();
  
  //S3 material
  G4Material* mat = nist->FindOrBuildMaterial("G4_Si");
    
  //Radial and angular spacing
  G4double dr = (outerRadius - innerRadius)/(double)nRings;
  G4double dphi = 2.0*pi/(double)nSectors;
  
  for(G4int det=0;det<2;det++) {
    for(G4int ring=0;ring<nRings;ring++) {
      for(G4int sec=0;sec<nSectors;sec++) {
      
        G4double in = innerRadius + ring*dr;

        G4Tubs* segs = new G4Tubs("SegS",in,in+dr,thickness/2.0,(sec-0.5)*dphi,dphi);

	G4int copy = det*10000 + (ring+1)*100;
	if(sec<9)
	  copy += 9-sec; 
	else
	  copy += 32-(sec-9);
	
	G4String nameLV = "S3Log" + std::to_string(copy);
        G4LogicalVolume* segl = new G4LogicalVolume(segs,mat,nameLV);
	
	if(ring%2) {
	  if(sec%2) {
	    segl->SetVisAttributes(vis1);
	  }
	  else {
	    segl->SetVisAttributes(vis2);
	  }
	}
	else {
	  if(sec%2) {
	    segl->SetVisAttributes(vis2);
	  }
	  else {
	    segl->SetVisAttributes(vis1);
	  }
	}
	
	if(det==0) //Upstream 
	  new G4PVPlacement(0,G4ThreeVector(0,0,-USoff),segl,"S3",world,false,copy,check);
	else //Downstream
	  new G4PVPlacement(0,G4ThreeVector(0,0,DSoff),segl,"S3",world,false,copy,check);
	
      }
    }
  }

  //Fill inner radius with lead to remove weird Si events
  G4Material* lead = nist->FindOrBuildMaterial("G4_Pb");
  G4Tubs* lead_diskS = new G4Tubs("LeadDiskS",0.0*mm,innerRadius,thickness/2.0,0.0*deg,360.0*deg);
  G4LogicalVolume* lead_diskL = new G4LogicalVolume(lead_diskS,lead,"LeadDiskL");
  new G4PVPlacement(0,G4ThreeVector(0,0,DSoff),lead_diskL,"LeadDisk1",world,false,1,check);
  new G4PVPlacement(0,G4ThreeVector(0,0,-USoff),lead_diskL,"LeadDisk0",world,false,0,check);
   
  //Plastic material for holders and connectors
  G4Material* delrin = nist->FindOrBuildMaterial("G4_POLYOXYMETHYLENE");

  //Make pcb holder
  G4double xy = outerRadius + 1.5*cm;
  G4Box* box = new G4Box("box",xy,xy,1.1*thickness/2.0);
  G4Box* box1 = new G4Box("box1",2.0*cm,2.0*cm,1.2*thickness/2.0);
  G4Tubs* tub = new G4Tubs("tub",0,outerRadius,1.2*thickness/2.0,0,2.0*pi);
  
  G4SubtractionSolid* tmp = new G4SubtractionSolid("tmp",box,tub);

  G4ThreeVector vec(0,xy + 3.0*cm,0);
  vec.rotateZ(pi/4.0);

  G4RotationMatrix rot = G4RotationMatrix::IDENTITY;
  rot.rotateZ(45.0*deg);

  G4SubtractionSolid* tmp1 = new G4SubtractionSolid("tmp1",tmp,box1,&rot,vec);

  vec.rotateZ(pi/2.0);
  G4SubtractionSolid* tmp2 = new G4SubtractionSolid("tmp2",tmp1,box1,&rot,vec);

  vec.rotateZ(pi/2.0);
  G4SubtractionSolid* tmp3 = new G4SubtractionSolid("tmp3",tmp2,box1,&rot,vec);

  vec.rotateZ(pi/2.0);
  G4SubtractionSolid* pcb_holderS = new G4SubtractionSolid("pcb_hS",tmp3,box1,&rot,vec);
  
  G4LogicalVolume* pcb_holderL = new G4LogicalVolume(pcb_holderS,delrin,"pcb_hL");
  pcb_holderL->SetVisAttributes(vis3);

  new G4PVPlacement(0,G4ThreeVector(0,0,DSoff),pcb_holderL,"pcbH1",world,false,1,check);
  new G4PVPlacement(0,G4ThreeVector(0,0,-USoff),pcb_holderL,"pcbH0",world,false,0,check);
  
  /*
  //Make connectors between pcb holders
  G4Tubs* conS = new G4Tubs("conS",0,0.394*cm,(DSoff+USoff-1.1*thickness)/2.0,0,2.0*pi);
  G4LogicalVolume* conL = new G4LogicalVolume(conS,delrin,"conL");
  conL->SetVisAttributes(vis4);

  G4ThreeVector vec1(0,xy,0);
  
  vec1.rotateZ(pi/4.0);
  new G4PVPlacement(0,vec1,conL,"con1",world,false,1,check);

  vec1.rotateZ(pi/2.0);
  new G4PVPlacement(0,vec1,conL,"con2",world,false,2,check);

  vec1.rotateZ(pi/2.0);
  new G4PVPlacement(0,vec1,conL,"con3",world,false,3,check);

  vec1.rotateZ(pi/2.0);
  new G4PVPlacement(0,vec1,conL,"con4",world,false,4,check);
  */
  
  return;
}
