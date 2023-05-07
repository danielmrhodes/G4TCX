void MakeLevelScheme() {

  //This script takes a Cygnus nucleus file as input and outputs the level scheme file necessary for G4TCX
  //I would double-check all the branching ratios came out correct as those are most important

  const std::string input = "pd110.txt"; //Cygnus nucleus file, must already exist
  const std::string output = "pd110.lvl"; //A new file will be created with this name

  std::ofstream file;
  file.open(output.c_str(),std::ios::out);
  
  NucleusReader* reader = new NucleusReader(input.c_str());
  Nucleus* nuc = reader->GetNucleus();
  TransitionRates* trans = new TransitionRates(nuc);

  std::vector<double> energies = nuc->GetLevelEnergies();
  std::vector<double> spins = nuc->GetLevelJ();
  
  TVectorD lifetimes = trans->GetLifetimes();
  TMatrixD branchings = trans->GetBranchingRatios();
  TMatrixD mixings = trans->GetMixingRatios();
  std::vector<TMatrixD> elements = trans->GetMatrixElements();

  int numS = nuc->GetNstates();
  for(int i=1;i<numS;i++) {

    double en = 1000.0*energies.at(i);
    double sp = spins.at(i);
    double lt = lifetimes[i];

    int num = 0;
    for(int j=0;j<i;j++) {
      if(branchings[j][i] > 0.0)
	num++;
    }

    file << i << " " << en << " " << sp << " " << lt << " " << num << "\n";
    for(int j=0;j<numS;j++) {

      double br = branchings[j][i];
      if(br <= 0.0)
	continue;

      if(j>i)
	std::cout << "Warning: State " << i << " has a decay branch to state " << j
		  << ". This will make the output file unparsable by G4TCX. " << std::endl;

      int l1 = 2;
      int l2 = 0;
      
      for(int k=0;k<5;k++) {
	TMatrixD mes = elements.at(k);

	if(mes[j][i] > 0.0)
	  l1 = k+1;
      }  

      double mx = mixings[j][i];
      if(mx > 0.0)
	l2 = 1;
      
      if(l2 == 1 && l1 != 2) { //Not implemented yet
	std::cout << "BOOOO!" << std::endl;
	file.close();
	return;
      }
      
      file << " " << j << " " << br << " " << l1 << " " << l2 << " " << mx << " " << 0.0 << "\n";
    }
    
  }
  file.close();
  
  return;
}
