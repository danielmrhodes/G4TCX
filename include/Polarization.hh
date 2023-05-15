#ifndef Polarization_h
#define Polarization_h 1

#include "Polarization_Messenger.hh"
#include "Polarized_Particle.hh"
#include <gsl/gsl_spline2d.h>

class Polarization_Messenger;
class Polarization {

public:

  Polarization(G4bool prj);
  ~Polarization();

  void BuildStatisticalTensors();
  void SetSpins();

  std::vector< std::vector<G4complex> > GetPolarization(G4int state, G4double en, G4double th,
							G4double ph);

  void SetFileName(G4String n) {fn = n;}
  void SetCalcGk(G4bool calc) {calcGk = calc;}
  void SetAverageJ(G4double avj) {Avji = avj;}
  void SetGamma(G4double gam) {Gam = gam;}
  void SetLambdaStar(G4double lam) {Xlamb = lam;}
  void SetTauC(G4double tc) {TimeC = tc;}
  void SetGFac(G4double gf) {Gfac = gf;}
  void SetFieldCoef(G4double coef) {Field = coef;}
  void SetFieldExp(G4double ex) {Power = ex;}

  static void Print(const std::vector< std::vector<G4complex> > polar);

private:

  G4int MaxK(G4double spin);
  G4int NumComps(G4double spin);
  G4int GetOffset(G4int index, G4int k, G4int kappa);
  void ReadTensorFile();
  void ApplyGk();
  void Clean();
  
  //calculate Gk coefficients
  std::array<G4double,7> GKK(const G4int iz, const G4double beta,
			     const G4double spin, const G4double time);
  G4double ATS(const G4int Ne);
  void XSTATIC(const G4int iz, const G4double beta, G4int& id, G4int& iu, G4double& qcen,
	       G4double& dq,
	       G4double& xnor);

  Polarization_Messenger* messenger;
  const G4bool proj;

  G4int threadID;

  //Energy-theta grid with values
  std::vector<G4double> energies;
  std::vector<G4double> thetas;
  std::vector<G4double> values;
  std::vector<gsl_spline2d*> interps; //one interpolator for each component
  
  std::vector<G4double> spins; //Spins of the excited states
  std::vector< std::vector<G4complex> > polarization;
  std::vector< std::vector<G4complex> > unpolarized; //Unpolarized statistical tensor

  G4String fn; //Statistical tensor file name
  G4bool calcGk; //Flag to calculate depolarization coefficients

  //Deorientation effect model parameters
  G4double Avji; // Average atomic spin
  G4double Gam; // FWHM of frequency distribution
  G4double Xlamb; //Fluctuating state to static state transition rate
  G4double TimeC; //Mean time between random reorientations of fluctuating state 
  G4double Gfac; //gyromagnetic ratio (g-factor)
  G4double Field; //Hyperfine field coefficient
  G4double Power; //Hyperfine field exponent

  //Stuff for 2D interpolation
  gsl_interp_accel* xacc;
  gsl_interp_accel* yacc;

};

#endif
