#include <stdlib.h>
#include <random>
#include <math.h>
#include "ObStatusTransitionEvent.h"


namespace OB {
	StatusTransitionEvent::StatusTransitionEvent(void) {
		m_type = EVENT_ENTERING;
	}

	StatusTransitionEvent::~StatusTransitionEvent(){};

	StatusTransitionEvent& StatusTransitionEvent::operator= (const StatusTransitionEvent&) {
		return *this;
	};

	
	bool StatusTransitionEvent::readObXML(TiXmlElement * element, string& error) {
		for(const TiXmlNode* child = element->FirstChild(); child; child = child->NextSibling() ) 
		{ 
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child; 
				
			if (!strcmp(child->Value(), "EventType")) {
				string type = childElement->GetText();
				
				if (type == "Arrival") {
					m_type = StatusTransitionEvent::EVENT_ENTERING;
				} else if (type == "Departure") {
					m_type = StatusTransitionEvent::EVENT_LEAVING;
				} else if (type == "ShortTermLeaving") {
					m_type = StatusTransitionEvent::EVENT_SHORTABSENCE;
				} else if (type == "ShortTermVisiting") {
					m_type = StatusTransitionEvent::EVENT_SHORTPRESENCE;
				} 
			} else if (!strcmp(child->Value(), "EventOccurModel")) {
				readEventOccurence(childElement, error);
			} else if (!strcmp(child->Value(), "EventDuration")) {
				readEventDurationInput(childElement, error);

			} else {
				error += "Find unexpected Type: (";
				error += child->Value();
				error += ") in StatusTransitionEvent element.\n";
				return false;

			} 
		}
		return true;
	}

	bool StatusTransitionEvent::readEventOccurence(TiXmlElement * element, string& error) {
		for(const TiXmlNode* child = element->FirstChild(); child; child = child->NextSibling() ) 
		{ 
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child; 
			if (!strcmp(child->Value(), "CustomProbabilityModel")) {          
				m_eventOccurModel = m_fac->createCustomProbabilityModel();
				m_eventOccurModel->readObXML(childElement, error);

			} else if (!strcmp(child->Value(), "MarkovChainModel")) {                  
				m_eventOccurModel = m_fac->createMarkovChainModel();
				m_eventOccurModel->readObXML(childElement, error);

			} else if (!strcmp(child->Value(), "NormalProbabilityModel")) {                  
				m_eventOccurModel = m_fac->createNormalProbabilityModel();
				m_eventOccurModel->readObXML(childElement, error);
			} else {
				error += "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}
		return true;
	}

	bool StatusTransitionEvent::readEventDurationInput(TiXmlElement * element, string& error) {
		for(const TiXmlNode* child = element->FirstChild(); child; child = child->NextSibling() ) 
		{ 
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child; 
			if (!strcmp(child->Value(), "NormalDurationModel")) {
				m_durationModel = new NormalDurationModel();
				m_durationModel->readObXML(childElement, error);
			}
		}
		return true;
	}

	StatusTransitionEvent::EventType StatusTransitionEvent::getEventType() {
		return m_type;
	}

	int StatusTransitionEvent::getDuration() {
		return m_durationModel->getDuration();
	}

	int StatusTransitionEvent::getOccurTime() {
		return m_eventOccurModel->getOccurTime();
	}
	
	/// initialize event models
	void StatusTransitionEvent::initEvent(int stepPerHour) { 
		m_eventOccurModel->initEvent(stepPerHour);
		if (m_type == EVENT_SHORTABSENCE || m_type == EVENT_SHORTPRESENCE) {
			m_durationModel->initModel(stepPerHour);
		}
	}
}