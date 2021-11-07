#ifndef ObStatusTransitionEvent_h
#define ObStatusTransitionEvent_h

/// This class is used for calculating the status transision events (e.g. arrival and departure)
#include "ObObject.h"
#include "ObEventOccurModel.h"
#include "ObNormalDurationModel.h"
#include "ObEventOccurModelFactory.h"

#include <math.h>
using namespace std;
namespace OB {
	/// TODO: change the class name to match the schema
	class StatusTransitionEvent: public Object {
	public:

		enum EventType{
			EVENT_ENTERING,
			EVENT_LEAVING,
			EVENT_SHORTABSENCE,
			EVENT_SHORTPRESENCE,
			EVENT_NONE
		};
		StatusTransitionEvent(void);
		~StatusTransitionEvent(void);
		StatusTransitionEvent& operator= (const StatusTransitionEvent&);

		/// initialize by reading an obXML file
		/// params: 
		/// - filename: the filename of the obXML file including the path
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		bool	     readObXML(TiXmlElement * parentElem, string& error);
		
		/// initialize event models
		void         initEvent(int stepPerHour);
		int          getOccurTime();
		int          getDuration();
		EventType    getEventType();

	private:
		EventType               m_type;
		DurationModel*          m_durationModel;
		EventOccurModel*        m_eventOccurModel;
		EventOccurModelFactory* m_fac;

		bool	     readEventOccurence(TiXmlElement * parentElem, string& error);
		bool         readEventDurationInput(TiXmlElement * element, string& error);
	};
};

#endif