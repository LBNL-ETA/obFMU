#include "ObMovementBehavior.h"
#include "ObSpace.h"
#include "ObStatusTransitionEvent.h"
#include <math.h>

namespace OB{
	MovementBehavior::MovementBehavior(Object* parent):Object(parent){

	};
	MovementBehavior::~MovementBehavior(){};
	MovementBehavior::MovementBehavior(const MovementBehavior& other){};
	MovementBehavior& MovementBehavior::operator= (const MovementBehavior&){
		return *this;
	};
	   
	/// initialize by reading an TiXmlElement
	/// params:
	/// - element: the TiXMLElement
	/// - error: the error information, when return false
	/// Return:
	/// - bool: readObXML successful or not
	bool MovementBehavior::readObXML(TiXmlElement* element, std::string& error) {
		string id;
		if (!getStringAttribute(element, "ID", id, error)) {
			error += "Fail to find ID attribute for Movement Behavior element.\n";
			return false;
		}

		/// Set the ID
		if (!setID(id, error)) {
			error += "Fail to set ID for Space.\n";
			return false;
		}

		bool hasMovements = false;
		int transEventCount = 0;
		
		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) { 
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if (!strcmp(child->Value(), "Description")) {
				setDescription(childElement->GetText());
			} else if (!strcmp(child->Value(), "DayofWeek")) {

				/// TODO: YXC: should change to some more general method
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
			} else if (!strcmp(child->Value(), "RandomMovementEvent")) {
				///Read random movement events
				hasMovements = true;
				RandomMovementEvent* obj = new RandomMovementEvent();
				if (!obj->readObXML(childElement, error)) {
					error += "Fail to read the RandomMovementEvent element.\n";
					return false;
				}
				m_randomMovementModel = obj;

			} else if (!strcmp(child->Value(), "StatusTransitionEvent")) {
				transEventCount += 1;
				StatusTransitionEvent* obj = new StatusTransitionEvent();
				if (!obj->readObXML(childElement, error)) {
					error += "Fail to read the StatusTransitionEvent element.\n";
					return false;
				}
				m_transEvents.push_back(obj);

			} else {
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		if (!hasMovements) {
			error+= "Fail to find the Movement events element.\n";
			return false;
		} 
		if (transEventCount < 1) {
			error+= "Fail to find the Status transition events element.\n";
			return false;
		}
		return true;
	};

	void MovementBehavior::setTimeStep(int dt){
		m_dt = dt;
	};

	vector<Season::SeasonType> MovementBehavior::getSeasonTypes(){
		return m_seasonTypes;
	};

	void MovementBehavior::addSeasion(Season* obj){
		m_seasons.push_back(obj);
	};
	
	vector<DayofWeek::DayofWeekType> MovementBehavior::getDayofWeekTypes(){
		return m_dayofWeekTypes;
	};

	void MovementBehavior::addDayofWeek(DayofWeek* obj){
		m_dayofWeeks.push_back(obj);
	};
    
	///Check number of movement behavior on a specific day, report error if not one.
	int MovementBehavior::movementNumOnDay(int dayNum) {
		int count = 0;
		if (m_dayofWeeks.size() > 0) {			
			for (size_t i = 0; i < m_dayofWeeks.size(); i++) {
				if (m_dayofWeeks[i]->checkDayStepFlag(dayNum)) {
					count++;
				}
			}
		}
		return count;
	}

	RandomMovementEvent*  MovementBehavior::getRandomMoveModel() {
		return m_randomMovementModel;
	}

	double MovementBehavior::getMeetingPercent() {
		return m_randomMovementModel->getMeetingPercent();
	}
	
	///Initialize status trandistion and random movement events
	void MovementBehavior::initEvents(SimulationSettings* settings, std::string& error) {
		for(size_t i = 0; i < m_transEvents.size(); i++) {
			m_transEvents[i]->initEvent(60 / m_dt); 
		}
		m_randomMovementModel->initEvent(60 / m_dt, error); 
	}

	void MovementBehavior::initLocationCategory(vector<vector<int>> locationIdMap) {
		m_randomMovementModel->initLocationCategory(locationIdMap);	
	}
	
	// Simulate status schedule for each day.
	bool MovementBehavior::simStatusSchedule(std::vector<int> &m_statusSchedule){
		/// Set event flags
		for (size_t i = 0; i < m_transEvents.size(); i++) {
			int t = m_transEvents[i]->getOccurTime();
			int d;
			if (m_transEvents[i]->getEventType() == StatusTransitionEvent::EVENT_SHORTABSENCE || 
				m_transEvents[i]->getEventType() == StatusTransitionEvent::EVENT_SHORTPRESENCE) {
			    d = m_transEvents[i]->getDuration();
			} else {
				d = 0;
			}
			if (t + d > (int)m_statusSchedule.size()) {
				return false;
			}
			if (t == (int)m_statusSchedule.size() - 1) {
				continue;
			}

			if (m_transEvents[i]->getEventType() == StatusTransitionEvent::EVENT_SHORTABSENCE) {
				// Set the next time step
				m_statusSchedule[t + 1] = 0;
				/// Avoid overwriting previous events
				if (t + d != m_statusSchedule.size() - 1 && m_statusSchedule[t + d + 1] != 0 ) {
					m_statusSchedule[t + d + 1] = 1;
				}
			} else if (m_transEvents[i]->getEventType() == StatusTransitionEvent::EVENT_SHORTPRESENCE) {
				// Set the next time step
				m_statusSchedule[t + 1] = 1;
				if (m_statusSchedule[t + d + 1] != 1) {
					m_statusSchedule[t + d + 1] = 0;
				}
			} else if (m_transEvents[i]->getEventType() == StatusTransitionEvent::EVENT_ENTERING) {
				m_statusSchedule[t + 1] = 1;
			} else {
				m_statusSchedule[t + 1] = 0;
			}
		}

		///Set schedules
		int flag = 0;

		for (size_t i = 0; i < m_statusSchedule.size(); i++) {
			if (m_statusSchedule[i] == 1) {
				flag = 1;
			} else if (m_statusSchedule[i] == 0) {
				flag = 0;
			}
			m_statusSchedule[i] = flag;
		}
		return true;
	}
}