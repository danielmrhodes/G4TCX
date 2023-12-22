#include "S3.hh"

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4Color.hh"
#include "G4VisAttributes.hh"
#include "G4SDManager.hh"
#include "G4NistManager.hh"
#include "G4RunManager.hh"
#include "G4AssemblyVolume.hh"
#include "G4Box.hh"
#include "G4Cons.hh"
#include "G4Sphere.hh"
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
  G4VisAttributes* vis4 = new G4VisAttributes(G4Colour::Grey());
  G4VisAttributes* vis5 = new G4VisAttributes(G4Colour::White());
  vis1->SetVisibility(true);
  vis1->SetForceSolid(true);
  vis2->SetVisibility(true);
  vis2->SetForceSolid(true);
  vis3->SetVisibility(true);
  vis3->SetForceSolid(true);
  vis4->SetVisibility(true);
  vis4->SetForceSolid(true);
  vis5->SetVisibility(true);

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
  G4Tubs* lead_diskS = new G4Tubs("LeadDiskS",innerRadius - 1.0*mm,innerRadius,thickness/2.0,0.0*deg,360.0*deg);
  G4LogicalVolume* lead_diskL = new G4LogicalVolume(lead_diskS,lead,"LeadDiskL");
  new G4PVPlacement(0,G4ThreeVector(0,0,DSoff),lead_diskL,"LeadDisk",world,false,1,check);
  new G4PVPlacement(0,G4ThreeVector(0,0,-USoff),lead_diskL,"LeadDisk",world,false,0,check);
   
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

  G4RotationMatrix* rotZ = new G4RotationMatrix(G4RotationMatrix::IDENTITY);
  rotZ->rotateZ(22.5*deg);
  
  new G4PVPlacement(rotZ,G4ThreeVector(0,0,DSoff),pcb_holderL,"pcbH",world,false,1,check);
  new G4PVPlacement(rotZ,G4ThreeVector(0,0,-USoff),pcb_holderL,"pcbH",world,false,0,check);
  
  //Target Chamber
  G4Material* al = nist->FindOrBuildMaterial("G4_Al");
  G4double chamber_outter_radius = 10.0*cm;
  G4double chamber_thickness = 1.6*mm;
  
  G4Sphere* chamberS = new G4Sphere("chamberS",chamber_outter_radius - chamber_thickness,chamber_outter_radius,0.0,2.0*pi,0.0,pi);
  G4LogicalVolume* chamberL = new G4LogicalVolume(chamberS,al,"chamberL");
  chamberL->SetVisAttributes(vis5);
  
  new G4PVPlacement(0,G4ThreeVector(),chamberL,"chamber",world,false,1,check);

  //Upstream mask 
  G4Box* mask_tmp = new G4Box("mask_tmp",5.0*cm,5.0*cm,0.5*mm);
  G4Tubs* cut = new G4Tubs("cut",0.0,0.5*cm,1.0*mm,0.0,2.0*pi);
  G4SubtractionSolid* maskS = new G4SubtractionSolid("maskS",mask_tmp,cut);
  
  G4LogicalVolume* maskL = new G4LogicalVolume(maskS,al,"maskL");
  maskL->SetVisAttributes(vis4);
  
  new G4PVPlacement(rotZ,G4ThreeVector(0,0,-USoff - 2.0*cm),maskL,"mask",world,false,1,check);

  ///Everything below here is not very accurate
  
  //Holder for S3 detectors//
  G4double holder_width = 3.0*mm;
  G4Box* box2 = new G4Box("box2",3.0*cm,3.0*cm,holder_width);
  G4Tubs* tub2 = new G4Tubs("tub2",0.0,2.0*cm,5.0*mm,0.0,2.0*pi);
  G4SubtractionSolid* square_pieceS = new G4SubtractionSolid("square_piece",box2,tub2);

  G4LogicalVolume* square_pieceL = new G4LogicalVolume(square_pieceS,al,"square_piece");
  square_pieceL->SetVisAttributes(vis4);

  G4RotationMatrix* rotXY = new G4RotationMatrix(G4RotationMatrix::IDENTITY);
  rotXY->rotateX(90.0*deg);
  rotXY->rotateY(-22.5*deg);

  G4double hoff = 2.0*cm - chamber_outter_radius;
  G4double hy = hoff*cos(22.5*deg);
  G4double hx = hoff*sin(22.5*deg);
  
  G4ThreeVector holder_off(hx,hy,0);
  new G4PVPlacement(rotXY,holder_off,square_pieceL,"square_piece",world,false,1,check);

  G4double rod_width = 0.15*cm;
  G4double rod_length = 1.7*cm;
  G4Box* rodS = new G4Box("rod",rod_width,rod_length,rod_width);
  G4LogicalVolume* rodL = new G4LogicalVolume(rodS,al,"rodL");
  rodL->SetVisAttributes(vis4);

  G4ThreeVector shift(-2.0*cm,0.0,0.0);
  G4AssemblyVolume* holder = new G4AssemblyVolume(rodL,shift,0);

  shift.setX(2.0*cm);
  holder->AddPlacedVolume(rodL,shift,0);

  G4double long_rod_length = 4.0*cm;
  G4Box* long_rodS = new G4Box("long_rod",long_rod_length,rod_width,rod_width);
  G4LogicalVolume* long_rodL = new G4LogicalVolume(long_rodS,al,"long_rodL");
  long_rodL->SetVisAttributes(vis4);

  shift.setX(0.0);
  shift.setY(rod_length + rod_width);
  holder->AddPlacedVolume(long_rodL,shift,0);

  G4double rod3_length = 3.0*cm;
  G4Box* rod3S = new G4Box("rod3S",rod_width,rod3_length,rod_width);
  G4LogicalVolume* rod3L = new G4LogicalVolume(rod3S,al,"rod3L");
  rod3L->SetVisAttributes(vis4);

  shift.setX(long_rod_length + rod_width);
  shift.setY(shift.getY() + rod3_length);
  holder->AddPlacedVolume(rod3L,shift,0);

  shift.setX(-shift.getX());
  holder->AddPlacedVolume(rod3L,shift,0);
  
  hoff += holder_width + rod_length;
  hy = hoff*cos(22.5*deg);
  hx = hoff*sin(22.5*deg);
  
  holder_off.setX(hx);
  holder_off.setY(hy);
  holder_off.setZ(DSoff - 0.5*cm);

  G4RotationMatrix* rotZn = new G4RotationMatrix(G4RotationMatrix::IDENTITY);
  rotZn->rotateZ(-22.5*deg);
  holder->MakeImprint(world,holder_off,rotZn,0,check);

  holder_off.setZ(-USoff + 0.5*cm);
  holder->MakeImprint(world,holder_off,rotZn,0,check);
  ///////////////////////////

  //Target Ladder (inside chamber)
  G4Tubs* top_pieceS = new G4Tubs("top_pieceS",2.75*cm,3.0*cm,holder_width,0.0,2.0*pi);
  G4LogicalVolume* top_pieceL = new G4LogicalVolume(top_pieceS,al,"top_pieceL");
  top_pieceL->SetVisAttributes(vis4);

  G4double toff2 = chamber_outter_radius - 2.0*cm;
  G4double yoff2 = toff2*cos(22.5*deg);
  G4double xoff2 = toff2*sin(22.5*deg);
  
  G4ThreeVector top_off(xoff2,yoff2,0);
  new G4PVPlacement(rotXY,top_off,top_pieceL,"top_piece",world,false,0,check);

  G4Box* target_rodS = new G4Box("target_rodS",0.25*mm,3.5*cm,0.25*mm);
  G4LogicalVolume* target_rodL = new G4LogicalVolume(target_rodS,al,"target_rodL");
  target_rodL->SetVisAttributes(vis4);

  new G4PVPlacement(rotZ,G4ThreeVector(0.75*cm,0.0,0.0),target_rodL,"target_rod",world,false,0,check);

  G4Tubs* target_rod1S = new G4Tubs("target_rod1S",0.0,rod_width,3.0*cm,0.0,2.0*pi);
  G4LogicalVolume* target_rod1L = new G4LogicalVolume(target_rod1S,al,"target_rod1L");
  target_rod1L->SetVisAttributes(vis4);

  G4double toff3 = 6.0*cm;
  G4double yoff3 = toff3*cos(22.5*deg);
  G4double xoff3 = toff3*sin(22.5*deg);

  new G4PVPlacement(rotXY,G4ThreeVector(xoff3,yoff3,0.0),target_rod1L,"target_rod1",world,false,1,
		    check);

  //Target Ladder (outside chamber)
  G4double top_outter = 32.5*mm;
  G4double top_width = 9.0*mm;
  G4double top_length = 3.5*mm;
  G4Tubs* top1S = new G4Tubs("top1S",top_outter-top_width,top_outter,top_length,0.0,2.0*pi);

  G4Material* ss = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
  G4LogicalVolume* top1L = new G4LogicalVolume(top1S,ss,"top1L");
  top1L->SetVisAttributes(vis4);

  G4double toff1 = chamber_outter_radius + top_length; //total offset
  G4double yoff1 = toff1*cos(22.5*deg);
  G4double xoff1 = toff1*sin(22.5*deg);

  G4ThreeVector pot_off1(xoff1,yoff1,0);
  new G4PVPlacement(rotXY,pot_off1,top1L,"top1",world,false,1,check);
  
  G4double mid_length = 10.0*mm;
  G4Tubs* midS = new G4Tubs("midS",top_outter-top_width-0.5*chamber_thickness,
			    top_outter-top_width,mid_length,0.0,2.0*pi);
  G4LogicalVolume* midL = new G4LogicalVolume(midS,ss,"midL");
  midL->SetVisAttributes(vis4);

  toff1 += mid_length + top_length;
  yoff1 = toff1*cos(22.5*deg);
  xoff1 = toff1*sin(22.5*deg);

  pot_off1.setX(xoff1);
  pot_off1.setY(yoff1);
  new G4PVPlacement(rotXY,pot_off1,midL,"mid",world,false,1,check);
  
  G4double top1_length = 7.5*mm;
  G4Tubs* top2S = new G4Tubs("top2S",top_outter-top_width,top_outter+2.0*mm,top1_length,0.0,2.0*pi);
  G4LogicalVolume* top2L = new G4LogicalVolume(top2S,ss,"top2L");
  top2L->SetVisAttributes(vis4);

  toff1 += mid_length + top1_length;
  yoff1 = toff1*cos(22.5*deg);
  xoff1 = toff1*sin(22.5*deg);

  pot_off1.setX(xoff1);
  pot_off1.setY(yoff1);
  new G4PVPlacement(rotXY,pot_off1,top2L,"top2",world,false,1,check);
  
  G4double last_out = top_outter-top_width;
  G4double last_width = 5.0*mm;
  G4double last_length = 55.0*mm;
  G4Tubs* top_lastS =  new G4Tubs("top_lastS",last_out-last_width,last_out,last_length,0.0,2.0*pi);
  G4LogicalVolume* top_lastL = new G4LogicalVolume(top_lastS,delrin,"top_lastL");
  top_lastL->SetVisAttributes(vis3);

  toff1 += top1_length + last_length;
  yoff1 = toff1*cos(22.5*deg);
  xoff1 = toff1*sin(22.5*deg);
  new G4PVPlacement(rotXY,G4ThreeVector(xoff1,yoff1,0.0),top_lastL,"top_last",world,false,1,check);
  
  G4double plug_length = 17.5*mm;
  G4Tubs* top_plugS = new G4Tubs("top_plugS",0.0,last_out-last_width,plug_length,0.0,2.0*pi);
  G4LogicalVolume* top_plugL = new G4LogicalVolume(top_plugS,delrin,"top_plugL");
  top_plugL->SetVisAttributes(vis3);

  new G4PVPlacement(rotXY,G4ThreeVector(xoff1,yoff1,0.0),top_plugL,"top_plug",world,false,1,check);
  
  //Protrusion at the bottom
  G4double cone_outter = 5.5*cm;
  G4double cone_length = 4.5*cm;
  G4Cons* coneS = new G4Cons("coneS",cone_outter - chamber_thickness,cone_outter,2.0*cm - chamber_thickness,2.0*cm,cone_length,0.0,2.0*pi);
  G4LogicalVolume* coneL = new G4LogicalVolume(coneS,al,"coneL");
  coneL->SetVisAttributes(vis4);

  G4double toff = -chamber_outter_radius - cone_length; //total offset
  G4double yoff = toff*cos(22.5*deg);
  G4double xoff = toff*sin(22.5*deg);
  
  G4ThreeVector pot_off(xoff,yoff,0);
  new G4PVPlacement(rotXY,pot_off,coneL,"cone",world,false,1,check);

  G4double ring_outter = 7.0*cm;
  G4double ring_width = 12.5*mm;
  G4Tubs* ringS = new G4Tubs("ringS",cone_outter,ring_outter,ring_width,0.0,2.0*pi);
  G4LogicalVolume* ringL = new G4LogicalVolume(ringS,al,"ringL");
  ringL->SetVisAttributes(vis4);

  toff -= cone_length + ring_width;
  yoff = toff*cos(22.5*deg);
  xoff = toff*sin(22.5*deg);

  pot_off.setX(xoff);
  pot_off.setY(yoff);
  new G4PVPlacement(rotXY,pot_off,ringL,"ring",world,false,1,check);

  G4Tubs* bottomS = new G4Tubs("bottomS",0.0,cone_outter,0.5*chamber_thickness,0.0,2.0*pi);
  G4LogicalVolume* bottomL = new G4LogicalVolume(bottomS,al,"bottomL");
  bottomL->SetVisAttributes(vis4);

  toff -= ring_width - 0.5*chamber_thickness;
  yoff = toff*cos(22.5*deg);
  xoff = toff*sin(22.5*deg);

  pot_off.setX(xoff);
  pot_off.setY(yoff);
  new G4PVPlacement(rotXY,pot_off,bottomL,"bottom",world,false,1,check);

  G4double block_x = 47.5*mm;
  G4double block_y = 10.0*mm;
  G4double block_z = 12.5*mm;
  G4Box* blockS = new G4Box("blockS",block_x,block_y,block_z);
  G4LogicalVolume* blockL = new G4LogicalVolume(blockS,al,"blockL");
  blockL->SetVisAttributes(vis4);

  toff -= 0.5*chamber_thickness + block_y;
  yoff = toff*cos(22.5*deg);
  xoff = toff*sin(22.5*deg);

  pot_off.setX(xoff);
  pot_off.setY(yoff);
  pot_off.setZ(2.0*block_z);
  new G4PVPlacement(rotZ,pot_off,blockL,"block",world,false,1,check);

  pot_off.setZ(-2.0*block_z);
  new G4PVPlacement(rotZ,pot_off,blockL,"block",world,false,0,check);
  
  //Beam pipe
  G4Tubs* pipeS = new G4Tubs("pipeS",3.75*cm,4.0*cm,0.25*m,0.0,2.0*pi);
  G4LogicalVolume* pipeL = new G4LogicalVolume(pipeS,al,"pipeL");
  pipeL->SetVisAttributes(vis4);

  G4double off = 0.25*m + 10.01*cm;
  new G4PVPlacement(0,G4ThreeVector(0,0,off),pipeL,"pipe",world,false,0,check);
  new G4PVPlacement(0,G4ThreeVector(0,0,-off),pipeL,"pipe",world,false,1,check);
  
  return;
}
