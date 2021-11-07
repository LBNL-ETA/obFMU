#include <stdlib.h>
#include <random>
#include <math.h>
#include "ObMarkovChainModel.h"


namespace OB {

	bool MarkovChainModel::readObXML(TiXmlElement * element, string& error) {
		for(const TiXmlNode* child = element->FirstChild(); child; child = child->NextSibling()) { 
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child; 
			if (!strcmp(child->Value(), "EarlyOccurTime")) {
				int hour, min;
				if (getTimeInDay(childElement->GetText(), hour, min, error)) {
					m_earliestOccurHour = hour;
					m_earliestOccurMin = min;
				} else {
					error += "Found unexpected time format (";
					error += child->Value();
					error += ") in StatusTransitionEvent element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "TypicalOccurTime")) {
				int hour, min;
				if (getTimeInDay(childElement->GetText(), hour, min, error)) {
					m_typicalOccurHour = hour;
					m_typicalOccurMin = min;
				} else {
					error += "Found unexpected time format (";
					error += child->Value();
					error += ") in StatusTransitionEvent element.\n";
					return false;
				}
			}  else {
				error += "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}
		return true;
	}

	void MarkovChainModel::initEvent(int stepPerHour) { ///TODO CHECK FALSE
		for (int i = 0; i < stepPerHour * 24; i++) {
			m_timeCumuProb.push_back(0);
		}
		int minPerStep = 60 / stepPerHour;
		
		int meanTimeStep = (int)((m_typicalOccurHour  + (double)m_typicalOccurMin / 60) * stepPerHour);
		int earlyTimeStep = (int)((m_earliestOccurHour  + (double)m_earliestOccurMin / 60) * stepPerHour);
		int timeSpan =(int)( meanTimeStep - (m_earliestOccurHour + (double)m_earliestOccurMin / 60) * stepPerHour);
		double prob = 1 / double(timeSpan);
		for (int i = 0; i < earlyTimeStep; i++) {
			m_timeCumuProb[i] = 0;
		}
		for (int i = earlyTimeStep; i < stepPerHour * 24; i++) {
			if (i == 0) {
				m_timeCumuProb[i] = prob;
			} else {
				m_timeCumuProb[i] =  m_timeCumuProb[i - 1] + (1 - m_timeCumuProb[i - 1]) * prob;
			}
		}			
	}

	int MarkovChainModel::getOccurTime() {   
		double rNum = (double)rand() / (double)RAND_MAX;
		for (size_t i = 1; i < m_timeCumuProb.size(); i++) {
			if (rNum > m_timeCumuProb[i - 1] && rNum <= m_timeCumuProb[i]) {
				m_occurTime = i - 1;
				break;
			}
			if (abs(m_timeCumuProb[i] - 1) < 0.00001) {
				m_occurTime = i;
				break;
			}
		}
		return m_occurTime;
	}

};