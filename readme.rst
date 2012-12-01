================================ 
Helios++ -  A Continuous-energy Monte Carlo Reactor Physics Code
================================ 

The "plus plus" on the code's name is not because it is better than any other code (especially if it has the same name) or because it is an evolution of something. Is because is coded in C++ using OOP techniques, design patterns [1] and some naive meta-programming techniques [2]. Combining this three things is possible to design a very flexible MC code maintaining a decent performance when  doing a simulation.

Object-oriented design encourages the distribution of behavior among objects. If you analyze a MC code from a OOP perspective, you can easily identify the  various components / objects of the problem: cells, surfaces, lattices, tallies,  sources, distributions, isotopes, materials, etc. Such partitioning of the  system can result in an object structure with many connections between objects;  in the worst case, every object ends up knowing about every other. For example,  cells are filled by materials and surrounded by surfaces, materials are made up  by isotopes, sources can be constrained to a specific cell or surface, and so  on. Partitioning a system into many objects generally enhances reusability but proliferating interconnections tend to reduce it again .

Lots of interconnections make it less likely that an object can work without the  support of others. This can make unit testing VERY difficult, since is impossible to isolate the behavior of one object from others. Moreover, it can be difficult to change the system's behavior in any significant way, since  behavior is distributed among many objects. As a result, you may be forced to  define many subclasses to customize the system's behavior (ultimately, this hurts the performance of the code). 

I started to code Helios with the main objective of exploring monte carlo methods applied to neutronics. Since I am a big fan of design patterns, I decided to apply some of them to overcome the issues described above.  

================================ 
Helios layout
================================ 

Helios contains a few modules:

* Geometry module: Is some kind of Mediator between geometric objects (cells, surfaces, lattices, pins, etc). Also each geometric object has its own Factory to encapsulates the knowledge of which object subclass should create and moves this knowledge out of the rest of the system (for example, there are surfaces  such as cylinders, planes and spheres, also you there are concave or convex  cells, etc). 
The geometry in Helios is treated in a different way from other MC codes. When a particle enters to a Cell which is filled by an universe, there is no need to change the coordinate system of the particle. The geometric module takes care to interpret the logic of the “fill attribute” on the input cells, and creates the geometric entities using one global coordinate system. For example, if the same universe is used to fill different cells, the universe is “cloned” and each cell/surface inside it will be moved to the appropriate place. This leads to a very easy solution of the geometric tracking “routines” using simple recursion. The geometric module also keeps track of this operations and provides a way to the “user / client” to access cells/surfaces on different levels in the same way than MCNP (i.e. 1<3<4[2,3,0]). 

* Materials module: Is a very simple Mediator between materials and isotopes (although there is no need to have isotopes on a material, for example, macroscopic cross sections are supported by Helios). The most important task of this module is to provide a centralized place for other module to look for materials created for a specific problem.

* ACE module: It was a big dilemma for me whether or not to expose ACE isotopes as a module. At first, I wanted to keep within a single module (Materials module) everything related to materials and isotopes. But let's face it, ACE tables play a major role in neutron MC, so they deserve their own module :-). From this module you can access to any reaction / cross section (with the MT) of any isotope (using ZAID) defined on the xsdir. Once you got the reaction, using a random number stream you can sample the phase space coordinates of the particle (energy, direction and number of particles). If the isotope is fissile, you can also access to the NU-Block information. This module made ​​my life much easier when I had to create tests for ACE tables. Since all ACE isotopes required from other modules are taken from here, this module takes care of the energy grid management (different techniques to speed up the interpolation on ACE cross section tables). The ACE module acts as a Mediator between isotopes and ACE reaction laws too. A  reaction (such as inelastic scattering, fission, elastic scattering, etc) is  constructed used a Policy Based design [3]. The ACE module contains a pseudo-Factory that put together all this policies to create concrete reactions required for a problem  (this avoid a big sub-classing required to combine all ACE energy laws, mu laws, CM-LAB frame transformation and NU samplers). Currently, Helios supports all ACE tables distributed with serpent.

* Source module: Source modeling on a MC code is a very important task  (especially for fixed source calculations). Helios does not support fixed source calculations ATM, but eventually it will. In Helios you can have can have as many sources as you want, each of which is composed by different distributions acting on a portion of the particle's phase space. You can create distributions using ACE reactions taken from ACE tables on the xsdir and combine them with your own distributions (thanks to the ACE module!). Coding new distributions is extremely simple. You just need to code a functor  which takes a random number stream as an argument, modify the particle's phase space coordinates as you want, put a name to the distribution and finally register that distribution to the Factory. 

One cool thing: since the lattice "operation (algorithm in STL terms)” accepts any object that have a position and support a translation (such as sources,  universes, cells, surfaces...) you can create a lattice of sources (this is very useful to model the initial source of a KEFF problem with a lot of “fuel pins”).

* Tally module: Coming Soon! :-p I'm still trying to figure it out what is the  best way to put tallies in the current system. Since parallelism in Helios is  completely transparent to the simulation (is handled as a policy too) you can  combine MPI + OpenMP + IntelTbb + (hopefully CUDA + thrust in the future) in any  nasty way you want. This sounds nice but brings brings new problems to solve: reproducibility. The result should be the same no matter how the problem is executed. This is accomplished in Helios using a naive Tally class (some kind of proof of concept) to estimate the KEFF of the system and a few global reaction rates.
 
================================ 
Helios environment
================================ 

As explained above, not only objects inside each module are connected but there  are also connections between modules (cells are filled with materials, the source  may need a cell or a material, tallies are associated to cells, surfaces and/or materials, etc). The Mediator (and sometimes Director) between modules is the McEnvironment class. The main task of the McEnvironment is to abstracts how objects of a module cooperate and communicate between objects of other modules. It centralize the control and promotes loose coupling between modules. This means that the internal representation of each module (and the internal objects) can vary independently without propagating those changes to the “outside world”. As an example, the simulation “routine” in Helios is exactly the same no matter if the material is defined by macroscopic cross sections or with ACE isotopes (and will be the same if any other neutron-matter interaction representation is added on the future).

The goal of design patterns is to isolate changes in the code. This is accomplished by adding layers of abstraction. One of the most compelling motivations behind abstracting something is to separate things that change from things that stay the same. For example, the simulation “routine” on a MC code is something that should NEVER change or be modified when one tries to extend the functionality of the code. Or, if I want to add a new type of reaction, surface or material I’ll want to keep those changes from propagating other modifications throughout my code.

The McEnvironment also deals with other things. For example, is completely valid to create a stand alone geometry (for plotting for example). Or to create the source and sample particles using a random number generator. Or to load one ACE isotope into the environment and test differents reactions. This could be easily accomplished using Helios as a library.

================================ 
How to use
================================ 

Helios is not mature enough to be used on a daily basis (because I make changes almost every day) but you can test the functionality and performance once in a while :-). You can use Helios as a library(and create MC problems from a C++ program) or as a code using a XML input format almost identical to OpenMC [4]. 

After compiling Helios (see How to compile section), you can execute it:

$ helios++ --output file.output input1.xml input2.xml ... inputN.xml

Or with MPI:

$ mpiexec.openmpi -n 4 -machinefile machine -x LD_LIBRARY_PATH helios++ --output file.output input1.xml input2.xml … inputN.xml

Helios use a combination of MPI + shared memory paradigm. It has two levels of parallelism: node-parallelism (MPI) + intranode-parallelism (OpenMp or IntelTbb, by default is IntelTbb). So if you have a little cluster with 4 machines with 4 cores per node, a common way to execute a MPI executable is:

$ mpiexec.openmpi -n 16 ... args ...

This is NOT the best execution configuration for Helios. You just need to give the number of NODES to mpiexec. i.e.

$ mpiexec.openmpi -n 4 helios++ ...args...

This command will create one process per node and each process will use the available resources in the node in the best way possible. This is accomplished with task based parallelism. This execution configuration is better for two main reasons:

* Task based parallelism is better suited than a paradigm where threads (or  processes) are mapped to logical tasks. This is true on a neutron MC simulation, because the computational load of a set histories could be very different from  another set. 

* RAM memory is not multiplied by the number of MPI processes inside each node.  The cross sections tables, geometry, sources, etc, are shared by the “threads”  whithin the node.

I'll be adding new benchmarks / examples on this repository:

https://github.com/pellegre/benchmarks.git

================================ 
Parser
================================ 

Helios is designed to be completely independent from the parser. The definition and creation of objects are done in different stages. The parser deals with definitions of objects. The McEnvironment deals with the creation of objects. The workflow is:

1) Definition of objects: This are “high-level” definitions, something like: “I want a cell with id 1 surrounded by surfaces -1 2 3 in universe 3”, “I want a material with id 2 density 1.0 and isotopes H-1 O-16”, and so on. Usually this definitions will came from a Parser (which is a base class in Helios) or a c++ main (if used as a library).

2) Feed the definitions to the environment: The environment grabs the “high-levels” definitions and dispatch each one to the respective module. 

3) Each module process the dispatched definitions and solves internal dependencies (such as cells with universe and surfaces, sources and distributions or materials and isotopes). Eventually, each module make use of internal factories to create the concrete product that the “high-level” definition is representing.

4) Once internal dependencies are solve, each module (using the environment as a Mediator) solve external dependencies with other modules (such as cells with materials, source with cells or surfaces, and son on).

5) If all this process ends gracefully, you got a sane environment to run a MC simulation. You got a source to produce neutrons, a geometry filled with materials to transport the neutrons and a set of tallies to accumulate stuff on each history. If something goes wrong, on normal conditions an exception will be thrown. The only place  you can get a seg-fault (theoretically, of course could be hidden bugs somewhere) is when reading a malformed ACE table (I'm assuming ACE tables are good, almost no checks when reading the table).

6) Run the simulation.

================================ 
How to compile
================================ 

You will need to solve some dependencies before compiling Helios :

1) Tina's Random Number Generator Library - http://numbercrunch.de/trng/
2) Blitz++ - http://blitz.sourceforge.net/
3) Boost libraries (program_options mpi serialization). On debian based distros  you can install all boost components executing “sudo apt-get install libboost-all-dev” 
4) Some MPI implementation (I recommend to use openmpi because works well with boost mpi). “sudo apt-get install libopenmpi-dev libopenmpi1.3 openmpi-bin openmpi-common”
5) Intel Tbb - http://threadingbuildingblocks.org/
6) OpenMP-capable compiler

When you have all this installed in your system, you should execute:

$ git clone https://github.com/pellegre/helios.git

$ cd helios

$ mkdir Build

$ cd Build

$ cmake .. # You need to install cmake too – sudo apt-get install cmake

$ make && sudo make install 

This will install helios++ and plottermc++ (a simple app to generate PNG files 
of helios geometry, and to simulate source points). 

Helios is Open Source and is distributed under the New BSD license.

================================ 
References
================================ 

[1] Design Patterns: Elements of Reusable Object-Oriented Software (Gang of Four) 
[2] Modern C++ Design: Generic Programming and Design Patterns Applied - Andrei Alexandrescu
[3] http://en.wikipedia.org/wiki/Policy-based_design
[4] OpenMC - http://mit-crpg.github.com/openmc/
