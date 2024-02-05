#ifndef _Cable_h_

#define _Cable_h_

#include "Transmission_line.h"

class Cable : public Transmission_line {
private:
	struct Conductor {
		// Conductor is preseting the conducting layer, in the ring format between
		// r_i < r < r_o 
		double r_i = 0; // inner radius [m]
		double r_o = 0; // outer radius [m]
		double rho = 0; // conductor resistivity[\Omega m]
		double mu_r = 1; // relative permeability
		double a = 0; // nominal area
	};
	
	struct Insulator {
		double r_i = 0; // inner radius
		double r_o = 0; // outer radius
		// Insulator defined between rᵢ < r < rₒ
		double epsilon_r = 1; // realtive premittivity
		double mu_r = 1; // relative permeability
		// If a semiconductor is present, in an insulator, we have : rᵢ < semiconductor < a + a < insulator < b + b < semiconductor < rₒ
		double a = 0; // inner semiconductor outer radius -> Inner semiconductor rᵢ < r < a
		double b = 0; // outer semiconductor inner radius -> Outer semiconductor b < r < rₒ
	};

	double length = 0; // line length[m]

		//conductors::OrderedDict{ Symbol, Conductor } = OrderedDict{ Symbol, Conductor }() #OrderedDict->dictionary with a particular order.Key: Symbol->C1, C2, C3 and C4.Value : Conductor->Mutable Struct Conductor, defined above
		//insulators::OrderedDict{ Symbol, Insulator } = OrderedDict{ Symbol, Insulator }() #OrderedDict->dictionary with a particular order.Key: Symbol->I1, I2, I3 and I4.Value : Insulator->Mutable Struct Insulator, defined above
		//positions::Vector{ Tuple{Real,Real} } = [] #Real->indicates all variables are real number, vector composed by tuple of real numbers.e.g.positions = [(0, 0), (1, 1)].Cables positions 1st:x = 0, y = 0. 2nd : x = 1, y = 1.
		//earth_parameters::NTuple{ N, Union{Int,Float64} } where N = (1, 1, 1) # (μᵣ, ϵᵣ, ρ) in units([], [], [Ωm]) compact way of representing the type for a tuple of length N where all elements are of type Int or Float64.
		//configuration::Symbol = : coaxial #Configuration is a datatype symbol with value coaxial Symbol->Type of data.Symbols can be entered using the quote operator ":"
		//type::Symbol = :underground   # or aerial. for the description, see above.

		//P::Array{ Basic } = [] #initialization(still no value inside) of the array P with datatype Basic
		//Z::Array{ Basic } = [] #same as row above
public:
	Cable() {};
	~Cable() {};
};

#endif

