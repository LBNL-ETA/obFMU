/* ----------------------------------------------------------------------------
** The object represents the SimulationSettings node in obCoSim schema.   
** Author: Yixing Chen
** History:
** 6/30/2015: first release
** -----------------------------------------------------------------------------*/
#ifndef ObSimulationSettings_h
#define ObSimulationSettings_h

#include "ObObject.h"
using namespace std;

namespace OB{
	class SimulationSettings: public Object {
	public:
		SimulationSettings();
		~SimulationSettings();
		SimulationSettings(const SimulationSettings& other);
		SimulationSettings& operator= (const SimulationSettings&);		
		
		/// initialize by reading an TiXmlElement
		/// params:
		/// - element: the TiXMLElement
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		bool	readObXML(TiXmlElement* element, std::string& error);

		//Year?????
		int     getSimulationStartYear();

		int		getTotalNumberofTimeSteps();
		int		getTotalNumberofDays();
		int		getDayofWeek(int dayIndex);
		int		getNumberofTimestepsPerHour();
		int		getStartDayofYear();
		int		getEndDayofYear();
		int		getTimeStepInMinutes();
		bool	isLeepYear();
		bool	isDebugMode();
		bool	useEPLaunch();
		bool	doMovementCalculation();
		string	getUserMovementResultFilename();

		bool    shouldExportCSVResults();

		void	display();

		bool	checkSimulationPeriod(string startTime, string endTime, int numberOfStepsPerHour, int& totalNumberOfSteps, string& error);

	private:
		int			m_simulationStartMonth;
		int			m_simulationStartDay;
		int			m_simulationEndMonth;
		int			m_simulationEndDay;
		int			m_simulationNumberofTimestepsPerHour;
		bool		m_isLeapYear;
		bool		m_doMovementCalculation;
		bool		m_isDebugMode;
		string		m_userMovementResultFilename;

		bool		m_shouldExportCSVResults;

		bool		m_useEPLaunch;
		
		map<string, int>	DAYOFWEEKNAMES;
		/// 0 - Sunday, 1- Monday, 6- Saturday;
		int			m_dayofWeekForStartDay;

		int			m_startDayofYear;
		int			m_endDayofYear;
		int			m_totalNumberofTimeSteps;
		int			m_totalNumberofDays;
	};
};
#endif