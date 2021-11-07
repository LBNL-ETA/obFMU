#include <stdlib.h>
#include <random>
#include <math.h>
#include "ObCustomProbabilityModel.h"


namespace OB {
	/// initialize by reading an TiXmlElement
	/// params:
	/// - element: the TiXMLElement
	/// - error: the error information, when return false
	/// Return:
	/// - bool: readObXML successful or not
	bool CustomProbabilityModel::readObXML(TiXmlElement * element, string& error) {
		int timeInMin;
		double prob;
		for(const TiXmlNode* child = element->FirstChild(); child; child = child->NextSibling() ) 
		{ 
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child; 
			if (!strcmp(child->Value(), "StatusTransitionProbability")) {
				for(const TiXmlNode* secondChild = childElement->FirstChild(); secondChild; secondChild = secondChild->NextSibling() ) 
				{ 
					if (secondChild->Type() != TiXmlNode::ELEMENT)	continue;
					TiXmlElement *secondChildElement = (TiXmlElement*)secondChild; 
					
					if (!strcmp(secondChild->Value(), "Time")) {
						int hour, min;
						if (getTimeInDay(secondChildElement->GetText(), hour, min, error)) {
							timeInMin = hour * 60 + min;
						} else {
							error += "Found unexpected time format (";
							error += secondChild->Value();
							error += ") in StatusTransitionEvent element.\n";
							return false;
						}

					} else if (!strcmp(secondChild->Value(), "Probability")) {
						prob = atof(secondChildElement->GetText());
						
					} else {
						error += "Find unexpected Type: (";
						error += secondChild->Value();
						error += ") in StatusTransitionEvent element.\n";
						return false;

					} 
				}
				m_timeCumuProbMap.insert(std::pair<int,double>(timeInMin, prob));
			} else {
				error += "Find unexpected Type: (";
				error += child->Value();
				error += ") in StatusTransitionEvent element.\n";
				return false;
			} 
		}
		return true;
	}
	
	/// initialize cumulative probability for a day
	/// params:
	/// - stepPerHour: time step per hour
	void CustomProbabilityModel::initEvent(int stepPerHour) {
		/// Init daily probability vector by minute
		vector<double> minCumuProb;
		for (int i = 0; i < 60 * 24; i++) {
			minCumuProb.push_back(0);
		}
		///interpolation
		typedef std::map<int, double>::iterator it_type;
		int lastMin = 1;
		double lastProb = 0;
		int curMin;
		double curProb;
		for(it_type iterator = m_timeCumuProbMap.begin(); iterator != m_timeCumuProbMap.end(); iterator++) {
			curMin = iterator->first;
			curProb = iterator->second;
			for (int i = lastMin; i < curMin; i++) {
				minCumuProb[i] = minCumuProb[i - 1] + (curProb - lastProb) / (curMin - lastMin);
			}
			lastMin = curMin;
			lastProb = curProb;
		}
		for (int i = lastMin; i < 60 * 24; i++) {
			minCumuProb[i] = 1;
		}
		/// Get probability for each time step
		int minPerStep = 60 / stepPerHour;
		for (int i = 0; i < stepPerHour * 24; i++) {
			m_timeCumuProb.push_back(minCumuProb[i * minPerStep]);
		}
	}

	/// calculate event occur time
	/// return:
	/// - stepPerHour: event occur time
	int CustomProbabilityModel::getOccurTime() {
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