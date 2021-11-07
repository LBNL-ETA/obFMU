#include "ObBuilding.h"

namespace OB {
	Building::Building(Object* parent):Object(parent) {
	};
	Building::~Building() {
		for(size_t i=0; i < m_spaces.size(); i++)
			delete m_spaces[i];
	};
	Building::Building(const Building& other) {
		
	};
	Building& Building::operator= (const Building&) {
		return *this;
	};
	   
	/// initialize by reading an TiXmlElement
	/// params:
	/// - element: the TiXMLElement
	/// - error: the error information, when return false
	/// Return:
	/// - bool: readObXML successful or not
	bool Building::readObXML(TiXmlElement* element, std::string& error) {

		string id;
		if (!getStringAttribute(element, "ID", id, error)){
			error += "Fail to find ID attribute for Building element.\n";
			return false;
		}

		/// Set the ID
		if (!setID(id, error)) {
			error += "Fail to set ID for Building.\n";
			return false;
		}

		bool hasType = false;


		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) { 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;


			TiXmlElement *childElement = (TiXmlElement*)child;
			if (!strcmp(child->Value(),"Address")) {
				m_address = childElement->GetText();
			} else if (!strcmp(child->Value(),"Type")) {
				hasType = true;
				string type = childElement->GetText();
				if (type == "Office")
					m_type = Building_Office;
				else {
					error+= "Find unexpected Building type: (";
					error += type;
					error += ") in Building element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(),"Description")) {
				setDescription(childElement->GetText());
			} else if (!strcmp(child->Value(),"Spaces")) {
				if (!readSpaces(childElement, error)) {
					error+= "Fail to read the Spaces element.\n";
					return false;
				}
			} else {
				error += "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}
		

		if (!hasType) {
			error+= "Fail to find the Type element.\n";
			return false;
		}

		return true;
	};
	
	/// Read the Spaces element
	bool Building::readSpaces(TiXmlElement *element, std::string& error) {
		int spaceId = 0;
		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) {
			if (child->Type() != TiXmlNode::ELEMENT) continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if (!strcmp(child->Value(),"Space")) {
				Space* obj = new Space(this);
				obj->setLocationId(spaceId);
				if (!obj->readObXML(childElement,error)) {
					error += "Fail to read the Space element.\n";
					return false;
				}
				m_spaces.push_back(obj);
			} else {
				error += "Found unexpected child element: (";
				error += child->Value();
				error += ") in Spaces element.\n";
				return false;
			}
			spaceId++;
		}

		return true;
	};

	Space*	Building::getSpace(string ID) {
		for (size_t i = 0; i < m_spaces.size(); i++) {
			/// string s1 = m_spaces[i]->getID();
			///  string s2 = ID;
			/// bool test = m_spaces[i]->getID().compare(ID);
			if(m_spaces[i]->getID().compare(ID) == 0)
				return m_spaces[i];
		}
		return NULL;
	};

	vector<Space*> Building::getSpaces() {
		return m_spaces;	
	};
	
	void Building::setSimulationNumberofTimestepsPerHour(int value) {
		for (size_t i=0; i < m_spaces.size(); i++) {
			m_spaces[i]->setSimulationNumberofTimestepsPerHour(value);
		}
	};








};