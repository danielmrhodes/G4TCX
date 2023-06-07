#include "Polarization_Transition.hh"
#include "Gamma_Decay.hh"

#include "G4Clebsch.hh"
#include "G4PolynomialPDF.hh"
#include "G4Fragment.hh"
#include "G4NuclearPolarization.hh"
#include "G4SystemOfUnits.hh"
#include "G4Exp.hh"
#include "Randomize.hh"

#include <iostream>
#include <vector>


using namespace std;

Polarization_Transition::Polarization_Transition() 
  : fVerbose(1), fTwoJ1(0), fTwoJ2(0), fLbar(1), fL(0), fDelta(0), 
    kEps(1.e-15), kPolyPDF(0, nullptr, -1, 1) {
  
  for(G4int i=0;i<30;i++)
    fgLegendrePolys.GetCoefficient(0,i);
}


G4double Polarization_Transition::FCoefficient(G4int K, G4int LL, G4int Lprime,  G4int twoJ2,
					       G4int twoJ1) const {
  
  G4double fCoeff = G4Clebsch::Wigner3J(2*LL, 2, 2*Lprime, -2, 2*K, 0);
  if(fCoeff == 0) return 0;
  fCoeff *= G4Clebsch::Wigner6J(2*LL, 2*Lprime, 2*K, twoJ1, twoJ1, twoJ2);
  if(fCoeff == 0) return 0;
  if(((twoJ1+twoJ2)/2 - 1) %2) fCoeff = -fCoeff;
  return fCoeff*std::sqrt(G4double((2*K+1)*(twoJ1+1)*(2*LL+1)*(2*Lprime+1)));
}

G4double Polarization_Transition::F3Coefficient(G4int K, G4int K2, G4int K1,  G4int LL, G4int Lprime, 
						G4int twoJ2, G4int twoJ1) const {
  
  G4double fCoeff = G4Clebsch::Wigner3J(2*LL, 2, 2*Lprime, -2, 2*K, 0);
  if(fCoeff == 0) return 0;
  fCoeff *= G4Clebsch::Wigner9J(twoJ2, 2*LL, twoJ1, twoJ2, 2*Lprime, twoJ1, 
				2*K2, 2*K, 2*K1);
  if(fCoeff == 0) return 0;
  if((Lprime+K2+K1+1) % 2) fCoeff = -fCoeff;

  //AR-13Jun2017 : apply Jason Detwiler's conversion to double 
  //               in the argument of sqrt() to avoid integer overflows.
  return fCoeff*std::sqrt(G4double((twoJ1+1)*(twoJ2+1)*(2*LL+1))
			  *G4double((2*Lprime+1)*(2*K+1)*(2*K1+1)*(2*K2+1)));
}

G4double Polarization_Transition::GammaTransFCoefficient(G4int K) const {
  
  double transFCoeff = FCoefficient(K, fLbar, fLbar, fTwoJ2, fTwoJ1);
  if(fDelta == 0) return transFCoeff;
  transFCoeff += 2.*fDelta*FCoefficient(K, fLbar, fL, fTwoJ2, fTwoJ1);
  transFCoeff += fDelta*fDelta*FCoefficient(K, fL, fL, fTwoJ2, fTwoJ1);
  return transFCoeff;
}

G4double Polarization_Transition::GammaTransF3Coefficient(G4int K, G4int K2,  G4int K1) const {
  
  double transF3Coeff = F3Coefficient(K, K2, K1, fLbar, fLbar, fTwoJ2, fTwoJ1);
  if(fDelta == 0) return transF3Coeff;
  transF3Coeff += 2.*fDelta*F3Coefficient(K, K2, K1, fLbar, fL, fTwoJ2, fTwoJ1);
  transF3Coeff += fDelta*fDelta*F3Coefficient(K, K2, K1, fL, fL, fTwoJ2, fTwoJ1);
  return transF3Coeff;
}

G4double Polarization_Transition::GenerateGammaCosTheta(const POLAR& pol) {
  
  std::size_t length = pol.size();
  // Isotropic case
  if(length <= 1) return G4UniformRand()*2.-1.;

  // kappa > 0 terms integrate out to zero over phi: 0->2pi, so just need (k,0)
  // terms to generate cos theta distribution
  vector<G4double> polyPDFCoeffs(length, 0.0);
  for(G4int k = 0; k < (G4int)length; k += 2) {
    if ((pol[k]).size() > 0 ) {
      if(fVerbose > 1 && std::abs(((pol)[k])[0].imag()) > kEps) {
        G4cout << "G4PolarizationTransition::GenerateGammaCosTheta WARNING: \n"
	     << "          fPolarization[" 
	     << k << "][0] has imag component: = " 
	     << ((pol)[k])[0].real() << " + " 
  	     << ((pol)[k])[0].imag() << "*i" << G4endl;
      }
      G4double a_k = std::sqrt((G4double)(2*k+1))*GammaTransFCoefficient(k)*((pol)[k])[0].real();
      std::size_t nCoeff = fgLegendrePolys.GetNCoefficients(k);
      for(std::size_t iCoeff=0; iCoeff < nCoeff; ++iCoeff) {
        polyPDFCoeffs[iCoeff] += a_k*fgLegendrePolys.GetCoefficient(iCoeff, k);
      }
    } else {
      G4cout << "G4PolarizationTransition::GenerateGammaCosTheta: WARNING: \n"
             << " size of pol[" << k << "] = " << (pol[k]).size()
             << " returning isotropic " << G4endl;
      return G4UniformRand()*2.-1.; 
    }
  }
  if(fVerbose > 1 && polyPDFCoeffs[polyPDFCoeffs.size()-1] == 0) {
    G4cout << "G4PolarizationTransition::GenerateGammaCosTheta: WARNING: "
           << "got zero highest-order coefficient." << G4endl;
    DumpTransitionData(pol);
  }
  kPolyPDF.SetCoefficients(polyPDFCoeffs);
  return kPolyPDF.GetRandomX();
}

G4double Polarization_Transition::GenerateGammaPhi(const G4double cosTheta, const POLAR& pol) {
  
  G4int length = (G4int)pol.size();
  // Isotropic case
  G4bool phiIsIsotropic = true;
  for(G4int i=0; i<length; ++i) {
    if(((pol)[i]).size() > 1) {
      phiIsIsotropic = false;
      break;
    }
  }
  if(phiIsIsotropic) { return G4UniformRand()*CLHEP::twopi; }

  map<G4int, map<G4int, G4double> >* cachePtr = nullptr;

  // Otherwise, P(phi) can be written as a sum of cos(kappa phi + phi_kappa).
  // Calculate the amplitude and phase for each term
  std::vector<G4double> amp(length, 0.0);
  std::vector<G4double> phase(length, 0.0);
  for(G4int kappa = 0; kappa < length; ++kappa) {
    G4complex cAmpSum(0.,0.);
    for(G4int k = kappa + (kappa % 2); k < length; k += 2) {
      G4int kmax = (G4int)pol[k].size();
      if(kmax > 0) {
	if(kappa >= kmax || std::abs(((pol)[k])[kappa]) < kEps) { continue; }
        G4double tmpAmp = GammaTransFCoefficient(k);
        if(tmpAmp == 0) { continue; }
        tmpAmp *= std::sqrt((G4double)(2*k+1)) 
	  * fgLegendrePolys.EvalAssocLegendrePoly(k, kappa, cosTheta, cachePtr);
        if(kappa > 0) tmpAmp *= 2.*G4Exp(0.5*(LnFactorial(k-kappa) - LnFactorial(k+kappa)));
        cAmpSum += ((pol)[k])[kappa]*tmpAmp;
      } else {
	if(fVerbose > 1) {
	  G4cout << "G4PolarizationTransition::GenerateGammaPhi: WARNING: \n"
		 << " size of pol[" << k << "] = " << (pol[k]).size()
		 << " returning isotropic " << G4endl;
	}
        return G4UniformRand()*CLHEP::twopi;
      }
    }
    if(fVerbose > 1 && kappa == 0 && std::abs(cAmpSum.imag()) > kEps) {
      G4cout << "G4PolarizationTransition::GenerateGammaPhi: WARNING: \n"
	     << "    Got complex amp for kappa = 0! A = " << cAmpSum.real() 
	     << " + " << cAmpSum.imag() << "*i" << G4endl;
    }
    amp[kappa] = std::abs(cAmpSum);
    phase[kappa] = arg(cAmpSum);
  }

  // Normalize PDF and calc max (note: it's not the true max, but the max
  // assuming that all of the phases line up at a max)
  G4double pdfMax = 0.;
  for(G4int kappa = 0; kappa < length; ++kappa) { pdfMax += amp[kappa]; }
  if(fVerbose > 1 && pdfMax < kEps) {
    G4cout << "G4PolarizationTransition::GenerateGammaPhi: WARNING "
	   << "got pdfMax = 0 for \n";
    DumpTransitionData(pol);
    G4cout << "I suspect a non-allowed transition! Returning isotropic phi..." 
	   << G4endl;
    return G4UniformRand()*CLHEP::twopi;
  }

  // Finally, throw phi until it falls in the pdf
  for(std::size_t i=0; i<100; ++i) {
    G4double phi = G4UniformRand()*CLHEP::twopi;
    G4double prob = G4UniformRand()*pdfMax;
    G4double pdfSum = amp[0];
    for(G4int kappa = 1; kappa < length; ++kappa) {
      pdfSum += amp[kappa]*std::cos(phi*kappa + phase[kappa]);
    }
    if(fVerbose > 1 && pdfSum > pdfMax) {
      G4cout << "G4PolarizationTransition::GenerateGammaPhi: WARNING: \n"
             << "got pdfSum (" << pdfSum << ") > pdfMax (" 
	     << pdfMax << ") at phi = " << phi << G4endl;
    }
    if(prob <= pdfSum) { return phi; }
  }
  if(fVerbose > 1) {
    G4cout << "G4PolarizationTransition::GenerateGammaPhi: WARNING: \n"
	   << "no phi generated in 1000 throws! Returning isotropic phi..." 
           << G4endl;
  }
  return G4UniformRand()*CLHEP::twopi;
}

//void Polarization_Transition::SampleGammaTransition(Polarized_Particle* pol_part,  G4int twoJ1,
//						    G4int twoJ2, G4int L0, G4int Lp, G4double mpRatio,
//						    G4double& cosTheta, G4double& phi) {

//std::array<G4double,2> Polarization_Transition::SampleGammaTransition(const POLAR pol, G4int twoJ1,
//								      G4int twoJ2, G4int L0, G4int Lp,
//								      G4double mpRatio) {

void Polarization_Transition::SampleGammaTransition(G4bool proj, G4int twoJ1, G4int twoJ2,
						    G4int L0, G4int Lp, G4double mpRatio,
						    G4double& cosTheta, G4double& phi) {
  
  fTwoJ1 = twoJ1;  // add abs to remove negative J
  fTwoJ2 = twoJ2;
  fLbar  = L0;
  fL     = Lp;
  fDelta = mpRatio;
  if(fVerbose > 2) {
    G4cout << "G4PolarizationTransition: 2J1= " << fTwoJ1 << " 2J2= " << fTwoJ2
	   << " Lbar= " << fLbar << " delta= " << fDelta << " Lp= " << fL 
           << G4endl;
  }

  POLAR pol;
  if(proj)
    pol = Gamma_Decay::GetProjectilePolarization();
  else
    pol = Gamma_Decay::GetRecoilPolarization();
  
  if(fTwoJ1 == 0) {
    
    if(proj)
      Gamma_Decay::UnpolarizeProjectile();
    else
      Gamma_Decay::UnpolarizeRecoil();
    
    cosTheta = 2*G4UniformRand() - 1.0;
    phi = CLHEP::twopi*G4UniformRand();
  }
  else {
    cosTheta = GenerateGammaCosTheta(pol);
    if(fVerbose > 2) {
      G4cout << "G4PolarizationTransition::SampleGammaTransition: cosTheta= "
	     << cosTheta << G4endl;
    }
    phi = GenerateGammaPhi(cosTheta, pol);
    if(fVerbose > 2) {
      G4cout << "G4PolarizationTransition::SampleGammaTransition: phi= "
	     << phi << G4endl;
    }
  }

  if(fTwoJ2 == 0) {
    if(proj)
      Gamma_Decay::UnpolarizeProjectile();
    else
      Gamma_Decay::UnpolarizeRecoil();
    return;
  }

  std::size_t newlength = fTwoJ2+1;
  POLAR newPol;
  map<G4int, map<G4int, G4double> >* cachePtr = nullptr;

  for(G4int k2=0; k2<(G4int)newlength; ++k2) {
    std::vector<G4complex> npolar;
    npolar.resize(k2+1, 0);
    for(G4int k1=0; k1<(G4int)pol.size(); ++k1) {
      for(G4int k=0; k<=k1+k2; k+=2) {
        // TransF3Coefficient takes the most time. Only calculate it once per
        // (k, k1, k2) triplet, and wait until the last possible moment to do
        // so. Break out of the inner loops as soon as it is found to be zero.
        G4double tF3 = 0.;
        G4bool recalcTF3 = true;
        for(G4int kappa2=0; kappa2<=k2; ++kappa2) {
          G4int ll = (G4int)pol[k1].size();
          for(G4int kappa1 = 1 - ll; kappa1<ll; ++kappa1) {
            if(k+k2<k1 || k+k1<k2) continue;
            G4complex tmpAmp = (kappa1 < 0) ?  
	      conj((pol[k1])[-kappa1])*(kappa1 % 2 ? -1.: 1.) : (pol[k1])[kappa1];
            if(std::abs(tmpAmp) < kEps) continue;
            G4int kappa = kappa1-(G4int)kappa2;
            tmpAmp *= G4Clebsch::Wigner3J(2*k1, -2*kappa1, 2*k, 2*kappa, 2*k2, 2*kappa2);
            if(std::abs(tmpAmp) < kEps) continue;
            if(recalcTF3) {
              tF3 = GammaTransF3Coefficient(k, k2, k1);
              recalcTF3 = false;
            }
            if(std::abs(tF3) < kEps) break;
            tmpAmp *= tF3;
            if(std::abs(tmpAmp) < kEps) continue;
            tmpAmp *= ((kappa1+(G4int)k1)%2 ? -1. : 1.) 
	      * sqrt((2.*k+1.)*(2.*k1+1.)/(2.*k2+1.));
            //AR-13Jun2017 Useful for debugging very long computations
            //G4cout << "G4PolarizationTransition::UpdatePolarizationToFinalState : k1=" << k1 
            //       << " ; k2=" << k2 << " ; kappa1=" << kappa1 << " ; kappa2=" << kappa2 << G4endl;
            tmpAmp *= fgLegendrePolys.EvalAssocLegendrePoly(k, kappa, cosTheta, cachePtr);
            if(kappa != 0) {
              tmpAmp *= G4Exp(0.5*(LnFactorial(((G4int)k)-kappa) 
				   - LnFactorial(((G4int)k)+kappa)));
              tmpAmp *= polar(1., phi*kappa);
            }
            npolar[kappa2] += tmpAmp;
          }
          if(!recalcTF3 && std::abs(tF3) < kEps) break;
        }
      }
    }
    newPol.push_back(npolar);
  }

  // sanity checks
  if(fVerbose > 2 && 0.0 == newPol[0][0]) {
    
    G4cout << "G4PolarizationTransition::SampleGammaTransition WARNING:"
	   << " P[0][0] is zero!" << G4endl;
    
    DumpTransitionData(newPol);
    G4cout << "Unpolarizing..." << G4endl;
    if(proj)
      Gamma_Decay::UnpolarizeProjectile();
    else
      Gamma_Decay::UnpolarizeRecoil();
    
    return;
  }
  if(fVerbose > 2 && std::abs((newPol[0])[0].imag()) > kEps) {
    
    G4cout << "G4PolarizationTransition::SampleGammaTransition WARNING: \n"
	   << " P[0][0] has a non-zero imaginary part! Unpolarizing..." 
	   << G4endl;
    
    if(proj)
      Gamma_Decay::UnpolarizeProjectile();
    else
      Gamma_Decay::UnpolarizeRecoil();
    return;
    
  }
  if(fVerbose > 2) {
    G4cout << "Before normalization: " << G4endl;
    DumpTransitionData(newPol);
  }
  // Normalize and trim
  std::size_t lastNonEmptyK2 = 0;
  for(std::size_t k2=0; k2<newlength; ++k2) {
    G4int lastNonZero = -1;
    for(std::size_t kappa2=0; kappa2<(newPol[k2]).size(); ++kappa2) {
      if(k2 == 0 && kappa2 == 0) {
        lastNonZero = 0;
        continue;
      }
      if(std::abs((newPol[k2])[kappa2]) > 0.0) {
        lastNonZero = (G4int)kappa2;
        (newPol[k2])[kappa2] /= (newPol[0])[0];
      }
    }
    while((newPol[k2]).size() != std::size_t (lastNonZero+1)) (newPol[k2]).pop_back();
    if((newPol[k2]).size() > 0) lastNonEmptyK2 = k2;
  }

  // Remove zero-value entries 
  while(newPol.size() != lastNonEmptyK2+1) { newPol.pop_back(); }
  (newPol[0])[0] = 1.0;

  if(proj)
    Gamma_Decay::SetProjectilePolarization(newPol);
  else
    Gamma_Decay::SetRecoilPolarization(newPol);
  
  return;
  
}

void Polarization_Transition::DumpTransitionData(const POLAR& pol) const {
  
  G4cout << "G4PolarizationTransition: ";
  (fTwoJ1 % 2) ? G4cout << fTwoJ1 << "/2" : G4cout << fTwoJ1/2;
  G4cout << " --(" << fLbar;
  if(fDelta != 0) G4cout << " + " << fDelta << "*" << fL;
  G4cout << ")--> ";
  (fTwoJ2 % 2) ? G4cout << fTwoJ2 << "/2" : G4cout << fTwoJ2/2;
  G4cout << ", P = [ { ";
  for(std::size_t k=0; k<pol.size(); ++k) {
    if(k>0) G4cout << " }, { ";
    for(std::size_t kappa=0; kappa<(pol[k]).size(); ++kappa) {
      if(kappa > 0) G4cout << ", ";
      G4cout << (pol[k])[kappa].real() << " + " << (pol[k])[kappa].imag() << "*i";
    }
  }
  G4cout << " } ]" << G4endl;
}