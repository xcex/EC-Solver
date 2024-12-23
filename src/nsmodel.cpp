#include "nsmodel.hpp"

using namespace FBS;

/* This wrapper function allows to call the class member function dy_dr
 * if the params pointer points to the class instance
 * */
vector NSmodel::dy_dr_static(const double r, const vector& y, const void* params) {
    NSmodel* m = (NSmodel*)params;
    return m->dy_dr(r, y);
}

/* This function calls the integrator for the NSmodel class
 * The dy_dr function is automatically called through the wrapper dy_dr_static class
 * The results are saved in result (only the initial and last step, unless intOpts.save_intermediate=true)
 * A list of events to be tracked during the integration can be passed, but this is optional
 * The initial conditions have to be specified - usually given by NSmodel::get_initial_conditions - but they can be modified
 * The IntegrationOptions will be passed to the integrator
 * The integration starts at r_init and tries to reach r_end
 * The return value is the one given by the integrator, compare integrator::return_reason
 * */
int NSmodel::integrate(std::vector<integrator::step>& result, std::vector<integrator::Event>& events, const vector initial_conditions, integrator::IntegrationOptions intOpts, double r_init, double r_end) const {
    return FBS::integrator::RKF45(&(this->dy_dr_static), (r_init < 0. ? this->r_init : r_init), initial_conditions, (r_end < 0. ? this->r_end : r_end), (void*) this,  result,  events, intOpts);
}

/* same functions as above but for NSmodelv2: */
vector NSmodelv2::dy_dr_static(const double r, const vector& y, const double r_prev, const vector& y_prev, const void* params) {
    NSmodelv2* m = (NSmodelv2*)params;
    return m->dy_dr(r, y, r_prev, y_prev);
}
int NSmodelv2::integrate(std::vector<integrator::step>& result, std::vector<integrator::Event>& events, const vector initial_conditions, integrator::IntegrationOptions intOpts, double r_init, double r_end) const {
    return FBS::integrator::RKF45(&(this->dy_dr_static), (r_init < 0. ? this->r_init : r_init), initial_conditions, (r_end < 0. ? this->r_end : r_end), (void*) this,  result,  events, intOpts);
}