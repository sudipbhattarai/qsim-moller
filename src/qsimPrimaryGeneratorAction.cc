#include "qsimPrimaryGeneratorAction.hh"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <TFile.h>
#include <TH2.h>
#include <TTree.h>
#include <TLeaf.h>
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "qsimIO.hh"
#include "qsimEvent.hh"
#include "qsimtypes.hh"
#include "globals.hh"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGauss.h"

#include "G4SystemOfUnits.hh"

#include <assert.h> // Added to facilitate assert error


bool qsimPrimaryGeneratorAction::Thetaspectrum(double Th) {
	double test = CLHEP::RandFlat::shoot(0.0,1.0);

	if ( fSourceMode == 1 || ((cos(Th)*cos(Th)) > test) )
		return true;
	else
		return false;
}

//void qsimPrimaryGeneratorAction::SourceModeSet() {
//	SourceModeSet(0); // point to the one below with default settings = 0. // should I just use default parameters?
//}

// allow user modifications of private member and functional modifiable definition of primary generator variables
void qsimPrimaryGeneratorAction::SourceModeSet(G4int mode = 0) {
	fSourceMode = mode;
	// 0 is cosmic mode
	// 1 is beam mode
	// 2 is remoll mode
	if (fSourceMode==0){
		fXmin =  -5.0*cm;
		fXmax =  5.*cm;
		
		fYmin =  -5.*cm;
		fYmax =  5.*cm;

		fZmin = 90.*cm;
		fZmax = 90.*cm;
		
		fEmin = 10.0*MeV;
		fEmax = 50.0*GeV;
		
		fthetaMin = 0.0*deg;
		fthetaMax = 90.0*deg;

	} else if (fSourceMode==1) {
		fXmin =  0.0*cm;//-0.05*cm-0.1*mm-(53/4)*mm+10*mm;//-245.2*mm/2; // pinpoint at Mainz
		fXmax =  0.0*cm;//-0.05*cm-0.1*mm-(53/4)*mm+10*mm;//245.2*mm/2; // questionable at JLab
		
		fYmin =  0.0*mm;//-123.0*mm;
		fYmax =  0.0*mm;//123.0*mm;
		
		fEmin = 855.0*MeV; // = 855 MeV at Mainz
		fEmax = 855.0*MeV; // = 1.063 Gev for JLab
		
		fthetaMin = 0*deg;
		fthetaMax = 0*deg;

		fphiMin = 0*deg;
		fphiMax = 0*deg;

	} else if (fSourceMode==2){
		fEmin = 2.0*GeV; 
		fEmax = 8.0*GeV; 
	}
}

qsimPrimaryGeneratorAction::qsimPrimaryGeneratorAction() {
	G4int n_particle = 1;
	
	SourceModeSet(); // Accelerator beam mode, default set to 0, setting the mode to cosmic stand mode.
		
	fParticleGun = new G4ParticleGun(n_particle);
	fDefaultEvent = new qsimEvent();
	
	fZ = -41.0*mm;
}


qsimPrimaryGeneratorAction::~qsimPrimaryGeneratorAction() {
	delete fParticleGun;
	delete fDefaultEvent;
}


bool qsimPrimaryGeneratorAction::pspectrum(double p) {
	double test = CLHEP::RandFlat::shoot(0.0,1.0) ;
	
	// Muon energy spctrum obtained from and fit to PDG data for 0 degree incident angle
	// good to 25% out to 36 GeV.
	// if the accelerator mode is on then just return true anyway.
	if ( fSourceMode==1 || fSourceMode == 2 || (((pow(2.5*p/GeV,-2.7)*(exp(-0.7324*(pow(log(2.5*p/GeV),2))+4.7099*log(2.5*p/GeV)-1.5)))/(0.885967*2.5)) > test) ) 
		return true;
	else 
		return false;
}


void qsimPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
  
	/*  Generate event, set IO data */
  
	// Use default, static single generator
	// Update this just in case things changed
	// from the command user interface
	fDefaultEvent->Reset();
  
	// Set data //////////////////////////////////
	// Magic happens here
  
  
	if( fSourceMode == 0 || fSourceMode == 1) {
		xPos = CLHEP::RandFlat::shoot( fXmin, fXmax );
		yPos = CLHEP::RandFlat::shoot( fYmin, fYmax );
	}
	zPos = CLHEP::RandFlat::shoot( fZmin, fZmax );
	
	// begin changed stuff to generate probability distribution of energies as expected
	bool good_p = false;
	double p3sq, E;
	double mass = fParticleGun->GetParticleDefinition()->GetPDGMass();
	
	while ( good_p == false ) {
		E = CLHEP::RandFlat::shoot( fEmin, fEmax );
		p3sq = E*E - mass*mass;
		if( p3sq < 0 ) continue;
		
		good_p = pspectrum(sqrt(p3sq));
	}
  
	// fTheta needs to be a random distribution determined by the cos^2(theta) distribution	
  
	p = sqrt( E*E - mass*mass );
	double randTheta, randPhi;
  
	if (fSourceMode == 0 || fSourceMode == 1) {
		bool goodTheta = false;
		while ( goodTheta == false ) {
			if (fSourceMode==1){
				randTheta = CLHEP::RandFlat::shoot( fthetaMin, fthetaMax );
				goodTheta = Thetaspectrum(randTheta);
                randPhi = CLHEP::RandFlat::shoot( fphiMin, fphiMax );
			}
			else if (fSourceMode==0){ //cosmic mode
				randTheta = CLHEP::RandGauss::shoot(0.0, 40/degree);
				if (randTheta <=fthetaMax && randTheta >= fthetaMin) goodTheta = true;
				else goodTheta = false;
                randPhi = CLHEP::RandFlat::shoot( fphiMin, fphiMax );
			}
		}
		
		// randPhi = CLHEP::RandFlat::shoot( 0.0, 360.0)*deg ;
		
		pX = sin(randTheta)*cos(randPhi)*p;
		pY = sin(randTheta)*sin(randPhi)*p;
		pZ = cos(randTheta)*p;
	}
	
	if (fSourceMode == 2) {
		static int currentEvent; // Counter to keep track of the current entry

		//G4String extGenFileName ="InputEventDistribution/smEvGen_moller_open_electron.root"; 
		G4String extGenFileName = fExtGenFileName;
		TFile *primaryFile = new TFile(extGenFileName);
		TTree *T = (TTree*)primaryFile->Get("T");
		nEvents = T->GetEntries(); // total number of events in the file

		if (currentEvent >= nEvents) {
			currentEvent = 0; // Reset to the first entry if we reach the end
		}
		T->GetEntry(currentEvent);
		// G4cout << "Event number: " << currentEvent << G4endl;

		xPos = T->GetLeaf("x")->GetValue()*mm;
		yPos = T->GetLeaf("y")->GetValue()*mm;
		pX = T->GetLeaf("px")->GetValue()*MeV;
		pY = T->GetLeaf("py")->GetValue()*MeV;
		pZ = T->GetLeaf("pz")->GetValue()*MeV;
		E = T->GetLeaf("energy")->GetValue()*MeV;
		pid = T->GetLeaf("pid")->GetValue();
		rate = T->GetLeaf("rate")->GetValue()*hertz;

		currentEvent++; // Increment the counter for the next event call
		primaryFile->Close();
	}
  
	//assert( E > 0.0 );
	//assert( E > mass );
  
	if (fSourceMode!=2){
		fDefaultEvent->ProduceNewParticle(
						G4ThreeVector(xPos, yPos, zPos),
						G4ThreeVector(pX, pY, pZ ),
						fParticleGun->GetParticleDefinition()->GetParticleName(), 
						rate );
	} else{
		fDefaultEvent->ProduceNewParticle(
						G4ThreeVector(xPos, yPos, zPos),
						G4ThreeVector(pX, pY, pZ ),
						pid,
						rate);

	}
  
	// Register and create event
  
	double kinE = sqrt(fDefaultEvent->fPartMom[0].mag()*fDefaultEvent->fPartMom[0].mag()
				+ fDefaultEvent->fPartType[0]->GetPDGMass()*fDefaultEvent->fPartType[0]->GetPDGMass() )
				- fDefaultEvent->fPartType[0]->GetPDGMass();
	
	fParticleGun->SetParticleDefinition(fDefaultEvent->fPartType[0]);
	fParticleGun->SetParticleMomentumDirection(fDefaultEvent->fPartMom[0].unit());
	fParticleGun->SetParticleEnergy( kinE  );
	fParticleGun->SetParticlePosition( fDefaultEvent->fPartPos[0] );
  
  
	fIO->SetEventData(fDefaultEvent);
  
	double randNum = CLHEP::RandFlat::shoot( 0.0, 1.0);
	//One may need to apply beam multiplicity
	double zero_pe = 0;
	double one_pe = 1;
	double two_pe = 0;
	double three_pe = 0;
	double four_pe = 0;
  
  
	if ( randNum >= 0 && randNum <= zero_pe) {
		//fParticleGun->GeneratePrimaryVertex(anEvent);
	} else if (randNum > zero_pe && randNum <= zero_pe + one_pe){
		fParticleGun->GeneratePrimaryVertex(anEvent);
	} else if (randNum > zero_pe + one_pe && randNum <= zero_pe + one_pe + two_pe){
		fParticleGun->GeneratePrimaryVertex(anEvent);
		fParticleGun->GeneratePrimaryVertex(anEvent);
	} else if (randNum > zero_pe + one_pe + two_pe && randNum <= zero_pe + one_pe + two_pe + three_pe){
		fParticleGun->GeneratePrimaryVertex(anEvent);
		fParticleGun->GeneratePrimaryVertex(anEvent);
		fParticleGun->GeneratePrimaryVertex(anEvent);
	} else if (randNum > zero_pe + one_pe + two_pe + three_pe && randNum <= zero_pe + one_pe + two_pe + three_pe + four_pe) {
		fParticleGun->GeneratePrimaryVertex(anEvent);
		fParticleGun->GeneratePrimaryVertex(anEvent);
		fParticleGun->GeneratePrimaryVertex(anEvent);
		fParticleGun->GeneratePrimaryVertex(anEvent);
	} else if (randNum > zero_pe + one_pe + two_pe + three_pe + four_pe && randNum <= 1) {
		fParticleGun->GeneratePrimaryVertex(anEvent);
		fParticleGun->GeneratePrimaryVertex(anEvent);
		fParticleGun->GeneratePrimaryVertex(anEvent);
		fParticleGun->GeneratePrimaryVertex(anEvent);
		fParticleGun->GeneratePrimaryVertex(anEvent);
	}
}

G4ParticleGun* qsimPrimaryGeneratorAction::GetParticleGun() {
	return fParticleGun;
} 

void qsimPrimaryGeneratorAction::setExtEvGenerator(G4String filename){
	fExtGenFileName = filename;
}
