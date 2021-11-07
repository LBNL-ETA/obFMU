#include "ObBehavior.h"
#include "ObSpace.h"
#include <math.h>

namespace OB{
	Behavior::Behavior(Object* parent):Object(parent){
		m_systemType = System_NONE;
		m_availableSystemType = -1;
		m_actionType = Action_None;
		m_interactionType = Interaction_None;
		m_parameter1 = NULL;
		m_parameter2 = NULL;
		m_parameter3 = NULL;
		m_isSpecialBehavior = false;
	};
	Behavior::~Behavior(){};
	Behavior::Behavior(const Behavior& other){};
	Behavior& Behavior::operator= (const Behavior&){
		return *this;
	};
	   
	/// initialize by reading an TiXmlElement
	/// params:
	/// - element: the TiXMLElement
	/// - error: the error information, when return false
	/// Return:
	/// - bool: readObXML successful or not
	bool Behavior::readObXML(TiXmlElement* element, std::string& error){

		string id;
		if (!getStringAttribute(element, "ID", id, error)) {
			error += "Fail to find ID attribute for Space element.\n";
			return false;
		}

		/// Set the ID
		if (!setID(id, error)) {
			error += "Fail to set ID for Space.\n";
			return false;
		}

		bool hasActions = false;
		
		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) { 
			if (child->Type() != TiXmlNode::ELEMENT) continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if (!strcmp(child->Value(), "Description")) {
				setDescription(childElement->GetText());
			} else if (!strcmp(child->Value(), "Drivers")) {
				if (!readDrivers(childElement, error)) {
					error+= "Fail to read the Drivers element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "Needs")) {
				if (!readNeeds(childElement, error)) {
					error+= "Fail to read the Needs element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "Actions")) {
				hasActions = true;
				if(!readActions(childElement, error)){
					error+= "Fail to read the Actions element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "Systems")) {
				if (!readSystems(childElement, error)) {
					error+= "Fail to read the Systems element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "IsSpecialBehavior")) {
				string type = childElement->GetText();
				if(type == "yes" || type == "true"){
					m_isSpecialBehavior = true;
				}else{
					m_isSpecialBehavior = false;
				} 
			} else {
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		if(!hasActions){
			error+= "Fail to find the Actions element.\n";
			return false;
		} 
		return true;
	};

	/// connect all the reference IDs with their objects.
	/// Return
	/// - return false when can't find objects with the given ID.
	bool	Behavior::connectIDswithObjects(string& error){
		/// connect parameterlimts with enviorment parameter
		for(size_t i=0; i < m_needsParameterRanges.size(); i++){
			string id = m_needsParameterRanges[i]->getParameterID();
			Parameter* obj = getParameter(id);
			if(obj == NULL){
				error += "Can't find Environment Parameter with ID (";
				error += id;
				error += ").\n";
				return false;
			}else{
				m_needsParameterRanges[i]->setParameter(obj);
			}
		}

		m_parameter1 = getParameter(m_parameter1ID);
		m_parameter2 = getParameter(m_parameter2ID);
		m_parameter3 = getParameter(m_parameter3ID);

		/// connect formula parameter with enviorment parameter
		if(m_actionType == Action_Interaction){
			switch(m_formulaType){
			case Formula_Linear1D:
			case Formula_Quadratic1D:
			case Formula_Logit1D:
			case Formula_Weibull1D:
			case Formula_Logit1DQuadratic:
				if(m_parameter1 == NULL){
					error += "Can't find Environment Parameter1 with ID (";
					error += m_parameter1ID;
					error += ") for the Formula.\n";
					return false;
				}
				break;
			case Formula_Linear2D:
			case Formula_Logit2D:
				if(m_parameter1 == NULL){
					error += "Can't find Environment Parameter1 with ID (";
					error += m_parameter1ID;
					error += ") for the Formula.\n";
					return false;
				}
				
				if(m_parameter2 == NULL){
					error += "Can't find Environment Parameter2 with ID (";
					error += m_parameter2ID;
					error += ") for the Formula.\n";
					return false;
				}
				break;
			case Formula_Linear3D:
			case Formula_Logit3D:
				if(m_parameter1 == NULL){
					error += "Can't find Environment Parameter1 with ID (";
					error += m_parameter1ID;
					error += ") for the Formula.\n";
					return false;
				}
				
				if(m_parameter2 == NULL){
					error += "Can't find Environment Parameter2 with ID (";
					error += m_parameter2ID;
					error += ") for the Formula.\n";
					return false;
				}
				
				if(m_parameter3 == NULL){
					error += "Can't find Environment Parameter3 with ID (";
					error += m_parameter3ID;
					error += ") for the Formula.\n";
					return false;
				}
				break;
			}
		}

		return true;
	};
	
	Parameter* Behavior::getParameter(string ID){
		for(size_t i=0; i < m_environmentParameters.size(); i++){
			if(m_environmentParameters[i]->getID() == ID)
				return m_environmentParameters[i];
		}
		return NULL;
	};

	/// Perform interaction
	/// Params:
	/// - numberofTimeStep: the number of current time step
	/// - systemType: which system, window, HVAC, or lights
	/// - interactionType: turn on or turn off
	/// - results: the output; 1 for perform the action (turn on or turn off), 0 for not perform the action.
	/// - error: the error information if return false
	/// Return:
	/// - Return false when fail the performn the interaction
	bool	Behavior::performInteraction(int numberofTimeStep, Behavior::SystemType systemType, Behavior::InteractionType interactionType, int spaceSystemControlType, vector<Behavior::EventType> eventTypes, bool isSpaceEmptyNextTimeStep, map<Parameter::ParameterType,double> parameterValues, double& result, string& error, bool isSpecialBehavior){
		result = 0;
		
		writeToLogFile(",,,,");
		writeToLogFile(getID());
		writeToLogFile(",");
		writeToLogFile(getDescription());

		/// Check the special event type
		if(isSpecialBehavior != m_isSpecialBehavior){
			writeToLogFile(",different special behavior type\n");
			return true;
		}

		/// Check the Systems
		if(systemType != m_systemType){
			writeToLogFile(",different system type\n");
			return true;
		}		
		
		if( spaceSystemControlType != m_availableSystemType){
			writeToLogFile(",not available system control type\n");
			return true;
		}

		/// Check the action type
		if(m_actionType != Action_Interaction || m_interactionType != interactionType){
			writeToLogFile(",different interaction type.\n");
			return true;
		}
		
		if(m_eventTypes.size() > 0){
			bool eventOK = false;
			/// Check the Event Type
			for(size_t i =0; i < m_eventTypes.size(); i++){
				for(size_t j=0; j < eventTypes.size(); j++){
					if(m_eventTypes[i] == eventTypes[j]){
						eventOK = true;
						break;
					}
				}
			}
			if(!eventOK){
				writeToLogFile(",different event type.\n");
				return true;
			}
		}

		/// Check the other constrain types
		for(size_t i=0; i< m_otherConstraintTypes.size(); i++){
			OtherConstraintType constraint = m_otherConstraintTypes[i];
	
			if(constraint == OtherConstraint_NoOccupantsInRoom){
				/// The Behavior will be triggled only when the space is empty next time step
				if(!isSpaceEmptyNextTimeStep){
					writeToLogFile(",fail no occupants next time step constraint.\n");
					return true;
				}
			}
		}

		/// Check the needs
		/// if the parameter is within the needs range, just return true.
		for(size_t i=0; i< m_needsParameterRanges.size(); i++){
			Parameter::ParameterType type = m_needsParameterRanges[i]->getParameterType();
			double value = parameterValues[type];
			if(m_needsParameterRanges[i]->checkParameterValue(value)){
				writeToLogFile(",fail parameter limits constraint.\n");
				return true;
			}
		}
		
		/// Check the Drives
		/// If the behavior is outside the drive range, result =0; return true
		/// Check time
		if(m_seasons.size() > 0){
			bool ok = false;
			for(size_t i=0; i < m_seasons.size(); i++){
				if(m_seasons[i]->checkTimeStepFlag(numberofTimeStep)){
					ok = true;
					break;
				}
			}
			if(!ok){
				writeToLogFile(",fail season constraint.\n");
				return true;
			}
		}
		
		if(m_timeofDays.size() > 0){
			bool ok = false;
			for(size_t i=0; i < m_timeofDays.size(); i++){
				if(m_timeofDays[i]->checkTimeStepFlag(numberofTimeStep)){
					ok = true;
					break;
				}
			}
			if(!ok){
				writeToLogFile(",fail time of days constraint.\n");
				return true;
			}
		}
		
		if(m_dayofWeeks.size() > 0){
			bool ok = false;
			for(size_t i=0; i < m_dayofWeeks.size(); i++){
				if(m_dayofWeeks[i]->checkTimeStepFlag(numberofTimeStep)){
					ok = true;
					break;
				}
			}
			if(!ok){
				writeToLogFile(",fail day of week constraint.\n");
				return true;
			}
		}

		/// Now we need to throw the dice
		double parameter1Value =0;
		double parameter2Value =0;
		double parameter3Value =0;
		
		if(m_parameter1 != NULL)
			parameter1Value = parameterValues[m_parameter1->getParameterType()];

		if(m_parameter2 != NULL)
			parameter2Value = parameterValues[m_parameter2->getParameterType()];

		if(m_parameter3 != NULL)
			parameter3Value = parameterValues[m_parameter3->getParameterType()];

		long double probabilityValue = 1; 
		long double logValue;
		switch(m_formulaType){
			case Formula_ConstantValue:
				probabilityValue = m_coefficientA;
				break;
			case Formula_Linear1D:
				probabilityValue = m_coefficientA * parameter1Value + m_coefficientB;
				break;
			case Formula_Linear2D:
				probabilityValue = m_coefficientA * parameter1Value + m_coefficientB * parameter2Value + m_coefficientC;
				break;
			case Formula_Linear3D:
				probabilityValue = m_coefficientA * parameter1Value + m_coefficientB * parameter2Value + m_coefficientC * parameter3Value + m_coefficientD;
				break;
			case Formula_Quadratic1D:
				probabilityValue = m_coefficientA * parameter1Value * parameter1Value + m_coefficientB * parameter1Value + m_coefficientC;
				break;
			case Formula_Logit1D:
				logValue =  m_coefficientA * parameter1Value + m_coefficientB;
				probabilityValue = 1.0/(1.0+exp(-logValue));
				break;
			case Formula_Logit2D:
				logValue = m_coefficientA * parameter1Value + m_coefficientB * parameter2Value + m_coefficientC;
				probabilityValue = 1.0/(1.0+exp(-logValue));
				break;
			case Formula_Logit3D:
				logValue =  m_coefficientA * parameter1Value + m_coefficientB * parameter2Value + m_coefficientC * parameter3Value + m_coefficientD;
				probabilityValue = 1.0/(1.0+exp(-logValue));
				break;
			case Formula_Weibull1D:{
				double xu_l = (parameter1Value-m_coefficientA)/m_coefficientB;
				if(xu_l <=0)
					probabilityValue =0;
				else
					probabilityValue = 1- exp(-pow(xu_l,m_coefficientC)*m_dt);
				break;
				}
			case Formula_Logit1DQuadratic:{
				double logP1 = -10;
				if(parameter1Value > 0)
					logP1 = log10(parameter1Value);

				probabilityValue = m_coefficientA + m_coefficientC / (1 + exp(-m_coefficientB*(logP1-m_coefficientD)));
				break;
			}
		};

		/// dice
		long double random_variable = (double)std::rand()/(double)RAND_MAX;

		writeToLogFile(",throw dice,random value,");
		writeToLogFile(std::to_string(random_variable));
		writeToLogFile(",probability value,");
		writeToLogFile(std::to_string(probabilityValue));

		if(random_variable <= probabilityValue){
			result = 1;
			writeToLogFile(",Win\n");
		}else{
			writeToLogFile(",Lost\n");
		}
		

		return true;
	};
		
	void	Behavior::setTimeStep(int dt){
		m_dt = dt;
	};

	vector<Season::SeasonType>		Behavior::getSeasonTypes(){
		return m_seasonTypes;
	};
	void	Behavior::addSeasion(Season* obj){
		m_seasons.push_back(obj);
	};
	
	vector<TimeofDay::TimeofDayType>		Behavior::getTimeofDayTypes(){
		return m_timeofDayTypes;
	};
	void	Behavior::addTimeofDay(TimeofDay* obj){
		m_timeofDays.push_back(obj);
	};
	
	int				Behavior::getAvailableSystemType(){
		return m_availableSystemType;
	};
	vector<string>	Behavior::getAvailableSystemIDs(){
		return m_availableSystemIDs;
	};
	void			Behavior::addAvailableSystem(Object* obj){
		m_availableSystems.push_back(obj);
	};

	Behavior::SystemType	Behavior::getSystemType(){
		return m_systemType;
	};
	
	vector<DayofWeek::DayofWeekType>		Behavior::getDayofWeekTypes(){
		return m_dayofWeekTypes;
	};
	void									Behavior::addDayofWeek(DayofWeek* obj){
		m_dayofWeeks.push_back(obj);
	};

	/// Read the Drivers element
	bool	Behavior::readDrivers(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if(!strcmp(child->Value(),"Time")){
				if(!readTime(childElement, error)){
					error+= "Fail to read the Time element.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"Environment")){
				if(!readEnvironment(childElement, error)){
					error+= "Fail to read the Environment element.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"EventType")){
				string type = childElement->GetText();
				if(type == "LeavingRoom"){
					m_eventTypes.push_back(Event_LeavingRoom);
				}else if(type == "EnteringRoom"){
					m_eventTypes.push_back(Event_EnteringRoom);
				}else if(type == "StayingInRoom"){
					m_eventTypes.push_back(Event_StayingInRoom);
				}else if(type == "LeavingRoomMoreThan1Hour"){
					m_eventTypes.push_back(Event_LeavingRoomMoreThan1Hour);
				}else if(type == "LeavingRoomMoreThan6Hours"){
					m_eventTypes.push_back(Event_LeavingRoomMoreThan6Hours);
				}else{
					error+= "Find unexpected EventType: (";
					error += type;
					error += ") in Drivers element.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"OtherConstraint")){
				string type = childElement->GetText();
				if(type == "NoOccupantsInRoom"){
					m_otherConstraintTypes.push_back(OtherConstraint_NoOccupantsInRoom);
				}else{
					error+= "Find unexpected OtherConstraint: (";
					error += type;
					error += ") in Drivers element.\n";
					return false;
				}
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		return true;
	};

	/// Read the Time element
	bool	Behavior::readTime(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if(!strcmp(child->Value(),"TimeofDay")){
				string type = childElement->GetText();
				if(type == "Morning"){
					m_timeofDayTypes.push_back(TimeofDay::TimeofDay_Morning);
				}else if(type == "Noon"){
					m_timeofDayTypes.push_back(TimeofDay::TimeofDay_Noon);
				}else if(type == "Afternoon"){
					m_timeofDayTypes.push_back(TimeofDay::TimeofDay_Afternoon);
				}else if(type == "Evening"){
					m_timeofDayTypes.push_back(TimeofDay::TimeofDay_Evening);
				}else if(type == "Day"){
					m_timeofDayTypes.push_back(TimeofDay::TimeofDay_Day);
				}else if(type == "Night"){
					m_timeofDayTypes.push_back(TimeofDay::TimeofDay_Night);
				}else if(type == "All"){
					m_timeofDayTypes.push_back(TimeofDay::TimeofDay_All);
				}else{
					error+= "Find unexpected TimeofDay: (";
					error += type;
					error += ") in Time element.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"DayofWeek")){
				string type = childElement->GetText();
				if(type == "Monday"){
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Monday);
				}else if(type == "Tuesday"){
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Tuesday);
				}else if(type == "Wednesday"){
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Wednesday);
				}else if(type == "Thursday"){
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Thursday);
				}else if(type == "Friday"){
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Friday);
				}else if(type == "Saturday"){
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Saturday);
				}else if(type == "Sunday"){
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Sunday);
				}else if(type == "Weekdays"){
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Weekdays);
				}else if(type == "Weekends"){
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Weekends);
				}else if(type == "Holiday"){
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_Holiday);
				}else if(type == "All"){
					m_dayofWeekTypes.push_back(DayofWeek::DayofWeek_All);
				}else{
					error+= "Find unexpected DayofWeek: (";
					error += type;
					error += ") in Time element.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"SeasonType")){
				string type = childElement->GetText();
				if(type == "Spring"){
					m_seasonTypes.push_back(Season::Season_Spring);
				}else if(type == "Summer"){
					m_seasonTypes.push_back(Season::Season_Summer);
				}else if(type == "Fall"){
					m_seasonTypes.push_back(Season::Season_Fall);
				}else if(type == "Winter"){
					m_seasonTypes.push_back(Season::Season_Winter);
				}else if(type == "All"){
					m_seasonTypes.push_back(Season::Season_All);
				}else{
					error+= "Find unexpected SeasonType: (";
					error += type;
					error += ") in Time element.\n";
					return false;
				}
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		return true;
	};

	/// Read the Environment element
	bool	Behavior::readEnvironment(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if(!strcmp(child->Value(),"Parameter")){
				Parameter* obj = new Parameter(this);
				if(!obj->readObXML(childElement,error)){
					error+= "Fail to read the Parameter element.\n";
					return false;
				}
				m_environmentParameters.push_back(obj);
			}else{
				error+= "Found unexpected child element: (";
				error += child->Value();
				error += ") in Environment element.\n";
				return false;
			}
		}
		return true;
	};

	/// Read the Needs element
	bool	Behavior::readNeeds(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if(!strcmp(child->Value(),"Physical")){
				if(!readPhysical(childElement, error)){
					error+= "Fail to read the Physical element.\n";
					return false;
				}
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		return true;
	};
	
	/// Read the Physical element
	bool	Behavior::readPhysical(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if(!strcmp(child->Value(),"Thermal")){
				if(!readThermal(childElement, error)){
					error+= "Fail to read the Thermal element.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"Visual")){
				if(!readParameterRanges(childElement, error)){
					error+= "Fail to read the Visual element.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"IAQ")){
				if(!readParameterRanges(childElement, error)){
					error+= "Fail to read the IAQ element.\n";
					return false;
				}
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		return true;
	
	};
	
	/// Read the Thermal element
	bool	Behavior::readThermal(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if(!strcmp(child->Value(),"OtherComfortEnvelope")){
				if(!readParameterRanges(childElement, error)){
					error+= "Fail to read the OtherComfortEnvelope element.\n";
					return false;
				}
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		return true;
	
	};
	
	/// Read the OtherComfortEnvelope element
	bool	Behavior::readParameterRanges(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if(!strcmp(child->Value(),"ParameterRange")){
				ParameterRange* obj = new ParameterRange(this);
				if(!obj->readObXML(childElement, error)){
					error+= "Fail to read the ParameterRange element.\n";
					return false;
				}
				m_needsParameterRanges.push_back(obj);
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		return true;
	
	}; 

	/// Read the Actions element
	bool	Behavior::readActions(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if(!strcmp(child->Value(),"Interaction")){
				if(m_actionType == Action_None)
					m_actionType = Action_Interaction;
				else{
					error+= "Multiple actions defined in the Actions element.\n";
					return false;
				}

				if(!readInteraction(childElement, error)){
					error+= "Fail to read the Interaction element.\n";
					return false;
				}
			} else {
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		return true;
	};
	
	/// Read the Interaction element
	bool	Behavior::readInteraction(TiXmlElement *element, std::string& error){
		bool hasType = false;
		bool hasFormula = false;
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if(!strcmp(child->Value(),"Type")){
				hasType = true;
				string type = childElement->GetText();
				if(type == "TurnOn")
					m_interactionType = Interaction_TurnOn;
				else if(type == "TurnOff")
					m_interactionType = Interaction_TurnOff;
				else if(type == "SetToControlValue")
					m_interactionType = Interaction_SetToControlValue;
				else{
					error+= "Find unexpected Type: (";
					error += type;
					error += ") in Interaction element.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"Formula")){
				hasFormula = true;
				if(!readFormula(childElement, error)){
					error+= "Fail to read the Formula element.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"ControlValue")){
				m_interactionControlValue = atof(childElement->GetText());
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}
		if (!hasType) {
			error+= "Fail to find the Type element.\n";
			return false;
		}
		if (!hasFormula) {
			error+= "Fail to find the Formula element.\n";
			return false;
		}
		return true;
	};


	/// Read the Formula element
	bool	Behavior::readFormula(TiXmlElement *element, std::string& error){
		bool hasFormula = false;
		vector<string>	formulaNames;
		formulaNames.push_back("ConstantValue");
		formulaNames.push_back("Linear1D");
		formulaNames.push_back("Linear2D");
		formulaNames.push_back("Linear3D");
		formulaNames.push_back("Quadratic1D");
		formulaNames.push_back("Logit1D");
		formulaNames.push_back("Logit2D");
		formulaNames.push_back("Logit3D");
		formulaNames.push_back("Weibull1D");
		formulaNames.push_back("Logit1DQuadratic");
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			bool fouldElement = false;
			for(size_t i=0; i < formulaNames.size() ;i++){
				if(!strcmp(child->Value(),formulaNames[i].c_str())){
					if(!hasFormula)
						hasFormula = true;
					else{
						error += "Multiple formulas in the Formula element.\n";
						return false;
					}

					if(!readFormulaChild(childElement, error,(FormulaType)i)){
						error+= "Fail to read the child of Formula element.\n";
						return false;
					}
					fouldElement = true;
					break;
				}
			}
			
			if(!fouldElement){
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}
		if( !hasFormula)
		{
			error+= "Fail to find the child of Formula element.\n";
			return false;
		}

		return true;
	};
	
		/// Read the child of Formula element
	bool	Behavior::readFormulaChild(TiXmlElement *element, std::string& error, FormulaType type){
		m_formulaType = type;
		vector<string>	coefficients;
		vector<string>	parameterIDs;
		switch(type){
		case Formula_ConstantValue:
			coefficients.push_back("CoefficientA");
			break;
		case Formula_Linear1D:
		case Formula_Logit1D:
			coefficients.push_back("CoefficientA");
			coefficients.push_back("CoefficientB");
			parameterIDs.push_back("Parameter1ID");
			break;
		case Formula_Linear2D:
		case Formula_Logit2D:
			coefficients.push_back("CoefficientA");
			coefficients.push_back("CoefficientB");
			coefficients.push_back("CoefficientC");
			parameterIDs.push_back("Parameter1ID");
			parameterIDs.push_back("Parameter2ID");
			break;
		case Formula_Linear3D:
		case Formula_Logit3D:
			coefficients.push_back("CoefficientA");
			coefficients.push_back("CoefficientB");
			coefficients.push_back("CoefficientC");
			coefficients.push_back("CoefficientD");
			parameterIDs.push_back("Parameter1ID");
			parameterIDs.push_back("Parameter2ID");
			parameterIDs.push_back("Parameter3ID");
			break;
		case Formula_Quadratic1D:
		case Formula_Weibull1D:
			coefficients.push_back("CoefficientA");
			coefficients.push_back("CoefficientB");
			coefficients.push_back("CoefficientC");
			parameterIDs.push_back("Parameter1ID");
			break;
		case Formula_Logit1DQuadratic:
			coefficients.push_back("CoefficientA");
			coefficients.push_back("CoefficientB");
			coefficients.push_back("CoefficientC");
			coefficients.push_back("CoefficientD");
			parameterIDs.push_back("Parameter1ID");
			break;
		}
		vector<double>	coefficientValues;
		vector<string>	parameterIDValues;
		vector<bool>	coefficientFlags;
		vector<bool>	parameterIDFlags;
		for(size_t i=0; i < coefficients.size();i++){
			coefficientValues.push_back(0);
			coefficientFlags.push_back(false);
		}

		for(size_t i=0; i < parameterIDs.size();i++){
			parameterIDValues.push_back("");
			parameterIDFlags.push_back(false);
		}

		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			bool fouldElement = false;

			
			if(!strcmp(child->Value(),"Description")){
				fouldElement= true;
				setDescription(childElement->GetText());
			}

			if(!fouldElement){
				for(size_t i=0; i < coefficients.size() ;i++){
					if(!strcmp(child->Value(),coefficients[i].c_str())){
						coefficientValues[i] = atof(childElement->GetText());
						fouldElement = true;
						coefficientFlags[i] = true;
						break;
					}
				}	
			}
			if(!fouldElement){
				for(size_t i=0; i < parameterIDs.size() ;i++){
					if(!strcmp(child->Value(),parameterIDs[i].c_str())){
						parameterIDValues[i] = childElement->GetText();
						fouldElement = true;
						parameterIDFlags[i] = true;
						break;
					}
				}
			}

			
			if(!fouldElement){
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		for(size_t i=0; i < coefficientFlags.size(); i++){
			if(!coefficientFlags[i]){
				error+= "Can't find ";
				error+= coefficients[i];
				error+= " in the formula.\n";
				return false;
			}
		}

		for(size_t i=0; i < parameterIDFlags.size(); i++){
			if(!parameterIDFlags[i]){
				error+= "Can't find ";
				error+= parameterIDs[i];
				error+= " in the formula.\n";
				return false;
			}
		}

		switch(type){
		case Formula_ConstantValue:
			m_coefficientA = coefficientValues[0];
			break;
		case Formula_Logit1D:
		case Formula_Linear1D:
			m_coefficientA = coefficientValues[0];
			m_coefficientB = coefficientValues[1];
			m_parameter1ID = parameterIDValues[0];
			break;
		case Formula_Logit2D:
		case Formula_Linear2D:
			m_coefficientA = coefficientValues[0];
			m_coefficientB = coefficientValues[1];
			m_coefficientC = coefficientValues[2];
			m_parameter1ID = parameterIDValues[0];
			m_parameter2ID = parameterIDValues[1];
			break;
		case Formula_Linear3D:
		case Formula_Logit3D:
			m_coefficientA = coefficientValues[0];
			m_coefficientB = coefficientValues[1];
			m_coefficientC = coefficientValues[2];
			m_coefficientD = coefficientValues[3];
			m_parameter1ID = parameterIDValues[0];
			m_parameter2ID = parameterIDValues[1];
			m_parameter3ID = parameterIDValues[2];
			break;
		case Formula_Quadratic1D:
		case Formula_Weibull1D:
			m_coefficientA = coefficientValues[0];
			m_coefficientB = coefficientValues[1];
			m_coefficientC = coefficientValues[2];
			m_parameter1ID = parameterIDValues[0];
			break;
		case Formula_Logit1DQuadratic:
			m_coefficientA = coefficientValues[0];
			m_coefficientB = coefficientValues[1];
			m_coefficientC = coefficientValues[2];
			m_coefficientD = coefficientValues[3];
			m_parameter1ID = parameterIDValues[0];
			break;
		}

		return true;
	
	};
	/// Read the Systems element
	bool	Behavior::readSystems(TiXmlElement *element, std::string& error){
		int systemTypeCount = 0;

		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;

			
			if(!strcmp(child->Value(),"Windows")){
				systemTypeCount += 1;
				if(m_systemType == System_NONE)
					m_systemType = System_Windows;
				else{
					error+= "Multiple Windows defined in the Systems element.\n";
					return false;
				}
				if(!readWindows(childElement, error)){
					error+= "Fail to read the Windows element.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"Lights")){
				systemTypeCount += 1;
				if(m_systemType == System_NONE)
					m_systemType = System_Lights;
				else{
					error+= "Multiple Lights defined in the Systems element.\n";
					return false;
				}
				if(!readLights(childElement, error)){
					error+= "Fail to read the Lights element.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"HVAC")){
				systemTypeCount += 1;
				if(m_systemType == System_NONE)
					m_systemType = System_HVAC;
				else{
					error+= "Multiple HVAC defined in the Systems element.\n";
					return false;
				}
				if(!readHVAC(childElement, error)){
					error+= "Fail to read the HVAC element.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"PlugLoad")){
				systemTypeCount += 1;
				if(m_systemType == System_NONE)
					m_systemType = System_PlugLoad;
				else{
					error+= "Multiple PlugLoad defined in the Systems element.\n";
					return false;
				}
				if(!readPlugLoad(childElement, error)){
					error+= "Fail to read the PlugLoad element.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"Thermostats")){
				systemTypeCount += 1;
				if(m_systemType == System_NONE)
					m_systemType = System_Thermostat;
				else{
					error+= "Multiple Thermostats defined in the Systems element.\n";
					return false;
				}
				if(!readThermostat(childElement, error)){
					error+= "Fail to read the Thermostats element.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"ShadesAndBlinds")){
				systemTypeCount += 1;
				if(m_systemType == System_NONE)
					m_systemType = System_ShadeAndBlind;
				else{
					error+= "Multiple ShadesAndBlinds defined in the Systems element.\n";
					return false;
				}
				if(!readShadeAndBlind(childElement, error)){
					error+= "Fail to read the ShadesAndBlinds element.\n";
					return false;
				}
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		if(systemTypeCount==0){
			error += "Can't find any system in the Systems element.\n";
			return false;
		}else if(systemTypeCount > 1){
			error += "Multiple systems found in the Systems element. Current, only support one system per behavior.\n";
			return false;
		}

		return true;
	};
	
	/// Read the Windows elemeent
	bool	Behavior::readWindows(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if(!strcmp(child->Value(),"WindowType")){
				string type = childElement->GetText();
				if(type == "Operable"){
					if(m_availableSystemType == -1)
						m_availableSystemType = Space::Window_Operable;
					else{						
						error+= "Multiple WindowType.\n";
						return false;
					}
				}else if(type == "Fixed"){
					if(m_availableSystemType == -1)
						m_availableSystemType = Space::Window_Fixed;
					else{						
							error+= "Multiple WindowType.\n";
							return false;
					}
				}else{
					error+= "Wrong window type.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"WindowID")){
				m_availableSystemIDs.push_back(childElement->GetText());
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		if(m_availableSystemType == -1){
			error += "Can't find WindowType.\n";
			return false;
		}

		return true;
	};

	/// Read the Lights element
	bool	Behavior::readLights(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if(!strcmp(child->Value(),"LightType")){
				string type = childElement->GetText();
				if(type == "OnOff"){
					if(m_availableSystemType == -1)
						m_availableSystemType = Space::Light_OnOff;
					else{						
						error+= "Multiple LightType.\n";
						return false;
					}
				}else if(type == "ContinuousControl"){
					if(m_availableSystemType == -1)
						m_availableSystemType = Space::Light_ContinuousControl;
					else{						
						error+= "Multiple ContinuousControl.\n";
						return false;
					}
				}else{
					error+= "Wrong light type.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"LightID")){
				m_availableSystemIDs.push_back(childElement->GetText());
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}
		
		if(m_availableSystemType == -1){
			error += "Can't find LightType.\n";
			return false;
		}

		return true;
	};
		
	/// Read the HVAC element
	bool	Behavior::readHVAC(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if(!strcmp(child->Value(),"HVACType")){
				string type = childElement->GetText();
				if(type == "ZoneOnOff"){
					if(m_availableSystemType == -1)
						m_availableSystemType = Space::HVAC_ZoneOnOff;
					else{						
						error+= "Multiple HVACType.\n";
						return false;
					}
				}else{
					error+= "Wrong HVAC type.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"HVACID")){
				m_availableSystemIDs.push_back(childElement->GetText());
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}
		
		if(m_availableSystemType == -1){
			error += "Can't find HVACType.\n";
			return false;
		}

		return true;
	};
	
	/// Read the PlugLoad element
	bool	Behavior::readPlugLoad(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if(!strcmp(child->Value(),"PlugLoadType")){
				string type = childElement->GetText();
				if(type == "OnOff"){
					if(m_availableSystemType == -1)
						m_availableSystemType = Space::PlugLoad_OnOff;
					else{						
						error+= "Multiple PlugLoadType.\n";
						return false;
					}
				}else if(type == "ContinuousControl"){
					if(m_availableSystemType == -1)
						m_availableSystemType = Space::PlugLoad_ContinuousControl;
					else{						
						error+= "Multiple ContinuousControl.\n";
						return false;
					}
				}else{
					error+= "Wrong Plug Load type.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"PlugLoadID")){
				m_availableSystemIDs.push_back(childElement->GetText());
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}
		
		if(m_availableSystemType == -1){
			error += "Can't find PlugLoadType.\n";
			return false;
		}

		return true;
	};

	
	/// Read the Thermostat element
	bool	Behavior::readThermostat(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if(!strcmp(child->Value(),"ThermostatType")){
				string type = childElement->GetText();
				if(type == "Adjustable"){
					if(m_availableSystemType == -1)
						m_availableSystemType = Space::Thermostat_Adjustable;
					else{						
						error+= "Multiple ThermostatType.\n";
						return false;
					}
				}else{
					error+= "Wrong thermostat type.\n";
					return false;
				}
			}else if(!strcmp(child->Value(),"ThermostatID")){
				m_availableSystemIDs.push_back(childElement->GetText());
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}
		
		if(m_availableSystemType == -1){
			error += "Can't find ThermostatType.\n";
			return false;
		}

		return true;
	};

	/// Read the ShadeAndBlind element
	bool Behavior::readShadeAndBlind(TiXmlElement *element, std::string& error) {
		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) { 
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			
			if (!strcmp(child->Value(), "ShadeAndBlindType")) {
				string type = childElement->GetText();
				if (type == "Operable") {
					if (m_availableSystemType == -1)
						m_availableSystemType = Space::ShadeAndBlind_Operable;
					else {						
						error += "Multiple ShadeAndBlindType.\n";
						return false;
					}
				} else {
					error += "Wrong Shade And Blind type.\n";
					return false;
				}
			} else if (!strcmp(child->Value(),"ShadeAndBlindID")) {
				m_availableSystemIDs.push_back(childElement->GetText());
			} else {
				error += "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}
		
		if (m_availableSystemType == -1) {
			error += "Can't find ShadeAndBlindType.\n";
			return false;
		}

		return true;
	};

	
	double	Behavior::getControlValue() {
		return m_interactionControlValue;	
	};

	Behavior::ActionType Behavior::getActionType() {
		return m_actionType;	
	};

	
};