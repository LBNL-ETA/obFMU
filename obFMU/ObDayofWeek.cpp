#include "ObDayofWeek.h"

namespace OB{
	DayofWeek::DayofWeek(){
	};
	DayofWeek::DayofWeek(DayofWeekType type){
		m_type = type;
	};
	DayofWeek::~DayofWeek(){

	};
	DayofWeek::DayofWeek(const DayofWeek& other){
		m_type = other.m_type;
		//m_simulationSettings = other.m_simulationSettings;
	};
	DayofWeek& DayofWeek::operator= (const DayofWeek& other){
		if(this != &other){
			m_type = other.m_type;
			//m_simulationSettings = other.m_simulationSettings;
		}
		return *this;
	};

	bool DayofWeek::operator== (const DayofWeek& other){
		if (m_type == other.m_type) return true;
		else return false;
	}

	/// Check whehther the time step is with in the DayofWeek.
	bool	DayofWeek::checkTimeStepFlag(int timeStepIndex){
		int dayofTimeStep = timeStepIndex / m_simulationNumberofTimestepsPerDay;
		return m_timeStepFlags[dayofTimeStep];
	};

	/// Check whehther the time step is with in the DayofWeek.
	bool	DayofWeek::checkDayStepFlag(int dayNum){
		return m_timeStepFlags[dayNum];
	};


	void	DayofWeek::initTimeStepFlags(SimulationSettings* settings){
		m_simulationNumberofTimestepsPerDay = settings->getNumberofTimestepsPerHour() * 24;
		m_timeStepFlags.clear();
		int typeInt =(int)m_type;
		int numberofDays = settings->getTotalNumberofDays() + 1;
		for(int i=0; i < numberofDays; i++){
			if(m_type==DayofWeek_All)
				m_timeStepFlags.push_back(true);
			else{
				int dayofWeek = settings->getDayofWeek(i);
				if(typeInt <= 6){
					if(typeInt == dayofWeek)
						m_timeStepFlags.push_back(true);
					else
						m_timeStepFlags.push_back(false);
				}else if(m_type==DayofWeek_Weekdays){
					if(dayofWeek >=1 && dayofWeek <= 5)
						m_timeStepFlags.push_back(true);
					else
						m_timeStepFlags.push_back(false);					
				}else if(m_type==DayofWeek_Weekends || m_type==DayofWeek_Holiday){
					if(dayofWeek==0 || dayofWeek == 6)
						m_timeStepFlags.push_back(true);
					else
						m_timeStepFlags.push_back(false);
				}else{
					m_timeStepFlags.push_back(false);
				}
			}
		}
	};

};