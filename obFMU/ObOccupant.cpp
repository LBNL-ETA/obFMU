#include "ObOccupant.h"

namespace OB {

	Occupant::Occupant(Object* parent):Object(parent) {
		m_priority = 0; /// Default priority is 0
	};

	Occupant::~Occupant(){};
	Occupant::Occupant(const Occupant& other) {
		m_name = other.m_name;
		m_age = other.m_age;
		m_gender = other.m_gender;
		m_lifeStyle = other.m_lifeStyle;
		m_jobType = other.m_jobType;
		m_priority = other.m_priority; 

		m_behaviorIDs = other.m_behaviorIDs;
		m_behaviors = other.m_behaviors;
	};

	Occupant& Occupant::operator= (const Occupant& other) {
		if (this != &other) {
			m_name = other.m_name;
			m_age = other.m_age;
			m_gender = other.m_gender;
			m_lifeStyle = other.m_lifeStyle;
			m_jobType = other.m_jobType;
			m_priority = other.m_priority;

			m_behaviorIDs = other.m_behaviorIDs;
			m_behaviors = other.m_behaviors;
		}
		return *this;
	};

	/// initialize by reading an TiXmlElement
	/// params:
	/// - element: the TiXMLElement
	/// - error: the error information, when return false
	/// Return:
	/// - bool: readObXML successful or not
	bool Occupant::readObXML(TiXmlElement* element, std::string& error) {
		string id;
		if (!getStringAttribute(element, "ID", id, error)) {
			error += "Fail to find ID attribute for Occupant element.\n";
			return false;
		}

		if (!getStringAttribute(element, "Name", m_name, error, true)) {
			error += "Fail to find Name attribute for Occupant element.\n";
			return false;
		}

		/// Set the ID
		if (!setID(id, error)) {
			error += "Fail to set ID for Building.\n";
			return false;
		}
		
		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) { 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;

			if(!strcmp(child->Value(), "Age")) {
				m_age = atoi(childElement->GetText());
			} else if (!strcmp(child->Value(), "Priority")) {
				m_priority = atoi(childElement->GetText());
			} else if (!strcmp(child->Value(), "Description")) {
				setDescription(childElement->GetText());
			} else if (!strcmp(child->Value(), "Gender")) {
				string gender = childElement->GetText();
				if (gender == "Male") {
					m_gender = Gender_Male;
				} else if(gender == "Female") {
					m_gender = Gender_Female;
				} else {
					error += "Wrong gender for occupant ";
					error += getID();
					error += ".\n";
					return false;
				}

			} else if (!strcmp(child->Value(), "LifeStyle")) {
				m_lifeStyle = childElement->GetText();
			} else if (!strcmp(child->Value(), "JobType")) {
				m_jobType = childElement->GetText();
			} else if (!strcmp(child->Value(), "MovementBehaviorID")) {
				m_movementBehaviorIDs.push_back(childElement->GetText());
			} else if (!strcmp(child->Value(), "BehaviorID")) {
				m_behaviorIDs.push_back(childElement->GetText());
			} else {
				error += "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		return true;
	};

	/// Get the behavior IDs
	vector<string>	Occupant::getMovementBehaviorIDs(){
		return m_movementBehaviorIDs;
	};


	/// Get the behavior IDs
	vector<string>	Occupant::getBehaviorIDs(){
		return m_behaviorIDs;
	};

	/// Add Movement Behavior to the list                                                    ///new move behavior list
	void Occupant::addMoveBehavior(MovementBehavior* obj){
		m_moveBehaviors.push_back(obj);
	}

	/// Add Behavior to the list
	void Occupant::addBehavior(Behavior* obj){
		m_behaviors.push_back(obj);

	}

	/// Set scheduled meeting time
	void Occupant::setMeetingTime(int meetingTime) {
		curMeetingTimeInDay = meetingTime;
	}

	/// Get scheduled meeting time for checking
	int Occupant::getCurMeetingTimeInDay() {
		return curMeetingTimeInDay;
	}

	/// Get proposed meeting time percentage for checking
	double Occupant::getMaxMeetingTimePercent(int day) {
		return m_dailyMeetingPercentVec[day];
	}
	
	/// Get the priority, the larger value means higher priority
	int Occupant::getPriority(){
		return m_priority;
	};

	/// Perform interaction
	/// Params:
	/// - numberofTimeStep: the number of current time step
	/// - systemType: which system, window, HVAC, or lights
	/// - interactionType: turn on or turn off
	/// - results: the output; 1 for perform the action (turn on or turn off), 0 for not perform the action.
	/// - error: the error information if return false
	/// Return:
	/// - Return false when fail the performn the interaction
	bool Occupant::performInteraction(int numberofTimeStep, Behavior::SystemType systemType, 
		Behavior::InteractionType interactionType, int spaceSystemControlType, 
		vector<Behavior::EventType> eventTypes, bool isSpaceEmptyNextTimeStep, 
		map<Parameter::ParameterType,double> parameterValues, double& result, 
		string& error, double& controlValue,bool isSpecialBehavior){
		/// By default, 
		result = 0;
		/// Check all the behaivor, if any of the behavior retult is 1, then return 1
		for(size_t i = 0; i < m_behaviors.size(); i++) {
			if(m_behaviors[i]->performInteraction(numberofTimeStep, systemType, interactionType, spaceSystemControlType, eventTypes, isSpaceEmptyNextTimeStep, parameterValues, result, error, isSpecialBehavior)){
				if (result == 1) {
					controlValue = m_behaviors[i]->getControlValue();
					writeToLogFile(",,,");
					writeToLogFile(getID());
					writeToLogFile(",Behavior (");
					writeToLogFile(m_behaviors[i]->getID());
					writeToLogFile("),Win\n");
					return true;
				}
			} else {
				error += "Fail to perform interaction simulation at time step (";
				error += numberofTimeStep;
				error += ") for behavior (";
				error += m_behaviors[i]->getID();
				error += ").\n";
				return false;
			}
		}
		
		writeToLogFile(",,,");
		writeToLogFile(getID());
		if(isSpecialBehavior)
			writeToLogFile(",Special Behavior all lost.\n");
		else
			writeToLogFile(",Behavior all lost.\n");

		return true;
	};

	/// Simulate occupancy status of a day
	void Occupant::initOccupantSchedules(int simulationSteps) {
		m_statusSchedule.clear();
		m_locationSchedule.clear();

		for (int i = 0; i < simulationSteps; i++)  {
			m_statusSchedule.push_back(-1);
			m_locationSchedule.push_back(-1);
		}
		m_statusSchedule[0] = m_lastStepStatus;
	}

	/// Calculate meeting percentage based on behavior inputs
	bool  Occupant::initDailyMeetingPercent(int numberOfDays, string error) {
		for (int i = 0; i < numberOfDays; i++) {
			MovementBehavior* bh;
			bool found = false;
			double meetingPercent;
			for(size_t j = 0; j < m_moveBehaviors.size(); j++) {
				int count = m_moveBehaviors[j]->movementNumOnDay(i);
				if (count > 1) {
					error += "Conflict movement behavior input.\n";
					return false;
				} else if (count == 0) {
					continue;
				} else {
					bh = m_moveBehaviors[j];
					found = true;
					break;
				}
			}
			if (found) {
				meetingPercent = bh->getMeetingPercent();
			} else {
				meetingPercent = 0;
			}
			m_dailyMeetingPercentVec.push_back(meetingPercent);
		}
		return true;
	}


	/// Simulate occupancy status of a day
	bool Occupant::simOccupancyStatus(int dayNum, string error) {
		MovementBehavior* bh;
		if (m_moveBehaviors.size() <= 0) {
			return true;
		}
		for(size_t i = 0; i < m_moveBehaviors.size(); i++) {
			int count = m_moveBehaviors[i]->movementNumOnDay(dayNum);
			if (count > 1) {
				error += "Conflict movement behavior input.\n";
				return false;
			} else if (count == 0) {
				return true;
			} else {
				bh = m_moveBehaviors[i];
				break;
			}
		}
		if (!bh->simStatusSchedule(m_statusSchedule)) {
			error += "Fail to simulate occupancy status.\n";
			return false;
		}
		m_lastStepStatus = m_statusSchedule[m_statusSchedule.size() - 1];
		return true;
		
	}

	/// Simulate occupancy location of a day      
	bool Occupant::simOccupantLocation(int dayNum, string error) {
		MovementBehavior* behavior;
		RandomMovementEvent* model;
		bool found = false;
		for (size_t j = 0; j < m_moveBehaviors.size(); j++) {
			if (m_moveBehaviors[j]->movementNumOnDay(dayNum) == 1) {
				behavior = m_moveBehaviors[j];
				found = true;
				break;
			} else if (m_moveBehaviors[j]->movementNumOnDay(dayNum) > 1) {
				error += "Duplicate movement behavior Day ";
				error += dayNum;
				error += "\n";
				return false;
			}
		}
		if (found) {
			model =  behavior->getRandomMoveModel();
			for(size_t i = 1; i < m_statusSchedule.size(); i++) {

				if (m_statusSchedule[i] == 1) {
					if (m_statusSchedule[i-1] == 0) {
						m_locationSchedule[i] = m_ownLocationID;
					} else {
						if (m_locationSchedule[i-1] == -1) {
							cout << "Not init from own office of occupant " << getID() << " on day " << dayNum << " \n";
							m_locationSchedule[i-1] = m_ownLocationID;
						}
						m_locationSchedule[i] = model->updateLocation(m_locationSchedule[i-1], m_ownLocationID);
					}
				}
			}
		} 
		return true;
	}

	void Occupant::setlLastStepStatus(int status) {
		m_lastStepStatus = status;
	}

	int Occupant::getStatusAt(int i) {
		return m_statusSchedule[i];
	}

	int Occupant::getLocationAt(int i) {
		return m_locationSchedule[i];
	}

	void Occupant::setStatusAt(int i, int status) {
		m_statusSchedule[i] = status;
	}

	void Occupant::setLocationAt(int i, int locationId) {
		m_locationSchedule[i] = locationId;
	}

	void Occupant::setOwnLocationId(int id) {
		m_ownLocationID = id;
	}

	int Occupant::getLocationId() {
		return m_ownLocationID;
	}
};

