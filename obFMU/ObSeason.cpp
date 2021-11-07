#include "ObSeason.h"

namespace OB{
	Season::Season(SeasonType type){
		m_type = type;
		switch(m_type){
		case Season_Spring:
			m_startMonth = 3;
			m_startDay = 1;
			m_endMonth = 5;
			m_endDay = 31;
			break;
		case Season_Summer:
			m_startMonth = 6;
			m_startDay = 1;
			m_endMonth = 8;
			m_endDay = 31;
			break;
		case Season_Fall:
			m_startMonth = 9;
			m_startDay = 1;
			m_endMonth = 11;
			m_endDay = 30;
			break;
		case Season_Winter:
			m_startMonth = 12;
			m_startDay = 1;
			m_endMonth = 2;
			m_endDay = 28;
			break;
		case Season_All:
			m_startMonth = 1;
			m_startDay = 1;
			m_endMonth = 12;
			m_endDay = 31;
			break;
		}
		m_timeStepFlags.clear();
	};
	Season::~Season(){};
	Season::Season(const Season& other){
		m_type = other.m_type;
		m_startMonth = other.m_startMonth;
		m_startDay = other.m_startDay;
		m_endMonth = other.m_endMonth;
		m_endDay = other.m_endDay;
	};
	Season& Season::operator= (const Season& other){
		if(this != &other){
			m_type = other.m_type;
			m_startMonth = other.m_startMonth;
			m_startDay = other.m_startDay;
			m_endMonth = other.m_endMonth;
			m_endDay = other.m_endDay;
		}
		return *this;
	};

	/// initialize by reading an TiXmlElement
	/// params:
	/// - element: the TiXMLElement
	/// - error: the error information, when return false
	/// Return:
	/// - bool: readObXML successful or not
	bool	Season::readObXML(TiXmlElement* element, std::string& error)
	{
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if(!strcmp(child->Value(),"StartMonth")){
				m_startMonth = atoi(childElement->GetText());
			}else if(!strcmp(child->Value(),"StartDay")){
				m_startDay = atoi(childElement->GetText());
			}else if(!strcmp(child->Value(),"EndMonth")){
				m_endMonth = atoi(childElement->GetText());
			}else if(!strcmp(child->Value(),"EndDay")){
				m_endDay = atoi(childElement->GetText());
			}else{
				error+= "Found unexpected child element: (";
				error += child->Value();
				error += ") in Season element.\n";
				return false;
			}
		}

		if(getDayofYear(m_startMonth, m_startDay)==0 && getDayofYear(m_startMonth, m_startDay, true)==0){
			error += "Wrong combination of StartMonth and StartDay.\n";
			return false;
		}

		if(getDayofYear(m_endMonth, m_endDay)==0 && getDayofYear(m_endMonth, m_endDay, true)==0){
			error += "Wrong combination of EndMonth and EndDay.\n";
			return false;
		}

		return true;
	};

			
	/// Init the time step flags
	void	Season::initTimeStepFlags(SimulationSettings* settings){
		m_simulationNumberofTimestepsPerDay = settings->getNumberofTimestepsPerHour() * 24;
		bool isLeapYear = settings->isLeepYear();
		int simulationStartDayofYear = settings->getStartDayofYear();
		m_timeStepFlags.clear();

		int seasonStartDayofYear = getDayofYear(m_startMonth, m_startDay, isLeapYear);
		int seasonEndDayofYear = getDayofYear(m_endMonth, m_endDay, isLeapYear);
		/// The start month, start day, end month and end day have been checked when read from the obXML.
		/// So if the day of year == 0, they must be Feb 29.
		/// In this case, juse sent the day of year to Feb 28, which is 59. 
		if(seasonStartDayofYear == 0)
			seasonStartDayofYear = 59;

		if(seasonEndDayofYear ==0)
			seasonEndDayofYear = 59;
			
		bool isCross365;
		if(seasonStartDayofYear > seasonEndDayofYear)
			isCross365 = true;
		else
			isCross365 = false;

		int numberofDays = settings->getTotalNumberofDays() + 1;
		for(int i=0; i < numberofDays; i++){
			int currentDayofYear = simulationStartDayofYear + i;
			if(isCross365){
				if(currentDayofYear >= seasonStartDayofYear || currentDayofYear <= seasonEndDayofYear)
					m_timeStepFlags.push_back(true);
				else
					m_timeStepFlags.push_back(false);
			}else{
				if(currentDayofYear >= seasonStartDayofYear && currentDayofYear <= seasonEndDayofYear)
					m_timeStepFlags.push_back(true);
				else
					m_timeStepFlags.push_back(false);
			}
		}
	
	};

	/// Check whehther the time step is with in the scrop.
	bool	Season::checkTimeStepFlag(int timeStepIndex){
		int dayofTimeStep = timeStepIndex / m_simulationNumberofTimestepsPerDay;
		return m_timeStepFlags[dayofTimeStep];	
	};

	bool	Season::checkDayStepFlag(int dayIndex){
		return m_timeStepFlags[dayIndex];	
	};

};