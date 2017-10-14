#pragma once

#include <flext.h>
#include "GVF.h"

class gvf: public flext_base{

    FLEXT_HEADER_S(gvf,flext_base,setup)
    

    
public:
    gvf(int argc,t_atom *argv);
    ~gvf();

    void m_start();
    void m_clear();
    void m_print();

    void m_state(int argc,t_atom *argv);
    void m_list(const t_symbol *s, int argc, t_atom *argv);
    


   	void m_tolerance(int argc, t_atom *argv);
   	void m_particles(int argc, t_atom *argv);
   	void m_steps(int argc, t_atom *argv);
   	void m_resampling(int argc, t_atom *argv);

    void m_save(int argc, t_atom *argv);
    void m_load(int argc, t_atom *argv);

    static void setup(t_classid c);

private:
  	void add(int argc, t_atom *argv);
    void update(int argc, t_atom *argv);
	int validateArgs(int argc,t_atom *argv);
	bool checkDimensionValidity(int dim);
	bool checkDimensionMatch(int dim);
	bool checkNumberOfElements(int argc, int intended);
	bool checkTypeOfElements(int argc, t_atom* argv, std::vector<t_atomtype> intended);

	std::vector<float> vectorFromList(int argc, t_atom *argv);
	std::vector<t_atom> listFromVector(const std::vector<float> &vec);
	t_atom atomFromFloat(float value);


    GVF gvfObj;
    GVF::GVFState gvfState;

    int dimension;
    FLEXT_CALLBACK(m_start);
    FLEXT_CALLBACK(m_clear);
    FLEXT_CALLBACK(m_print);

    FLEXT_CALLBACK_V(m_state);

   	FLEXT_CALLBACK_V(m_tolerance);
    FLEXT_CALLBACK_V(m_particles);
    FLEXT_CALLBACK_V(m_steps);
    FLEXT_CALLBACK_V(m_resampling);

    FLEXT_CALLBACK_V(m_save);
    FLEXT_CALLBACK_V(m_load);

	FLEXT_CALLBACK_A(m_list); 

};

