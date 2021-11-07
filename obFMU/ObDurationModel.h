/// Duration model an abstract class
#ifndef ObDurationModel_h
#define ObDurationModel_h

#include "ObObject.h"
#include <math.h>
using namespace std;
namespace OB {

	class DurationModel{
	public:
		/// initialize by reading an TiXmlElement
		/// params:
		/// - element: the TiXMLElement
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		virtual bool readObXML(TiXmlElement * parentElem, string& error) = 0;
		///Initialize duration model based on simulation settings
		/// params:
		/// - stepPerHour: time step per hour
		virtual void initModel(int stepPerHour) = 0;
		/// Randomly generate event duration
		/// return:
		/// - duration
		virtual int getDuration() = 0;

	protected:

		int m_duration;

	};
};

#endif
