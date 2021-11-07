#include "ObParameter.h"

namespace OB{
	Parameter::Parameter(Object* parent):Object(parent){
	};
	Parameter::~Parameter(){
	};
	Parameter::Parameter(const Parameter& other):Object(other){
		m_type = other.m_type;
		m_name = other.m_name;	
	};
	Parameter& Parameter::operator= (const Parameter& other){
		if(this != &other){
			Object::operator=(other);
			m_type = other.m_type;
			m_name = other.m_name;		
		}
		return *this;
	};

	/// initialize by reading an TiXmlElement
	/// params:
	/// - element: the TiXMLElement
	/// - error: the error information, when return false
	/// Return:
	/// - bool: readObXML successful or not
	bool	Parameter::readObXML(TiXmlElement* element, std::string& error)
	{
		string id;
		if(!getStringAttribute(element, "ID", id, error)){
			error += "Fail to find ID attribute for Parameter element.\n";
			return false;
		}
		
		if(!getStringAttribute(element, "Name", m_name, error, true)){
			error += "Fail to find Name attribute for Parameter element.\n";
			return false;
		}

		/// Set the ID
		if(!setID(id, error)){
			error += "Fail to set ID for Parameter.\n";
			return false;
		}

		bool hasType = false;
		bool hasUnit = false;

		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if(!strcmp(child->Value(),"Description")){
				setDescription(childElement->GetText());
			}else if(!strcmp(child->Value(),"Unit")){
				m_unit = atoi(childElement->GetText());
			}else if(!strcmp(child->Value(),"Type")){
				hasType = true;
				string type = childElement->GetText();
				if(type == "RoomAirTemperature")
					m_type = Parameter_RoomTemperature;
				else if(type == "RoomCO2Concentration")
					m_type = Parameter_RoomCO2Concentration;
				else if(type == "RoomWorkPlaneDaylightIlluminance")
					m_type = Parameter_RoomWorkPlaneDaylightIlluminance;
				else if(type == "RoomLightsPowerDensity")
					m_type = Parameter_RoomLightsPowerDensity;
				else if(type == "OutdoorDryBulbTemperature")
					m_type = Parameter_OutdoorDryBulbTemperature;
				else if(type == "OutdoorRainIndicator")
					m_type = Parameter_OutdoorRainIndicator;
				else{
					error+= "Find unexpected parameter type: (";
					error += type;
					error += ") in Space element.\n";
					return false;
				}
			}else{
				error+= "Found unexpected child element: (";
				error += child->Value();
				error += ") in Parameter element.\n";
				return false;
			}
		}
		return true;
	};
	
	Parameter::ParameterType	Parameter::getParameterType(){
		return m_type;
	};
};