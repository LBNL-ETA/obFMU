/// Event Occurence model abstract class
#ifndef ObEventOccurModel_h
#define ObEventOccurModel_h

#include "ObObject.h"
#include <math.h>
using namespace std;
namespace OB {

	class EventOccurModel {
	public:
		EventOccurModel(void) {
			m_occurTime = 0;
		}
		/// initialize by reading an TiXmlElement
		/// params:
		/// - element: the TiXMLElement
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		virtual bool readObXML(TiXmlElement * parentElem, string& error) = 0;
		///Initialize event based on simulation settings
		/// params:
		/// - stepPerHour: time step per hour
		virtual void initEvent(int stepPerHour) = 0;
		/// Randomly generate event occur time
		/// return:
		/// - event occur time
		virtual int getOccurTime() = 0;

	protected:
		int m_occurTime;

	};
};

#endif