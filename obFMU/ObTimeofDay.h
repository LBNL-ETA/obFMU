/* ----------------------------------------------------------------------------
** The object represents the TimeofDay node in obXML schema.   
** Author: Yixing Chen
** History:
** 6/30/2015: first release
** -----------------------------------------------------------------------------*/
#ifndef ObTimeofDay_h
#define ObTimeofDay_h

#include "ObObject.h"
#include "ObSimulationSettings.h"
using namespace std;

namespace OB{
	class TimeofDay: public Object {
	public:
		enum TimeofDayType{
			TimeofDay_Morning,
			TimeofDay_Noon,
			TimeofDay_Afternoon,
			TimeofDay_Evening,
			TimeofDay_Day,
			TimeofDay_Night,
			TimeofDay_All
		};

		TimeofDay(TimeofDayType type);
		~TimeofDay();
		TimeofDay(const TimeofDay& other);
		TimeofDay& operator= (const TimeofDay&);		
		
		/// initialize by reading an TiXmlElement
		/// params:
		/// - element: the TiXMLElement
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		bool	readObXML(TiXmlElement* element, std::string& error);

		/// Init the time step flags
		void	initTimeStepFlags(SimulationSettings* settings);

		/// Check whehther the time step is with in the scrop.
		bool	checkTimeStepFlag(int timeStepIndex);

	private:
		TimeofDayType	m_type;
		int				m_startHour;
		int				m_startMinute;
		int				m_endHour;
		int				m_endMinute;

		int				m_simulationNumberofTimestepsPerDay;

		/// The flags related to each day
		/// True means the day is within the DayofWeek
		/// False means the day is ouside the DayofWeek
		vector<bool>		m_timeStepFlags;

	};
};
#endif