
#ifndef qsimPrimaryGeneratorAction_h
#define qsimPrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4String.hh"

class G4ParticleGun;
class G4Event;
class qsimIO;
class qsimEvent;

class qsimPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
    public:
        qsimPrimaryGeneratorAction();
        ~qsimPrimaryGeneratorAction();

    public:
        void GeneratePrimaries(G4Event* anEvent);
        G4ParticleGun* GetParticleGun();
        void SetIO( qsimIO *io ){ fIO = io; }
		bool Thetaspectrum(double );
		bool pspectrum(double );
		//void SourceModeSet();
		void SourceModeSet(G4int );
        void setExtEvGenerator(G4String );

	private:
        G4ParticleGun* fParticleGun;
		G4int fSourceMode;

        qsimEvent *fDefaultEvent;
        qsimIO *fIO;
        G4String fExtGenFileName;

  public:
		G4double fXmin, fXmax, fYmin, fYmax, fZmin, fZmax;
        G4double fZ; // unused
        G4double fEmin, fEmax;
		G4double fthetaMin, fthetaMax;
		G4double fphiMin, fphiMax;
        G4double xPos, yPos, zPos;  // Positions of the event particle generated
        G4double p, pX, pY, pZ;  // Momentum of the event
        G4double E;           // Energy of the event
        G4int pid;            // Particle ID
        G4double rate;       // Rate of the event for fSource==2, get from event generator file.
        G4int nEvents;        // number of events for fSource==2, get from event generator file.
};

#endif


