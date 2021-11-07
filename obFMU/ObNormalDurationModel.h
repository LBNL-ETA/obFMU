/// Normal probability model for duration
#ifndef ObNormalDurationModel_h
#define ObNormalDurationModel_h
#include "ObDurationModel.h"
#include <math.h>
using namespace std;
namespace OB {
	/// TODO: change the class name to match the schema
	class NormalDurationModel : public DurationModel {
	public:
        NormalDurationModel(void);
        
		/// initialize by reading an TiXmlElement
		/// params:
		/// - element: the TiXMLElement
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		
		bool	     readObXML(TiXmlElement * parentElem, string& error);
		///Initialize duration model based on simulation settings
		/// params:
		/// - stepPerHour: time step per hour
		void         initModel(int stepPerHour);
		/// Randomly generate event duration
		/// return:
		/// - duration
		int          getDuration();

	private:
		int                     m_shortDuration;
		int                     m_typicalDuration;
		std::vector<double>     m_durationCumuProb; //Size: minutes in a day
		bool                    getTimeInDay(string time, int& hour, int& min,  string& error);
	};
};

#endif
