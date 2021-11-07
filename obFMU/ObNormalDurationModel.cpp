#include <stdlib.h>
#include <random>
#include <math.h>
#include "ObNormalDurationModel.h"


namespace OB {
    NormalDurationModel::NormalDurationModel(void){
        m_duration = 0;
    }


	bool NormalDurationModel::readObXML(TiXmlElement * element, string& error) {
		for(const TiXmlNode* child = element->FirstChild(); child; child = child->NextSibling() ) 
		{ 
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child; 
			if (!strcmp(child->Value(), "TypicalDuration")) {
				double minValue =0;
				if(!getXsDurationInMinutes(childElement->GetText(), minValue)){
					error += "Fail to get Duration.\n";
					return false;
				};
				m_typicalDuration = int(minValue);
			} else if (!strcmp(child->Value(), "MinimumDuration")) {
				double minValue =0;
				if(!getXsDurationInMinutes(childElement->GetText(), minValue)){
					error += "Fail to get Duration.\n";
					return false;
				};
				m_shortDuration = int(minValue);
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
	double phi_duration(double x) {
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


	void NormalDurationModel::initModel(int stepPerHour) {
		m_durationCumuProb.resize(stepPerHour * 24);
		int minPerStep = 60 / stepPerHour;
		int mu = m_typicalDuration;
		int sigma = (m_typicalDuration - m_shortDuration) / 3;
		for (int i = 0; i < stepPerHour * 24; i++) {
			double x = ((i + 1) * minPerStep - mu) / sigma;
			m_durationCumuProb[i] = phi_duration(x);
		}

	}

	int NormalDurationModel::getDuration() {   
		double rNum = (double)rand() / (double)RAND_MAX;
		for (size_t i = 1; i < m_durationCumuProb.size(); i++) {
			if (rNum > m_durationCumuProb[i - 1] && rNum <= m_durationCumuProb[i]) {
				m_duration = i - 1;
				break;
			}
			if (abs(m_durationCumuProb[i] - 1) < 0.00001) {
				m_duration = i;
				break;
			}
		}
		return m_duration;
	}

};
