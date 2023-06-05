#ifndef Polarization_Transition_h
#define Polarization_Transition_h

#include "Polarized_Particle.hh"

#include "globals.hh"
#include "G4LegendrePolynomial.hh"
#include "G4PolynomialPDF.hh"
#include "G4Pow.hh"

class Polarization_Transition {
  
  typedef std::vector< std::vector<G4complex> > POLAR;

public:
  
  explicit Polarization_Transition();
  ~Polarization_Transition() {;}

  /*
  void SampleGammaTransition(Polarized_Particle* np, G4int twoJ1, G4int twoJ2, G4int L0, G4int Lp, 
                             G4double mpRatio, G4double& cosTheta, G4double& phi);
  

  std::array<G4double,2> SampleGammaTransition(Polarized_Particle* np, G4int twoJ1, G4int twoJ2, 
					       G4int L0, G4int Lp, G4double mpRatio);
  */

  //std::array<G4double,2> SampleGammaTransition(const POLAR pol, G4int twoJ1, G4int twoJ2, G4int L0,
  //					       G4int Lp, G4double mpRatio);

  void SampleGammaTransition(G4bool proj, G4int twoJ1, G4int twoJ2, G4int L0, G4int Lp,
			     G4double mpRatio, G4double& cosTheta, G4double& phi);
  
  // generic static functions
  G4double FCoefficient(G4int K, G4int L, G4int Lprime, 
			G4int twoJ2, G4int twoJ1) const;
  G4double F3Coefficient(G4int K, G4int K2, G4int K1, G4int L, 
			 G4int Lprime, G4int twoJ2, G4int twoJ1) const;

  // transition-specific functions
  G4double GammaTransFCoefficient(G4int K) const;
  G4double GammaTransF3Coefficient(G4int K, G4int K2, G4int K1) const;

  void DumpTransitionData(const POLAR& pol) const;

  inline void SetVerbose(G4int val) { fVerbose = val; };

private:

  Polarization_Transition(const Polarization_Transition& right) = delete;
  const Polarization_Transition operator=(const Polarization_Transition right) = delete;

  // Gamma angle generation and decay: call these functions in this order!
  // All angles are in the same coordinate system: user may choose any axis
  G4double GenerateGammaCosTheta(const POLAR& pol);
  G4double GenerateGammaPhi(const G4double cosTheta, const POLAR& pol);

  inline G4double LnFactorial(int k) const { return G4Pow::GetInstance()->logfactorial(k); }

  G4int fVerbose;
  G4int fTwoJ1, fTwoJ2;
  G4int fLbar, fL;
  G4double fDelta;
  G4double kEps;
  G4PolynomialPDF kPolyPDF;
  G4LegendrePolynomial fgLegendrePolys;
};


#endif
