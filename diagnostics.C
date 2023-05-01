#include "/path/to/G4TCX/include/Data_Format.hh"

void diagnostics(std::string input_filename = "output-info.dat",
		 std::string output_filename = "output-info.root") {

  if(!strcmp(input_filename.c_str(),output_filename.c_str())) {
    std::cout << "Give your input and output files different names" << std::endl;
    return;
  }

  const int nStatesP = 1;
  const int nStatesR = 1;

  TH1* hevt = new TH1D("hevt","Event Number",10000,0.0,50000000.0);
  
  TH1* htCM = new TH1D("htCM","Theta CM",360,0.0,180.0);
  TH1* hEn = new TH1D("hEn","Reaction Energy",1000,200.0,800.0);
  
  TH1* hp = new TH1D("hp","Projectile population",nStatesP,0,nStatesP);
  TH1* hr = new TH1D("hr","Recoil population",nStatesR,0,nStatesR);

  TH1* htCM_pds = new TH1D("htCM_PDS","Theta CM, projDS Gate",360,0.0,180.0);
  TH1* hp_pds = new TH1D("hpPDS","Projectile population, projDS Gate",nStatesP,0,nStatesP);
  TH1* hr_pds = new TH1D("hrPDS","Recoil population, projDS Gate",nStatesR,0,nStatesR);

  TH1* htCM_pus = new TH1D("htCM_PUS","Theta CM, projUS Gate",360,0.0,180.0);
  TH1* hp_pus = new TH1D("hpPUS","Projectile population, projUS Gate",nStatesP,0,nStatesP);
  TH1* hr_pus = new TH1D("hrPUS","Recoil population, projUS Gate",nStatesR,0,nStatesR);

  TH1* htCM_rec = new TH1D("htCM_REC","Theta CM, Recoil Gate",360,0.0,180.0);
  TH1* hp_rec = new TH1D("hpREC","Projectile population, Recoil Gate",nStatesP,0,nStatesP);
  TH1* hr_rec = new TH1D("hrREC","Recoil population, Recoil Gate",nStatesR,0,nStatesR);

  //TH3* hp3 = new TH3D("hp3","Projectile population",360,0,360,600,200,800,nStatesP,0,nStatesP);
  //TH3* hr3 = new TH3D("hr3","Recoil population",360,0,360,600,200,800,nStatesR,0,nStatesR);

  std::vector<TH2*> pGrids;
  std::vector<TH2*> pGrids_pds;
  std::vector<TH2*> pGrids_pus;
  std::vector<TH2*> pGrids_rec;
  for(int i=0;i<nStatesP;i++) {
    pGrids.push_back(new TH2D(Form("pG%02d",i),Form("Projectile State %02d Population",i),
			      360,0.0,180.0,1000,200.0,800.0));

    pGrids_pds.push_back(new TH2D(Form("pGpds%02d",i),
				  Form("Projectile State %02d Population, projDS Gate",i),
				  360,0.0,180.0,1000,200.0,800.0));

    pGrids_pus.push_back(new TH2D(Form("pGpus%02d",i),
				  Form("Projectile State %02d Population, projUS Gate",i),
				  360,0.0,180.0,1000,200.0,800.0));

    pGrids_rec.push_back(new TH2D(Form("pGrec%02d",i),
				  Form("Projectile State %02d Population, Recoil Gate",i),
				  360,0.0,180.0,1000,200.0,800.0));
  }

  std::vector<TH2*> rGrids;
  std::vector<TH2*> rGrids_pds;
  std::vector<TH2*> rGrids_pus;
  std::vector<TH2*> rGrids_rec;
  for(int i=0;i<nStatesR;i++) {
    rGrids.push_back(new TH2D(Form("rG%02d",i),Form("Recoil State %02d Population",i),
			      360,0.0,180.0,1000,200.0,800.0));

    rGrids_pds.push_back(new TH2D(Form("rGpds%02d",i),
				  Form("Recoil State %02d Population, projDS Gate",i),
				  360,0.0,180.0,1000,200.0,800.0));

    rGrids_pus.push_back(new TH2D(Form("rGpus%02d",i),
				  Form("Recoil State %02d Population, projUS Gate",i),
				  360,0.0,180.0,1000,200.0,800.0));

    rGrids_rec.push_back(new TH2D(Form("rGrec%02d",i),
				  Form("Recoil State %02d Population, Recoil Gate",i),
				  360,0.0,180.0,1000,200.0,800.0));
  }
  
  FILE* input_file = fopen(input_filename.c_str(),"rb");
  
  INFO info;
  while(fread(&info,info.bytes(),1,input_file)) {

    const int num = info.evtNum;
    const int pIndex = info.indexP;
    const int rIndex = info.indexR;
    
    const double energy = info.beamEn;
    const double thetaCM = info.thetaCM;
    
    const bool prjDS = info.projDS;
    const bool prjUS = info.projUS;
    const bool rec = info.rec;
    
    hevt->Fill(num);
    hEn->Fill(energy);
    htCM->Fill(thetaCM);
    
    hp->Fill(pIndex);
    hr->Fill(rIndex);

    //hp3->Fill(thetaCM,energy,pIndex);
    //hr3->Fill(thetaCM,energy,rIndex);

    if(pIndex < nStatesP)
      pGrids.at(pIndex)->Fill(thetaCM,energy);

    if(rIndex < nStatesR)
      rGrids.at(rIndex)->Fill(thetaCM,energy);

    if(prjDS) {

      htCM_pds->Fill(thetaCM);
      hp_pds->Fill(pIndex);
      hr_pds->Fill(rIndex);
      
      if(pIndex < nStatesP)
	pGrids_pds.at(pIndex)->Fill(thetaCM,energy);

      if(rIndex < nStatesR)
	rGrids_pds.at(rIndex)->Fill(thetaCM,energy);
      
    }

    if(prjUS) {

      htCM_pus->Fill(thetaCM);
      hp_pus->Fill(pIndex);
      hr_pus->Fill(rIndex);
      
      if(pIndex < nStatesP)
	pGrids_pus.at(pIndex)->Fill(thetaCM,energy);

      if(rIndex < nStatesR)
	rGrids_pus.at(rIndex)->Fill(thetaCM,energy);
      
    }

    if(rec) {

      htCM_rec->Fill(thetaCM);
      hp_rec->Fill(pIndex);
      hr_rec->Fill(rIndex);

      if(pIndex < nStatesP)
	pGrids_rec.at(pIndex)->Fill(thetaCM,energy);

      if(rIndex < nStatesR)
	rGrids_rec.at(rIndex)->Fill(thetaCM,energy);
      
    }
    
  }

  TFile* outFile = new TFile(output_filename.c_str(),"RECREATE");
  outFile->cd();

  outFile->mkdir("Grids");
  
  outFile->mkdir("projDS");
  outFile->mkdir("projDS/Grids");

  outFile->mkdir("projUS");
  outFile->mkdir("projUS/Grids");

  outFile->mkdir("Recoil");
  outFile->mkdir("Recoil/Grids");
  
  hevt->Write();
  htCM->Write();
  hEn->Write();
  
  hp->Write();
  hr->Write();

  //hp3->Write();
  //hr3->Write();

  outFile->cd("Grids");
  for(auto h : pGrids)
    h->Write();

  for(auto h : rGrids)
    h->Write();

  outFile->cd("projDS");
  htCM_pds->Write();
  hp_pds->Write();
  hr_pds->Write();

  outFile->cd("projDS/Grids");

  for(auto h : pGrids_pds)
    h->Write();

  for(auto h : rGrids_pds)
    h->Write();

  outFile->cd("projUS");
  htCM_pus->Write();
  hp_pus->Write();
  hr_pus->Write();
  
  outFile->cd("projUS/Grids");

  for(auto h : pGrids_pus)
    h->Write();

  for(auto h : rGrids_pus)
    h->Write();

  outFile->cd("Recoil");
  htCM_rec->Write();
  hp_rec->Write();
  hr_rec->Write();

  outFile->cd("Recoil/Grids");

  for(auto h : pGrids_rec)
    h->Write();

  for(auto h : rGrids_rec)
    h->Write();
  
  outFile->Close();
  
  return;
}

