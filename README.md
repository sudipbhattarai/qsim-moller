# Quartz simulation (Qsim) of quartz based detector for MOLLER Experiment at Jefferson Lab

This version of qsim supports GDML geometry and it can be used to run optical simulation for any Cherenkov/scintillator based detector in MOLLER experiment.

**Original Developer**:
*Seamus Riordan*
<sriordan@physics.umass.edu>  
September 26, 2013(Updated June 26, 2015)

**GDML version initiation:**
*Carlos Bula Villareal*

**GDML version improvement and shower-max script developer:**
*Sudip Bhattarai*
<sudipbhattarai@isu.edu>  
July 12, 2022

## qsim installation and running instructions

Tested and ran on Ubuntu 22 inside UTM in macOS 12

**Requirements:**

* Geant4 >= 11.3.1 (11.2 give GDML schema warning in ifarm)
* cmake > 3.21 (tested with cmake 4.1 in macos, xcode 26.1)
* root >= 6.32 (tested with root 6.36 in macos, xcode 26.1)

### Download the qsim-moller

To download the code, use git clone:

```bash
git clone https://github.com/sudipbhattarai/qsim-moller
```

### Compilation

To build, create the directory you would like to build in, say `build`:

```bash
mkdir build
cd build
cmake ..
make
```

Or, run the shell script [compile](./compile) in the qsim directory.

```bash
./compile
```

### Running simulation

Simulation can be run in two different modes: Batch mode and visualization mode.
If your GDML file includes linked files with relative paths, either convert all paths to absolute paths or run `qsim-moller` from the directory where those paths are valid relative to your current location.
**Batch mode:**

To run in batch mode, execute with a macro such as:

```bash
./qsim gdml_file.gdml runmacro.mac
```

The example runmacros are inside the [macro](./macros/) directory and 

**Visualization:**

Visualization macros are found in vis/

To run, execute

```bash
./qsim gdml_file.gdml
```

which should bring up a graphical command interface and it should load the gdml file.

To see the particle visualization, run the macros from visualization command line

```bash
/control/execute macros/runexample.mac
```

Make sure the number of beam (/run/beamOn) in the runexample.mac file is not large (just 1 is good for basic visualization)

After completion of the simulation, the output rootfile is created in the location provided in the run macros. The output root file will have several variables that is described in [readme file](README.variables.md).

CLI User Commands

Using the Geant4 CLI it is possible to pass commands to modify behavior
and utilize the vis.mac macro from the command line.
These are all visible from the menu on the left.

## Source mode

Set by: /qsim/fSourceMode <0, 1, 2> in the run macros

0 = cosmic muons
Generates primary particles following cosmic muon angular distribution and energy spectrum.  
Energy spectrum obtained from fit to PDG data for muons with 0 deg incidence (good to 25% out to 36 GeV).  
The theta-angular distribution of the muon is obatined from the [report: page48](https://dspace.mit.edu/bitstream/handle/1721.1/61208/701107722-MIT.pdf?sequence=2&isAllowed=y)
Note that this does not automatically change the primary particle type to muons; this must be set by "/gun/particle mu-"

1 = beam
Generates perfectly straight, monoenergetic beam.
Current implementation generates particles at pinpoint, but beam spot size and beam energy can be changed from macros with the messenger command variables (xmin, xmax, ymin, ymax, thetamin, thetamax, emin, emax).\
For example: /qsim/xmin -75 mm

2 = remoll  
*For shower-max detector*  
The remoll simulation utilizes stack planes (sensitive detectors: 730XX, where XX denotes the sector number from 00 to 27) positioned just in front of the first tungsten layer of the shower-max detector. For primary studies, only three sectors—open (beam right), closed (beam left), and transition (beam up)—are enabled. The remoll ROOT output is then skimmed to generate an event distribution file, which serves as input for optical simulations in qsim.
In qsim run macro the event generator rootfile is included using the command

```text
/qsim/evgenfilename ./InputEventDistribution/<event_generator_input_file.root>
```

The z-position of the primary vertex, from which particles are emitted toward the detector, must be set according to the location of the sensitive detectors in the remoll simulation relative to the center of the shower-max detector. This position can be adjusted using the following commands:

```text
/qsim/zmin <value> mm
/qsim/zmax <value> mm
```
 
## GDML Geometry description

The gdml file defines and describes the geometry of the detector system in the sturctured way to represent the shapes, positions, and materials of detector components in a simulation environment. Here is the link to [GDML user guide](https://gdml.web.cern.ch/GDML/doc/GDMLmanual.pdf) developed by CERN.  
*For shower-max:* The gdml for shower-max is created using the [python wrapper](https://github.com/sudipbhattarai/remoll-showermax-generator). The properties of different volumes are used from [martices file](./geometry/mainDetMatrices.xml). Here are the list of the material and optical properties used in the gdml:  

**Sensitive detector:** 
Qsim can only have one sensitive detector, i.e. the photo cathode of the PMT. In order to make it sensitive we need to specify the auxiliary properties (`auxtype`) to the PMT cathode `logical volume` as shown below:

```xml
	<volume name="logic_pmt_cathode_0">
		<materialref ref="Cathode"/>
		<solidref ref="solid_pmt_cathode"/>
		<auxiliary auxtype="SensDet" auxvalue="PhotoCathode" />
	</volume>
```

**Quartz:**
* Index of refraction: Speciication sheet for Heraeus Spectrosil 2000 provides >25 data points for n(E). Fit with polynomial.
* Absorption length: Specification sheet for Heraeus Spectrosil 2000 provides only 2 data points for L(E). Current functional form of L(E) in qsim is of unknown origin and is inconsistent with Heraeus data points.
* Surface roughness: Glisur model

**Light guide:**
* Reflectivity (lightguide mirror): Currently defined as a function of photon energy only (and possibly incorrect). Needs to be defined as a function of photon energy and photon angle (not yet implemented)

**Photo cathode:**
* Quantum Efficiency (QE): Obtained from PMT specification data sheet. Here is the link to the 3" [ET-9305QKB](https://et-enterprises.com/images/data_sheets/9305KB.pdf) PMT that MOLLER main detector and shower-max uses.
* Reflectivity : We do not explicityly use reflectivity data as it is included in QE values.
