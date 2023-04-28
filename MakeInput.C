void MakeInput() {
  
  ///Change these///
  
  const int beamA = 78;
  const int beamZ = 38;
  const int targA = 197;
  const int targZ = 79;

  //Energy range for deltaE across target
  const double eMax = 432.0;
  const double eMin = 384.0;
  const int nBinsE = 12; //Nmber of energy meshpoints

  const int nBinsT = 30; //Number of scattering angle meshpoints

  const std::string input = "kr78.txt"; //Cygnus nucleus file, must already exist
  const bool projectile_excitation = true; //Change projectile or target excitation

  //New files will be created with these names
  const std::string outputP = "probabilities.prb";
  const std::string outputT = "tensors.ten";
  
  //////////////////

  if(eMax < eMin) {
    std::cout << "eMax should be larger than eMin" << std::endl;
    return;
  }
  
  std::ofstream fileP;
  fileP.open(outputP.c_str(),std::ios::out);
  
  std::ofstream fileT;
  fileT.open(outputT.c_str(),std::ios::out);

  NucleusReader* reader = new NucleusReader(input.c_str());
  Nucleus* nuc = reader->GetNucleus();
  
  std::cout << (nBinsE+1)*(nBinsT) << " total CoulEx calculations (" << nBinsE+1
	    << " energy meshpoints, " << nBinsT << " angle meshpoints)" << std::endl;

  for(int i=0;i<nBinsE+1;i++) {

    double en = eMin + i*(eMax-eMin)/double(nBinsE);
    fileP << en;
    fileT << en;
    
    if(i < nBinsE) {
      fileP << " ";
      fileT << " ";
    }
    else {
      fileP << "\n";
      fileT << "\n";
    }
    
  }

  for(int i=0;i<nBinsT;i++) {
    
    double thetaCM = (TMath::Pi()/(double)nBinsT)*(i+1);
    fileP << thetaCM;
    fileT << thetaCM;
    
    if(i < nBinsT - 1) {
      fileP << " ";
      fileT << " ";
    }
    else {
      fileP << "\n\n";
      fileT << "\n\n";
    }
  }

  int nStates = nuc->GetNstates();
  int num = 1;
  for(int i=0;i<nBinsE+1;i++) {

    double en = eMin + i*(eMax-eMin)/double(nBinsE);
    
    Reaction* reac = new Reaction(beamA,beamZ,targA,targZ,en);
    PointCoulEx* poin = new PointCoulEx(nuc,reac);
    poin->SetProjectileExcitation(projectile_excitation);
    poin->PrepareConnections();

    std::vector<TVectorD> probs;
    for(int j=0;j<nBinsT;j++) {

      double thetaCM = (TMath::Pi()/(double)nBinsT)*(j+1)*TMath::RadToDeg();
      std::cout << "Point " << num << ": Ebeam = " << en << " MeV, ThetaCM = " << thetaCM << " deg\r"
		<< std::flush;

      probs.push_back(poin->PointProbabilities(thetaCM));
      poin->CalculateTensors();

      StatisticalTensor	tensors = poin->GetTensors();
      for(int k=0;k<nStates-1;k++) {
	StateTensor tmpTensor = tensors.GetStateTensor(k);
	for(int l=0;l<tmpTensor.GetNelements();l++) {
	  fileT << tmpTensor.GetState() << " " << nuc->GetLevelJ()[k+1] << " " << tmpTensor.GetK(l)
		<< " " << tmpTensor.GetKappa(l) << " " << tmpTensor.GetTensor(l) << "\n";
	}
      }
      
      fileT << "\n";
      num++;
    }
    
    for(int j=0;j<nStates;j++) {
      for(int k=0;k<nBinsT;k++) {
	fileP << probs[k][j];
	if(k<nBinsT-1)
	  fileP << " ";
	else
	  fileP << "\n";
      }
    }

    fileP << "\n";
    fileT << "\n";
  }
  std::cout << "\nDone!" << std::endl;
  
  fileP.close();
  fileT.close();

  return;
}
