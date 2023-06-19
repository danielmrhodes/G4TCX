std::vector<double> ReadProbFile(std::string fn, std::vector<double>& energies,
				 std::vector<double>& thetas) {

  thetas.clear();
  energies.clear();
  
  std::ifstream file;
  file.open(fn.c_str(),std::ios::in);

  std::string line;
  std::getline(file,line);

  double en;
  std::stringstream ss1(line);

  while(ss1 >> en)
    energies.push_back(en);
  int numE = energies.size();

  std::getline(file,line);

  double th;
  std::stringstream ss2(line);

  while(ss2 >> th)
    thetas.push_back(th);
  int numT = thetas.size();

  std::getline(file,line);
  
  
  int indexE = 0;
  int indexT = 0;
  int indexS = 0;

  std::vector<double> probs;
  while(std::getline(file,line)) {

    if(line.empty()) {
      indexS = 0;
      indexE++;
      continue;
    }

    if(probs.size() == indexS*numE*numT)
      probs.resize(probs.size() + numE*numT);
    
    double prb;
    std::stringstream ss3(line);
    while(ss3 >> prb) {
      int num = indexS*numE*numT + indexT*numE + indexE;
      probs[num] = prb;
      indexT++;
    }

    indexT = 0;
    indexS++;
  }
  
  return probs;
}

TGraph2D* g;
void prob_reader(int index = 0) {

  std::string file_name = "Examples/Probabilities/cd106_on_ti48.prb";
  
  std::vector<double> energies;
  std::vector<double> thetas;
  std::vector<double> probs = ReadProbFile(file_name,energies,thetas);

  int numE = energies.size();
  int numT = thetas.size();

  int numS = probs.size()/(numE*numT);
  if(index >= numS) {
    std::cout << "Only " << numS << " states (max. index = " << numS-1 << ")" << std::endl;
    return NULL;
  }
  
  if(!g)
    g = new TGraph2D(numT*numE);
  
  g->SetTitle(Form("State %d Excitation Probabilities; Energy (MeV); Theta (deg); Probability",index));
  g->SetMarkerStyle(20);
  g->SetMarkerSize(1.5);
  g->GetXaxis()->SetTitleOffset(2.2);
  g->GetYaxis()->SetTitleOffset(2.2);
  g->GetZaxis()->SetTitleOffset(1.2);
  
  for(int i=0;i<numE;i++) {
    double en = energies[i];

    for(int j=0;j<numT;j++) {
    
      double th = thetas[j];
      double p = probs[index*numE*numT + j*numE + i];
      
      g->SetPoint(i*numT + j,en,th,p);
      
    }
  }

  g->Draw("pcol");
  
  return;
}
