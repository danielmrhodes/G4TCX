#include "Physics_List.hh"

#include "G4SystemOfUnits.hh"

#include "G4GenericIon.hh"
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4Proton.hh"

#include "G4PhysicsListHelper.hh"
#include "G4StepLimiter.hh"

#include "G4ionIonisation.hh"
#include "G4IonParametrisedLossModel.hh"
#include "G4NuclearStopping.hh"

#include "G4eIonisation.hh"
#include "G4eMultipleScattering.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"

#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
#include "G4PhotoElectricEffect.hh"
#include "G4RayleighScattering.hh"
#include "G4PEEffectFluoModel.hh"
#include "G4KleinNishinaModel.hh"
#include "G4LowEPComptonModel.hh"
#include "G4PenelopeGammaConversionModel.hh"
#include "G4LivermorePhotoElectricModel.hh"

Physics_List::Physics_List() {}
Physics_List::~Physics_List() {}

void Physics_List::ConstructProcess() {

  AddTransportation();
  
  G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();

  //Electron
  ph->RegisterProcess(new G4eMultipleScattering(),G4Electron::Definition());
  ph->RegisterProcess(new G4eIonisation(),G4Electron::Definition());
  ph->RegisterProcess(new G4eBremsstrahlung(),G4Electron::Definition());
  
  //Positron
  ph->RegisterProcess(new G4eMultipleScattering(),G4Positron::Definition());
  ph->RegisterProcess(new G4eIonisation(),G4Positron::Definition());
  ph->RegisterProcess(new G4eBremsstrahlung(),G4Positron::Definition());
  ph->RegisterProcess(new G4eplusAnnihilation(),G4Positron::Definition());
  
  //Generic Ion
  G4NuclearStopping* inuc = new G4NuclearStopping();
  
  G4ionIonisation* ionIoni = new G4ionIonisation();
  ionIoni->SetEmModel(new G4IonParametrisedLossModel());
  ionIoni->SetStepFunction(0.1,1*um);

  //ph->RegisterProcess(new G4hMultipleScattering("ionmsc"),G4GenericIon::Definition());
  ph->RegisterProcess(ionIoni,G4GenericIon::Definition());
  ph->RegisterProcess(inuc,G4GenericIon::Definition());
  ph->RegisterProcess(new G4StepLimiter(),G4GenericIon::Definition());

  inuc->SetMaxKinEnergy(MeV);
  
  //Gamma
  G4PhotoElectricEffect* pe = new G4PhotoElectricEffect();
  pe->SetEmModel(new G4LivermorePhotoElectricModel());
  ph->RegisterProcess(pe,G4Gamma::Definition());

  // Compton scattering
  G4ComptonScattering* cs = new G4ComptonScattering;
  cs->SetEmModel(new G4KleinNishinaModel());
  
  G4VEmModel* theLowEPComptonModel = new G4LowEPComptonModel();
  theLowEPComptonModel->SetHighEnergyLimit(20*MeV);
  cs->AddEmModel(0,theLowEPComptonModel);
  ph->RegisterProcess(cs,G4Gamma::Definition());

  // Gamma conversion
  G4GammaConversion* gc = new G4GammaConversion();
  G4VEmModel* thePenelopeGCModel = new G4PenelopeGammaConversionModel();
  
  thePenelopeGCModel->SetHighEnergyLimit(1*GeV);
  gc->SetEmModel(thePenelopeGCModel);
  ph->RegisterProcess(gc,G4Gamma::Definition());

  // Rayleigh scattering
  ph->RegisterProcess(new G4RayleighScattering(),G4Gamma::Definition());
  
  return;
}

void Physics_List::ConstructParticle() {

  G4Gamma::Definition();
  G4Proton::Definition();
  G4Electron::Definition();
  G4Positron::Definition();
  
  G4GenericIon::Definition();

  return;
  
}

