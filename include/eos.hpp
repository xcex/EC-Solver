#pragma once

#include <vector>
#include <cmath>
#include <fstream>	// filestream for file input
#include <sstream>	// string stream used for reading the EOS table
#include <iostream>
#include <stdexcept> // for std::runtime_error
#include <map>


namespace FBS{

/* Equation of State
 *  an abstract class to model what an equation of state should contain
 *
 *  min_P, min_rho describe minimal P, minimal rho where the EoS is valid
 * */
class EquationOfState
{
public:
    virtual double get_P_from_rho(const double rho, const double epsilon)  = 0;
	virtual double get_P_from_e(const double e)  = 0;
	virtual double get_e_from_P(const double P)  = 0;
	virtual double get_rho_from_P(const double P) = 0;
    virtual double dP_drho(const double rho, double epsilon) = 0;
	virtual double dP_de(const double e) = 0;

    /* This gives the derivative dP/de depending on the matter density rho and internal energy epsilon */
    virtual double dP_de(const double rho, double epsilon)
	{  return dP_de(rho*(1. + epsilon));  }

    /* This gives the matter density rho and internal energy epsilon depending on the pressure P */
	virtual void callEOS(double& myrho, double& epsilon, const double P) = 0;

	/* Functions giving minimal P and minimal rho for the EoS to be valid */
    virtual double min_P() = 0;		// minimal value of pressure
    virtual double min_rho() = 0;	// minimal value of restmass density
	virtual double min_e() = 0;	// minimal value of total energy density e:=rho(1+epsilon)

};

/* PolytropicEoS
 * a class modeling a Polytropic equation of state
 * with the parameters Kappa, Gamma
 * such that
 *  P = Kappa * rho^{Gamma}
 * */
class PolytropicEoS : public EquationOfState
{
protected:
    double kappa, Gamma;

public:
    PolytropicEoS(const double kappa=100., const double Gamma =2.) : kappa(kappa), Gamma(Gamma) {}

    /* This gives the pressure P depending on the matter density rho. The internal energy density is ignored */
    double get_P_from_rho(const double rho_in, const double epsilon);
	double get_P_from_e(const double etot_in);
	double get_e_from_P(const double P_in);

	double dP_drho(const double rho, double epsilon);

    /* This gives the derivative dP/de depending on the matter density rho. The internal energy density is ignored */
    double dP_de(const double e);
    /* This gives the matter density rho and internal energy density depending on the pressure P */
	void callEOS(double& myrho, double& epsilon, const double P);

    /* Functions giving minimal P and minimal rho. For the polytrope both are 0. */
    double min_P();
    double min_rho();
	double min_e();

};


/* CausalEoS
 * a class modeling a causal equation of state
 * with parameters eps_f, P_F,
 * such that
 *  P = P_f + rho(1+eps) - eps_f
 * */
class CausalEoS : public EquationOfState
{
protected:
    double eps_f, P_f;

public:
    CausalEoS(const double eps_f, const double P_f =0.) : eps_f(eps_f), P_f(P_f) {}

    /* This gives the pressure P depending on the matter density rho and internal energy epsilon */
    double get_P_from_rho(const double rho_in, const double epsilon);
	double get_P_from_e(const double etot_in);
	double get_e_from_P(const double P_in);

    /* This gives the derivative dP/de depending on the matter density rho and internal energy epsilon */
    double dP_de(const double e);
	double dP_drho(const double rho, double epsilon);

    /* This gives the matter density rho and internal energy epsilon depending on the pressure P */
	void callEOS(double& myrho, double& epsilon, const double P);

    /* Functions giving minimal P and minimal rho. For the causal EoS both are 0. */
    double min_P();
    double min_rho();
	double min_e();
};


/* a class modeling the effective equation of state for a bosonic condensate of self-interacting bosons */
class EffectiveBosonicEoS : public EquationOfState
{
protected:
    double rho0;	// parameter computed from boson mass and self-interaction-parameter, corresponds to total energy density of the boson-fluid
					// rho0 = mu^4 / ( 2 * lambda ) in our normalization-convention for Phi and lambda (different convention to the two papers below:)
					// compare to: PHYSICAL REVIEW LETTERS VOLUME 57, Number 20 17 NOVEMBER 1986 Boson Stars: Gravitational Equilibria of Self-Gravitating scalar fields
					// and: Tidal deformability of dark matter admixed neutron stars PHYSICAL REVIEW D 105, 123010 (2022)
	double mu, lambda;	// make the variables part of the class explicitly
public:
    EffectiveBosonicEoS(const double mu=1.0, const double lambda =1.0) : rho0(std::pow(mu,4) / (2.* lambda)), mu(mu), lambda(lambda) {}

    double get_P_from_rho(const double rho_in, const double epsilon);
	double get_P_from_e(const double etot_in);
	double get_e_from_P(const double P_in);
	void callEOS(double& myrho, double& epsilon, const double P);
    double dP_drho(const double rho, double epsilon);
	double dP_de(const double etot);

	double get_mu();
	double get_lambda();

    double min_P();
    double min_rho();
	double min_e();
};


/* EoStable
 * a class representing a tabulated equation of state
 * On construction, the tabulated EoS has to be loaded into the
 *  vectors rho, Pres, e_tot,   the mass density, Pressure, and energy density respectively
 * On call, the values are interpolated linearly from the table
 * */
class EoStable : public EquationOfState
{
protected:
	std::vector<double> rho_table, P_table, e_table;

public:
    EoStable(const std::vector<double>& rho_table, const std::vector<double>& P_table, const std::vector<double>& e_table)
        : rho_table(rho_table), P_table(P_table), e_table(e_table) {}

    /* Constructor expects link to file */
    EoStable(const std::string filename) : rho_table({}), P_table({}), e_table({})
        { if(! load_from_file(filename))
                throw std::runtime_error("File '" + filename + "' could not be loaded") ;  }

    /* This function loads an EoS from file, the first one with rigid column indices, the second one with arbitrary indices*/
    bool load_from_file(const std::string filename);
    bool load_from_file(const std::string filename, std::map<std::string, int> indices);

    /* This gives the pressure P depending on the matter density rho and internal energy epsilon by linear interpolation of the table*/
	void callEOS(double& myrho, double& epsilon, const double P);
    double dP_drho(const double rho, double epsilon);
	double dP_de(const double e);
    /* This gives the derivative dP/de depending on the matter density rho and internal energy epsilon by linear interpolation */
    /* This gives the matter density rho and internal energy epsilon depending on the pressure P by linear interpolation*/
    double get_P_from_rho(const double rho, const double epsilon);
	double get_P_from_e(const double e);
	double get_e_from_P(const double P);
	double get_rho_from_P(const double P);

    /* Functions giving minimal P and minimal rho. These depend on the lowest values in the tables */
    double min_P();
    double min_rho();
	double min_e();

};


}
