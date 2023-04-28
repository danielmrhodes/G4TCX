#include "Tracking_Action.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"

Tracking_Action::Tracking_Action()  {
  simple_source = false;
}
Tracking_Action::~Tracking_Action() {}

void Tracking_Action::PreUserTrackingAction(const G4Track* track) {

  switch(mode) {

    case Primary_Generator::MODE::Scattering: {
      break;
    }

    case Primary_Generator::MODE::Source: {

      G4int PID = track->GetParentID();
      G4int TID = track->GetTrackID();
      
      if(simple_source) {	
	
	if(!PID) {
	  enMap[TID] = track->GetKineticEnergy();
	  idMap[TID].push_back(TID);
	}
	else
	  idMap.begin()->second.push_back(TID);
	
      }
      else {
	
	if(track->GetDefinition()->GetParticleType() == "nucleus") {
	  ionIDs.push_back(TID);
	
	  break;
	}
      
	if((std::find(ionIDs.begin(),ionIDs.end(),PID) != ionIDs.end()) &&
	   (track->GetDefinition()->GetParticleName() == "gamma")) {
	
	  enMap[TID] = track->GetKineticEnergy();
	  idMap[TID].push_back(TID);

	}
	else  {
	  if(idMap.count(PID)) {
	    idMap[PID].push_back(TID);	  
	  }
	  else {
	    for(auto it = idMap.begin();it != idMap.end();++it) {
	      if(std::find(it->second.begin(),it->second.end(),PID) != it->second.end()) { 
		it->second.push_back(TID);
		break;
	      }
	    }
	  }
	}
      }
      
      break;
    }

    case Primary_Generator::MODE::Full: {

      G4int PID = track->GetParentID();
      G4int TID = track->GetTrackID();
     
      if(track->GetDefinition()->GetParticleType() == "nucleus") {
	ionIDs.push_back(TID);

	if(track->GetDefinition()->GetParticleName().find(projName) != std::string::npos) {
	  projIDs.push_back(TID);
	}
	
	break;
      }
      
      if((std::find(ionIDs.begin(),ionIDs.end(),PID) != ionIDs.end()) &&
	 (track->GetDefinition()->GetParticleName() == "gamma")) {
	
	enMap[TID] = track->GetKineticEnergy();
	idMap[TID].push_back(TID);

	if(std::find(projIDs.begin(),projIDs.end(),PID) != projIDs.end()) {
	  projGammas.push_back(TID);
	}
      }
      else  {
	if(idMap.count(PID)) {
	  idMap[PID].push_back(TID);	  
	}
	else {
	  for(auto it = idMap.begin();it != idMap.end();++it) {
	    if(std::find(it->second.begin(),it->second.end(),PID) != it->second.end()) { 
	      it->second.push_back(TID);
	      break;
	    }
	  }
	}
      }
      
      break;
    }
      
  }
  
  return;
}

void Tracking_Action::Clear() {

  ionIDs.clear();
  projIDs.clear();
  projGammas.clear();
  
  idMap.clear();
  enMap.clear();

  return;
  
}
