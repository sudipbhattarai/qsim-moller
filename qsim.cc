/*
qsim - Quartz Detector Simluation
  
  Seamus Riordan, et al.
  riordan@jlab.org
  
  Based on prexsim by Jon Wexler and Luis Mercado
  and remoll
  
*/

#include "CLHEP/Random/Random.h"
#include "qsimRunAction.hh"
#include "qsimPrimaryGeneratorAction.hh"
#include "qsimEventAction.hh"
#include "qsimSteppingAction.hh"
#include "qsimSteppingVerbose.hh"
#include "qsimOpticalPhysics.hh"
#include "qsimDetectorConstruction.hh"
#include "qsimIO.hh"
#include "qsimMessenger.hh"

//  Standard physics list
#include "G4PhysListFactory.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4RunManagerKernel.hh"
#include "G4StepLimiterPhysics.hh"

//to make gui.mac work
#include <G4AutoLock.hh>
#include <G4UImanager.hh>
#include <G4UIExecutive.hh>
#include "G4GDMLParser.hh"
#include <G4UIterminal.hh>

#include "G4UIQt.hh"
#include "G4UItcsh.hh"
#include "G4VisExecutive.hh"

#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char** argv){
	
	unsigned int seed = time(0);
	
	CLHEP::HepRandom::createInstance();
	CLHEP::HepRandom::setTheSeed(seed);
	
	qsimIO *io = new qsimIO();
	
	// User Verbose output class
	G4VSteppingVerbose* verbosity = new qsimSteppingVerbose;
	G4VSteppingVerbose::SetInstance(verbosity);
	
	//-------------------------------
	// Initialization of Run manager
	//-------------------------------
	
	G4cout << "RunManager construction starting...." << G4endl;
	G4RunManager * runManager = new G4RunManager;

    // Create a serial run manager
    // auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial);

	
	qsimMessenger *rmmess = new qsimMessenger();
	rmmess->SetIO(io);

	// Detect interactive mode (if only one argument) and define UI session
	
	G4UIExecutive* ui = 0;
	if ( argc == 2 ) {
		ui = new G4UIExecutive(argc, argv);
	}
	
	G4GDMLParser parser;
	parser.Read(argv[1]);
	
	// Detector geometry
	G4VUserDetectorConstruction* detector = new qsimDetectorConstruction(parser);
	runManager->SetUserInitialization(detector);
	rmmess->SetDetCon( ((qsimDetectorConstruction *) detector) );
	
	// Physics we want to use
	G4int verbose = 4;
	G4PhysListFactory factory;
	G4VModularPhysicsList* physlist = factory.GetReferencePhysList("FTFP_BERT");//FTFP_BERT_HP
	physlist->RegisterPhysics(new G4StepLimiterPhysics());
	physlist->SetVerboseLevel(verbose);
	runManager->SetUserInitialization(physlist);
	physlist->RegisterPhysics( new qsimOpticalPhysics() );
	
	//-------------------------------
	// UserAction classes
	//-------------------------------

	G4UserRunAction* run_action = new qsimRunAction;
	((qsimRunAction *) run_action)->SetIO(io);
	runManager->SetUserAction(run_action);
	
	G4VUserPrimaryGeneratorAction* gen_action = new qsimPrimaryGeneratorAction;
	((qsimPrimaryGeneratorAction *) gen_action)->SetIO(io);
	rmmess->SetPriGen((qsimPrimaryGeneratorAction *)gen_action);
	runManager->SetUserAction(gen_action);
	
	G4UserEventAction* event_action = new qsimEventAction;
	((qsimEventAction *) event_action)->SetIO(io);

	runManager->SetUserAction(event_action);
	G4UserSteppingAction* stepping_action = new qsimSteppingAction;
	runManager->SetUserAction(stepping_action);
	rmmess->SetStepAct((qsimSteppingAction *) stepping_action);
	
	// Initialize Run manager
	
	runManager->Initialize(); 
	
	// do initialization in all macro files, 
	//see remoll examples for assistance. 
	
	G4UIsession* session = 0;
	
	//----------------
	// Visualization:
	//----------------
	
	if (argc==1){   // Define UI session for interactive mode.
		session = new G4UIQt(argc,argv);
		session = new G4UIterminal();
	}
	
	//#ifdef G4VIS_USE
	// Visualization, if you choose to have it!
	//
	// Simple graded message scheme - give first letter or a digit:
	//  0) quiet,         // Nothing is printed.
	//  1) startup,       // Startup and endup messages are printed...
	//  2) errors,        // ...and errors...
	//  3) warnings,      // ...and warnings...
	//  4) confirmations, // ...and confirming messages...
	//  5) parameters,    // ...and parameters of scenes and views...
	//  6) all            // ...and everything available.
	
	//this is the initializing the run manager?? Right?
	G4VisManager* visManager = new G4VisExecutive;
	//visManager -> SetVerboseLevel (1);
	visManager ->Initialize();
	
	//get the pointer to the User Interface manager
	G4UImanager * UI = G4UImanager::GetUIpointer();
	
	if (session){   // Define UI session for interactive mode.
		session->SessionStart();
		delete session;
	} else {          // Batch mode - not using the GUI
		visManager->SetVerboseLevel("quiet");
		// Get the pointer to the User Interface manager
		G4UImanager* UImanager = G4UImanager::GetUIpointer();
		
		// Process macro or start UI session
        if ( ! ui ){   // batch mode  
            G4String command = "/control/execute ";
            G4String fileName = argv[2];

            UI->ApplyCommand(command+fileName);
        } else{           // interactive mode
            UImanager->ApplyCommand("/control/execute vis/vis.mac");     
            UImanager->ApplyCommand("/control/execute macros/gui.mac");     
            ui->SessionStart();
            delete ui;
        }
	}

	//// Ucomment 3 lines to decide the number of the events to throw from the event generator (need more work on it) 
	//qsimPrimaryGeneratorAction *primGenAction = new qsimPrimaryGeneratorAction();
	//G4int numberOfEvents = primGenAction->nEvents;
	//runManager->BeamOn(10);
	delete runManager;
	delete visManager;
}
