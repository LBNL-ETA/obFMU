#include "ObSpace.h"

namespace OB{
	Space::Space(Object* parent):Object(parent){
		m_maxNumOfOccupants = 0;
		m_windowType = Window_NONE;
		m_lightType = Light_NONE;
		m_hvacType = HVAC_NONE;
		m_plugLoadType = PlugLoad_NONE;
		m_thermostatType = Thermostat_NONE;
		m_shadeAndBlindType = ShadeAndBlind_NONE;
		m_groupPriority = GroupPriority_Majority;
		OUTPUT_VARIABLE_NAMES.push_back("HVAC");
		OUTPUT_VARIABLE_NAMES.push_back("Light");
		OUTPUT_VARIABLE_NAMES.push_back("Infiltration");
		OUTPUT_VARIABLE_NAMES.push_back("Plug Load");
		OUTPUT_VARIABLE_NAMES.push_back("Thermostat");
		OUTPUT_VARIABLE_NAMES.push_back("Shade and Blind");
		OUTPUT_VARIABLE_NAMES.push_back("Occupancy");
		OUTPUT_SYSTEM_NAMES.push_back(Behavior::System_HVAC);
		OUTPUT_SYSTEM_NAMES.push_back(Behavior::System_Lights);
		OUTPUT_SYSTEM_NAMES.push_back(Behavior::System_Windows);
		OUTPUT_SYSTEM_NAMES.push_back(Behavior::System_PlugLoad);
		OUTPUT_SYSTEM_NAMES.push_back(Behavior::System_Thermostat);
		OUTPUT_SYSTEM_NAMES.push_back(Behavior::System_ShadeAndBlind);
		OUTPUT_SYSTEM_NAMES.push_back(Behavior::System_NONE);
	};
	Space::~Space(){};
	Space::Space(const Space& other){};
	Space& Space::operator= (const Space&){
		return *this;
	};

	/// initialize by reading an TiXmlElement
	/// params:
	/// - element: the TiXMLElement
	/// - error: the error information, when return false
	/// Return:
	/// - bool: readObXML successful or not
	bool Space::readObXML(TiXmlElement* element, std::string& error)
	{
		string id;
		if (!getStringAttribute(element, "ID", id, error)) {
			error += "Fail to find ID attribute for Space element.\n";
			return false;
		}

		/// Set the ID
		if(!setID(id, error)) {
			error += "Fail to set ID for Space.\n";
			return false;
		}

		bool hasType = false;


		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) { 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;

			TiXmlElement *childElement = (TiXmlElement*)child;
			if(!strcmp(child->Value(), "Type")) {
				hasType = true;
				string type = childElement->GetText();
				if (type == "ResidentialOwn")
					m_type = Space_ResidentialOwn;
				else if (type == "ResidentialRent")
					m_type = Space_ResidentialRent;
				else if (type == "OfficeShared")
					m_type = Space_OfficeShared;
				else if (type == "OfficePrivate")
					m_type = Space_OfficePrivate;
				else if (type == "MeetingRoom")
					m_type = Space_MeetingRoom;
				else if (type == "Corridor" || type == "Other")
					m_type = Space_Corridor;
				else if (type == "Outdoor")
					m_type = Space_Outdoor;
				else if (type == "Office")
					m_type = Space_Office;
				else {
					error+= "Find unexpected Space type: (";
					error += type;
					error += ") in Space element.\n";
					return false;
				}				
			} else if (!strcmp(child->Value(),"GroupPriority")) {
				hasType = true;
				string type = childElement->GetText();
				if (type == "Majority")
					m_groupPriority = GroupPriority_Majority;
				else {
					error+= "Find unexpected GroupPriority: (";
					error += type;
					error += ") in Space element.\n";
					return false;
				}				
			} else if (!strcmp(child->Value(),"Description")) {
				setDescription(childElement->GetText());
			} else if (!strcmp(child->Value(),"MeetingEvent")){
				MeetingEvent* obj = new MeetingEvent(this);
				if (!obj->readObXML(childElement, error)) {
					error += "Fail to read the Building element.\n";
					return false;
				}
				obj->setLocationId(m_locationIdInBuilding);
				m_meetingEvents.push_back(obj);
			} else if (!strcmp(child->Value(),"Systems")) {
				if (!readSystems(childElement, error)) {
					error+= "Fail to read the Systems element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(),"OccupantID")) {
				m_occupantIDs.push_back(childElement->GetText());
			} else {
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

		return true;
	};

	/// Read the Systems element
	bool Space::readSystems(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) {
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if (!strcmp(child->Value(), "Window")){
				if (!readWindow(childElement, error)){
					error+= "Fail to read the Window element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "Light")){
				if (!readLight(childElement, error)){
					error+= "Fail to read the Light element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "HVAC")){
				if (!readHVAC(childElement, error)){
					error+= "Fail to read the HVAC element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "PlugLoad")) {
				if (!readPlugLoad(childElement, error)) {
					error+= "Fail to read the PlugLoad element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "Thermostat")) {
				if (!readThermostat(childElement, error)) {
					error+= "Fail to read the Thermostat element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(),"ShadeAndBlind")) {
				if (!readShadeAndBlind(childElement, error)) {
					error+= "Fail to read the ShadeAndBlind element.\n";
					return false;
				}
			} else {
				error+= "Found unexpected child element: (";
				error += child->Value();
				error += ") in Systems element.\n";
				return false;
			}
		}
		return true;
	};


	/// Read the Window element
	bool Space::readWindow(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) {
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if (!strcmp(child->Value(),"Type")) {
				string type = childElement->GetText();
				if (type == "Operable")
					m_windowType = Window_Operable;
				else if (type == "Fixed")
					m_windowType = Window_Fixed;
				else {
					error+= "Find unexpected type: (";
					error += type;
					error += ") in Window element.\n";
					return false;
				}
			}
		}
		return true;
	};

		
	/// Read the Light element
	bool Space::readLight(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{
			if (child->Type() != TiXmlNode::ELEMENT) continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if (!strcmp(child->Value(),"Type")) {
				string type = childElement->GetText();

				if (type == "OnOff")
					m_lightType = Light_OnOff;
				else {
					error+= "Find unexpected type: (";
					error += type;
					error += ") in Light element.\n";
					return false;
				}
			}
		}
		return true;
	};

		
	/// Read the HVAC element
	bool Space::readHVAC(TiXmlElement *element, std::string& error){
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if (!strcmp(child->Value(),"Type")) {
				string type = childElement->GetText();
				if (type == "ZoneOnOff")
					m_hvacType = HVAC_ZoneOnOff;
				else {
					error+= "Find unexpected type: (";
					error += type;
					error += ") in HVAC element.\n";
					return false;
				}
			}
		}
		return true;
	};
	
	/// Read the PlugLoad element
	bool Space::readPlugLoad(TiXmlElement *element, std::string& error) {
		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) {
			if (child->Type() != TiXmlNode::ELEMENT) continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if (!strcmp(child->Value(), "Type")) {
				string type = childElement->GetText();
				if (type == "OnOff"){
					if (m_plugLoadType == PlugLoad_NONE) {
						m_plugLoadType = PlugLoad_OnOff;
					} else {
						error += "Multiple systems defined in the Systems element.\n";
						return false;
					}
				} else if (type == "ContinuousControl") {
					m_plugLoadType = PlugLoad_ContinuousControl;
				} else {
					error += "Find unexpected type: (";
					error += type;
					error += ") in PlugLoad element.\n";
					return false;
				}
			}
		}
		return true;
	};
	
	/// Read the Thermostat element
	bool Space::readThermostat(TiXmlElement *element, std::string& error) {
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) {
			if(child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if (!strcmp(child->Value(),"Type")) {
				string type = childElement->GetText();
				if (type == "Adjustable") {
					if (m_thermostatType == Thermostat_NONE) {
						m_thermostatType = Thermostat_Adjustable;
					} else {
						error += "Multiple systems defined in the Systems element.\n";
						return false;
					}
				} else {
					error += "Find unexpected type: (";
					error += type;
					error += ") in Thermostat element.\n";
					return false;
				}
			}
		}
		return true;
	};

	/// Read the Shade and Blind element
	bool Space::readShadeAndBlind(TiXmlElement *element, std::string& error) {
		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) {
			if (child->Type() != TiXmlNode::ELEMENT) continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if (!strcmp(child->Value(),"Type")) {
				string type = childElement->GetText();
				if (type == "Operable") {
					if (m_shadeAndBlindType == ShadeAndBlind_NONE) {
						m_shadeAndBlindType = ShadeAndBlind_Operable;
					} else {
						error += "Multiple systems defined in the Systems element.\n";
						return false;
					}
				} else {
					error += "Find unexpected type: (";
					error += type;
					error += ") in ShadeAndBlind element.\n";
					return false;
				}
			}
		}
		return true;
	};
	
	/// Get the list of occupant IDs
	vector<string> Space::getOccupantIDs() {
		return m_occupantIDs;
	};


	/// Add the occupant to the list
	void Space::addOccupant(Occupant* obj) {
		m_occupants.push_back(obj);
	};

	
	bool Space::performnOnOff(size_t iCurrentTimeStep, OutputParameterType outputType, map<Parameter::ParameterType, double> parameterValues, 
		map<Space::OutputParameterType,double>& outputValues, string& error, FILE* logFile) {
		double result =0;
		double controlValue = 0;
		bool foundSpecialBehavior = false;
		bool isSpecialBehavior = true;

		/// Check whether there are special behaviors
		/// For special behaviors, the results will be determined regardless the current output value
		
		/// Check any special behaviors to turn on
		Behavior::InteractionType interactionType = Behavior::Interaction_TurnOn;
		string title = "(Special Behavior) Turn on " + OUTPUT_VARIABLE_NAMES[outputType];

		if (!performInteraction(iCurrentTimeStep, OUTPUT_SYSTEM_NAMES[outputType], interactionType, 
			parameterValues, result, error, controlValue, isSpecialBehavior, foundSpecialBehavior)) {
			error += "Fail to " + title + ".\n";
			return false;
		}else{
			if(foundSpecialBehavior){
				if(logFile != NULL)
					fprintf(logFile, ",%s,", title.c_str());

				if (result > 0.9) {
					if (logFile != NULL)
						fprintf(logFile, "Win\n");

					outputValues[outputType] = 1.0;
					return true;
				}else{
					if(logFile != NULL)
						fprintf(logFile, "Lost\n");
				}
			}else{
				if(logFile != NULL)
					fprintf(logFile, ",%s,Do not find any special behaviors\n", title.c_str());
			}
		}

		if(!foundSpecialBehavior){
			Behavior::InteractionType interactionType = Behavior::Interaction_TurnOff;
			string title = "(Special Behavior) Turn off " + OUTPUT_VARIABLE_NAMES[outputType];

			if (!performInteraction(iCurrentTimeStep, OUTPUT_SYSTEM_NAMES[outputType], interactionType, parameterValues, result, error, controlValue,isSpecialBehavior, foundSpecialBehavior)) {
				error += "Fail to " + title + ".\n";
				return false;
			} else {
				if(foundSpecialBehavior){
					if (logFile != NULL)
					fprintf(logFile,",%s,",title.c_str());

					if (result > 0.9){
						if (logFile != NULL)
							fprintf(logFile, "Win\n");
						outputValues[outputType] = 0.0;
						return true;
					} else {
						if (logFile != NULL)
							fprintf(logFile, "Lost\n");
					}
				}else{
					if(logFile != NULL)
						fprintf(logFile, ",%s,Do not find any special behaviors\n", title.c_str());
				}
			}
		}

		/// The function will return when special behaviors are found.
		isSpecialBehavior = false;

		/// For normal behaivors, the turn on behaviors are only triggered when the existing output value is off
		/// For normal behaivors, the turn off behaviors are only triggered when the existing output value is on
		if (outputValues[outputType] < 0.1) {
			Behavior::InteractionType interactionType = Behavior::Interaction_TurnOn;
			string title = "Turn on " + OUTPUT_VARIABLE_NAMES[outputType];

			if (!performInteraction(iCurrentTimeStep, OUTPUT_SYSTEM_NAMES[outputType], interactionType, 
				parameterValues, result, error, controlValue,isSpecialBehavior,foundSpecialBehavior)) {
				error += "Fail to " + title + ".\n";
				return false;
			} else {
				if (logFile != NULL)
					fprintf(logFile, ",%s,", title.c_str());

				if (result > 0.9) {
					if (logFile != NULL)
						fprintf(logFile, "Win\n");

					outputValues[outputType] = 1.0;
				} else {
					if (logFile != NULL)
						fprintf(logFile, "Lost\n");
				}
			}
		} else {
			Behavior::InteractionType interactionType = Behavior::Interaction_TurnOff;
			string title = "Turn off " + OUTPUT_VARIABLE_NAMES[outputType];

			if (!performInteraction(iCurrentTimeStep, OUTPUT_SYSTEM_NAMES[outputType], interactionType,
				parameterValues, result, error, controlValue,isSpecialBehavior,foundSpecialBehavior)) {
				error += "Fail to " + title + ".\n";
				return false;
			} else {
				if (logFile != NULL)
					fprintf(logFile,",%s,",title.c_str());

				if (result > 0.9){
					if (logFile != NULL)
						fprintf(logFile, "Win\n");
					outputValues[outputType] = 0.0;
				} else {
					if (logFile != NULL)
						fprintf(logFile, "Lost\n");
				}
			}
		}
		return true;
	}
	
	
	bool Space::performnSetToControlValue(size_t iCurrentTimeStep, OutputParameterType outputType, map<Parameter::ParameterType, 
		double> parameterValues, map<Space::OutputParameterType,double>& outputValues, string& error, FILE* logFile) {
		double result =0;
		double controlValue = 0;
		Behavior::InteractionType interactionType = Behavior::Interaction_SetToControlValue;

		bool foundSpecialBehavior = false;
		bool isSpecialBehavior = true;

		/// Check whether there are special behaviors
		/// For special behaviors, the results will be determined regardless the current output value
		
		/// Check any special behaviors to set to control value
		string title = OUTPUT_VARIABLE_NAMES[outputType] + " Set to Control Value (Special Behavior)";
		if (!performInteraction(iCurrentTimeStep, OUTPUT_SYSTEM_NAMES[outputType], interactionType, 
			parameterValues, result, error, controlValue,isSpecialBehavior, foundSpecialBehavior)) {
			error += "Fail to " + title + ".\n";
			return false;
		}else{
			if(foundSpecialBehavior){
				if (logFile != NULL)
					fprintf(logFile, ",%s,", title.c_str());

				if (result > 0.9) {
					if (logFile != NULL)
						fprintf(logFile, "Win\n");

					outputValues[outputType] = controlValue;
				} else {
					if (logFile != NULL)
						fprintf(logFile, "Lost\n");

					/// Don't change the output value
				}
				return true;
			}
		}
		
		isSpecialBehavior = false;
		title = OUTPUT_VARIABLE_NAMES[outputType] + " Set to Control Value";

		if (!performInteraction(iCurrentTimeStep, OUTPUT_SYSTEM_NAMES[outputType], interactionType, 
			parameterValues, result, error, controlValue,isSpecialBehavior, foundSpecialBehavior)) {
			error += "Fail to " + title + ".\n";
			return false;
		} else {
			if (logFile != NULL)
				fprintf(logFile, ",%s,", title.c_str());
			if (result > 0.9) {
				if (logFile != NULL)
					fprintf(logFile, "Win\n");
				outputValues[outputType] = controlValue;
			} else {
				if (logFile != NULL)
					fprintf(logFile, "Lost\n");
			}
		}
		return true;
	}

		/// Calculate the output
	bool	Space::performCalculation(size_t numberofTimeStep, map<Parameter::ParameterType, double> parameterValues, 
		map<Space::OutputParameterType,double>& outputValues, string& error, FILE* logFile) {		
		switch (m_hvacType) {
			case HVAC_ZoneOnOff:
				if (!performnOnOff(numberofTimeStep, Output_HVAC_Schedule, parameterValues,outputValues,error,logFile)) {
					error += "Fail to perform HVAC on off control.\n";
					return false;
				}
				break;
			case HVAC_NONE:
			default:
				break;
		}

		switch (m_lightType) {
			case Light_OnOff:
				if (!performnOnOff(numberofTimeStep, Output_Light_Schedule, parameterValues,outputValues,error,logFile)) {
					error += "Fail to perform light on off control.\n";
					return false;
				}
				break;

			case Light_ContinuousControl:
				if (!performnSetToControlValue(numberofTimeStep, Output_Light_Schedule, parameterValues,outputValues,error,logFile)) {
					error += "Fail to perform light on off control.\n";
					return false;
				}
				break;
				break;
			case Light_NONE:
			default:
				break;
		}

		switch (m_windowType) {
			case Window_Operable:
				if (!performnOnOff(numberofTimeStep, Output_Infiltration_Schedule, parameterValues,outputValues,error,logFile)) {
					error += "Fail to perform Infiltration on off control.\n";
					return false;
				}
				break;
			case Window_Fixed:
			case Window_NONE:
			default:
				break;
		}

		switch (m_plugLoadType) {
			case PlugLoad_OnOff:
				if (!performnOnOff(numberofTimeStep, Output_PlugLoad_Schedule, parameterValues,outputValues,error,logFile)) {
					error += "Fail to perform PlugLoad on off control.\n";
					return false;
				}
				break;
			case PlugLoad_ContinuousControl:
				if (!performnSetToControlValue(numberofTimeStep, Output_PlugLoad_Schedule, parameterValues,outputValues,error,logFile)) {
					error += "Fail to perform PlugLoad set to control value.\n";
					return false;
				}
				break;
			case PlugLoad_NONE:
			default:
				break;
		}

		switch (m_thermostatType) {
			case Thermostat_Adjustable:
				if (!performnSetToControlValue(numberofTimeStep, Output_Thermostat_Schedule, parameterValues,outputValues,error,logFile)) {
					error += "Fail to perform Thermostat set to control value.\n";
					return false;
				}
				break;
			case Thermostat_NONE:
			default:
				break;
		}

		switch (m_shadeAndBlindType) {
			case ShadeAndBlind_Operable:
				if (!performnOnOff(numberofTimeStep, Output_ShadeAndBlind_Schedule, parameterValues,outputValues,error,logFile)) {
					error += "Fail to perform Shade and Blind on off control.\n";
					return false;
				}
				break;
			case ShadeAndBlind_NONE:
			default:
				break;
		}

		/// Ouput the occupnats
		outputValues[Output_Occupant_Schedule] = getRunTimeOccupantNumber(numberofTimeStep);
		return true;
	};

	/// Perform interaction
	bool Space::performInteraction(size_t numberofTimeStep, Behavior::SystemType systemType, 
		Behavior::InteractionType interactionType, map<Parameter::ParameterType,double> parameterValues, 
		double& result, string& error, double& controlValue, bool isSpecialBehavior, bool& foundSpecialBehavior) {

		/// Get the list of occupants in the space
		result = 0; 
		if (numberofTimeStep < 0 || numberofTimeStep > m_runTimeOccupants.size()) {
			error += "The number of time step (";
			error += numberofTimeStep;
			error += ") is out of scope.";
			return false;
		}

		/// Get all the occuapnts in this the space at the time step
		vector<Occupant*> time_occupants = m_runTimeOccupants[numberofTimeStep];

		/// No occupants in the space, do not perform any action
		if (time_occupants.size() == 0) {
			writeToLogFile(",,,No Occupants\n");
			return true;
		}
		
		/// Check the highest priority
		int highestPriority = 0;
		for (size_t i = 0; i < time_occupants.size(); i++) {
			highestPriority = max(highestPriority,time_occupants[i]->getPriority());
		}

		/// Check the previous occupnats
		vector<Occupant*> previousOccupants;
		if (numberofTimeStep > 0)
			previousOccupants = m_runTimeOccupants[numberofTimeStep - 1];

		/// Check the next occuapnts
		vector<Occupant*> nextOccupants;
		if (numberofTimeStep < m_runTimeOccupants.size() - 1)
			nextOccupants = m_runTimeOccupants[numberofTimeStep + 1];

		bool isSpaceEmptyNextTimeStep = false;
		if (nextOccupants.size() == 0)
			isSpaceEmptyNextTimeStep = true;

		/// only include the occupants with highest priority
		vector<Occupant*> occupants;
		for (size_t i=0; i < time_occupants.size(); i++) {
			if (time_occupants[i]->getPriority() == highestPriority) {
				occupants.push_back(time_occupants[i]);
			}
		}
		
		/// Check the status of all occupants
		/// Entering, Stay, or Leaving
		vector<vector<OB::Behavior::EventType>>	eventTypesVector;

		for (size_t i=0; i < occupants.size(); i++) {

			/// Check the status of each occupant
			/// Entering, Stay, or Leaving
			vector<OB::Behavior::EventType> eventTypes;

			/// Assume all the people is in the room
			eventTypes.push_back(OB::Behavior::Event_StayingInRoom);
			
			/// Check entering
			bool isEntering = true;
			for (size_t j=0; j < previousOccupants.size(); j++ ) {
				if(previousOccupants[j]->getID() == occupants[i]->getID()) {
					isEntering = false;
					break;
				}
			}

			if (isEntering)
				eventTypes.push_back(OB::Behavior::Event_EnteringRoom);

			/// Check the Leaving
			bool isLeaving = true;
			for (size_t j=0; j < nextOccupants.size(); j++ ) {
				if(nextOccupants[j]->getID() == occupants[i]->getID()) {
					isLeaving = false;
					break;
				}
			}

			if (isLeaving){
				eventTypes.push_back(OB::Behavior::Event_LeavingRoom);
				
				if(numberofTimeStep < m_runTimeOccupants.size() - m_simulationNumberofTimestepsPerHour){
					/// Check Leaving for 1 hours
					vector<vector<Occupant*>> next1HourOccupants;

					/// Start with 2 as the next time step is already checked.
					for(int i_1hour = 2; i_1hour <= m_simulationNumberofTimestepsPerHour; i_1hour++) {
						next1HourOccupants.push_back(m_runTimeOccupants[numberofTimeStep + i_1hour]);
					}
					bool isLeavingMoreThan1Hour = true;
					
					/// Check whether leave for more than 1 hour
					for (size_t i_1 = 0; i_1< next1HourOccupants.size(); i_1++) {
						for (size_t j_1 = 0; j_1 < next1HourOccupants[i_1].size(); j_1++) {
							if (next1HourOccupants[i_1][j_1]->getID() == occupants[i]->getID()) {
								isLeavingMoreThan1Hour = false;
								break;
							}
						}
						if (!isLeavingMoreThan1Hour)
							break;
					}

					if (isLeavingMoreThan1Hour) {
						eventTypes.push_back(OB::Behavior::Event_LeavingRoomMoreThan1Hour);
						
						if(numberofTimeStep < m_runTimeOccupants.size() - 6 * m_simulationNumberofTimestepsPerHour){
							/// Check Leaving for 6 hours

							vector<vector<Occupant*>> next2to6HourOccupants;
							for (int i_2to6hour = m_simulationNumberofTimestepsPerHour + 1; i_2to6hour <= m_simulationNumberofTimestepsPerHour * 6;  i_2to6hour++) {
								next2to6HourOccupants.push_back(m_runTimeOccupants[numberofTimeStep + i_2to6hour]);
							}

							bool isLeavingMoreThan6Hours = true;
							/// Check whether leave for more than 6 hour
							for (size_t i_6 = 0; i_6< next2to6HourOccupants.size(); i_6++) {
								for (size_t j_6 = 0; j_6 < next2to6HourOccupants[i_6].size(); j_6++) {
									if (next2to6HourOccupants[i_6][j_6]->getID() == occupants[i]->getID()) {
										isLeavingMoreThan6Hours = false;
										break;
									}
								}
								if(!isLeavingMoreThan6Hours) break;
							}

							if (isLeavingMoreThan6Hours) {
								eventTypes.push_back(OB::Behavior::Event_LeavingRoomMoreThan6Hours);
							}
						}
					} /// isLeavingMoreThan1Hour
				}
			}		

			eventTypesVector.push_back(eventTypes);
		}

		/// Specify the system type
		int spaceSystemControlType = -1;
		switch (systemType) {
			case Behavior::System_HVAC:
				spaceSystemControlType = (int)m_hvacType;
				break;
			case Behavior::System_Lights:
				spaceSystemControlType = (int)m_lightType;
				break;
			case Behavior::System_Windows:
				spaceSystemControlType = (int)m_windowType;
				break;
			case Behavior::System_PlugLoad:
				spaceSystemControlType = (int)m_plugLoadType;
				break;
			case Behavior::System_ShadeAndBlind:
				spaceSystemControlType = (int)this->m_shadeAndBlindType;
				break;
			case Behavior::System_Thermostat:
				spaceSystemControlType = (int)this->m_thermostatType;
				break;
			default:
				error += "Fail to find the system type (";
				error += systemType;
				error += ").\n";
				return false;
		}

		/// Get the results for each high priority occuapnt
		vector<double>	results;
		vector<double>	controlValueResults;
		vector<bool>	foundSpecialBehaviorResults;
		for (size_t i = 0; i < occupants.size(); i++) {
			double occResult;
			double controlValue;

			if (occupants[i]->performInteraction(numberofTimeStep, systemType, interactionType, 
				spaceSystemControlType, eventTypesVector[i], isSpaceEmptyNextTimeStep, parameterValues, 
				occResult, error, controlValue,isSpecialBehavior)){
					results.push_back(occResult);
					controlValueResults.push_back(controlValue);
			} else {
				error += "Fail to perform interaction for occupant (";
				error += occupants[i]->getID();
				error += ").\n";
				return false;
			}
		}

		/// Get the results from all the occupants
		/// Perform priority calculation.
		if ( m_groupPriority == GroupPriority_Majority) {
			if(isSpecialBehavior){
				bool voting_result = false;
				double occControlValue = 0;
				int numberOfItem = 0;
				for (size_t i=0; i < results.size(); i++) {
					if (results[i] > 0.9) {
						voting_result = true;
						occControlValue += controlValueResults[i];
						numberOfItem++;
					}
				}
				controlValue = occControlValue/numberOfItem;
								
				writeToLogFile(",,,");
				writeToLogFile(getID());
				writeToLogFile(",Special Vote,");
				if(voting_result){
					result = 1.0;
					foundSpecialBehavior = true;
					writeToLogFile(",Win\n");
				}else{
					foundSpecialBehavior = false;
					result = 0.0;
					writeToLogFile(",Lost\n");
				}

				return true;
			}else{
				/// Get the voting results
				long double avg =0;
				double occControlValue = 0;
				int numberOfItem = 0;
				for (size_t i=0; i < results.size(); i++) {
					avg += results[i];
					if (results[i] > 0.9) {
						occControlValue += controlValueResults[i];
						numberOfItem++;
					}
				}
				controlValue = occControlValue/numberOfItem;
				avg /= results.size();
				
				writeToLogFile(",,,");
				writeToLogFile(getID());
				writeToLogFile(",Average Vote:");
				writeToLogFile(std::to_string(avg));

				if(avg >= 0.5){
					result = 1.0;
					writeToLogFile(",Win\n");
				} else {
					result = 0.0;
					writeToLogFile(",Lost\n");
				}

				return true;
			}
		} else {
			error += "Wrong Group Priority Type.\n";
			return false;
		}
	};

	
	void Space::clearRunTimeOccupants() {
		m_runTimeOccupants.clear();
		m_currentMoveTimeStep = -1;
	};
	void Space::initRunTimeOccupnatsForOneTimeStep() {
		m_runTimeOccupants.push_back(vector<Occupant*>());
		m_currentMoveTimeStep++;
	};
	void Space::addRunTimeOccupnat(Occupant* occupant) {
		m_runTimeOccupants[m_currentMoveTimeStep].push_back(occupant);
	};
	
	int Space::getRunTimeOccupantNumber(int timeStep) {
		return m_runTimeOccupants[timeStep].size();
	};

	Space::SpaceType Space::getSpaceType(){
		return m_type;
	}
	
	vector<Occupant*> Space::getOccupants() {
		return m_occupants;
	};


	vector<MeetingEvent*> Space::getMeetingEvents() {
		return m_meetingEvents;
	};
	
	void Space::setSimulationNumberofTimestepsPerHour(int value) {
		m_simulationNumberofTimestepsPerHour = value;
	};

	/// Edited by Xuan - June 7th

	void Space::initSpaceSchedules(int simulationSteps) {
		m_meetingSchedule.resize(simulationSteps);
		/// TODO: Set init value to -1;
	}

	/// Get location id of a space in a building
	int Space::getLocationId() {
		return m_locationIdInBuilding;                                   
	}

	/// Set location id of a space
	void Space::setLocationId(int id) {
		m_locationIdInBuilding = id;		                                 
	}

	void Space::setMaxNumOccupants(int num) {
		if (num > m_maxNumOfOccupants) {
			m_maxNumOfOccupants = num;
		}
	}

	int Space::getMaxNumOccupants() {
		return m_maxNumOfOccupants;
	}

};