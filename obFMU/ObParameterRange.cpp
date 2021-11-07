#include "ObParameterRange.h"

namespace OB{
	ParameterRange::ParameterRange(Object* parent):Object(parent){
		m_minValue = NULL;
		m_maxValue = NULL;
	};
	ParameterRange::~ParameterRange(){
		if(m_minValue != NULL)
			delete m_minValue;

		if(m_maxValue != NULL)
			delete m_maxValue;
	};
	ParameterRange::ParameterRange(const ParameterRange& other):Object(other){
		m_parameterID = other.m_parameterID;
		m_parameter = other.m_parameter;
		m_minValue = NULL;
		m_maxValue = NULL;
		if(other.m_minValue != NULL)
			m_minValue = new double(*other.m_minValue);

		if(other.m_maxValue != NULL)
			m_maxValue = new double(*other.m_maxValue);
	};
	ParameterRange& ParameterRange::operator= (const ParameterRange& other){
		if(this != &other){
			Object::operator=(other);
			m_parameterID = other.m_parameterID;
			m_parameter = other.m_parameter;
			m_minValue = NULL;
			m_maxValue = NULL;
			if(other.m_minValue != NULL)
				m_minValue = new double(*other.m_minValue);

			if(other.m_maxValue != NULL)
				m_maxValue = new double(*other.m_maxValue);
		}
		return *this;
	};

	/// initialize by reading an TiXmlElement
	/// params:
	/// - element: the TiXMLElement
	/// - error: the error information, when return false
	/// Return:
	/// - bool: readObXML successful or not
	bool	ParameterRange::readObXML(TiXmlElement* element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if(!strcmp(child->Value(),"ParameterID")){
				m_parameterID = childElement->GetText();
			}else if(!strcmp(child->Value(),"Min")){
				m_minValue = new double(atof(childElement->GetText()));
			}else if(!strcmp(child->Value(),"Max")){
				m_maxValue = new double(atof(childElement->GetText()));
			}else{
				error+= "Found unexpected child element: (";
				error += child->Value();
				error += ") in ParameterRange element.\n";
				return false;
			}
		}
		return true;
	};
	string	ParameterRange::getParameterID(){
		return m_parameterID;
	};
	void	ParameterRange::setParameter(Parameter* obj){
		m_parameter = obj;
	};

	Parameter::ParameterType	ParameterRange::getParameterType(){
		return m_parameter->getParameterType();
	};
	

	/// Check whether the value is within the limit or not.
	/// return true if the value is within the min and max values;
	/// return false if the value is outside the range.
	bool	ParameterRange::checkParameterValue(double value){
		if(m_minValue != NULL){
			if(value < *m_minValue)
				return false;
		}		
		
		if(m_maxValue != NULL){
			if(value > *m_maxValue)
				return false;
		}

		return true;
	};
};