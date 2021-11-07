/// This class is used to generate ObEventOccurModels
#ifndef ObEventOccurModelFactory_h
#define ObEventOccurModelFactory_h
#include "ObEventOccurModel.h"
#include "ObCustomProbabilityModel.h"
#include "ObNormalProbabilityModel.h"
#include "ObMarkovChainModel.h"

#include <math.h>
using namespace std;
namespace OB {
	class EventOccurModelFactory {
	  public:
		EventOccurModel* createCustomProbabilityModel() {
		  return new CustomProbabilityModel();
		}
		EventOccurModel* createNormalProbabilityModel() {
		  return new NormalProbabilityModel();
		}
		EventOccurModel* createMarkovChainModel() {
		  return new MarkovChainModel();
		}
	};
};

#endif