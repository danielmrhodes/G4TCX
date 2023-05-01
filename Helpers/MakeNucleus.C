void MakeNucleus() {

  ////////////////
  //This script takes a GOSIA output file and creates a CYGNUS nucleus file

  //Change these
  int Z = 36;
  int A = 78;
  int nStates = 18;

  //This is which printout of matrix elements you want from the GOSIA output. There can be up to three printouts.
  int nBlock = 0; //zero means the first printout, normally what is input to OP,ME

  std::ifstream inFile;
  inFile.open("GOSIA.out",std::ios::in); //GOSIA output, must already exist
  
  std::ofstream outFile;
  outFile.open("kr78.txt",std::ios::out); //New file will be created with this name
  ////////////////
  
  if(!inFile.is_open() ) {	
    std::cout << "Unable to open input file" << std::endl;
    return;
  }
  
  std::map<const char,std::string> map ={{'1',"E1"},{'2',"E2"},{'3',"E3"},{'4',"E4"},
					 {'5',"E5"},{'6',"E6"},{'7',"M1"},{'8',"M2"}};
  
  outFile << Z << "\t" << A << "\t" << nStates << "\t7\n";
  
  string qry = "                                        LEVELS";
  std::string line;
  while(std::getline(inFile,line)) {

    if(line.compare(qry) == 0)
      break;

  }

  std::getline(inFile,line);
  std::getline(inFile,line);
  while(std::getline(inFile,line)) {

    if(line.empty())
      break;
    
    int index;
    string parity;
    double energy, spin;

    std::stringstream ss(line);
    ss >> index >> parity >> spin >> energy;

    int par = 1;
    if(parity.compare("-")==0)
      par=-1;
    
    outFile << index << "\t" << par << "\t" << spin << "\t" << energy << "\n";
    
  }
  

  int count = 0;
  qry = "                                        MATRIX ELEMENTS";
  while(std::getline(inFile,line)) {
    
    if(line.compare(qry) != 0)
      continue;
    if(count < nBlock) {
      count++;
      continue;
    }
    
    std::getline(inFile,line);
    std::getline(inFile,line);

    std::string mult;
    while(std::getline(inFile,line)) {
      if(line.find("MULTIPOLARITY") != std::string::npos) {
	mult = map[line.back()];
	std::getline(inFile,line);
	continue;
      }

      if(line.empty() ||
	 line.compare("               ********* END OF EXECUTION **********") == 0)
	return;

      int ni, nf;
      double me;

      std::stringstream ss(line);
      ss >> ni >> ni >> nf >> me;

      outFile << ni-1 << "\t" << nf-1 << "\t" << me << "\t\t" << mult << "\n";
      
    }
    
    return;
  }
  
  return;
}
