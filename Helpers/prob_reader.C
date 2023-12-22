std::string file_nameP = "Examples/Probabilities/cd106_on_ti48.prb";

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

void prob_reader(int index = 0) {

  gStyle->SetOptStat(0);
  
  std::vector<double> energies;
  std::vector<double> thetas;
  std::vector<double> probs = ReadProbFile(file_nameP,energies,thetas);

  int numE = energies.size();
  int numT = thetas.size();

  int numS = probs.size()/(numE*numT);
  if(index >= numS) {
    std::cout << "Only " << numS << " states (max. index = " << numS-1 << ")" << std::endl;
    return;
  }
  
  TGraph2D* g = new TGraph2D(numT*numE);
  g->SetName(Form("gP%02d",index));
  
  g->SetTitle(Form("State %d Excitation Probabilities; Energy (MeV); ThetaCM (deg); Probability",index));
  g->SetMarkerStyle(20);
  g->SetMarkerSize(1.5);
  g->GetXaxis()->SetTitleOffset(2.2);
  g->GetYaxis()->SetTitleOffset(2.2);
  g->GetZaxis()->SetTitleOffset(1.2);

  double spE = (energies.back() - energies.front())/double(numE - 1);
  double elow = energies.front() - spE/2.0;
  double emax = energies.back() + spE/2.0;

  double spT = (thetas.back() - thetas.front())/double(numT - 1);
  double tlow = thetas.front() - spT/2.0;
  double tmax = thetas.back() + spT/2.0;
  
  GH2D* h = new GH2D(Form("hP%02d",index),
		     Form("State %d Excitation Probabilities; Energy (MeV); ThetaCM (deg); Probability",index),
		     numE,elow,emax,numT,tlow,tmax);
  
  for(int i=0;i<numE;i++) {
    double en = energies[i];

    for(int j=0;j<numT;j++) {
    
      double th = thetas[j];
      double p = probs[index*numE*numT + j*numE + i];
      
      g->SetPoint(i*numT + j,en,th,p);
      h->Fill(en,th,p);
    }
  }

  new GCanvas();
  g->Draw("pcol");

  new GCanvas();
  h->Draw("colz");
  
  return;
}
