#include "Polarization.hh"
#include "Reaction.hh"
#include "Primary_Generator.hh"
#include <fstream>

#include "G4Clebsch.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4MTRunManager.hh"
#include "G4WorkerRunManager.hh"

Polarization::Polarization(G4bool prj) : proj(prj) {
  
  messenger = new Polarization_Messenger(this,proj);
  
  fn = "";
  calcGk = true;

  threadID = G4Threading::G4GetThreadId();

  /*
  polarization = new std::vector< std::vector<G4complex> >();
  unpolarized = new std::vector< std::vector<G4complex> >();

  polarization->clear();
  unpolarized->clear();
  */
  
  unpolarized.resize(1);
  unpolarized[0].push_back(1.0);
  
  //deorientation effect model parameters
  //default values
  Avji = 3.0;
  Gam = 0.02;
  Xlamb = 0.0345;
  TimeC = 3.5;
  Gfac = -1.0; //default is Z/A, which is assigned later
  Field = 6.0*std::pow(10.0,-6.0);
  Power = 0.6;

  xacc = gsl_interp_accel_alloc();
  yacc = gsl_interp_accel_alloc();
  
}

Polarization::~Polarization() {

  delete messenger;
  //delete polarization;
  //delete unpolarized;
  
  for(unsigned int i=0;i<interps.size();i++)
    gsl_spline2d_free(interps.at(i));
  
  gsl_interp_accel_free(xacc);
  gsl_interp_accel_free(yacc);
  
}

G4int Polarization::MaxK(G4double spin) {

  G4int twoJ = G4int(2.0*spin + 0.01);
  if(twoJ >= 6)
    return 6;
  
  return twoJ;
  
}

G4int Polarization::NumComps(G4double spin) {
  
  G4int num = 0;
  for(G4int k=0;k<=MaxK(spin);k+=2)
    num += k + 1;

  return num;
}

G4int Polarization::GetOffset(G4int index, G4int k, G4int kappa) {
  
  G4int offset = 0;
  for(G4int i=0;i<index-1;i++)
    offset += NumComps(spins[i]);

  for(G4int i=0;i<k;i+=2)
    offset += i + 1;

  offset += kappa;
  
  return offset;
}

void Polarization::ReadTensorFile() {

  G4String nuc;
  if(proj)
    nuc = "projectile";
  else
    nuc = "recoil";
  
  thetas.clear();
  energies.clear();
  values.clear();
  
  std::ifstream file;
  file.open(fn.c_str(),std::ios::in);

  if(!file.is_open()) {
    if(!threadID)
      std::cout << " \033[1;31mCould not open " << nuc << " statistical tensor file " << fn
		<< "! No polarization will occur! \033[m" << std::endl;
    spins.clear();
    return;
  }
  
  std::string line;
  std::getline(file,line);

  G4double en;
  std::stringstream ss1(line);

  while(ss1 >> en)
    energies.push_back(en);

  std::getline(file,line);

  G4double th;
  std::stringstream ss2(line);

  while(ss2 >> th)
    thetas.push_back(th);

  G4int numE = energies.size();
  G4int numT = thetas.size();
  G4int indexE = 0;
  G4int indexT = 0;

  for(auto sp : spins) 
    values.resize(values.size() + numE*numT*NumComps(sp));
  
  std::getline(file,line);
  while(std::getline(file,line)) {

    if(line.empty()) {
      
      indexT++;
      if(indexT == numT) {
	indexE++;
	indexT = 0;
	std::getline(file,line);
      }
      
      continue;
    }

    G4int index, k, kappa;
    G4double spin, val;
    
    std::stringstream ss3(line);
    ss3 >> index >> spin >> k >> kappa >> val;

    G4int offset = GetOffset(index,k,kappa)*numE*numT;
    values.at(offset + indexT*numE + indexE) = val;
 
  }
  
  return;
  
}

void Polarization::BuildStatisticalTensors() {

  G4String nuc;
  if(proj)
    nuc = "projectile";
  else
    nuc = "recoil";
  
  if(fn == "") {
    if(!threadID)
      std::cout << "\nNo " << nuc << " polarization\n";
    return;
  }
  
  if(!threadID)
    std::cout << "\nBuilding " << nuc << " statistical tensors from " << fn  << "\n";

  SetSpins();
  ReadTensorFile();
  ApplyGk();

  G4int numE = energies.size();
  G4int numT = thetas.size();
  G4int numS = spins.size();
  G4int numV = values.size();

  if(numE && numT && numS && numV) {
    if(!threadID)
      std::cout << "All " << nuc << " statistical tensors successfully built!" << std::endl;
  }
  else {
    if(!threadID)
      std::cout << "\033[1;31mFailed :( No polarization for the " << nuc << "\033[m\n";
    spins.clear();
    return;
  }
  
  for(G4int i=0;i<numS;i++) {

    G4double sp = spins.at(i);
    for(G4int k=0;k<=MaxK(sp);k+=2) {
      
      for(G4int kp=0;kp<=k;kp++) {

	interps.push_back(gsl_spline2d_alloc(gsl_interp2d_bicubic,numE,numT));  

	G4int offset = GetOffset(i+1,k,kp)*numE*numT;
	gsl_spline2d_init(interps.back(),&energies[0],&thetas[0],&values[offset],numE,numT);
	
      }
    }
    
  }
  
  return;
  
}

void Polarization::SetSpins() {

  G4WorkerRunManager* Rman = (G4WorkerRunManager*)G4MTRunManager::GetRunManager();
  Primary_Generator* gen = (Primary_Generator*)Rman->GetUserPrimaryGeneratorAction();
  std::vector<G4double> sps = gen->GetExcitedStateSpins(proj);
  
  spins.clear();
  for(auto sp : sps)
    spins.push_back(sp);
  
  return;
}

void Polarization::ApplyGk() {

  if(calcGk) {
    if(!threadID)
      std::cout << " Gk coefficients will be applied\n";
  }
  else {
    if(!threadID)
      std::cout << " No Gk coefficients\n";
    return;
  }

  G4WorkerRunManager* Rman = (G4WorkerRunManager*)G4MTRunManager::GetRunManager();
  Primary_Generator* gen = (Primary_Generator*)Rman->GetUserPrimaryGeneratorAction();
  
  G4int Z = gen->GetZ(proj);
  G4int A = gen->GetA(proj);
  std::vector<G4double> times = gen->GetExcitedStateLifetimes(proj);
  G4double beam_mass = gen->GetMass(true);
  G4double targ_mass = gen->GetMass(false);
  
  if(Gfac < 0.0)
    Gfac = G4double(Z)/G4double(A);
  
  G4int numE = energies.size();
  G4int numT = thetas.size();
  G4int numS = spins.size();

  for(G4int i=0;i<numS;i++) {

    G4double sp = spins.at(i);
    for(G4int j=0;j<numE;j++) {

      G4double en = energies.at(j);
      for(G4int l=0;l<numT;l++) {
	    
	G4double th = thetas.at(l);
	G4double beta;
	if(proj)
	  beta = Reaction::Beta_LAB(th,en,beam_mass,targ_mass,0.0*MeV);
	else
	  beta = Reaction::Recoil_Beta_LAB(th,en,beam_mass,targ_mass,0.0*MeV);
	    
	std::array<G4double,7> Gks = GKK(Z,beta,sp,times.at(i)/ps);
	for(G4int k=0;k<=MaxK(sp);k+=2) {

	  for(G4int kp=0;kp<=k;kp++) {

	    G4int offset = GetOffset(i+1,k,kp)*numE*numT;
	    values.at(offset + l*numE + j) *= Gks.at(k);

	  }
	}
      }
    }
  }

  return;
  
}

void Polarization::Clean() {

  if(!polarization.empty()) {
    for(std::vector<G4complex> pol : polarization) {
      pol.clear();
    }
    polarization.clear();
  }
  
  return;
}

std::vector< std::vector<G4complex> > Polarization::GetPolarization(G4int state, G4double en,
								    G4double th, G4double ph) {

  if((unsigned int)state >= spins.size())
    return unpolarized;

  Clean();
  
  G4int numE = energies.size();
  G4int numT = thetas.size();
  if(en < energies[0]) {

    static G4int counter = 0;
    if(counter < 5)
      G4cout << "\033[1;36mYou need to go to a lower energy when making the "
	     << "polarization input file (E = " << en << ", min = " << energies[0] << ")\033[m"
	     << G4endl;
    counter++;
    
    en = energies[0];
  }
  else if(en > energies[numE-1]) {

    static G4int counter = 0;
    if(counter < 5)
      G4cout << "\033[1;36mYou need to go to a higher energy when making the "
	     << "polarization input file (E = " << en << ", max = " << energies[numE-1] << ")\033[m"
	     << G4endl;
    counter++;
    
    en = energies[numE-1];
  }

  if(th < thetas[0]) {

    static G4int counter = 0;
    if(counter < 5)
      G4cout << "\033[1;36mYou need to go to a lower theta when making the "
	     << "polarization input file (th = " << th << ", min = " << thetas[0] << ")\033[m"
	     << G4endl;
    counter++;
    
    th = thetas[0]; 
  }
  else if(th > thetas[numT-1]) {

    static G4int counter = 0;
    if(counter < 5)
      G4cout << "\033[1;36mYou need to go to a higher theta when making the "
	     << "polarization input file (th = " << th << ", max = " << thetas[numT-1] << ")\033[m"
	     << G4endl;
    counter++;
    
    th = thetas[numT-1];
  }
  
  G4int maxK = MaxK(spins[state-1]);
  polarization.resize(maxK+1);
  
  G4complex imp;
  imp.real(0.0);
  
  for(G4int k=0;k<=maxK;k++) {

    if(k%2)
      continue;

    polarization[k].resize(k+1);
    for(G4int kp=0;kp<=k;kp++) {
      
      G4complex val;
      val.imag(0.0);

      G4int index = GetOffset(state,k,kp);
      val.real(gsl_spline2d_eval(interps[index],en,th,xacc,yacc));
      
      imp.imag(kp*ph);
      polarization[k][kp] = std::exp(-imp)*val;
      
    }
  }

  G4complex p00 = polarization[0][0];
  for(G4int k=0;k<=maxK;k++) {

    if(k%2)
      continue;
   
    for(G4int kp=0;kp<=k;kp++)
      polarization[k][kp] /= p00;
    
  }
  
  return polarization;
  
}

void Polarization::Print(const std::vector< std::vector<G4complex> > polar) {

  G4cout << " P = [ {";
  size_t kk = polar.size();
  for(size_t k=0; k<kk; ++k) {
    if(k>0) { G4cout << "       {"; }
    size_t kpmax = (polar[k]).size();
    for(size_t kappa=0; kappa<kpmax; ++kappa) {
      if(kappa > 0) { G4cout << "}  {"; }
      G4cout << polar[k][kappa].real() << " + " 
	  << polar[k][kappa].imag() << "*i";
    }
    if(k+1 < kk) { G4cout << "}" << G4endl; }
  }
  G4cout << "} ]" << G4endl;

  return;
}

std::array<G4double,7> Polarization::GKK(const G4int iz, const G4double beta,
					 const G4double spin, const G4double time) {

  /*
  //model parameters
  const G4double Avji = 3.0; // Average atomic spin
  const G4double Gam = 0.02; // FWHM of frequency distribution
  const G4double Xlamb = 0.0345; //Fluctuating state to static state transition rate
  const G4double TimeC = 3.5; //Mean time between random reorientations of fluctuating state 
  const G4double Gfac = iz/G4double(ia); //Nuclear gyromagnetic factor
  const G4double Field = 6.0*std::pow(10.0,-6.0); //Hyperfine field coefficient (600 T)
  const G4double Power = 0.6; //Hyperfine field exponent
  */
  
  G4int  inq, ifq;
  G4double qcen, dq, xnor;
  XSTATIC(iz,beta,inq,ifq,qcen,dq,xnor);
  
  G4double aks[6] = {0.0,0.0,0.0,0.0,0.0,0.0}; //alpha_k
  G4double sum[3] = {0.0,0.0,0.0}; //stores sum over 6j symbols

  for(G4int j=inq;j<ifq+1;j++) {

    G4int nz = iz - j;
    G4double xji = ATS(nz);
    G4double sm = spin;
    if(spin > xji) {
      sm = xji;
    }

    G4int ncoup = G4int(2.0*sm + 0.5) + 1;
    sum[0] = 0.0;
    sum[1] = 0.0;
    sum[2] = 0.0;

    G4double valmi = spin - xji;
    if(valmi < 0.0) {
      valmi *= -1;
    }

    for(G4int mi=0;mi<ncoup;mi++) {

      G4double f = valmi + mi;
      for(G4int k=0;k<3;k++) {
	
	G4double rk = 2.0*k + 2.0;
	G4int if2 = G4int(2.0*f + 0.0001);
	G4int irk2 = G4int(2.0*rk + 0.0001);
	G4int ispin2 = G4int(2.0*spin + 0.0001);
	G4int ixji2 = G4int(2.0*xji + 0.0001);

	sum[k] += std::pow((2.0*f + 1.0)*G4Clebsch::Wigner6J(if2,if2,irk2,ispin2,ispin2,ixji2),2.0)/(2.0*xji + 1.0);
      }
      
    }

    for(G4int k=0;k<3;k++) {
      G4int k1 = 2*k;
      aks[k1] += sum[k]*std::exp(-std::pow((qcen-j)/dq,2)/2.0)/xnor;
    }
    
  } //end loop over j (charge states)

  G4double xji = Avji;
  G4double sm = spin;
  if(spin > xji) {
    sm = xji;
  }

  G4int ncoup = G4int(2.0*sm + 0.5) + 1;
  sum[0] = 0.0;
  sum[1] = 0.0;
  sum[2] = 0.0;

  G4double valmi = spin - xji;
  if(valmi < 0.0) {
    valmi *= -1;
  }

  for(G4int mi=0;mi<ncoup;mi++) {

    G4double f = valmi + mi;
    for(G4int k=0;k<3;k++) {
	
      G4double rk = 2.0*k + 2.0;
      G4int if2 = G4int(2.0*f + 0.0001);
      G4int irk2 = G4int(2.0*rk + 0.0001);
      G4int ispin2 = G4int(2.0*spin + 0.0001);
      G4int ixji2 = G4int(2.0*xji + 0.0001);

      sum[k] += std::pow((2.0*f + 1.0)*G4Clebsch::Wigner6J(if2,if2,irk2,ispin2,ispin2,ixji2),2.0)/(2.0*xji + 1.0);
    }
      
  }

  for(G4int k=0;k<3;k++) {   
    G4int k1 = 2*k + 1;
    aks[k1] += sum[k];
  }

  G4double hmean = Field*iz*std::pow(beta,Power);
  G4double wsp = 4789.0*Gfac*hmean/Avji;
  wsp *= TimeC;
  wsp *= (wsp*Avji*(Avji+1.0)/3.0);

  std::array<G4double,7> Gk = {1.0,1.0,1.0,1.0,1.0,1.0,1.0};
  for(G4int k=0;k<3;k++) {
    
    G4int k2 = 2*k + 2;
    G4int k1 = 2*k + 1;

    G4double wrt = wsp*k2*(k2+1);
    G4double w2 = wrt;

    wrt *= (-1.0/(1.0 - aks[k2-1]));

    G4double xlam = (1.0 - aks[k2-1])*(1.0 - std::exp(wrt))/TimeC;
    G4double up = (Gam*time*aks[k1-1] + 1.0)/(time*Gam + 1.0);
    up *= (Xlamb*time);
    up += 1.0;

    G4double down = time*(xlam+Xlamb) + 1.0;
    Gk.at(k2) = up/down;
    
    G4double alp = std::sqrt(9.0*xlam*xlam + 8.0*xlam*TimeC*(w2 - xlam*xlam)) - 3.0*xlam;
    alp /= (4.0*xlam*TimeC);
    
    G4double upc = xlam*time*(down - 2.0*alp*alp*time*TimeC);
    G4double dwc = (down + alp*time)*(down + 2.0*alp*time);
    G4double ccf = 1.0 + upc/dwc;
    Gk.at(k2) *= ccf;

  }
  
  return Gk;

}

void Polarization::XSTATIC(const G4int iz, const G4double beta, G4int& id, G4int& iu, G4double& qcen,
			   G4double& dq, G4double& xnor) {

  G4double h = 1.0/(1.0 + std::pow(std::pow(iz,0.45)*0.012008/beta,5.0/3.0));
  qcen = iz*std::pow(h,0.6);
  dq = std::sqrt(qcen*(1.0-h))/2.0;

  iu = G4int(qcen + 3.0*dq + 0.5);
  id = G4int(qcen - 3.0*dq - 0.5);

  if(iu > iz) {
    iu = iz;
  }
  if(id < 1) {
    id = 1;
  }

  xnor = 0.0;
  for(G4int i=id;i<iu+1;i++) {
    xnor += std::exp(-std::pow((qcen-i)/dq,2)/2.0);
  }

  return;
  
}

G4double Polarization::ATS(G4int Ne) {
  
  if ( Ne <= 0 || Ne > 96 ) {
    return 0.0;
  }
    
  G4int mi = Ne/2 + 1;
  if (Ne%2) {
    
    if ( mi==1 || mi==2 || mi==3 || mi==6 || 
	 mi==7 || mi==10 || mi==15 || mi==16 || 
	 mi==19 || mi==24 || mi==25 || mi==28 || 
	 mi==31 || mi==35 || mi==37 || mi==40 || 
	 mi==41 || mi==44 ) {
      return 0.5;
    }
    else if ( mi==4 || mi==5 || mi==8 || mi==9 || 
	      mi==11 || mi==17 || mi==18 || mi==20 ||
	      mi==26 || mi==27 || mi==36 || mi==42 ||
	      mi==43 || mi==45 ) {
      return 1.5;
    }
    else if ( mi==12 || mi==14 || mi==21 || mi==23 ||
	      mi==32 || mi==39 ) {
      return 2.5;
    }
    else if ( mi==13 || mi==22 || mi==38 ) {
      return 4.5;
    }
    else if ( mi==29 || mi==30 || mi==48 ) {
      return 3.5;
    }
    else if ( mi==33 ) {
      return 7.5;
    }
    else if ( mi==34 ) {
      return 6.5;
    }
    else if ( mi==46 || mi==47 ) {
      return 5.5;
    }
  }
	   
  mi -= 1;
  if ( mi==4 || mi==8 || mi==17 || mi==26 || 
       mi==28 || mi==30 || mi==32 || mi==42 || 
       mi==45 || mi==48 ) {
    return 2.0;
  }
  else if ( mi==10 || mi==36 ) {
    return 1.0;
  }
  else if ( mi==12 || mi==21 || mi==37 ) {
    return 3.0;
  }
  else if ( mi==13 || mi==22 || mi==29 || mi==31 || 
	    mi==34 || mi==38 || mi==47 ) {
    return 4.0;
  }
  else if ( mi==33 ) {
    return 8.0;
  }
  else if ( mi==46 ) {
    return 6.0;
  }

  return 0.0;
   
}
