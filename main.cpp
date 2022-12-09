#include <iostream> // for i/o e.g. std::cout etc.
#include <cmath>	// for mathematical functions
#include <vector>	// for std::vector
#include <iomanip> 	// for std::fixed and std::fixesprecission()
#include <fstream>	// file streams
#include <memory>

#include "vector.hpp"    // include custom 5-vector class
#include "integrator.hpp"
#include "eos.hpp" // include eos container class
#include "nsmodel.hpp"
#include "mr_curves.hpp"
#include "plotting.hpp"     // to use python/matplotlib inside of c++

// --------------------------------------------------------------------



int test_FBSTLN() {

    double mu = 1.;
    double lambda = 0.;

    //auto EOS_DD2 = std::make_shared<EoStable>("EOS_tables/eos_HS_DD2_with_electrons.beta");
    auto Polytrope = std::make_shared<PolytropicEoS>();

    double rho_0 = 1e-20;
    double phi_0 = 0.02;
    std::vector<integrator::step> steps;

    FermionBosonStar fbs(Polytrope, mu, lambda, 0.);
    fbs.set_initial_conditions(rho_0, phi_0);

    double omega_0 = 1., omega_1 = 10.;
    fbs.bisection(omega_0, omega_1);
    fbs.evaluate_model(steps, "test/fbs.txt");

    double H_0 = 1.;
    FermionBosonStarTLN fbstln(fbs);
    fbstln.set_initial_conditions(0., H_0);

    double phi_1_0 = 1e-20, phi_1_1 = 1e3;
    fbstln.bisection_phi_1(phi_1_0, phi_1_1);

    fbstln.evaluate_model(steps, "test/fbstln.txt");

    std::cout << fbs << "\n"
              << fbstln << std::endl;


    #ifdef DEBUG_PLOTTING
    //[> see https://github.com/lava/matplotlib-cpp/issues/268 <]
    matplotlibcpp::detail::_interpreter::kill();
    #endif

    return 0;
}

// compute here e.g. a few configurations with the full system and with the effective EOS for different lambda, to check if they produce the same results.
void test_effectiveEOS_pure_boson_star() {

	double mu = 1.0;
	double lambda = 10.0*mu*mu; //(7500./30.) / 100.;	// we can then later try this out with different lambda!

	// create the phi_c-grid:
	const unsigned NstarsPhi = 100;
	double rho_cmin = 1e-25;	// we want to consider a pure boson star
	double phi_cmin = 1e-4;
	double phi_cmax = 0.20;
	double dphi = (phi_cmax - phi_cmin) / (NstarsPhi -1.);
	/*
	std::vector<double> rho_c_grid, phi_c_grid;
    rho_c_grid.push_back(rho_cmin);
    for (unsigned j = 0; j < NstarsPhi; ++j) {
            phi_c_grid.push_back(j*dphi + phi_cmin);
            std::cout << phi_c_grid[j] << std::endl;}
	*/

	// part to vary only rho_c:
	const unsigned NstarsRho = 100;
	rho_cmin = 4e-5 / 2680.;	// we want to consider a pure neutron matter star
	double rho_cmax = 10. / 2680.;	// in units of nuclear saturation density
	phi_cmin = 1e-40;
	double drho = (rho_cmax - rho_cmin) / (NstarsRho -1.);
	std::vector<double> rho_c_grid, phi_c_grid;
	phi_c_grid.push_back(phi_cmin);
	for (unsigned j = 0; j < NstarsRho; ++j) {
            rho_c_grid.push_back(j*drho + rho_cmin);
            std::cout << rho_c_grid[j] << std::endl;}

	// declare different EOS types:
    //auto EOS_DD2 = std::make_shared<EoStable>("EOS_tables/eos_HS_DD2_with_electrons.beta");
	auto EOS_DD2 = std::make_shared<PolytropicEoS>();
	auto myEffectiveEOS = std::make_shared<EffectiveBosonicEoS>(mu, lambda);


	// compute full self-consistent system:
	// setup to compute a full NS configuration, including tidal deformability:
	std::vector<FermionBosonStar> MRphi_curve; std::vector<FermionBosonStarTLN> MRphi_tln_curve;
	// calc the unperturbed equilibrium solutions:
    calc_rhophi_curves(mu, lambda, EOS_DD2, rho_c_grid, phi_c_grid, MRphi_curve);
	// calc the perturbed solutions to get the tidal love number:
	//calc_MRphik2_curve(MRphi_curve, MRphi_tln_curve); // compute the perturbed solutions for TLN
	// save the results in a txt file:
	std::string plotname = "colpi-comparison_fullsys-mu_" + std::to_string(mu) + "_" + std::to_string(lambda);
	plotname = "test_fullsys_model_pureEOS";
	write_MRphi_curve<FermionBosonStar>(MRphi_curve, "plots/" + plotname + ".txt");
	// write_MRphi_curve<FermionBosonStarTLN>(MRphi_tln_curve, "plots/" + plotname + ".txt");

	// compute effective two-fluid model:
	std::vector<TwoFluidFBS> twofluid_MRphi_curve;
	calc_twofluid_curves(EOS_DD2, myEffectiveEOS, rho_c_grid, phi_c_grid, twofluid_MRphi_curve, mu, lambda, true);	// use the effective EOS
	plotname = "colpi-comparison_effectivesys-mu_" + std::to_string(mu) + "_" + std::to_string(lambda);
	plotname = "test_twofluid_model_pureEOS";
	write_MRphi_curve<TwoFluidFBS>(twofluid_MRphi_curve, "plots/" + plotname + ".txt");
}


int main() {
    /*[> see https://github.com/lava/matplotlib-cpp/issues/268
      if this doesn't work, look at the end of the function
    //matplotlibcpp::backend("TkAgg");
    */

    //return test_FBSTLN();

    // ----------------------------------------------------------------
    // generate MR curves:
    const unsigned Nstars = 50;     // number of stars in MR curve of constant Phi_c
    const unsigned NstarsPhi = 50;   // number of MR curves of constant rho_c
    const unsigned NstarsNbNf = 2;  // number of MR curves of constand NbNf ratio

    // define some global values:
    double mu = 0.25;        // DM mass
    double lambda = 5000*mu*mu;    //self-interaction parameter


    // declare different EOS types:
    auto EOS_DD2 = std::make_shared<EoStable>("EOS_tables/eos_HS_DD2_with_electrons.beta");
    auto Polytrope = std::make_shared<PolytropicEoS>();

    // declare initial conditions:
    double rho_cmin = 0.00001;   // central density of first star (good for DD2 is 0.0005)
    double phi_cmin = 1e-6;//0.0001;//1e-6;    // central value of scalar field of first star
    double rho_cmax = 0.004;
    double phi_cmax = 0.10;//0.10;

    double drho = (rho_cmax - rho_cmin) / (Nstars -1.);
    double dphi = (phi_cmax - phi_cmin) / (NstarsPhi -1.);

    std::vector<double> rho_c_grid, phi_c_grid, NbNf_grid;
    for (unsigned i = 0; i < Nstars; ++i) {
            rho_c_grid.push_back(i*drho + rho_cmin);
            std::cout << rho_c_grid[i] << std::endl;}
    for (unsigned j = 0; j < NstarsPhi; ++j) {
            phi_c_grid.push_back(j*dphi + phi_cmin);
            std::cout << phi_c_grid[j] << std::endl;}
    for (unsigned k = 0; k < NstarsNbNf; ++k) {
            NbNf_grid.push_back(k*0.1 + 0.1);
            std::cout << NbNf_grid[k] << std::endl; }

    //test_EOS(mu, lambda, EOS_DD2, rho_c_grid, phi_c_grid, "plots/DD2_MR_MRphi-plot4.txt");
    
	// setup to compute a full NS configuration, including tidal deformability:
	std::vector<FermionBosonStar> MRphi_curve;
	std::vector<FermionBosonStarTLN> MRphi_tln_curve;

	// calc the unperturbed equilibrium solutions:
    //calc_rhophi_curves(mu, lambda, EOS_DD2, rho_c_grid, phi_c_grid, MRphi_curve);
	// calc the perturbed solutions to get the tidal love number:
	//calc_MRphik2_curve(MRphi_curve, MRphi_tln_curve); // compute the perturbed solutions for TLN
	// save the results in a txt file:
	std::string plotname = "colpireproduceplots_full-system-mu_" + std::to_string(mu) + "_" + std::to_string(lambda);
	//write_MRphi_curve<FermionBosonStarTLN>(MRphi_tln_curve, "plots/" + plotname + ".txt");

	test_effectiveEOS_pure_boson_star();
    // space for more EOS

    // method for the bisection with respect to Nb/Nf:
    //double omega_0 = 1., omega_1 = 10.;
    //FermionBosonStar myFBS(EOS_DD2, mu, lambda, 0.);
    //myFBS.set_initial_conditions(rho_c, phi_c);
    //myFBS.shooting_NbNf_ratio(0.2, 1e-3, omega_0, omega_1); // evaluate model is included
    // myFBS.evaluate_model();

    // calc three MR-curves with different Nb/Nf Ratios!
    //calc_NbNf_curves(mu, lambda, EOS_DD2, rho_c_grid, NbNf_grid, "plots/NbNf_test1.txt");

	// ----------------------------------------------------------------

	// test two-fluid EOS:
	double rho1_0 = 1.0000000000e-04;
	double rho2_0 =  1.0000000000e-20; //0.00001;
	lambda = 300.; 	// the effective bosonic EoS only works for large values of lambda
	auto myEffectiveEOS = std::make_shared<EffectiveBosonicEoS>(mu, lambda);

	/* 	
	TwoFluidFBS my_twofluid_FBS(EOS_DD2, myEffectiveEOS);
	my_twofluid_FBS.set_initial_conditions(rho1_0, rho2_0);
	std::vector<integrator::step> my_twofluid_results;
	my_twofluid_FBS.evaluate_model(my_twofluid_results, "plots/twofluid_test2_nansearch.txt");
	std::cout << "Macroscopic values of twofluid model:" << std::endl << my_twofluid_FBS << std::endl;
	 */
	std::vector<TwoFluidFBS> twofluid_MRphi_curve;
	//calc_twofluid_curves(EOS_DD2, myEffectiveEOS, rho_c_grid, phi_c_grid, twofluid_MRphi_curve, mu, lambda, true);	// use the effective EOS
	//write_MRphi_curve<TwoFluidFBS>(twofluid_MRphi_curve, "plots/twofluid_MRphi-diagram3.txt");
	//test_effectiveEOS_pure_boson_star();
	/*Use the two-fluid model without the effective bosonic EOS like:
	calc_twofluid_curves(EOS1, EOS2, rho1_c_grid, rho2_c_grid, twofluid_MRphi_curve);
	*/

    // ----------------------------------------------------------------

    #ifdef DEBUG_PLOTTING
    //[> see https://github.com/lava/matplotlib-cpp/issues/268 <]
    matplotlibcpp::detail::_interpreter::kill();
    #endif
    return 0;
}
