#include "main.h"
// check for appropriate flext version
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 400)
#error You need at least flext version 0.4.0
#endif

gvf::gvf(int argc, t_atom *argv):
dimension(validateArgs(argc, argv)){
    AddInAnything(); 
    AddOutAnything(); 
    AddOutAnything(); 
}

gvf::~gvf(){
}

void gvf::setup(t_classid c){
    FLEXT_CADDMETHOD(c, 0, m_list);
    FLEXT_CADDMETHOD_(c, 0, "state", m_state);  
    FLEXT_CADDMETHOD_(c, 0, "start", m_start);  
    FLEXT_CADDMETHOD_(c, 0, "print", m_print);  

    FLEXT_CADDMETHOD_(c, 0, "tolerance", m_tolerance);
    FLEXT_CADDMETHOD_(c, 0, "particles", m_particles);
    FLEXT_CADDMETHOD_(c, 0, "steps", m_steps);
    FLEXT_CADDMETHOD_(c, 0, "resampling",  m_resampling);

    FLEXT_CADDMETHOD_(c, 0, "save", m_save);  
    FLEXT_CADDMETHOD_(c, 0, "load", m_load);  
}

void gvf::m_start(){
    gvfObj.startGesture();
}

void gvf::m_clear(){
    gvfObj.clear();
}

void gvf::m_print(){
    t_atom numTemplates = atomFromFloat(gvfObj.getNumberOfGestureTemplates());
    ToOutAnything(1, gensym("numberOfGestureTemplates"), 1, &numTemplates);

    t_atom tolerance = atomFromFloat(gvfObj.getTolerance());
    ToOutAnything(1, gensym("tolerance"), 1, &tolerance);

    t_atom numParticles = atomFromFloat(gvfObj.getNumberOfParticles());
    ToOutAnything(1, gensym("numberOfParticles"), 1, &numParticles);

    t_atom predictionSteps = atomFromFloat(gvfObj.getPredictionSteps());
    ToOutAnything(1, gensym("predictionSteps"), 1, &predictionSteps);

    t_atom resamplingThreshold = atomFromFloat(gvfObj.getResamplingThreshold());
    ToOutAnything(1, gensym("resamplingThreshold"), 1, &resamplingThreshold);

    std::vector<t_atom> dynamicsVariance = listFromVector(gvfObj.getDynamicsVariance());
    ToOutAnything(1, gensym("dynamicsVariance"), dynamicsVariance.size(), &dynamicsVariance[0]);

    std::vector<t_atom> scalingsVariance = listFromVector(gvfObj.getScalingsVariance());
    ToOutAnything(1, gensym("scalingsVariance"), scalingsVariance.size(), &scalingsVariance[0]);

    std::vector<t_atom> rotationsVariance = listFromVector(gvfObj.getRotationsVariance());
    ToOutAnything(1, gensym("rotationsVariance"), rotationsVariance.size(), &rotationsVariance[0]);

}

void gvf::m_list(const t_symbol *s, int argc, t_atom *argv){
    auto state = gvfObj.getState();
    switch(state){
        case GVF::STATE_LEARNING:{
            add(argc, argv);
            break;
        }
        case GVF::STATE_FOLLOWING:{
            update(argc, argv);
            break;
        }
        default:
            post("error: GVF state is neither LEARNING nor FOLLOWING");
    }
}

void gvf::m_state(int argc, t_atom *argv){

    if(argc != 1 ){
        post("error: state must be followed by one symbol");
        return;
    }else if(argv[0].a_type != A_SYMBOL){
        post("error: state must be followed by one symbol");
        return;
    }

    std::string state = atom_getsymbol(&argv[0])->s_name;
    if(state == "clear"){
        gvfObj.setState(GVF::STATE_CLEAR);
    }else if(state == "learning"){
        gvfObj.setState(GVF::STATE_LEARNING);
    }else if(state == "following"){
        gvfObj.setState(GVF::STATE_FOLLOWING);
    }else if(state == "bypass"){
        gvfObj.setState(GVF::STATE_BYPASS);
    }else{
        post("invalid state: %s", state.c_str());
    }
}


void gvf::add(int argc, t_atom *argv){
    if(!checkDimensionMatch(argc)) return;
    auto vec = vectorFromList(argc, argv);
    if(vec.empty()) return;

    gvfObj.addObservation(vec);
}

void gvf::update(int argc, t_atom *argv){
    if(!checkDimensionMatch(argc)) return;
    auto vec = vectorFromList(argc, argv);
    if(vec.empty()) return;
    GVFOutcomes & outcomes = gvfObj.update(vec);

    auto likelihoods = listFromVector(outcomes.likelihoods);
    auto alignments = listFromVector(outcomes.alignments);

    // vector<vector<float> > dynamics;
    // vector<vector<float> > scalings;
    // vector<vector<float> > rotations;

    t_atom gesture = atomFromFloat(outcomes.likeliestGesture);
    ToOutAnything(0, gensym("likeliestGesture"), 1, &gesture);
    ToOutAnything(0, gensym("likelihoods"), likelihoods.size(), &likelihoods[0]);
    ToOutAnything(0, gensym("alignments"), alignments.size(), &alignments[0]);
}

void gvf::m_tolerance(int argc, t_atom *argv){
    if(!checkNumberOfElements(argc, 1)) return;
    if(!checkTypeOfElements(argc, argv, {A_FLOAT})) return;
    gvfObj.setTolerance(atom_getfloat(&argv[0]));
}

void gvf::m_particles(int argc, t_atom *argv){
    if(!checkNumberOfElements(argc, 1)) return;
    if(!checkTypeOfElements(argc, argv, {A_FLOAT})) return;
    gvfObj.setNumberOfParticles(atom_getfloat(&argv[0]));
}

void gvf::m_steps(int argc, t_atom *argv){
    if(!checkNumberOfElements(argc, 1)) return;
    if(!checkTypeOfElements(argc, argv, {A_FLOAT})) return;
    gvfObj.setPredictionSteps(atom_getfloat(&argv[0]));
}

void gvf::m_resampling(int argc, t_atom *argv){
    if(!checkNumberOfElements(argc, 1)) return;
    if(!checkTypeOfElements(argc, argv, {A_FLOAT})) return;
    gvfObj.setResamplingThreshold(atom_getfloat(&argv[0]));
}

void gvf::m_load(int argc, t_atom *argv){
    if(!checkNumberOfElements(argc, 1)) return;
    if(!checkTypeOfElements(argc, argv, {A_SYMBOL})) return;
    t_symbol * file = atom_getsymbol(&argv[0]);
    post("load templates from %s", file->s_name);
    gvfObj.loadTemplates(file->s_name);
}

void gvf::m_save(int argc, t_atom *argv){
    if(!checkNumberOfElements(argc, 1)) return;
    if(!checkTypeOfElements(argc, argv, {A_SYMBOL})) return;
    t_symbol * file = atom_getsymbol(&argv[0]);
    post("save templates to %s", file->s_name);
    gvfObj.saveTemplates(file->s_name);
}

/// private
int gvf::validateArgs(int argc, t_atom *argv){
    if(!checkNumberOfElements(argc, 1)) return 3;
    if(!checkTypeOfElements(argc, argv, {A_FLOAT})) return 3;

    int dim = atom_getint(&argv[0]);
    if(!checkDimensionValidity(dim)){
        post("error: dimension out of range. must be 1 - 9. set to 3");
        return 3;
    }

    return dim;
}

bool gvf::checkDimensionMatch(int dim){
    if(dim != dimension){
        post("error: number of element must be %d", dimension);
        return false;
    }
    return true;
}

bool gvf::checkDimensionValidity(int dim){
    return 0 < dim && dim < 10;
}

bool gvf::checkNumberOfElements(int argc, int intended){
    if(argc < intended){
        post("error: too few arguments. number of args should be %d", intended);
        return false;
    }else if(argc > intended){
        post("error: too many arguments. number of args should be %d", intended);
        return false;
    }
    return true;
}

bool gvf::checkTypeOfElements(int argc, t_atom* argv, std::vector<t_atomtype> intended){
    if(argc != intended.size()) return false;
    for(int i = 0; i < argc; i++){
        if(intended[i] != argv[i].a_type){
            post("error : incorrect type of arg at index %d", i);
        }
    }
    return true;
}

std::vector<float> gvf::vectorFromList(int argc, t_atom *argv){
    std::vector<float> vec;
    for(int i = 0; i < argc; i++){
        if(argv[i].a_type != A_FLOAT){
            post("error: invalid type of data added. ignored.");
            return std::vector<float>();
        }
        vec.push_back(atom_getfloat(&argv[i]));
    }
    return std::move(vec);
}

std::vector<t_atom> gvf::listFromVector(const std::vector<float> &vec){
    std::vector<t_atom> list;
    list.resize(vec.size());
    for(int i = 0; i < vec.size(); i++){
        SETFLOAT(&list[i], vec[i]);
    }
    return std::move(list);
}

t_atom gvf::atomFromFloat(float value){
    t_atom atom;
    SETFLOAT(&atom, value);
    return atom;
}


FLEXT_NEW_V("gvf",gvf)
