#include "Primary_Generator.hh"

#include "G4GenericIon.hh"
#include "G4Gamma.hh"
#include "G4RandomDirection.hh"
#include "G4IonTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4PhysicalConstants.hh"

#include "G4RunManager.hh"
#include "Detector_Construction.hh"
#include "G4EmCalculator.hh"

Primary_Generator::Primary_Generator() {

  gun = new G4ParticleGun(1);
  messenger = new Primary_Generator_Messenger(this);
  
  reac = new Reaction();
  source = new Gamma_Source();
  exciteP = new Excitation(true);
  exciteR = new Excitation(false);
  
  projGS = NULL;
  recoilGS = NULL;

  pI = 0;
  rI = 0;
  th = 0.0*rad;
  en = 0.0*MeV;

  dedx = 0.0*(MeV/mm);
  //dedx1 = NULL;
  width = 0.0*mm;

  beam_X = 0.0*mm;
  beam_Y = 0.0*mm;
  beam_AX = 0.0*rad;
  beam_AY = 0.0*rad;
  beam_En = 0.0*MeV;
  
  sigma_X = 0.0*mm;
  sigma_Y = 0.0*mm;
  sigma_AX = 0.0*rad;
  sigma_AY = 0.0*rad;
  sigma_En = 0.0*MeV;

  deltaE = 0.0*MeV;
  mode = Primary_Generator::MODE::Source;
  
  optimize = false;
  onlyDS = false;
  onlyUS = false;
  onlyP = false;
  onlyR = false;
  rDS_pUS = false;

  s3 = new G4Tubs("S3",1.1*cm,3.5*cm,150.0*um,0.0*deg,360.0*deg);
  s3_0 = G4ThreeVector(0.0,0.0,0.0);
  s3_1 = G4ThreeVector(0.0,0.0,0.0);
}

Primary_Generator::~Primary_Generator() {
  
  delete gun;
  delete messenger;
  
  delete reac;
  delete source;
  delete exciteP;
  delete exciteR;
  
}

void Primary_Generator::GeneratePrimaries(G4Event* evt) {
  
  switch(mode) {
    
    case MODE::Scattering: {
 
      GenerateScatteringPrimaries(evt);

      /*
      //isotropic particle
      G4ThreeVector dir = G4RandomDirection();
      G4ThreeVector pos = G4ThreeVector(0.0*mm,0.0*mm,0.0*mm);
      
      gun->SetParticleDefinition(projGS);
      gun->SetParticleEnergy(300.0*MeV);
      gun->SetParticleMomentumDirection(dir);
      gun->SetParticlePosition(pos);
      gun->GeneratePrimaryVertex(evt);
      */
      
      break;
    }

    case MODE::Source: {
      
      GenerateSourcePrimaries(evt);

      break;
    }

    case MODE::Full: {
      
      GenerateFullPrimaries(evt);
      
      break;
    }

  }

  return;
  
}

void Primary_Generator::GenerateScatteringPrimaries(G4Event* evt) {

 label:
  
  //Choose thetaCM 
  th = reac->SampleRutherfordCM();
  
  //Randomize incoming beam energy using energy distribution
  en = G4RandGauss::shoot(beam_En,sigma_En);

  //Randomly choose reaction depth in target
  G4double depth = G4RandFlat::shoot(width);

  //Energy loss
  en -= dedx*depth;

  /*
  G4double dist = 0.0*mm;
  G4double step = 0.01*width;
  
  en -= std::fmod(depth,step)*dedx1->CubicSplineInterpolation(en);
  dist += std::fmod(depth,step);
  while(dist < depth) {
    en -= step*dedx1->CubicSplineInterpolation(en);
    dist += step;
  }
  */
  
  //Reaction position
  //Randomize X and Y based on beam spot
  G4ThreeVector pos = G4ThreeVector(G4RandGauss::shoot(beam_X,sigma_X),
				    G4RandGauss::shoot(beam_Y,sigma_Y),
				    -(width/2.0) + depth);
  //Outgoing vectors
  G4ThreeVector bdir = G4ThreeVector(0,0,1); //projectile direction
  bdir.setTheta(reac->Theta_LAB(th,en,deltaE)); //theta from kinematics
  bdir.setPhi(G4RandFlat::shoot(-pi,pi)); //randomly choose phi

  G4ThreeVector rdir = G4ThreeVector(0,0,1); //recoil direction
  rdir.setTheta(reac->Recoil_Theta_LAB(th,en,deltaE)); //theta from kinematics
  rdir.setPhi(bdir.phi()-pi); //Particles emerge back-to-back in LAB frame
  
  //Randomize direction using angle distributions
  G4double ax = G4RandGauss::shoot(beam_AX,sigma_AX);
  G4double ay = G4RandGauss::shoot(beam_AY,sigma_AY);
  
  bdir.rotateX(ax);
  bdir.rotateY(ay);
  rdir.rotateX(ax);
  rdir.rotateY(ay);
  
  if(optimize) {

    G4bool good = CheckIntersections(bdir,rdir);
    if(!good)
      goto label;
    
  }
  
  //Beam vertex
  gun->SetParticleDefinition(projGS);
  gun->SetParticleEnergy(reac->KE_LAB(th,en,deltaE));
  gun->SetParticlePosition(pos);
  gun->SetParticleMomentumDirection(bdir);
  gun->GeneratePrimaryVertex(evt);
  
  //Recoil vertex
  gun->SetParticleDefinition(recoilGS);
  gun->SetParticleEnergy(reac->Recoil_KE_LAB(th,en,deltaE));
  gun->SetParticlePosition(pos);
  gun->SetParticleMomentumDirection(rdir);
  gun->GeneratePrimaryVertex(evt);

  return;
}

void Primary_Generator::GenerateSourcePrimaries(G4Event* evt) {
  
  //Simple isotropic gamma
  if(source->GetEnergy() > 0.0*MeV) {
    
    gun->SetParticleDefinition(G4Gamma::Definition());
    gun->SetParticlePosition(G4ThreeVector());
    gun->SetParticleEnergy(source->GetEnergy());
    gun->SetParticleMomentumDirection(G4RandomDirection());
    gun->GeneratePrimaryVertex(evt);

    return;
  }

  //Remove polarization
  source->Unpolarize();

  //Choose excited state
  pI = source->ChooseState();

  //Make vertex
  gun->SetParticleDefinition(source->GetDefinition(pI));
  gun->SetParticleEnergy(0.0*MeV);
  gun->SetParticlePosition(G4ThreeVector());
  gun->SetParticleMomentumDirection(G4ThreeVector());
  gun->GeneratePrimaryVertex(evt);
  
  return;
}

void Primary_Generator::GenerateFullPrimaries(G4Event* evt) {

 label:
  
  //Choose thetaCM 
  th = reac->SampleRutherfordCM();
  
  //Randomize incoming beam energy using energy distribution
  en = G4RandGauss::shoot(beam_En,sigma_En);

  //Randomly choose reaction depth in target
  G4double depth = G4RandFlat::shoot(width);

  //Energy loss
  en -= dedx*depth;

  //Choose excited states
  pI = exciteP->ChooseState(en,th);
  rI = exciteR->ChooseState(en,th);

  //DelatE for inelastic scattering
  G4double ex = 0.0*MeV;
  ex += exciteP->GetExcitation(pI);
  ex += exciteR->GetExcitation(rI);

  //Reaction position
  //Randomize X and Y based on beam spot
  G4ThreeVector pos = G4ThreeVector(G4RandGauss::shoot(beam_X,sigma_X),
				    G4RandGauss::shoot(beam_Y,sigma_Y),
				    -(width/2.0) + depth);
  
  //Outgoing vectors
  G4ThreeVector bdir = G4ThreeVector(0,0,1); //projectile direction
  bdir.setTheta(reac->Theta_LAB(th,en,ex)); //theta from kinematics
  bdir.setPhi(G4RandFlat::shoot(-pi,pi)); //randomly choose phi
  
  G4ThreeVector rdir = G4ThreeVector(0,0,1); //recoil direction
  rdir.setTheta(reac->Recoil_Theta_LAB(th,en,ex)); //theta from kinematics
  rdir.setPhi(bdir.phi()-pi); //Particles emerge back-to-back in LAB frame

  G4double phiB = bdir.phi(); //save unrotated phi for polarization
  
  //Randomize direction using angle distributions
  G4double ax = G4RandGauss::shoot(beam_AX,sigma_AX);
  G4double ay = G4RandGauss::shoot(beam_AY,sigma_AY);
  
  bdir.rotateX(ax);
  bdir.rotateY(ay);
  rdir.rotateX(ax);
  rdir.rotateY(ay);

  if(optimize) {

    G4bool good = CheckIntersections(bdir,rdir);
    if(!good)
      goto label;
    
  }

  //Align excited states
  exciteP->Unpolarize(); //remove old polarization
  exciteP->Polarize(pI,en,th,phiB);

  exciteR->Unpolarize(); //remove old polarization
  exciteR->Polarize(rI,en,th,phiB);
  
  //Beam vertex
  gun->SetParticleDefinition(exciteP->GetDefinition(pI));
  gun->SetParticleEnergy(reac->KE_LAB(th,en,ex));
  gun->SetParticlePosition(pos);
  gun->SetParticleMomentumDirection(bdir);
  gun->GeneratePrimaryVertex(evt);
  
  //Recoil vertex
  gun->SetParticleDefinition(exciteR->GetDefinition(rI));
  gun->SetParticleEnergy(reac->Recoil_KE_LAB(th,en,ex));
  gun->SetParticlePosition(pos);
  gun->SetParticleMomentumDirection(rdir);
  gun->GeneratePrimaryVertex(evt);
  
  return;
}


void Primary_Generator::Update() {

  //Don't change the order of these functions
  switch(mode) {

    case MODE::Scattering: {
      
      G4IonTable* table = (G4IonTable*)(G4ParticleTable::GetParticleTable()->GetIonTable());
      
      projGS = table->GetIon(reac->GetBeamZ(),reac->GetBeamA(),0.0*MeV);
      recoilGS = table->GetIon(reac->GetRecoilZ(),reac->GetRecoilA(),0.0*MeV);
      projGS->SetPDGLifeTime(-1.0);
      recoilGS->SetPDGLifeTime(-1.0);

      UpdateReaction();
      
      break;
    }

    case MODE::Source: {
      
      source->BuildLevelScheme();
      
      break;
    }

    case MODE::Full: {
      
      exciteP->BuildLevelScheme();
      exciteR->BuildLevelScheme();
      
      exciteP->BuildProbabilities();
      exciteR->BuildProbabilities();
      
      projGS = exciteP->GetDefinition(0);
      recoilGS = exciteR->GetDefinition(0);
      UpdateReaction();

      exciteP->BuildStatisticalTensors();
      exciteR->BuildStatisticalTensors();
      
      break;
    }
  }
  
  return;
  
}

void Primary_Generator::UpdateReaction() {

  reac->SetBeamMass(projGS->GetPDGMass());
  reac->SetRecoilMass(recoilGS->GetPDGMass());
  reac->ConstructRutherfordCM(beam_En,deltaE);

  if(onlyP)
    reac->SetOnlyP();
  if(onlyR)
    reac->SetOnlyR();
  
  Detector_Construction* con =
    (Detector_Construction*)G4RunManager::GetRunManager()->GetUserDetectorConstruction();

  s3_0 = G4ThreeVector(-beam_X,-beam_Y,-con->GetUS_Offset());
  s3_1 = G4ThreeVector(-beam_X,-beam_Y,con->GetDS_Offset());

  G4Material* mat = con->GetTargetMaterial();
  //if(con->GetTargetMaterial()) {
  if(mat) {
    
    G4EmCalculator calc;
    //dedx = calc.ComputeTotalDEDX(beam_En,projGS,con->GetTargetMaterial());
    dedx = calc.ComputeTotalDEDX(beam_En,projGS,mat);
    width = con->GetTargetThickness();

    /*
    const int num = 101;
    G4double eMin = 0.1*beam_En;
    G4double eMax = 1.1*beam_En;
    G4double step = (eMax - eMin)/G4double(num-1);
    
    G4double px[num];
    G4double py[num];
    for(G4int i=0;i<num;i++) {
      G4double Ep = eMin + i*step;
      px[i] = Ep;
      py[i] = calc.ComputeTotalDEDX(Ep,projGS,mat);
    }
    
    dedx1 = new G4DataInterpolation(px,py,num,0.0,0.0);
    */
  }
  
  return;
}


G4bool Primary_Generator::CheckIntersections(const G4ThreeVector& bdir,
					     const G4ThreeVector& rdir) {

  if(onlyDS) { //Downstream S3 only
    if(onlyP) //Only projectiles
      return Intersects(bdir,s3_1) && bdir.getZ() > 0.0;
    
    if(onlyR) //Only Recoils
      return Intersects(rdir,s3_1);

    //No particle restrictions
    return (Intersects(bdir,s3_1) && bdir.getZ() > 0.0) || Intersects(rdir,s3_1);
  }
  
  if(onlyUS) //Upstream S3 only, can only be projectile
    return Intersects(bdir,s3_0) && bdir.getZ() < 0.0;
  
  if(rDS_pUS)  //Recoil Downstream or projectile upstream
    return (Intersects(rdir,s3_1) && rdir.getZ() > 0.0)
      || (Intersects(bdir,s3_0) && bdir.getZ() < 0.0);

  //No detector restrictions
  if(onlyP)  //Only projectiles
    return Intersects(bdir,s3_1) || Intersects(bdir,s3_0);
  
  if(onlyR) //Only Recoils, can only be downstream S3
    return Intersects(rdir,s3_1);

  //No restrictions
  return (Intersects(bdir,s3_1) && bdir.getZ() > 0.0) || Intersects(rdir,s3_1) ||
    (Intersects(bdir,s3_0) && bdir.getZ() < 0.0);
  
}


G4bool Primary_Generator::Intersects(const G4ThreeVector& dir, const G4ThreeVector& S3pos) {

  G4ThreeVector newVec(dir);

  G4double z = S3pos.getZ();
  G4double mag;
  if(z > 0.0)
    mag = (z - 0.99*150.0*um)/std::cos(dir.getTheta());
  else
    mag = -(z + 0.99*150.0*um)/std::cos(dir.getTheta());

  newVec.setMag(mag);
  
  G4AffineTransform Tm(S3pos);
  G4ThreeVector mp = Tm.Inverse().TransformPoint(newVec);
  
  if(s3->Inside(mp) == kInside)
    return true;

  return false;
  
}

void Primary_Generator::SetMode(G4String md) {

  if(md == "Scattering") {
    mode = MODE::Scattering;
  }
  else if(md == "Source") {
    mode = MODE::Source;
  }
  else if(md == "Full") {
    mode = MODE::Full;
  }
  
  return;
  
}

G4int Primary_Generator::GetZ(G4bool proj) {

  if(proj)
    return reac->GetBeamZ();
  
  return reac->GetRecoilZ();
  
}

G4int Primary_Generator::GetA(G4bool proj) {

  if(proj)
    return reac->GetBeamA();
  
  return reac->GetRecoilA();
  
}

G4double Primary_Generator::GetMass(G4bool proj) {

  if(proj)
    return reac->GetBeamMass();
  
  return reac->GetRecoilMass();
  
}

std::vector<G4double> Primary_Generator::GetExcitedStateLifetimes(G4bool proj) {

  std::vector<G4double> times;
  if(proj) {

    for(unsigned int i=1;i<exciteP->GetLevels().size();i++)
      times.push_back(exciteP->GetDefinition(i)->GetPDGLifeTime());

  }
  else {

    for(unsigned int i=1;i<exciteR->GetLevels().size();i++)
      times.push_back(exciteR->GetDefinition(i)->GetPDGLifeTime());
    
  }

  return times;
  
}

std::vector<G4double> Primary_Generator::GetExcitedStateSpins(G4bool proj) {

  std::vector<G4double> spins;
  if(proj) {

    for(unsigned int i=1;i<exciteP->GetLevels().size();i++)
      spins.push_back(exciteP->GetLevels().at(i)->GetSpin());

  }
  else {

    for(unsigned int i=1;i<exciteR->GetLevels().size();i++)
      spins.push_back(exciteR->GetLevels().at(i)->GetSpin());
  
  }

  return spins;
  
}
