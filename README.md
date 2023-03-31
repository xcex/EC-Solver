# FBS solver:

## Overview:

The code is written in C++, Cython and Python.
The C++ part models the FBS star, integrates the differential equations involved and writes the results to files.
There is also a cython wrapper included that wraps the C++ code in a more user friendly python experience and does simple plots.
Lastly, there is a some python functionality for reading out the solutions generated by the C++ code, generating the stability curves and much prettier plots.

To compile, a call to

    make

should suffice.
If you want to only compile the c++ code without the Cython components, call

    make fbs

In case dependencies are needed

    sudo apt install libboost-dev
    pip3 install numpy matplotlib cython pandas

## The C++ structure

The file structure is as follows:
 - `vector.hpp`: Defines a simple wrapper of a boost::vector for (mathematical) vectors.
 - `utilities.hpp`: Collection of some utility functions that may be used in other files.
 - `integrator.hpp`: Includes a Runge-Kutta-45-Fehlberg integrator that is accurate to 5th order with variable step size. The integrator allows to define events with an event_condition that will be checked for during the integration. Additional helber functions related to integrtion are also included.
 - `eos.hpp`: Defines a model for different Equations of State. Preimplemented are a Polytropic EoS, a Causal EoS and an effective bosonic EoS for a massive self-interacting complex scalar field. Additional tabulated EoS can be read from files.
 - `nsmodel.hpp`: Gives a base class for a star described by differential equations.
 - `fbs.hpp` The FermionBosonStar class gives the logic and differential equations for an FBS in equilibrium.
 - `fbstln.hpp`: The FBSTLN class extends the functionality to pertubations as described in the paper. The general idea is to first create a FermionBosonStar instance and integrate the equilibrium state, and from there create an instance of the TLN class to extend the computation. There are two conventions implemented, check the econvention branch.
 - `ns_twofluid.hpp`: The NS_TWOFLUID class describes a star consisting of two minimally coupled fluids (each with their own EoS) in equilibrium.
 - `fbs_twofluid.hpp`: A class derived from NS_TWOFLUID. Features custom initial conditions to use it together with the effective bosonic EoS.
 - `mr_curves.hpp`: Defines functions to calculate lists of FBS with OMP support and write them to file.
 - `plotting.hpp`: Allows to plot single integrations of stars. Only for debugging purposes, superseeded by the pyfbs implementation.
 - `matplotlibcpp.hpp`: For aforementioned debugging purposes. Generally not used, only if specified in Makefile. Does not work together with pyfbs. See at the end for more info.

See the `main.cpp` for some examples of how to integrate a single star, or create an MR-curve.

## The pyfbs structure

The pyfbs module contains a wrapper for the C++ files described above. This makes quick calculations and plots of single stars and curves much easier.
See the `pyfbs/TestFBS.ipynb` file for some examples.
Unfortunately, the cython wrapper (< version 3) misses some functionality, so usage requires modification of the cython library files as described [here](https://stackoverflow.com/questions/67626270/inheritance-and-stdshared-ptr-in-cython).
There are also functions for loading the files written by the `mr_curves.hpp` functions, extracting the stability curves and making pretty plots out of them.
See `plotting.ipynb` for some examples and generating the plots in our publication.

## Publication
This code has been used in our [publication](https://arxiv.org/abs/2303.04089). The data is generated with the `pyfbs/generate_tables.py` file, and plotted with the `plotting.ipynb` files.

## Acknowledgements
This work was supported by the Deutsche Forschungsgemeinschaft (DFG, German Research Foundation) through the [CRC-TR 211](https://crc-tr211.org/) ’Strong-interaction matter under extreme conditions’– project number 31547758.

## matplotlib-cpp
The code includes a modified version of the [matplotlib-cpp library](https://github.com/lava/matplotlib-cpp). See [this](https://github.com/lava/matplotlib-cpp/blob/master/LICENSE.matplotlib) for copyright and license purposes. The changes made include a wrapper for the `plt::xscale`, `plt::yscale` function included in matplotlib.

