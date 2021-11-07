#include "ObObject.h"

namespace OB{
	vector<string> Object::EXISTING_ID_LIST = vector<string>();
	Object::Object(Object* parent){
		m_parent = parent;
		m_logFile = NULL;
	};
	Object::~Object(){};
	Object::Object(const Object& other){
		m_ID = other.m_ID;
		m_description = other.m_description;
	};
	Object& Object::operator= (const Object& other){
		if(this != &other){
			m_ID = other.m_ID;
			m_description = other.m_description;		
		}
		return *this;
	};

	bool	Object::isUniqueID(string ID, string& error){
		if(ID.length() ==0 ){
			error += "The ID is empty.\n";
			return false;
		}

		std::transform(ID.begin(), ID.end(), ID.begin(), ::tolower);
		for(size_t i =0; i < EXISTING_ID_LIST.size(); i++){
			if(EXISTING_ID_LIST[i].compare(ID) ==0){
				error += "The ID (";
				error += ID;
				error += ")is already used by other object.\n"; 
				return false;
			}
		}
		EXISTING_ID_LIST.push_back(ID);
		return true;
	};

	bool	Object::setID(string ID, string& error){
		/// TODO: make sure the ID is unique
		if(isUniqueID(ID, error)){
			m_ID = ID;
			return true;
		}else{
			return false;
		}

	};

	
	void	Object::setLogFile(FILE* logFile){
		m_logFile = logFile;
	};
	void	Object::writeToLogFile(string info){
		if(m_logFile != NULL){
			fprintf(m_logFile, "%s",info.c_str());
		}
	};

	void	Object::setDescription(string description){
		m_description = description;
	};

	string	Object::getID(){
		return m_ID;
	};
	string	Object::getDescription(){
		return m_description;
	};

	bool	Object::getStringAttribute(TiXmlElement* element, string attributeName, string& result, string& error, bool isOptional){
		if(element == NULL){
			error += "The element is null.\n";
			return false;
		}

		if(element->Attribute(attributeName.c_str()) == NULL){
			if(isOptional)
				return true;
			else{
				error += "Can't find attribute  element is null.\n";
				return false;
			}
		}
		result = element->Attribute(attributeName.c_str());
		return true;
	};

	

	/// Calculate the day of year. Jan 1st = 1
	/// Assume common year (365 for Dec 31);
	int		Object::getDayofYear(int month, int day, bool isLeapYear){
		/// Out of range
		if(month > 12 || month <= 0 || day <=0 || day >31){
			return 0;
		}

		int dayOfYear = 0;
		vector<int> daysPerMonth;
		daysPerMonth.push_back(31); /// 1
		if(isLeapYear)
			daysPerMonth.push_back(29);/// 2
		else
			daysPerMonth.push_back(28);/// 2

		daysPerMonth.push_back(31);/// 3
		daysPerMonth.push_back(30);/// 4
		daysPerMonth.push_back(31);/// 5
		daysPerMonth.push_back(30);/// 6
		daysPerMonth.push_back(31);/// 7
		daysPerMonth.push_back(31);/// 8
		daysPerMonth.push_back(30);/// 9
		daysPerMonth.push_back(31);/// 10
		daysPerMonth.push_back(30);/// 11
		daysPerMonth.push_back(31);/// 12

		/// Out of range
		if(day > daysPerMonth[month-1])
			return 0;

		for(int i=0; i < month-1; i++){
			dayOfYear += daysPerMonth[i];
		}
		dayOfYear += day;
		return dayOfYear;
	};
};