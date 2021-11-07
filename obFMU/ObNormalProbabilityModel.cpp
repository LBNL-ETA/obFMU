#include <stdlib.h>
#include <random>
#include <math.h>
#include "ObNormalProbabilityModel.h"


namespace OB {

	
	bool getTimeInDay(string time, int& hour, int& min,  string& error) {                        
		int result = -1;
		int colon = time.find(":");
		if (colon >= 0) {
			hour = stoi(time.substr(0, colon));
			string minute = time.substr(colon + 1, time.length());
			int colon_2 = minute.find(":");
			if (colon_2 >= 0) {
				min = stoi(minute.substr(0, colon_2));
			}
		}
		if (hour >= 0 && hour <= 23 && min >= 0 && min <= 59) {
			return true;
		} else {
			return false;
		}
	};

	bool NormalProbabilityModel::readObXML(TiXmlElement * element, string& error) {
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

	//The function ¦µ(x) is the cumulative density function (CDF) of a standard normal (Gaussian) random variable. 
	double phi(double x) {
		// constants
		double a1 =  0.254829592;
		double a2 = -0.284496736;
		double a3 =  1.421413741;
		double a4 = -1.453152027;
		double a5 =  1.061405429;
		double p  =  0.3275911;

		// Save the sign of x
		int sign = 1;
		if (x < 0)
			sign = -1;
		x = fabs(x)/sqrt(2.0);

		// A&S formula 7.1.26
		double t = 1.0/(1.0 + p*x);
		double y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);

		return 0.5*(1.0 + sign*y);
	}


	void NormalProbabilityModel::initEvent(int stepPerHour) { ///TODO CHECK FALSE
		for (int i = 0; i < stepPerHour * 24; i++) {
			m_timeCumuProb.push_back(0);
		}
		int minPerStep = 60 / stepPerHour;
		int mu = m_typicalOccurHour * 60 + m_typicalOccurMin;
		int sigma = (m_typicalOccurHour * 60 + m_typicalOccurMin - m_earliestOccurHour * 60 + m_earliestOccurMin)/3;
		for (int i = 0; i < stepPerHour * 24; i++) {
			double x = double((i + 1) * minPerStep - mu) / sigma;
			m_timeCumuProb[i] = phi(x);
		}			
	}

	int NormalProbabilityModel::getOccurTime() {
		double rNum = (double)rand() / (double)RAND_MAX;
		for (size_t i = 1; i < m_timeCumuProb.size(); i++) {
			if (rNum > m_timeCumuProb[i - 1] && rNum <= m_timeCumuProb[i]) {
				m_occurTime = i;
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