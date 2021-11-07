/* ----------------------------------------------------------------------------
** The object represents the DayofWeek.   
** Author: Yixing Chen
** History:
** 6/30/2015: first release
** -----------------------------------------------------------------------------*/
#ifndef ObDayofWeek_h
#define ObDayofWeek_h

#include "ObObject.h"
#include "ObSimulationSettings.h"
using namespace std;

namespace OB{
	class DayofWeek: public Object {
	public:
		enum DayofWeekType{
			DayofWeek_Sunday,
			DayofWeek_Monday,
			DayofWeek_Tuesday,
			DayofWeek_Wednesday,
			DayofWeek_Thursday,
			DayofWeek_Friday,
			DayofWeek_Saturday,
			DayofWeek_Weekdays,
			DayofWeek_Weekends,
			DayofWeek_Holiday,
			DayofWeek_All
		};
		DayofWeek();
		DayofWeek(DayofWeekType type);
		~DayofWeek();
		DayofWeek(const DayofWeek& other);
		DayofWeek& operator= (const DayofWeek&);
		bool operator==(const DayofWeek&);

		/// Init the time step flags
		void	initTimeStepFlags(SimulationSettings* settings);

		/// Check whehther the time step is with in the scrop.
		bool	checkTimeStepFlag(int timeStepIndex);\

		/// Check whehther the day step is with in the scrop.
		bool	checkDayStepFlag(int timeStepIndex);

	private:
		DayofWeekType		m_type;
		int					m_simulationNumberofTimestepsPerDay;

		/// The flags related to each day
		/// True means the day is within the DayofWeek
		/// False means the day is ouside the DayofWeek
		vector<bool>		m_timeStepFlags;
	};
};
#endif