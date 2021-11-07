#include "ObTimeofDay.h"

namespace OB{
	TimeofDay::TimeofDay(TimeofDayType type){
		m_type = type;
		switch(m_type){
			case TimeofDay_Morning:
				m_startHour = 0;
				m_startMinute = 0;
				m_endHour = 12;
				m_endMinute = 0;
				break;
			case TimeofDay_Noon:
				m_startHour = 11;
				m_startMinute = 30;
				m_endHour = 12;
				m_endMinute = 30;
				break;
			case TimeofDay_Afternoon:
				m_startHour = 12;
				m_startMinute = 0;
				m_endHour = 18;
				m_endMinute = 0;
				break;
			case TimeofDay_Evening:
				m_startHour = 18;
				m_startMinute = 0;
				m_endHour = 24;
				m_endMinute = 0;
				break;
			case TimeofDay_Day:
				m_startHour = 6;
				m_startMinute = 0;
				m_endHour = 18;
				m_endMinute = 0;
				break;
			case TimeofDay_Night:
				m_startHour = 18;
				m_startMinute = 0;
				m_endHour = 6;
				m_endMinute = 0;
				break;
			case TimeofDay_All:
				m_startHour = 0;
				m_startMinute = 0;
				m_endHour = 24;
				m_endMinute = 0;
				break;
		}

	};
	TimeofDay::~TimeofDay(){};
	TimeofDay::TimeofDay(const TimeofDay& other){
		m_type = other.m_type;
		m_startHour = other.m_startHour;
		m_startMinute = other.m_startMinute;
		m_endHour = other.m_endHour;
		m_endMinute = other.m_endMinute;
	};
	TimeofDay& TimeofDay::operator= (const TimeofDay& other){
		if(this != &other){
			m_type = other.m_type;
			m_startHour = other.m_startHour;
			m_startMinute = other.m_startMinute;
			m_endHour = other.m_endHour;
			m_endMinute = other.m_endMinute;
		}
		return *this;
	};

	/// initialize by reading an TiXmlElement
	/// params:
	/// - element: the TiXMLElement
	/// - error: the error information, when return false
	/// Return:
	/// - bool: readObXML successful or not
	bool	TimeofDay::readObXML(TiXmlElement* element, std::string& error)
	{
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if(!strcmp(child->Value(),"StartHour")){
				m_startHour = atoi(childElement->GetText());
			}else if(!strcmp(child->Value(),"StartMinute")){
				m_startMinute = atoi(childElement->GetText());
			}else if(!strcmp(child->Value(),"EndHour")){
				m_endHour = atoi(childElement->GetText());
			}else if(!strcmp(child->Value(),"EndMinute")){
				m_endMinute = atoi(childElement->GetText());
			}else{
				error+= "Found unexpected child element: (";
				error += child->Value();
				error += ") in TimeofDay element.\n";
				return false;
			}
		}

		if(m_startHour > 24 || m_startHour < 0){
			error += "Wrong StartHour. Should be within 0 to 24.\n";
			return false;
		}

		if(m_endHour > 24 || m_endHour < 0){
			error += "Wrong EndHour. Should be within 0 to 24.\n";
			return false;
		}

		if(m_startMinute >= 60 || m_startMinute < 0){
			error += "Wrong StartMinute. Should be within 0 to 59.\n";
			return false;
		}

		if(m_endMinute >= 60 || m_endMinute < 0){
			error += "Wrong EndMinute. Should be within 0 to 59.\n";
			return false;
		}

		return true;
	};

	/// Init the time step flags
	void	TimeofDay::initTimeStepFlags(SimulationSettings* settings){
		m_simulationNumberofTimestepsPerDay = settings->getNumberofTimestepsPerHour() * 24;
		m_timeStepFlags.clear();
		int startInMinutes = m_startHour * 60 + m_startMinute;
		int endInMinutes = m_endHour * 60 + m_endMinute;
	
		bool isCross24;
		if(startInMinutes > endInMinutes)
			isCross24 = true;
		else
			isCross24 = false;

		for(int i=0; i< m_simulationNumberofTimestepsPerDay; i++){
			int timeInMinutes = i * 60 / settings->getNumberofTimestepsPerHour();
			if(isCross24){
				if(timeInMinutes >= startInMinutes || timeInMinutes <= endInMinutes)
					m_timeStepFlags.push_back(true);
				else
					m_timeStepFlags.push_back(false);
			}else{
				if(timeInMinutes >= startInMinutes && timeInMinutes <= endInMinutes)
					m_timeStepFlags.push_back(true);
				else
					m_timeStepFlags.push_back(false);
			}			
		}
	};

	/// Check whehther the time step is with in the scrop.
	bool	TimeofDay::checkTimeStepFlag(int timeStepIndex){
		int timeStepInDay = timeStepIndex % m_simulationNumberofTimestepsPerDay; 
		return m_timeStepFlags[timeStepInDay];
	};

};