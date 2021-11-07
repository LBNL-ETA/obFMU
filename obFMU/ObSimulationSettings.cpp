#include "ObSimulationSettings.h"

namespace OB{
	SimulationSettings::SimulationSettings(){
		DAYOFWEEKNAMES["Sunday"] = 0;
		DAYOFWEEKNAMES["Monday"] = 1;
		DAYOFWEEKNAMES["Tuesday"] = 2;
		DAYOFWEEKNAMES["Wednesday"] = 3;
		DAYOFWEEKNAMES["Thursday"] = 4;
		DAYOFWEEKNAMES["Friday"] = 5;
		DAYOFWEEKNAMES["Saturday"] = 6;
		m_simulationStartMonth =0;
		m_simulationStartDay =0;
		m_simulationEndMonth =0;
		m_simulationEndDay =0;
		m_simulationNumberofTimestepsPerHour =0;
		m_dayofWeekForStartDay =0;
		m_isLeapYear = false;
		m_doMovementCalculation = true;
		m_userMovementResultFilename = "";
		m_useEPLaunch = false;
		m_shouldExportCSVResults = true;
	};
	SimulationSettings::~SimulationSettings(){};
	SimulationSettings::SimulationSettings(const SimulationSettings& other):Object(other){
		m_simulationStartMonth = other.m_simulationStartMonth;
		m_simulationStartDay = other.m_simulationStartDay;
		m_simulationEndMonth = other.m_simulationEndMonth;
		m_simulationEndDay = other.m_simulationEndDay;
		m_simulationNumberofTimestepsPerHour = other.m_simulationNumberofTimestepsPerHour;
		m_dayofWeekForStartDay = other.m_dayofWeekForStartDay;
		m_isLeapYear = other.m_isLeapYear;
		m_doMovementCalculation = other.m_doMovementCalculation;
		m_userMovementResultFilename = other.m_userMovementResultFilename;
		m_useEPLaunch = other.m_useEPLaunch;
		m_shouldExportCSVResults = other.m_shouldExportCSVResults;
	};
	SimulationSettings& SimulationSettings::operator= (const SimulationSettings& other){
		if(this != &other){
			Object::operator=(other);
			m_simulationStartMonth = other.m_simulationStartMonth;
			m_simulationStartDay = other.m_simulationStartDay;
			m_simulationEndMonth = other.m_simulationEndMonth;
			m_simulationEndDay = other.m_simulationEndDay;
			m_simulationNumberofTimestepsPerHour = other.m_simulationNumberofTimestepsPerHour;	
			m_dayofWeekForStartDay = other.m_dayofWeekForStartDay;		
			m_isLeapYear = other.m_isLeapYear;
			m_doMovementCalculation = other.m_doMovementCalculation;
			m_userMovementResultFilename = other.m_userMovementResultFilename;
			m_useEPLaunch = other.m_useEPLaunch;
			m_shouldExportCSVResults = other.m_shouldExportCSVResults;
		}
		return *this;
	};	
	
	/// initialize by reading an TiXmlElement
	/// params:
	/// - element: the TiXMLElement
	/// - error: the error information, when return false
	/// Return:
	/// - bool: readObXML successful or not
	bool	SimulationSettings::readObXML(TiXmlElement* element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{
			if(child->Type() != TiXmlNode::ELEMENT)	continue;

			TiXmlElement *childElement = (TiXmlElement*)child;
			if(!strcmp(child->Value(),"StartMonth")) 
			{ 
				m_simulationStartMonth = atoi(childElement->GetText());
			}else if(!strcmp(child->Value(),"StartDay")) { 
				m_simulationStartDay = atoi(childElement->GetText());
			}else if(!strcmp(child->Value(),"EndMonth")) 
			{ 
				m_simulationEndMonth =  atoi(childElement->GetText());
			}else if(!strcmp(child->Value(),"EndDay")) { 
				m_simulationEndDay = atoi(childElement->GetText());
			}else if(!strcmp(child->Value(),"NumberofTimestepsPerHour")) { 
				m_simulationNumberofTimestepsPerHour = atoi(childElement->GetText());
			}else if(!strcmp(child->Value(),"DayofWeekForStartDay")) { 
				m_dayofWeekForStartDay = DAYOFWEEKNAMES[childElement->GetText()];

			}else if(!strcmp(child->Value(),"IsLeapYear")) {
				string text = childElement->GetText();
				if(text =="Yes"){
					m_isLeapYear = true;
				}else if(text =="No")
					m_isLeapYear = false;
				else{
					error+= "Wrong value for IsLeapYear. Should be Yes or No.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"UseEPLaunch")) {
				string text = childElement->GetText();
				if(text =="Yes"){
					m_useEPLaunch = true;
				}else if(text =="No")
					m_useEPLaunch = false;
				else{
					error+= "Wrong value for UseEPLaunch. Should be Yes or No.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"ShouldExportCSVResults")) {
				string text = childElement->GetText();
				if(text =="Yes"){
					m_shouldExportCSVResults = true;
				}else if(text =="No")
					m_shouldExportCSVResults = false;
				else{
					error+= "Wrong value for ShouldExportCSVResults. Should be Yes or No.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"IsDebugMode")) {
				string text = childElement->GetText();
				if(text =="Yes"){
					m_isDebugMode = true;
				}else if(text =="No")
					m_isDebugMode = false;
				else{
					error+= "Wrong value for IsDebugMode. Should be Yes or No.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"DoMovementCalculation")) {
				string text = childElement->GetText();
				if(text =="Yes"){
					m_doMovementCalculation = true;
				}else if(text =="No")
					m_doMovementCalculation = false;
				else{
					error+= "Wrong value for DoMovementCalculation. Should be Yes or No.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"UserMovementResultFilename")) {
				if(childElement->GetText() != NULL){
					m_userMovementResultFilename = childElement->GetText();
				}
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		m_startDayofYear = getDayofYear(m_simulationStartMonth, m_simulationStartDay, m_isLeapYear);
		if(m_startDayofYear ==0){
			error+= "Wrong combination of StartMonth and StartDay.\n";
			return false;
		}

		m_endDayofYear = getDayofYear(m_simulationEndMonth, m_simulationEndDay, m_isLeapYear);
		if(m_endDayofYear ==0){
			error+= "Wrong combination of EndMonth and EndDay.\n";
			return false;
		}

		if(m_startDayofYear > m_endDayofYear){
			error+= "The start day is later than the end day.\n";
			return false;		
		}

		if(!m_doMovementCalculation&&m_userMovementResultFilename==""){
			error+= "Movement calculation is disabled, but the user movement result file is not provided.\n";
			return false;		
		}

		vector<int>	availableNumberofTimesteps;
		availableNumberofTimesteps.push_back(1);
		availableNumberofTimesteps.push_back(2);
		availableNumberofTimesteps.push_back(3);
		availableNumberofTimesteps.push_back(4);
		availableNumberofTimesteps.push_back(5);
		availableNumberofTimesteps.push_back(6);
		availableNumberofTimesteps.push_back(10);
		availableNumberofTimesteps.push_back(12);
		availableNumberofTimesteps.push_back(15);
		availableNumberofTimesteps.push_back(20);
		availableNumberofTimesteps.push_back(30);
		availableNumberofTimesteps.push_back(60);

		bool isNumberofTimestepsPerHourOK= false;
		for(size_t i=0; i < availableNumberofTimesteps.size();i++){
			if(availableNumberofTimesteps[i]==m_simulationNumberofTimestepsPerHour){
				isNumberofTimestepsPerHourOK = true;
			}
		}
		if(!isNumberofTimestepsPerHourOK){
			error += "Wrong NumberofTimestepsPerHour (";
			error += m_simulationNumberofTimestepsPerHour;
			error += "). Except 1,2,3,4,5,6,10,12,15,20,30,or 60.\n";
			return false;
		}
		m_totalNumberofDays = m_endDayofYear - m_startDayofYear + 1;
		m_totalNumberofTimeSteps = m_totalNumberofDays * 24 * m_simulationNumberofTimestepsPerHour + 1;
		return true;
	};

	int     SimulationSettings::getSimulationStartYear() {
		return 0;
	}

	int		SimulationSettings::getTotalNumberofTimeSteps(){
		return m_totalNumberofTimeSteps;
	};
	
	int		SimulationSettings::getNumberofTimestepsPerHour(){
		return m_simulationNumberofTimestepsPerHour;
	};
	int		SimulationSettings::getTotalNumberofDays(){
		return m_totalNumberofDays;
	};
	int		SimulationSettings::getStartDayofYear(){
		return m_startDayofYear;
	};
	
	bool	SimulationSettings::useEPLaunch(){
		return m_useEPLaunch;
	};

	
	bool	SimulationSettings::isDebugMode(){
		return m_isDebugMode;
	};
	int		SimulationSettings::getDayofWeek(int dayIndex){
		int dayofWeek = (m_dayofWeekForStartDay + dayIndex) % 7;
		return dayofWeek;
	};
	bool	SimulationSettings::isLeepYear(){
		return m_isLeapYear;
	};
	int		SimulationSettings::getEndDayofYear(){
		return m_endDayofYear;
	};
	int		SimulationSettings::getTimeStepInMinutes(){
		return 60/m_simulationNumberofTimestepsPerHour;
	};
	bool	SimulationSettings::doMovementCalculation(){
		return m_doMovementCalculation;
	};
	string	SimulationSettings::getUserMovementResultFilename(){
		return m_userMovementResultFilename;
	};

	
	bool    SimulationSettings::shouldExportCSVResults(){
		return m_shouldExportCSVResults;
	};

	void	SimulationSettings::display(){
		std::cout << "SimulationSettings:\n";
		std::cout << "  m_simulationStartMonth:" << m_simulationStartMonth << endl;
		std::cout << "  m_simulationStartDay:" << m_simulationStartDay << endl;
		std::cout << "  m_simulationEndMonth:" << m_simulationEndMonth << endl;
		std::cout << "  m_simulationEndDay:" << m_simulationEndDay << endl;
		std::cout << "  m_simulationNumberofTimestepsPerHour:" << m_simulationNumberofTimestepsPerHour << endl;
		std::cout << "  m_isLeapYear:" << m_isLeapYear << endl;
		std::cout << "  m_doMovementCalculation:" << m_doMovementCalculation << endl;
		std::cout << "  m_userMovementResultFilename:" << m_userMovementResultFilename << endl;
		std::cout << "  m_dayofWeekForStartDay:" << m_dayofWeekForStartDay << endl;
		std::cout << "  m_startDayofYear:" << m_startDayofYear << endl;
		std::cout << "  m_endDayofYear:" << m_endDayofYear << endl;
		std::cout << "  m_totalNumberofTimeSteps:" << m_totalNumberofTimeSteps << endl;
		std::cout << "  m_totalNumberofDays:" << m_totalNumberofDays << endl;
		std::cout << "  m_shouldExportCSVResults:" << m_shouldExportCSVResults << endl;
	};

	
	bool	SimulationSettings::checkSimulationPeriod(string startTime, string endTime, int numberOfStepsPerHour, int& totalNumberOfSteps, string& error){
		vector<string> dateItems = split(startTime,'-');
		if(dateItems.size() < 2){
			error += "The Start Time don't have MM-DD format.\n";
			return false;
		}

		int startMonth = stoi(dateItems[0]);
		int startDay = stoi(dateItems[1]);
		if(startMonth != m_simulationStartMonth || startDay !=  m_simulationStartDay){
			error += "The Start Time (" + startTime + ") is different with the obCoSim ("+ to_string((long long)m_simulationStartMonth)+ "-" +to_string((long long)m_simulationStartDay)+ ").\n";
			return false;
		}

		dateItems = split(endTime,'-');
		if(dateItems.size() < 2){
			error += "The End Time don't have MM-DD format.\n";
			return false;
		}

		int endMonth = stoi(dateItems[0]);
		int endDay = stoi(dateItems[1]);
		if(endMonth != m_simulationEndMonth || endDay !=  m_simulationEndDay){
			error += "The End Time (" + startTime + ") is different with the obCoSim ("+ to_string((long long)m_simulationEndMonth)+ "-" +to_string((long long)m_simulationEndDay)+ ").\n";
			return false;
		}
		
		if(m_simulationNumberofTimestepsPerHour != numberOfStepsPerHour){
			error += "The Number of Steps per Hour (" + to_string((long long)numberOfStepsPerHour) + ") is different with the obCoSim ("+ to_string((long long)m_simulationNumberofTimestepsPerHour)+ ").\n";
			return false;
		}
		totalNumberOfSteps = m_totalNumberofTimeSteps-1;
		return true;
	};
};