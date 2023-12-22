G4TCX
==============================================================================
A GEANT4 simulation of multi-step Coulomb excitation experiments with TIGRESS.
==============================================================================

Requirements
------------------
- G4TCX has only been tested on GEANT4 v11.0.0.
- The GSL libraries are required
- In order to use the file correlator.cc to sort and histogram the simulated data, a ROOT installation is required. ROOT is also required to use any of the scripts in the Helpers folder.

Running G4TCX
-----------------
G4TCX takes an input macro file and writes the output to a data file. To run the simulation and subsequently histogram the simulation output, do

- G4TCX input.mac
- ./correlator output.dat hist_file.root

The ROOT file hist_file.root now contains many histograms and can be opened with any standard ROOT installation.

The correlator is a small program compiled with ROOT libraries. To compile the correlator, simply

- ./make_correlator.sh

If you want the Doppler correction to work, the correlator.cc file needs to be edited and recompiled. Immediately after the inlcude statements there are several variables which need to be changed to match the simulation input.

Additionally, the simulation will output a file, output-info.dat, along with the simulated data file. This file contains information on the CM scatttering angles and exicted state population distributions. This file can be unpacked using the diagnostics.C ROOT script, found in the Helpers folder.


Functionality
-----------------
The G4TCX simulation has three modes: Source, Scattering, and Full. Each mode has different functionality and input requirements. 

- Source: Simulates either a simple isotropic gamma-ray, or a user-definable gamma-ray cascade. No massive particles involved.
- Scattering: Simulates two-body scattering events. No gamma-rays involved. 
- Full: Simulates Coulomb excitation events with user-definable level schemes, excitation probabilities, and statistical tensors. Both the scattering angle and beam energy dependence of Coulomb excitation are accounted for.

Macro Files
-----------------
The three simulation modes require different commands in their macro files. However all share a common structure: 

<pre>
/Mode mode
<i>--optional geometry commands--</i>
<i>--mode specific commands--</i>
<i>--optional output commands--</i>
/run/numberOfThreads nThreads
/run/beamOn nEvents
</pre>

The /Mode command must come first, and the parameter (mode) can be Source, Scattering, or Full. The default number of threads is four. Example macros for each mode are in the Examples/Macros folder.

Output Commands
-----------------
The /Output commands are common across all modes. All /Output commands are optional.

| Command | Description |
| --- | --- |
| /Output/Filename *string* | Set the name of the output data file (Default: output.dat) |
| /Output/DiagnosticsFilename *string* | Set the name of the diagnostics data file (Default: output-info.dat) |
| /Output/WriteDiagnostics | Write the simulation's diagnostic information to file |
| /Output/OnlyWriteCoincidences | Only write particle-gamma coincidence data to file. This can significantly reduce the size of the output file |
| /Output/GammaMultTrigger *int* | Set gamma-ray multiplicity condition (number of Tigress cores) for writing data to the output file. (Default: 0) |

Note that the /Output/OnlyWriteCoincidences only affects the main data file (output.dat). The output-info.dat file will still contain the information from every simulated event.

Geometry Commands
-----------------
The /Geometry commands are common across all modes. All /Geometry commands are optional.

| Command | Description |
| --- | --- |
| /Geometry/Tigress/Construct | Include the Tigress array in the simulation |
| /Geometry/S3/Construct | Include the silicon detectors in the simulation |
| /Geometry/Target/Construct | Include the target in the simulation |
| /Geometry/Tigress/Configuration *int* | Set whether Tigress is in its high efficiency (0) or high peak-to-total (1) configuration (default: 0) |
| /Geometry/Tigress/FrameConfiguration *int* | Set the Tigress frame configuration. See text below for an explanation (default: 0) |
| /Geometry/Tigress/RemovePosition *int* |Remove a Tigress detector (clover) from the simulation |
| /Geometry/S3/UpstreamOffset *double unit* | Set (positive) z-offset of upstream silicon detector. (Default: 3 cm) |
| /Geometry/S3/DownstreamOffset *double unit* | Set (positive) z-offset of downstream silicon detector. (Default: 3 cm) |
| /Geometry/Target/StandardTarget *string* | Set parameters for a standard target: 208Pb, 48Ti, 196Pt, 197Au, or 110Pd. |
| /Geometry/Target/StepSize *double unit* | Set simulation step size inside the target. (Default: 0.05*thickness) |
| /Geometry/Target/Z *int* | Set Z of target nucleus. (Default: 82) |
| /Geometry/Target/N *int* | Set N of target nucleus. (Default: 126) |
| /Geometry/Target/Density *double unit* | Set (volume) density of target material. (Default: 11.382 g/cm<sup>3</sup>) |
| /Geometry/Target/Mass *double unit* | Set mass of target material. (Default: 207.97665 g/mole) |
| /Geometry/Target/Thickness *double unit* | Set (linear) thickness of target. (Default: 882 nm) |
| /Geometry/Target/Radius *double unit* | Set radius of target. (Default: 0.5 cm) |

The various /Construct commands are mandatory if you want to include that particular piece of the setup in the simulation.

The /Geometry/Tigress/SetFrameConfiguration has four possible parameters, 0-3. Choose 0 for the full structure, 1 for the corona plus upstream lampshade, 2 for the corona plus downstream lampshade, or 3 for only the corona. The appropriate Tigress detectors will be automatically removed from the simulation if you choose a non-zero frame configuration.

Note that the /Geometry/Target/ commands do **NOT** define the recoiling nucleus for the kinematics or excitation, it only defines "bulk" material properties of the target.

Source Mode Commands
-----------------
Either the /Source/Energy command or the /Source/LevelScheme command must be provided to G4TCX. These two commands are mutually exclusive. All other /Source commands are optional.

| Command | Description |
| --- | --- |
| /Source/Energy *double unit* | Simulate a single isotropic gamma-ray of this energy |
| /Source/LevelScheme *string* | Simulate a gamma-ray cascade defined by this file |
| /Source/GroundStateSpin *double* | Set the spin of the gamma source ground state. The spin must be integer or half-integer. (Default: 0.0) |
| /Source/Position *vector unit* | Set the position of the gamma-ray source (Default: 0.0 0.0 0.0 mm) |

See the Source Level Scheme File Format section below for details on the level scheme file.

Scattering Mode Commands
-----------------
The Scattering mode commands are divided into two categories: /Beam and /Reaction. The /Beam commands define the properties of the incoming beam. The /Reaction commands control the scattering kinematics. Each have mandatory and optional commands.

### Mandatory /Reaction commands
| Command | Description |
| --- | --- |
| /Reaction/ProjectileZ *int* | Set Z of projectile nucleus. |
| /Reaction/ProjectileA *int* | Set A of projectile nucleus. |
| /Reaction/RecoilZ *int* | Set Z of recoil (target) nucleus. |
| /Reaction/RecoilA *int* | Set A of recoil (target) nucleus. |

### Mandatory /Beam commands
| Command | Description |
| --- | --- |
| /Beam/Energy *double unit* | Set energy of incoming beam |

### Optional /Reaction commands
| Command | Description |
| --- | --- |
| /Reaction/AddThetaLAB *double unit* | Add an angle to desired LAB scattering angle ranges. This command must always be used two at a time, with the smaller angle coming first. Otherwise it doesn't work. |
| /Reaction/Optimize | Only sample parts of the Rutherford scattering distribution which will result in a particle entering a silicon detector. |
| /Reaction/OnlyProjectiles | Only consider the projectile when defining desired scattering angle ranges (above commands). |
| /Reaction/OnlyRecoils | Only consider the recoil when defining the desired scattering angle ranges (above commands). |
| /Reaction/DeltaE *double unit* | Set (positive) deltaE to simulate inelastic scattering. (Default: 0 MeV) |
 /Reaction/RecoilThreshold *double unit* | Set recoil energy detection threshold. Removes very low energy recoils, which scatter into the silicon detector, that occur at small CM angles during inelastic scattering. (Default: 0 MeV) |

The /Reaction/Optimize and /Reaction/AddThetaLAB commands can be used together. Doing this ensures every simulatd event will result in particle entering a silicon detector.

The use of optional /Reaction commands is strongly recommended. If you do not add a desired scattering angle range, a large scattering angle range of 13 to 180 degrees will be sampled according the Rutheford scattering distribution. Without the /Reaction/Optimize command, there will be useless simulated events where no particle is detected in an S3 detector.

There are no "safety checks" for these commands. For example, you could add an angle range corresponding the upstream S3 detector, and then call /Reaction/OnlyRecoils. This would set the entire scattering angle distribution to zero since the recoils can't scatter backwards. If you call /Reaction/Optimize in such a scenario, you will put the simulation in an infinite loop.

### Optional /Beam commands
| Command | Description |
| --- | --- |
| /Beam/SigmaEn *double unit* | Set Gaussian sigma of the kinetic energy distribution of the incoming beam (Default: 0 MeV) |
| /Beam/PositionX *double unit* | Set X position of incoming beam spot. (Default: 0 mm) |
| /Beam/PositionY *double unit* | Set Y position of incoming beam spot. (Default: 0 mm) |
| /Beam/AngleX *double unit* | Set angle about x-axis of incoming beam. (Default: 0 deg) |
| /Beam/AngleY *double unit* | Set angle about y-axis of incoming beam. (Default: 0 deg) |
| /Beam/SigmaX *double unit* | Set Gaussian sigma of x position distribution of the incoming beam. (Default: 0 mm) |
| /Beam/SigmaY *double unit* | Set Gaussian sigma of y position distribution of the incoming beam. (Default: 0 mm) |
| /Beam/SigmaAX *double unit* | Set Gaussian sigma of angle distribution about the x-axis. (Default: 0 deg) |
| /Beam/SigmaAY *double unit* | Set Gaussian sigma of angle distribution about the y-axis. (Default: 0 deg) |

Full Mode Commands
-----------------
For the Full Coulomb excitation simulation, all Scattering mode commands still apply. Additionally, /Excitation commands can be called which determine the level scheme, excitation pattern, and nuclear alignment in both the projectile and recoil nucleus. All /Excitation commands are optional.

| Command | Description |
| --- | --- |
| /Excitation/Projectile/LevelScheme *string* | Name of the file to be read-in which defines the projectile level scheme. |
| /Excitation/Projectile/Probabilities *string* | Name of the file to be read-in which defines the angle and energy dependence of the excitation probabilities for the projectile. |
| /Excitation/Projectile/StatisticalTensors *string* | Name of the file to be read-in which defines the angle and energy dependence of the statistical tensors for the projectile. |
| /Excitation/Projectile/OnlyConsiderState *int* | Turn off gamma decays from all states except this one. This requires a level scheme file and a probabilities file. |
| /Excitation/Projectile/NoFeeding | Turn off feeding to the state selected by the /OnlyConsiderState command. |
| /Excitation/Projectile/PopulateState *int* | Choose one state to populate in the projectile, irrespective of scattering angle and beam energy. This only requires a level scheme file (the probabilities file will simply be ignored). |
| /Excitation/Projectile/GroundStateSpin *double* | Set the spin of the projectile ground state. The spin must be integer or half-integer. (Default: 0.0) |
| /DeorientationEffect/Projectile/CalculateGk *bool* | Control whether the deorientation effect coefficients G<sub>k</sub>, which attenuate the nuclear alignment, will be calculated and applied for the projectile. (Default: true) |
| /DeorientationEffect/Projectile/AverageJ *double* | Set the average atomic spin in the projectile for the deorentation effect two-state model (Default: 3.0) |
| /DeorientationEffect/Projectile/Gamma *double* | Set the FWHM of the frequency distribution (ps^-1 ) in the projectile for the deorentation effect two-state model (Default: 0.02) |
| /DeorientationEffect/Projectile/Lambda *double* | Set the transition rate (ps^-1 ) between static and fluctuating states in the projectile for the deorentation effect two-state model (Default: 0.0345) |
| /DeorientationEffect/Projectile/TauC *double* | Set the correlation time (ps) of the fluctating state in the projectile for the deorentation effect two-state model (Default: 3.5) |
| /DeorientationEffect/Projectile/GFactor *double* | Set the gyromagnetic ratio (g-factor) of the projectile for the deorentation effect two-state model (Default: Z/A) |
| /DeorientationEffect/Projectile/FieldCoefficient *double* | Set the hyperfine field coefficient (10^8 T) in the projectile for the deorentation effect two-state model (Default: 6*10<sup>-6</sup>) |
| /DeorientationEffect/Projectile/FieldExponent *double* | Set the hyperfine field exponent in the projectile for the deorentation effect two-state model (Default: 0.6) |

The recoiling target nucleus can be controlled with identical commands. Simply replace /Projectile/ with /Recoil/ in any of the above commands.

The statistical tensors [1] and deorientation effect coefficients G<sub>k</sub> are needed to generate the particle-gamma angular distributions which occur in Coulomb excitation. They are also critical for reproducing the LAB frame gamma-ray spectra. See [2] for details on the two-state model, and the meaning of its parameters, which is used to describe the deorientation effect.

*If you input a level scheme, you must also input the probabilities or choose a state to populate. Otherwise the level scheme won't be used.*

*In a Full simulation, the /Reaction/DeltaE command only affects what CM angles will be sampled. The Q-value and LAB scattering anlges for each event are calculated based on which excited states get populated.*

Input Preparation
-----------------
The ROOT script MakeInput.C, found in the Helpers folder, will make the probabilities file and statistical tensors file that can be given to G4TCX. You will need to edit the script with your particular reaction parameters. The script uses the Coulomb excitation code Cygnus [3] for the calculations. The Cygnus libraries must be loaded into the ROOT session before loading MakeInput.C, and the Cygnus nucleus file must already be created. See [3] for details.

You can inspect the probabilities and statistical tensors calculated for the various states using the scripts prob_reader.C and tensor_reader.C, also found in the Helpers folder. You must edit the tensor_reader.C script with the excited state spins for the nucleus you are looking at.

There are two other scripts in the Helpers folder which can assist in preparing the necessary input files. The script MakeNucleus.C can take a Gosia output file and create a Cygnus nucleus file. The script MakeLevelScheme.C can take a Cygnus nucleus file and create the level scheme file needed by G4TCX (for a Full simulation). Therefore, you should only need to create one file manually: a Gosia file, a Cygnus file, or the level scheme file.

The level scheme file has a different format depending on the simulation mode (Source or Full). These are  described below.

Full Mode Level Scheme File Format
-----------------

<pre>
II<sub>1</sub> En<sub>1</sub> Sp<sub>1</sub> Tau<sub>1</sub> Nb<sub>1</sub>
 IF<sub>1</sub> P<sub>1</sub> L1<sub>1</sub> L2<sub>1</sub> DL<sub>1</sub> CC<sub>1</sub>
 ...
 IF<sub>Nb<sub>1</sub></sub> P<sub>Nb<sub>1</sub></sub> L1<sub>Nb<sub>1</sub></sub> L2<sub>Nb<sub>1</sub></sub> DL<sub>Nb<sub>1</sub></sub> CC<sub>Nb<sub>1</sub></sub>
II<sub>2</sub> En<sub>2</sub> Sp<sub>2</sub> Tau<sub>2</sub> Nb<sub>2</sub>
 IF<sub>1</sub> P<sub>1</sub> L1<sub>1</sub> L2<sub>1</sub> DL<sub>1</sub> CC<sub>1</sub>
 ...
 IF<sub>Nb<sub>2</sub></sub> P<sub>Nb<sub>2</sub></sub> L1<sub>Nb<sub>2</sub></sub> L2<sub>Nb<sub>2</sub></sub> DL<sub>Nb<sub>2</sub></sub> CC<sub>Nb<sub>2</sub></sub>
...
II<sub>N</sub> En<sub>N</sub> Sp<sub>N</sub> Tau<sub>N</sub> Nb<sub>N</sub>
 IF<sub>1</sub> P<sub>1</sub> L1<sub>1</sub> L2<sub>1</sub> DL<sub>1</sub> CC<sub>1</sub>
 ...
 IF<sub>Nb<sub>N</sub></sub> P<sub>Nb<sub>N</sub></sub> L1<sub>Nb<sub>N</sub></sub> L2<sub>Nb<sub>N</sub></sub> DL<sub>Nb<sub>N</sub></sub> CC<sub>Nb<sub>N</sub></sub>
</pre>

Here II<sub>i</sub> is the index of state i, En<sub>i</sub> is its energy in keV, Sp<sub>i</sub> is its spin (J), Tau<sub>i</sub> is its mean-lifetime in ps, and Nb<sub>i</sub> is the number of gamma decays from this state. Note that if state i cannot decay via gamma-ray emission, you should set Nb<sub>i</sub>=0. IF<sub>j</sub> is the index of the final state for gamma decay j of this state. P<sub>j</sub> is the probability of that decay (relative to the other gamma-ray decays). L1<sub>j</sub> is one multipolarity of the transition, L2<sub>j</sub> is the other multipolarity of the transition, and DL<sub>j</sub> is the L2/L1 mixing ratio. By most mixing ratio conventions, this implies L2 > L1. CC<sub>j</sub> is the total conversion coefficient of this gamma-ray transition. 

The states must be declared in order, i.e. II<sub>1</sub> = 1, II<sub>2</sub> = 2 and so on. This technically makes the initial state index redundant. The ground state (index 0) is not included in the level scheme file. There is no limit on the number of excited states or decays from a state.

The spin of a state must be integer or half-integer. For the gamma transitions, L1 > L2. If DL = 0, the transition will be pure L1 and L2 is ignored. Example level scheme files for a Full simulation are in the Examples/LevelSchemes/Full folder.

*An initial state II can only decay to a final state IF that has already been declared in the level scheme file, i.e. II > IF. If IF > II, the simulation will segfault.*

Source Mode Level Scheme File Format
-----------------

<pre>
II<sub>1</sub> En<sub>1</sub> Sp<sub>1</sub> Tau<sub>1</sub> Pop<sub>1</sub> Nb<sub>1</sub>
 IF<sub>1</sub> P<sub>1</sub> L1<sub>1</sub> L2<sub>1</sub> DL<sub>1</sub> CC<sub>1</sub>
 ...
 IF<sub>Nb<sub>1</sub></sub> P<sub>Nb<sub>1</sub></sub> L1<sub>Nb<sub>1</sub></sub> L2<sub>Nb<sub>1</sub></sub> DL<sub>Nb<sub>1</sub></sub> CC<sub>Nb<sub>1</sub></sub>
II<sub>2</sub> En<sub>2</sub> Sp<sub>2</sub> Tau<sub>2</sub> Pop<sub>2</sub> Nb<sub>2</sub>
 IF<sub>1</sub> P<sub>1</sub> L1<sub>1</sub> L2<sub>1</sub> DL<sub>1</sub> CC<sub>1</sub>
 ...
 IF<sub>Nb<sub>2</sub></sub> P<sub>Nb<sub>2</sub></sub> L1<sub>Nb<sub>2</sub></sub> L2<sub>Nb<sub>2</sub></sub> DL<sub>Nb<sub>2</sub></sub> CC<sub>Nb<sub>2</sub></sub>
...
II<sub>N</sub> En<sub>N</sub> Sp<sub>N</sub> Tau<sub>N</sub> Pop<sub>2</sub> Nb<sub>N</sub>
 IF<sub>1</sub> P<sub>1</sub> L1<sub>1</sub> L2<sub>1</sub> DL<sub>1</sub> CC<sub>1</sub>
 ...
 IF<sub>Nb<sub>N</sub></sub> P<sub>Nb<sub>N</sub></sub> L1<sub>Nb<sub>N</sub></sub> L2<sub>Nb<sub>N</sub></sub> DL<sub>Nb<sub>N</sub></sub> CC<sub>Nb<sub>N</sub></sub>
</pre>

The Source level scheme file is the same the Full level schem file (above), but has one additional entry, Pop<sub>i</sub>, which comes before Nb<sub>i</sub>. This is the relative population of the state i. An example level scheme file (co60.lvl) for a Source simulation is in the Examples/LevelSchemes/Source folder.

Notes
----------------
The simulation very rarely segfaults at the start, which I believe is due to some of the initial simulation setup not being strictly thread-safe. I've never seen a segfault during the event loop, and this doesn't seem to impact the simulation in any way.

If the incoming particle trajectory is not parallel to the z-axis, which can be accomplished with optional /Beam commands, the statistical tensor will not be correct. Correcting this requires a coordinate system rotation which is not yet implemented.

References
-----------------
[1] K. Alder and A. Winter, *Electromagnetic Excitation, Theory of Coulomb Excitation with Heavy Ions*, North Holland, Amsterdam (1975).

[2] T. Czosnyka, D. Cline, and C.Y. Wu, *GOSIA User's Manual*, Bull. Am. Phys. Soc. **28**, 745 (1983). [http://www.pas.rochester.edu/~cline/Gosia/Gosia_Manual_20120510.pdf](http://www.pas.rochester.edu/~cline/Gosia/Gosia_Manual_20120510.pdf)

[3] J. Henderson, *Cygnus*, [https://github.com/jhenderson88/Cygnus](https://github.com/jhenderson88/Cygnus)