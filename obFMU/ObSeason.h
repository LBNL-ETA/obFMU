/* ----------------------------------------------------------------------------
** The object represents the Season node in obXML schema.   
** Author: Yixing Chen
** History:
** 6/30/2015: first release
** -----------------------------------------------------------------------------*/
#ifndef ObSeason_h
#define ObSeason_h

#include "ObObject.h"
#include "ObSimulationSettings.h"
using namespace std;

namespace OB{
	class Season: public Object {
	public:
		enum SeasonType{
			Season_Spring,
			Season_Summer,
			Season_Fall,
			Season_Winter,
			Season_All
		};

		Season(SeasonType type);
		~Season();
		Season(const Season& other);
		Season& operator= (const Season&);		
		
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
		bool	checkDayStepFlag(int dayIndex);

	private:
		SeasonType	m_type;
		int			m_startMonth;
		int			m_startDay;
		int			m_endMonth;
		int			m_endDay;
		int			m_seasonStartDayofYear;
		int			m_seasonEndDayofYear;
		
		/// The flags related to each day
		/// True means the day is within the DayofWeek
		/// False means the day is ouside the DayofWeek
		vector<bool>		m_timeStepFlags;

		int			m_simulationNumberofTimestepsPerDay;
	};
};
#endif