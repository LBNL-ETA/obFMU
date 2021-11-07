#include "ObMeetingEvent.h"
#include <iostream>
#include <random>

namespace OB{
	MeetingEvent::MeetingEvent(Object* parent):Object(parent) {};
	MeetingEvent::~MeetingEvent() {};
	MeetingEvent::MeetingEvent(const MeetingEvent& other):Object(other) {
	};
	MeetingEvent& MeetingEvent::operator= (const MeetingEvent& other) {
		return *this;
	};

	bool MeetingEvent::readObXML(TiXmlElement* element, std::string& error) {		
		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) { 
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if (!strcmp(child->Value(), "DayofWeek")) {
				string type = childElement->GetText();
				if (type == "Monday") {
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Monday);
				} else if (type == "Tuesday") {
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Tuesday);
				} else if (type == "Wednesday") {
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Wednesday);
				} else if (type == "Thursday") {
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Thursday);
				} else if (type == "Friday") {
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Friday);
				} else if (type == "Saturday") {
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Saturday);
				} else if (type == "Sunday") {
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Sunday);
				} else if (type == "Weekdays") {
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Weekdays);
				} else if (type == "Weekends") {
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Weekends);
				} else if (type == "Holiday") {
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Holiday);
				} else if (type == "All") {
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_All);
				} else {
					error += "Find unexpected DayofWeek: (";
					error += type;
					error += ") in Time element.\n";
					return false;
				}

			} else if (!strcmp(child->Value(), "SeasonType")) {
				string type = childElement->GetText();
				if (type == "Spring") {
					m_seasonTypes.push_back(Season::Season_Spring);
				} else if (type == "Summer") {
					m_seasonTypes.push_back(Season::Season_Summer);
				} else if (type == "Fall") {
					m_seasonTypes.push_back(Season::Season_Fall);
				} else if (type == "Winter") {
					m_seasonTypes.push_back(Season::Season_Winter);
				} else if (type == "All") {
					m_seasonTypes.push_back(Season::Season_All);
				} else {
					error += "Find unexpected SeasonType: (";
					error += type;
					error += ") in Time element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(),"MaxNumOccupantsPerMeeting")) {
				m_maxNumOfPeople = atoi(childElement->GetText());
			} else if (!strcmp(child->Value(),"MinNumOccupantsPerMeeting")) {
				m_minNumOfPeople = atoi(childElement->GetText());
			} else if (!strcmp(child->Value(), "MaxNumberOfMeetingsPerDay")) {
				m_maxNumOfMeetingsPerDay = atoi(childElement->GetText());

			} else if (!strcmp(child->Value(), "MinNumberOfMeetingsPerDay")) {
				m_minNumOfMeetingsPerDay = atoi(childElement->GetText());

			} else if (!strcmp(child->Value(), "MeetingDurationProbability")) {
				if (!readMeetingProb(childElement, error)) {
					error += "Fail to read the MovementEvent element.\n";
					return false;
				}
			} else {
				error += "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}
		return true;
	}

	bool MeetingEvent::readMeetingProb(TiXmlElement* element, std::string& error) {
		bool hasDuration = false;
		bool hasProb = false;
		double duration, prob;
		
		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) { 
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if (!strcmp(child->Value(), "MeetingDuration")) {
				hasDuration = true;
				if(!getXsDurationInMinutes(childElement->GetText(), duration)){
					error += "Fail to get Duration.\n";
					return false;
				};

			} else if (!strcmp(child->Value(), "Probability")) {
				hasProb = true;
				prob = atof(childElement->GetText());

			} else {
				error += "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		if (!hasDuration || !hasProb) {
			error+= "Fail to find the Movement events element.\n";
			return false;
		} else {
			m_meetingDurationProb.insert(std::pair<double,double>(duration, prob));
		}
		return true;
	}

	vector<Season::SeasonType> MeetingEvent::getSeasonTypes(){
		return m_seasonTypes;
	};

	void MeetingEvent::addSeasion(Season* obj){
		m_seasons.push_back(obj);
	};
	
	vector<DayofWeek::DayofWeekType> MeetingEvent::getDayofWeekTypes(){
		return m_dayofWeekTypes;
	};

	void MeetingEvent::addDayofWeek(DayofWeek* obj){
		m_dayofWeeks.push_back(obj);
	};
	void MeetingEvent::setLocationId(int id) {
		m_locationId = id;
	}

	int MeetingEvent::getLocationId() {
		return m_locationId;
	}

	bool MeetingEvent::checkDayFlag(int dayNum) {
		bool dayFlag = false;
		bool seasonFlag = false;
		if (m_dayofWeeks.size() > 0) {			
			for (size_t i = 0; i < m_dayofWeeks.size(); i++) {
				if (m_dayofWeeks[i]->checkDayStepFlag(dayNum)) {
					dayFlag = true;
				}
			}
		}
		if (m_seasons.size() > 0) {			
			for (size_t i = 0; i < m_seasons.size(); i++) {
				if (m_seasons[i]->checkDayStepFlag(dayNum)) {
					seasonFlag = true;
				}
			}
		}
		return (dayFlag && seasonFlag);
	}

	/// Number of meetings: uniform distribution
	int	MeetingEvent::getNumOfMeetingsPerDay() {
		int rNum = rand() % (m_maxNumOfMeetingsPerDay - m_minNumOfMeetingsPerDay + 1) + m_minNumOfMeetingsPerDay;;
		return rNum;
	}

	/// Number of people: uniform distribution
	int	MeetingEvent::getNumOfOccupants() {
		int number = rand() % (m_maxNumOfPeople - m_minNumOfPeople + 1) + m_minNumOfPeople;
		return number;
	}
	/// Choose duration based on probability distribution
	int	MeetingEvent::getDuration() {
		int duration;
		std::vector<double> keySorted;
		for (map<double, double>::iterator it = m_meetingDurationProb.begin(); 
			it != m_meetingDurationProb.end(); ++it) {
			keySorted.push_back(it->first);
		}
		size_t size = keySorted.size();
		std::sort (keySorted.begin(), keySorted.begin() + size);
		double rNum = (double)rand() / (double)RAND_MAX;
		
		for (size_t i = 0; i < 10; i++) {
			rNum = (double)rand() / (double)RAND_MAX;
			
		}
		
		double cumProb = 0;
		for (size_t i = 0; i < keySorted.size(); i++) {
			cumProb += m_meetingDurationProb.at(keySorted[i]);
			if (rNum < cumProb) {
				duration = (int)keySorted[i];
				break;
			}
		}
		return duration;
	}

};