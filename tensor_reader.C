//Change this, DO NOT include the ground state spin
//Ordering must be the same as the level scheme file
//std::vector<double> spins = {2.0,4.0,2.0,0.0,4.0,3.0,6.0,5.0,8.0,2.0,6.0}; //106Cd
//std::vector<double> spins = {2.0,4.0,2.0,6.0,4.0}; //48Ti
std::vector<double> spins = {2.0,4.0,6.0,8.0,10.0,12.0,2.0,3.0,4.0,5.0,6.0,0.0,2.0,4.0,3.0,5.0,7.0}; //78Kr

int MaxK(double spin) {

  int twoJ = int(2.0*spin + 0.01);
  if(twoJ >= 6)
    return 6;
  return twoJ;
  
}

int NumComps(double spin) {
  
  int num = 0;
  for(int k=0;k<=MaxK(spin);k+=2)
    num += k + 1;

  return num;
}

int GetOffset(int index, int k, int kappa) {
  
  int offset = 0;
  for(int i=0;i<index-1;i++)
    offset += NumComps(spins[i]);

  for(int i=0;i<k;i+=2)
    offset += i + 1;

  offset += kappa;

  return offset;
}

void ReadTensorFile(std::string fn, std::vector<double>& energies, std::vector<double>& thetas,
		    std::vector<double>& vals) {

  thetas.clear();
  energies.clear();
  vals.clear();
  
  std::ifstream file;
  file.open(fn.c_str(),std::ios::in);

  std::string line;
  std::getline(file,line);

  double en;
  std::stringstream ss1(line);

  while(ss1 >> en)
    energies.push_back(en);

  std::getline(file,line);

  double th;
  std::stringstream ss2(line);

  while(ss2 >> th)
    thetas.push_back(th);

  int numE = energies.size();
  int numT = thetas.size();
  int indexE = 0;
  int indexT = 0;

  for(auto sp : spins) 
    vals.resize(vals.size() + numE*numT*NumComps(sp));
  
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

    int index, k, kappa;
    double spin, val;
    
    std::stringstream ss3(line);
    ss3 >> index >> spin >> k >> kappa >> val;

    int offset = GetOffset(index,k,kappa)*numE*numT;
    vals[offset + indexT*numE + indexE] = val;
 
  }
  
  return;
}

TGraph2D* g;
TGraph2D* tensor_reader(int index = 1, int k=0, int kappa=0, bool norm=true) {

  if(index < 1) {
    std::cout << "index must be larger than 0" << std::endl;
    return NULL;
  }

  if(index > spins.size()) {
    std::cout << "Only " << spins.size() << " excited states" << std::endl;
    return NULL;
  }
  
  if(k%2) {
    std::cout << "k should be even" << std::endl;
    return NULL;
  }
  
  if(k > MaxK(spins.at(index-1))) {
    std::cout << "max k is " << MaxK(spins.at(index-1)) << " for state " << index
	      << std::endl;
    return NULL;
  }

  if(kappa > k) {
    std::cout << "kappa can't be larger than k" << std::endl;
    return NULL;
  }
  
  std::string file_name = "tensors.ten";
  std::vector<double> energies;
  std::vector<double> thetas;
  std::vector<double> values;
  ReadTensorFile(file_name,energies,thetas,values);

  int numE = energies.size();
  int numT = thetas.size();
  
  if(!g)
    g = new TGraph2D(numT*numE);
  
  if(norm)
    g->SetTitle(Form("State %d Polarization Tensor Component %d %d; Energy (MeV); Theta (deg); Value",index,k,kappa));
  else
    g->SetTitle(Form("State %d Statistical Tensor Component %d %d; Energy (MeV); Theta (deg); Value",
		     index,k,kappa));
  
  g->SetMarkerStyle(20);
  g->SetMarkerSize(1.5);
  g->GetXaxis()->SetTitleOffset(2.2);
  g->GetYaxis()->SetTitleOffset(2.2);
  g->GetZaxis()->SetTitleOffset(1.2);

  int off00 = GetOffset(index,0,0)*numE*numT;
  int offset = GetOffset(index,k,kappa)*numE*numT;
  for(int i=0;i<numE;i++) {
    double en = energies[i];
   
    for(int j=0;j<numT;j++) {
      double th = thetas[j];
      
      double val = values[offset + j*numE + i];
      if(norm)
	val /= values[off00 + j*numE + i];
      
      g->SetPoint(i*numT + j,en,th,val);
      
    }
  }

  g->Draw("pcol");
  
  return g;
}
