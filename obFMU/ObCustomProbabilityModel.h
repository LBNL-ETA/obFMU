/// Customized probability model for event occurrence
#ifndef ObCustomProbabilityModel_h
#define ObCustomProbabilityModel_h
#include "ObEventOccurModel.h"
#include <math.h>
using namespace std;
namespace OB {
	class CustomProbabilityModel : public EventOccurModel{
	public:
		/// initialize by reading an TiXmlElement
		/// params:
		/// - element: the TiXMLElement
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		bool	     readObXML(TiXmlElement * parentElem, string& error);

		/// initialize cumulative probability for a day
		/// params:
		/// - stepPerHour: time step per hour
		void         initEvent(int stepPerHour);

		/// calculate event occur time
		/// return:
		/// - stepPerHour: event occur time
		int          getOccurTime();

	private:
		std::map<int, double>   m_timeCumuProbMap;
		std::vector<double>     m_timeCumuProb; //Size: minutes in a day
	};
};

#endif