#include "ObOccupantBehavior.h"

namespace OB {

	class CSVRow {
		public:
			std::string const& operator[](std::size_t index) const
			{
				return m_data[index];
			}
			std::size_t size() const
			{
				return m_data.size();
			}
			void readNextRow(std::istream& str)
			{
				std::string         line;
				std::getline(str, line);

				std::stringstream   lineStream(line);
				std::string         cell;

				m_data.clear();
				while(std::getline(lineStream, cell, ','))
				{
					m_data.push_back(cell);
				}
			}
		private:
			std::vector<std::string>    m_data;
	};

	std::istream& operator>>(std::istream& str, CSVRow& data)
	{
		data.readNextRow(str);
		return str;
	} 

	tm* getXsDate(string str){
		vector<string> items = split(str, '-');
		if(items.size() != 3)
			return NULL;

		/// Check integer numbers
		for(size_t i=0; i < items.size(); i++){
			string item = items[i];
			for(size_t j=0; j < item.length(); j++){
				if(item[j] >'9' || item[j] < '0')
					return NULL;
			}
		}

		int year = atoi(items[0].c_str());
		int month = atoi(items[1].c_str());
		int day = atoi(items[2].c_str());

		if(year < 1900 || year > 2100)
			return NULL;
		if(month < 1 || month > 12)
			return NULL;
		if(day < 1 || day > 31)
			return NULL;

		time_t rawtime;
		tm* timeinfo = new tm();

		/* get current timeinfo and modify it to the user's choice */
		time ( &rawtime );
#ifdef _WIN
		if(localtime_s (timeinfo, &rawtime )){
			std::cout << "Fail to localtime_s.\n";
			return NULL;
		};
#else
        timeinfo = localtime(&rawtime);
        if(timeinfo==NULL){
            std::cout << "Fail to localtime.\n";
            return NULL;
        };
#endif
		timeinfo->tm_year = year - 1900;
		timeinfo->tm_mon = month - 1;
		timeinfo->tm_mday = day;

		/* call mktime: timeinfo->tm_wday will be set */
		mktime ( timeinfo );

		return timeinfo;
	};

	OccupantBehavior::OccupantBehavior() {
		for(size_t i = 0; i <= Season::Season_All; i++){
			Season::SeasonType type = (Season::SeasonType)i;
			Season* obj = new Season(type);
			m_seasons[type] = obj;
		}
		 
		for(size_t i = 0; i <= TimeofDay::TimeofDay_All; i++){
			TimeofDay::TimeofDayType type = (TimeofDay::TimeofDayType)i;
			TimeofDay* obj = new TimeofDay(type);
			m_timeofDays[type] = obj;
		}

		for(size_t i = 0; i <= DayofWeek::DayofWeek_All; i++){
			DayofWeek::DayofWeekType type = (DayofWeek::DayofWeekType)i;
			DayofWeek* obj = new DayofWeek(type);
			m_dayofWeeks[type] = obj;
		}

		m_currentFMUSpace = NULL;
		m_fFlagName="/FmuFlag.tmp";  /// An temp fil as an flag for movement calculation
		std::srand((unsigned int)std::time(0)); // use current time as seed for random generator
	};

	OccupantBehavior::~OccupantBehavior(){
		for (std::map < Season::SeasonType, Season*>::iterator it = m_seasons.begin(); it != m_seasons.end(); ++it)
			delete it->second;

		for (std::map < TimeofDay::TimeofDayType, TimeofDay*>::iterator it = m_timeofDays.begin(); it != m_timeofDays.end(); ++it)
			delete it->second;
		
		for (std::map < DayofWeek::DayofWeekType, DayofWeek*>::iterator it = m_dayofWeeks.begin(); it != m_dayofWeeks.end(); ++it)
			delete it->second;

		for(size_t i = 0; i < m_buildings.size(); i++)
			delete m_buildings[i];

		for(size_t i = 0; i < m_behaviors.size(); i++)
			delete m_behaviors[i];

		for(size_t i = 0; i < m_occupants.size(); i++)
			delete m_occupants[i];


	};

	/// initialize by reading an obXML file
	/// params:
	/// - filename: the filename of the obXML file including the path
	/// - error: the error information, when return false
	/// Return:
	/// - bool: readObXML successful or not
	bool	OccupantBehavior::readObXML(std::string xmlName, std::string& error){
		TiXmlDocument ObXMLdoc;
		
		if(!ObXMLdoc.LoadFile(xmlName.c_str())) {
			error += "Fail to load the obXML file ";
			error += xmlName;
			error += "\n";
			return false;
		}

		TiXmlElement* root = ObXMLdoc.RootElement();

		string id;
		
		if (!getStringAttribute(root, "ID", id, error)) {
			error += "Fail to find ID attribute for OccupantBehavior element.\n";
			return false;
		}

		/// Set the ID
		if (!setID(id, error)) {
			error += "Fail to set ID for OccupantBehavior.\n";
			return false;
		}

		if (!getStringAttribute(root, "Version", m_version, error)) {
			error += "Fail to find version attribute for OccupantBehavior element.\n";
			return false;
		}

		/// Flags to check whether the elements exists.
		bool	haveBuildings = false;
		bool	haveOccupants = false;
		bool	haveBehaviors = false;

		/// Read the Buildings
		for (const TiXmlNode* child = root->FirstChild(); child; child=child->NextSibling()) { 
			if (child->Type() != TiXmlNode::ELEMENT)	continue;

			TiXmlElement *childElement = (TiXmlElement*)child;
			if (!strcmp(child->Value(), "Buildings")) { 
				haveBuildings = true;
				if (!readBuildings(childElement, error)) {
					error += "Fail to read the Buildings element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "Occupants")) { 
				haveOccupants = true;
				if (!readOccupants(childElement, error)) {
					error += "Fail to read the Occupants element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "Behaviors")) { 
				haveBehaviors = true;
				if (!readBehaviors(childElement, error)) {
					error += "Fail to read the Behaviors element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "Seasons")) { 
				if (!readSeasons(childElement, error)) {
					error += "Fail to read the Seasons element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "TimeofDays")) { 
				if (!readTimeofDays(childElement, error)) {
					error += "Fail to read the TimeofDays element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "Holidays")) { 
				if(!readHolidays(childElement, error)){
					error += "Fail to Get_HolidaysNote.\n";
					return false;
				}
			} else {
				error += "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		if (!haveBuildings) {
			error += "Fail to find the Buildings element.\n";
			return false;
		}

		if (!haveOccupants) {
			error += "Fail to find the Occupants element.\n";
			return false;
		}

		if (!haveBehaviors) {
			error += "Fail to find the Behaviors element.\n";
			return false;
		}

		if (!connectIDswithObjects(error)) {
			error += "Fail to connect the IDs with their objects.\n";
			return false;
		}

		return true;
	};

	bool OccupantBehavior::readHolidays(TiXmlElement * element, std::string& error) {
		for(const TiXmlNode *child = element->FirstChild(); child; child = child->NextSibling()) { 
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement* childElement = (TiXmlElement*)child; 
			if(!strcmp(child->Value(), "Holiday")) {
				if (!readHoliday(childElement,error)) {
					error += "Fail to get holiday note.\n";
					return false;
				}
			}
		}
		return true;
	};

	bool OccupantBehavior::readHoliday(TiXmlElement * element, std::string& error) {
		for(const TiXmlNode *child = element->FirstChild(); child; child = child->NextSibling()) { 
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement* childElement = (TiXmlElement*)child; 
			if(!strcmp(child->Value(), "Date")) {
				tm* timeinfo = getXsDate(childElement->GetText()); 
				if (timeinfo != NULL) {
					tm localTime = *timeinfo;
					m_holidays.push_back(localTime);
				} else {
					error += "Fail to get the date information.\n";
					return false;
				}				
			}
		}

		return true;
	};

	/// Read the Buildings element
	bool	OccupantBehavior::readBuildings(TiXmlElement *element, std::string& error){
		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) {
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if (!strcmp(child->Value(), "Building")) {
				Building* obj = new Building(this);
				if (!obj->readObXML(childElement, error)) {
					error += "Fail to read the Building element.\n";
					return false;
				}
				m_buildings.push_back(obj);
			} else {
				error += "Found unexpected child element: (";
				error += child->Value();
				error += ") in Buildings element.\n";
				return false;
			}
		}

		return true;
	};


	/// Read the Occupants element
	bool	OccupantBehavior::readOccupants(TiXmlElement *element, std::string& error ){
		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) {
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if (!strcmp(child->Value(), "Occupant")) {
				Occupant* obj = new Occupant(this);
				if (!obj->readObXML(childElement, error)) {
					error += "Fail to read the Occupant element.\n";
					return false;
				}
				m_occupants.push_back(obj);
			} else {
				error += "Found unexpected child element: (";
				error += child->Value();
				error += ") in Occupants element.\n";
				return false;
			}
		}
		return true;
	
	};

	/// Read the Behaviors element
	bool	OccupantBehavior::readBehaviors(TiXmlElement *element, std::string& error) {
		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) {
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;
			if (!strcmp(child->Value(), "Behavior")) {
				Behavior* obj = new Behavior(this);
				if (!obj->readObXML(childElement, error)) {
					error += "Fail to read the Behavior element.\n";
					return false;
				}
				m_behaviors.push_back(obj);
			} else if (!strcmp(child->Value(), "MovementBehavior")) {
				MovementBehavior* obj = new MovementBehavior(this);
				if (!obj->readObXML(childElement, error)) {
					error += "Fail to read the Behavior element.\n";
					return false;
				}
				m_movementBehaviors.push_back(obj);
			} else {
				error += "Found unexpected child element: (";
				error += child->Value();
				error += ") in Behaviors element.\n";
				return false;
			}
		}
		return true;
	};

	/// Read the Seasons element
	bool	OccupantBehavior::readSeasons(TiXmlElement *element, std::string& error) {
		/// Allow seasons to have default values
		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) {
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;

			if (!strcmp(child->Value(), "Season")) {
				string type;
				if (!getStringAttribute(childElement, "Type", type, error)) {
					error += "Fail to find Type attribute for Season element.\n";
					return false;
				}
				Season* obj;
				if (type== "Summer")
					obj = m_seasons[Season::Season_Summer];
				else if (type== "Winter")
					obj = m_seasons[Season::Season_Winter];
				else if (type== "Spring")
					obj = m_seasons[Season::Season_Spring];
				else if (type== "Fall")
					obj = m_seasons[Season::Season_Fall];
				else if(type== "All")
					obj = m_seasons[Season::Season_All];
				else {
					error += "Find unexpected type: (";
					error += type;
					error += ") in Season element.\n";
					return false;
				}
				if (!obj->readObXML(childElement, error)) {
					error += "Fail to read the Season element.\n";
					return false;
				}
			} else {
				error += "Found unexpected child element: (";
				error += child->Value();
				error += ") in Seasons element.\n";
				return false;
			}
		}

		return true;
	};
	

	/// Read the TimeofDays element
	bool	OccupantBehavior::readTimeofDays(TiXmlElement *element, std::string& error){
		for (const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) {
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child;

			if (!strcmp(child->Value(), "TimeofDay")) {
				string type;
				if (!getStringAttribute(childElement, "Type", type, error)) {
					error += "Fail to find Type attribute for Season element.\n";
					return false;
				}
				TimeofDay* obj;
				if (type == "Morning")
					obj = m_timeofDays[TimeofDay::TimeofDay_Morning];
				else if (type == "Noon")
					obj = m_timeofDays[TimeofDay::TimeofDay_Noon];
				else if (type == "Afternoon")
					obj = m_timeofDays[TimeofDay::TimeofDay_Afternoon];
				else if (type == "Evening")
					obj = m_timeofDays[TimeofDay::TimeofDay_Evening];
				else if (type == "Day")
					obj = m_timeofDays[TimeofDay::TimeofDay_Day];
				else if (type == "Night")
					obj = m_timeofDays[TimeofDay::TimeofDay_Night];
				else if (type == "All")
					obj = m_timeofDays[TimeofDay::TimeofDay_All];
				else {
					error+= "Find unexpected type: (";
					error += type;
					error += ") in TimeofDay element.\n";
					return false;
				}
				if (!obj->readObXML(childElement, error)) {
			 		error+= "Fail to read the TimeofDay element.\n";
					return false;
				}
			} else {
				error += "Found unexpected child element: (";
				error += child->Value();
				error += ") in TimeofDays element.\n";
				return false;
			}
		}

		return true;
	};


	vector<Space*>		OccupantBehavior::getSpaces(){
		vector<Space*> spaces;
		for(size_t i=0; i< m_buildings.size(); i++){
			vector<Space*> objs = m_buildings[i]->getSpaces();
			spaces.insert(spaces.end(), objs.begin(), objs.end());
		}
		return spaces;
	};

	/// Get the Space with the ID
	Space*  OccupantBehavior::getSpace(string ID){
		for(size_t i=0; i < m_buildings.size(); i++){
			Space* obj = m_buildings[i]->getSpace(ID);
			if(obj != NULL)
				return obj;
		}

		return NULL;
	};

	/// Get the occupant with the ID
	Occupant* OccupantBehavior::getOccupant(string ID){
		for (size_t i = 0; i < m_occupants.size(); i++) {
			if (m_occupants[i]->getID() == ID) {
				return m_occupants[i];
			}
		}
		return NULL;
	};

	/// Get the behavior with the ID
	Behavior* OccupantBehavior::getBehavior(string ID) {
		for (size_t i = 0; i < m_behaviors.size(); i++) {
			if (m_behaviors[i]->getID() == ID) {
				return m_behaviors[i];
			}
		}
		return NULL;
	};                                     

	/// Get the movement behavior with the ID
	MovementBehavior* OccupantBehavior::getMovementBehavior(string ID) {
		for (size_t i = 0; i < m_movementBehaviors.size(); i++) {
			if (m_movementBehaviors[i]->getID() == ID) {
				return m_movementBehaviors[i];
			}
		}
		return NULL;
	};

	SimulationSettings*	OccupantBehavior::getSimulationSettings(){
		return &m_simulationSettings;
	};



	/// connect all the reference IDs with their objects.
	/// Return
	/// - return false when can't find objects with the given ID.
	bool OccupantBehavior::connectIDswithObjects(std::string& error) {
		/// Connect space and occupant
		for(size_t i=0; i <m_buildings.size();i++) {
			vector<Space*> spaces = m_buildings[i]->getSpaces();
			for (size_t j = 0; j < spaces.size(); j++) {
				vector<string> occupantIDs = spaces[j]->getOccupantIDs();
				for (size_t k = 0; k < occupantIDs.size(); k++) {
					Occupant* obj = getOccupant(occupantIDs[k]);
					if (obj == NULL) {
						error += "Can't find occupant with ID (";
						error += occupantIDs[k];
						error += ") for space (";
						error += spaces[j]->getID();
						error += ").\n";
						return false;
					} else {
						spaces[j]->addOccupant(obj);
					}
				}
			}
		}

		/// Connect occupant and behavior
		for (size_t i = 0; i< m_occupants.size(); i++) {
			vector<string> behaviorIDs = m_occupants[i]->getBehaviorIDs();
			for (size_t j = 0; j < behaviorIDs.size(); j++) {
				Behavior* obj = getBehavior(behaviorIDs[j]);
				if (obj == NULL) {
					error += "Can't find Behavior with ID (";
					error += behaviorIDs[j];
					error += ") for Occupant (";
					error += m_occupants[i]->getID();
					error += ").\n";
					return false;
				} else {
					m_occupants[i]->addBehavior(obj);
				}
			}
		}

		/// Connect occupant and behavior
		for (size_t i = 0; i< m_occupants.size(); i++) {
			vector<string> movementBehaviorIDs = m_occupants[i]->getMovementBehaviorIDs();
			for (size_t j = 0; j < movementBehaviorIDs.size(); j++) {
				MovementBehavior* obj = getMovementBehavior(movementBehaviorIDs[j]);
				if (obj == NULL) {
					error += "Can't find Behavior with ID (";
					error += movementBehaviorIDs[j];
					error += ") for Occupant (";
					error += m_occupants[i]->getID();
					error += ").\n";
					return false;
				} else {
					m_occupants[i]->addMoveBehavior(obj);
				}
			}
		}

		for (size_t i = 0; i < m_behaviors.size(); i++) {
			/// Connect behavior and TimeofDay
			/// The TimeofDay types are already checked when reading the obXML.
			vector<TimeofDay::TimeofDayType> timeTypes = m_behaviors[i]->getTimeofDayTypes();
			for (size_t j = 0; j < timeTypes.size(); j++) {
				TimeofDay* obj = m_timeofDays[timeTypes[j]];
				m_behaviors[i]->addTimeofDay(obj);
			}
			
			/// Connect movement behavior and season
			/// The season types are already checked when reading the obXML.
			vector<Season::SeasonType> seasonTypes = m_behaviors[i]->getSeasonTypes();
			for (size_t j = 0; j < seasonTypes.size(); j++) {
				Season* obj = m_seasons[seasonTypes[j]];
				m_behaviors[i]->addSeasion(obj);
			}
			
			/// Connect behavior and DayofWeek
			vector<DayofWeek::DayofWeekType> weekTypes = m_behaviors[i]->getDayofWeekTypes();
			for(size_t j = 0; j < weekTypes.size(); j++) {
				DayofWeek* obj = m_dayofWeeks[weekTypes[j]];
				m_behaviors[i]->addDayofWeek(obj);
			}

			/// Connect the parameters in behaviors
			if (! m_behaviors[i]->connectIDswithObjects(error)) {
				error += "Fail to connect IDs with Objects for behavior (";
				error += m_behaviors[i]->getID();
				error += ").\n";
				return false;
			}
		}

		for (size_t i = 0; i < m_movementBehaviors.size(); i++) {
			
			/// Connect movement behavior and season
			/// The season types are already checked when reading the obXML.
			vector<Season::SeasonType> seasonTypes = m_movementBehaviors[i]->getSeasonTypes();
			for (size_t j = 0; j < seasonTypes.size(); j++) {
				Season* obj = m_seasons[seasonTypes[j]];
				m_movementBehaviors[i]->addSeasion(obj);
			}
			
			/// Connect movement behavior and DayofWeek
			vector<DayofWeek::DayofWeekType> weekTypes = m_movementBehaviors[i]->getDayofWeekTypes();
			for(size_t j = 0; j < weekTypes.size(); j++) {
				DayofWeek* obj = m_dayofWeeks[weekTypes[j]];
				m_movementBehaviors[i]->addDayofWeek(obj);
			}

		}

		
		vector<MeetingEvent*> meetingEvents;
		for (size_t i = 0; i < m_buildings.size(); i++) {
			for (size_t j = 0; j < m_buildings[i]->getSpaces().size(); j++) {
				vector<MeetingEvent*> newMeetingEvents = m_buildings[i]->getSpaces()[j]->getMeetingEvents();             
				for (size_t k = 0; k < newMeetingEvents.size(); k++) {
					meetingEvents.push_back(newMeetingEvents[k]);
				}
			}
		}
		for (size_t i = 0; i < meetingEvents.size(); i++) {
			// Connect meeting behavior and season
		    /// The season types are already checked when reading the obXML.
			vector<Season::SeasonType> seasonTypes = meetingEvents[i]->getSeasonTypes();
			for (size_t j = 0; j < seasonTypes.size(); j++) {
				Season* obj = m_seasons[seasonTypes[j]];
				meetingEvents[i]->addSeasion(obj);
			}
			
			/// Connect meeting and DayofWeek
			vector<DayofWeek::DayofWeekType> weekTypes = meetingEvents[i]->getDayofWeekTypes();
			for(size_t j = 0; j < weekTypes.size(); j++) {
				DayofWeek* obj = m_dayofWeeks[weekTypes[j]];
				meetingEvents[i]->addDayofWeek(obj);
			}
		}
		return true;

	};

	/// Prase the obCoSim file
	/// params:
	/// - filename: the filename of the obCoSim file including the path
	/// - error: the error information, when return false
	/// Return:
	/// - bool: readObCoSim successful or not
	bool	OccupantBehavior::readObCoSim(std::string xmlName, std::string& error){
		TiXmlDocument ObCoSimdoc;
		if(!ObCoSimdoc.LoadFile(xmlName.c_str())) {
			error += "Fail to load the obCoSim file ";
			error += xmlName;
			error += "\n";
			return false;
		}

		TiXmlElement* root = ObCoSimdoc.RootElement();

		/// Flags to check whether the elements exists.
		bool	haveSpaceNameMapping = false;
		bool	haveSimulationSettings = false;

		/// Read the Buildings
		for(const TiXmlNode* child = root->FirstChild(); child; child=child->NextSibling()) { 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;

			TiXmlElement *childElement = (TiXmlElement*)child;
			if(!strcmp(child->Value(), "SpaceNameMapping")) { 
				haveSpaceNameMapping = true;
				if (!readSpaceNameMapping(childElement, error)) {
					error += "Fail to read the SpaceNameMapping element.\n";
					return false;
				}
			} else if (!strcmp(child->Value(), "SimulationSettings")) { 
				haveSimulationSettings = true;
				if (!m_simulationSettings.readObXML(childElement, error)) {
					error += "Fail to read the SimulationSettings element.\n";
					return false;
				}
			} else {
				error += "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		if (!haveSpaceNameMapping) {
			error+= "Fail to find any SpaceNameMapping element.\n";
			return false;
		}

		if (!haveSimulationSettings) {
			error+= "Fail to find the SimulationSettings element.\n";
			return false;
		}
		 
		for (std::map<DayofWeek::DayofWeekType, DayofWeek*>::iterator it = m_dayofWeeks.begin(); it != m_dayofWeeks.end(); ++it) {
			it->second->initTimeStepFlags(&m_simulationSettings);
		}
		
		for (std::map<TimeofDay::TimeofDayType, TimeofDay*>::iterator it = m_timeofDays.begin(); it != m_timeofDays.end(); ++it ){
			it->second->initTimeStepFlags(&m_simulationSettings);
		}

		for (std::map<Season::SeasonType,Season*>::iterator it = m_seasons.begin(); it != m_seasons.end(); ++it) {
			it->second->initTimeStepFlags(&m_simulationSettings);
		}

		for (size_t i = 0; i <m_behaviors.size(); i++) {
			m_behaviors[i]->setTimeStep(60 / m_simulationSettings.getNumberofTimestepsPerHour());
		}

		for (size_t i = 0; i <m_movementBehaviors.size(); i++) {
			m_movementBehaviors[i]->setTimeStep(60 / m_simulationSettings.getNumberofTimestepsPerHour());
		}

		return true;
	};
	
	/// Read the SpaceNameMapping element
	bool	OccupantBehavior::readSpaceNameMapping(TiXmlElement *element, std::string& error){
		string obXMLSpaceID = "";
		string fmuInstanceName = "";
		for(const TiXmlNode* child = element->FirstChild(); child; child=child->NextSibling()) 
		{ 
			if(child->Type() != TiXmlNode::ELEMENT)	continue;

			TiXmlElement *childElement = (TiXmlElement*)child;
			if(!strcmp(child->Value(),"obXML_SpaceID")) 
			{ 
				obXMLSpaceID =  childElement->GetText();
			}else if(!strcmp(child->Value(),"FMU_InstanceName")) { 
				fmuInstanceName = childElement->GetText();
			}else{
				error+= "Found unexpected element: (";
				error += child->Value();
				error += ").\n";
				return false;
			}
		}

		Space* space = getSpace(obXMLSpaceID);
		if(space == NULL){
			error += "Fail to find an Space with ID (";
			error += obXMLSpaceID;
			error += ") in the Buildings.\n";
			return false;
		}

		if(fmuInstanceName == ""){
			error += "The FMU_InstanceName is empty.\n";
			return false;
		}
		m_fmuInstanceNameToSpaces[fmuInstanceName] = space;
		return true;
	};



	bool	OccupantBehavior::getLineItems(std::ifstream& moveFile, string& line, vector<string>& items, long long& line_number, size_t item_min_number, string& error){
		line_number++;
		if(!std::getline(moveFile, line)){
			error += "Fail to get text in line " + to_string(line_number) + " due to end of file.\n";
			return false;
		}
		items = split(line,',');
		if(items.size() < item_min_number){
			error += "Line " + std::to_string((long long)line_number) + " has less than " + to_string((long long)item_min_number)+ " items.\n";
			return false;
		}
	
		return true;
	};

	/// Obtain the movement results from the movement result csv file		
	/// params:
	/// - filename: the filename of the movement result file including the path
	/// - error: the error information, when return false
	/// Return:
	/// - bool: readObCoSim successful or not
	bool	OccupantBehavior::readMovementCSV(string filename, std::string& error){
		if(m_currentFMUSpace == NULL){
			error += "The current FMU Space is NULL. Please initalize the current FMU space before reading the movemnt results.\n";
			return false;
		}else{
			m_currentFMUSpace->clearRunTimeOccupants();
		}

		std::cout << "Reading movement CSV result:\n";
		std::cout << "|----------|100%\n";
		/// Open the file for reading
		std::ifstream moveFile(filename);
		std::string line;
		vector<string> items;
		long long line_number = 0;

		/// Read the room amout and occupant amount
		if(!getLineItems(moveFile, line, items, line_number, 6, error))
			return false;
			
		if(items[2] != "Room Amount"){
			error += "Line " + std::to_string((long long)line_number) + " 3rd item should be Room Amount.\n";
			return false;
		}
		int roomAmount = std::stoi(items[3])+1;
			
		if(items[4] != "Occupant Amount"){
			error += "Line " + std::to_string((long long)line_number) + " 5th item should be Occupant Amount.\n";
			return false;
		}
		int occupantAmount = std::stoi(items[5]);
		
		vector<Space*>  spaces; 
		for(size_t i=0; i < m_buildings.size();i++){
			for(size_t j=0; j< m_buildings[i]->getSpaces().size() ; j++){
				spaces.push_back(m_buildings[i]->getSpaces()[j]);
			}
		}

		if( spaces.size() != roomAmount){
			error += "Line " + std::to_string((long long)line_number) + ": The Room Amount (" + std::to_string((long long)roomAmount-1);
			error += ") is different with the model space number (" + std::to_string((long long)spaces.size()) +").\n";
			return false;
		}
		
		if( m_occupants.size() != occupantAmount){
			error += "Line " + std::to_string((long long)line_number) + ": The Occupant Amount (" + std::to_string((long long)occupantAmount);
			error += ") is different with the model occpuant number (" + std::to_string((long long)m_occupants.size()) +").\n";
			return false;
		}

		/// Read the simulation time
		if(!getLineItems(moveFile, line, items, line_number, 6, error))
			return false;
			
		if(items[0] != "Start Time"){
			error += "Line " + std::to_string((long long)line_number) + " 1st item should be Start Time.\n";
			return false;
		}
		string startTime = items[1];
	
			
		if(items[2] != "End Time"){
			error += "Line " + std::to_string((long long)line_number) + " 3rd item should be End Time.\n";
			return false;
		}
		string endTime = items[3];

		
		if(items[4] != "Number of Steps per Hour"){
			error += "Line " + std::to_string((long long)line_number) + " 5th item should be Number of Steps per Hour.\n";
			cout << "Instead is" << items[4] << "\n";
			return false;
		}
		int numberOfStepsPerHour = stoi(items[5]);
		int totalNumberOfSteps;
		
		if(!m_simulationSettings.checkSimulationPeriod(startTime, endTime, numberOfStepsPerHour, totalNumberOfSteps, error)){
			return false;
		}
		cout << "Total number of steps: " << totalNumberOfSteps << "\n";
		/// Process the Room number and Room ID
		/// Get room numbers
		if(!getLineItems(moveFile, line, items, line_number, roomAmount+1, error))
			return false;
		
		if(items[0] != "Room Number"){
			error += "Line " + std::to_string((long long)line_number) + " 1st item should be Room Number.\n";
			return false;
		}
		
		for(int i=1; i< roomAmount+1; i++){
			if(stoi(items[i]) != i -1){
				error += "Line " + std::to_string((long long)line_number);
				error += ": " + std::to_string((long long)i +1) + "th item (" + items[i] + ") should be " + std::to_string((long long)i -1);
				error += ". The Room Number should start with 0 and continuously increase by 1.\n";
				return false;
			}
		}

		/// Get room IDs
		if(!getLineItems(moveFile, line, items, line_number, roomAmount+1, error))
			return false;
		
		if(items[0] != "Room ID"){
			error += "Line " + std::to_string((long long)line_number) + " 1st item should be RoomRoom ID.\n";
			return false;
		}
		
		int currentFMUSapceIndex = -1;
		for(int i=1; i< roomAmount+1; i++){
			if(items[i] == m_currentFMUSpace->getID()){
				currentFMUSapceIndex = i-1;
				break;
			}
		}

		if(currentFMUSapceIndex==-1){
			error += "Line " + std::to_string((long long)line_number) + ": Can't find current room ID (" + m_currentFMUSpace->getID() +").\n";
			return false;
		}
		
		/// Process the occupant number and occpant ID
		/// Get Occupant numbers
		if(!getLineItems(moveFile, line, items, line_number, occupantAmount+1, error))
			return false;
		
		if(items[0] != "Occupant Number"){
			error += "Line " + std::to_string((long long)line_number) + " 1st item should be Occupant Number.\n";
			return false;
		}
		
		for(int i=1; i< occupantAmount+1; i++){
			if(stoi(items[i]) != i -1){
				error += "Line " + std::to_string((long long)line_number);
				error += ": " + std::to_string((long long)i +1) + "th item (" + items[i] + ") should be " + std::to_string((long long)i -1);
				error += ". The Occupant Number should start with 0 and continuously increase by 1.\n";
				return false;
			}
		}

		/// Get Occupant IDs
		if(!getLineItems(moveFile, line, items, line_number, occupantAmount+1, error))
			return false;
		
		if(items[0] != "Occupant ID"){
			error += "Line " + std::to_string((long long)line_number) + " 1st item should be Occupant ID.\n";
			return false;
		}
		
		vector<string>	occupantIDs;
		for(int i=1; i< occupantAmount+1; i++){
			occupantIDs.push_back(items[i]);
		}

		vector<Occupant*> occupantsInOrder;
		for(size_t i=0; i < occupantIDs.size(); i++){
			bool foundOccupant = false;
			for(size_t j=0; j< m_occupants.size(); j++){
				if(occupantIDs[i]==m_occupants[j]->getID()){
					occupantsInOrder.push_back(m_occupants[j]);
					foundOccupant = true;
					break;
				}
			}
			if(!foundOccupant){
				error += "Line " + std::to_string((long long)line_number) + ": Can't find Occupant with ID (" + occupantIDs[i] +") for the " + std::to_string((long long)(i+1)) + "th item.\n";
				return false;
			}
		}

		/// Check the occupant number
		/// Check Occupant numbers
		if(!getLineItems(moveFile, line, items, line_number, occupantAmount+2, error))
			return false;
		
		if(items[0] != "Step"){
			error += "Line " + std::to_string((long long)line_number) + " 1st item should be step.\n";
			error += "Instead is: " + items[0] + ".\n";
			return false;
		}

		if(items[1] != "Time"){
			error += "Line " + std::to_string((long long)line_number) + " 2nd item should be time.\n";
			error += "Instead is: " + items[1] + ".\n";
			return false;
		}
		
		for(int i=2; i< occupantAmount+2; i++){
			if(stoi(items[i]) != i -2){
				error += "Line " + std::to_string((long long)line_number);
				error += ": " + std::to_string((long long)i+1) + "th item (" + items[i] + ") should be " + std::to_string((long long)i -2);
				error += ". The Occupant Number should start with 0 and continuously increase by 1.\n";
				return false;
			}
		}
		
		std::cout << ".";
		/// Read through the data
		int reportTime = totalNumberOfSteps / 10;
		for(int i=0; i < totalNumberOfSteps; i++){
			if(!getLineItems(moveFile, line, items, line_number, occupantAmount+2, error))
				return false;

			if(line_number%reportTime==0)
				std::cout << ".";

			m_currentFMUSpace->initRunTimeOccupnatsForOneTimeStep();

			for( int j=2; j < occupantAmount+2;j++){
				if(stoi(items[j]) == currentFMUSapceIndex)
					m_currentFMUSpace->addRunTimeOccupnat(occupantsInOrder[j-2]);
			}
		}
		std::cout << "\n";
		return true;
	};

	
	/// Init Current FMU Space
	bool	OccupantBehavior::initCurrentFMUSpace(string instanceName, std::string& error){
		m_currentFMUSpace = m_fmuInstanceNameToSpaces[instanceName];
		if(m_currentFMUSpace ==NULL){
			error += "Cant find space with the FMU instance name (";
			error += instanceName;
			error += ").\n";
			return false;
		}

		return true;
	};

	/// Get Current FMU Space
	Space*	OccupantBehavior::getCurrentFMUSpace(){
		return m_currentFMUSpace;
	};

	void	OccupantBehavior::setupLogFile(FILE* file){
		vector<Space*> spaces = getSpaces();
		for(size_t i=0; i < spaces.size(); i++){
			spaces[i]->setLogFile(file);
		}
		for(size_t i=0; i < m_occupants.size(); i++){
			m_occupants[i]->setLogFile(file);
		}
		for(size_t i=0; i < m_behaviors.size(); i++){
			m_behaviors[i]->setLogFile(file);
		}
	};

	void	OccupantBehavior::init(){
		for(size_t i=0; i< m_buildings.size(); i++){
			m_buildings[i]->setSimulationNumberofTimestepsPerHour(m_simulationSettings.getNumberofTimestepsPerHour());
		}
	};







	/// Initalize the parameters for movement calculation
	bool OccupantBehavior::initMovementCalculation(string& error) {
		int numOfStepPerHour = m_simulationSettings.getNumberofTimestepsPerHour();
		int dayOfSimulation = m_simulationSettings.getTotalNumberofDays();
		int totalStepsPerday = numOfStepPerHour * 24;
		int totalSimulationStep = m_simulationSettings.getTotalNumberofTimeSteps();
		
		/// Init Occupants
		for (size_t i = 0; i < m_buildings.size(); i++) {
			for (size_t j = 0; j < m_buildings[i]->getSpaces().size(); j++) {
				// Initialize space meeting schedule to -1 at each time step;
				m_buildings[i]->getSpaces()[j]->initSpaceSchedules(totalStepsPerday);
				vector<Occupant*> occupants = m_buildings[i]->getSpaces()[j]->getOccupants();
			
				for (size_t k = 0; k < occupants.size(); k++) {
					// Set occupants' own office id
					occupants[k]->setOwnLocationId(m_buildings[i]->getSpaces()[j]->getLocationId());
					occupants[k]->setlLastStepStatus(-1);
				}
			}
		}

		/// Init the movement behavior events
		for (size_t i = 0; i < m_movementBehaviors.size(); i++) {
			/// Get the list of spaces for each category to initalize movement
			vector<vector<int>> locationIdMap; //0: office, 1: meeting, 2: others, 3: outdoor
			locationIdMap.resize(4);
			for (size_t j = 0; j < m_buildings.size(); j++) {
				for (size_t k = 0; k < m_buildings[j]->getSpaces().size(); k++) {
					OB::Space* curSpace = m_buildings[j]->getSpaces()[k];
					OB::Space::SpaceType spaceType = curSpace->getSpaceType();
					if (spaceType == OB::Space::Space_OfficePrivate || 
						spaceType == OB::Space::Space_OfficeShared || spaceType == OB::Space::Space_Office) {
						locationIdMap[0].push_back(curSpace->getLocationId());
					} else if (spaceType == OB::Space::Space_MeetingRoom) {
						locationIdMap[1].push_back(curSpace->getLocationId());
					} else if (spaceType == OB::Space::Space_Outdoor) {
						locationIdMap[3].push_back(curSpace->getLocationId());
					} else {
						locationIdMap[2].push_back(curSpace->getLocationId());
					}
					
				}
			}
			m_movementBehaviors[i]->initLocationCategory(locationIdMap);
			m_movementBehaviors[i]->initEvents(&m_simulationSettings, error);    ///update events, calculate matrix
		}
		///Calculate daily meeting percentage of occupant based on assigned movement behaviors
		for (size_t i = 0; i < m_occupants.size(); i++) {
			if (!m_occupants[i]->initDailyMeetingPercent(dayOfSimulation, error)) {
				error += "Fail to simulate occupancy status.\n";
				return false;
			};
		}

		return true;
	}

	///Write headers
	void OccupantBehavior::writeOutPutFiles(ofstream& file_byOccu, ofstream& file_byRoom, ofstream& pForIDFout, ofstream& pIDFout) {
		string ver = "1.3";

		int roomScheduleSize = 0;
		for (size_t i = 0; i < m_buildings.size(); i++) {
			roomScheduleSize += m_buildings[i]->getSpaces().size();
		}
		file_byOccu << "Version, " << ver << " ,Room Amount, " << roomScheduleSize - 1 << " ,Occupant Amount, "<< m_occupants.size() << "\n";
		pForIDFout << "Version, " << ver << " ,Room Amount, " << roomScheduleSize - 1 << " ,Occupant Amount, "<< m_occupants.size() << "\n";

		int startDate = m_simulationSettings.getStartDayofYear();
		int endDate = m_simulationSettings.getEndDayofYear();
		int startMon, startDay, endMon, endDay;
		getMonDay(&startMon, &startDay, startDate);
		getMonDay(&endMon, &endDay, endDate);
		file_byOccu << "Start Time, " << startMon << "-" << startDay << " ,End Time, " << endMon << "-" << endDay 
			<< ",Number of Steps per Hour, " << m_simulationSettings.getNumberofTimestepsPerHour() << "\n";
		pForIDFout << "Start Time, " << startMon << "-" << startDay << " ,End Time, " << endMon << "-" << endDay 
			<< ",Number of Steps per Hour, " << m_simulationSettings.getNumberofTimestepsPerHour() << "\n";

		file_byOccu << "Room Number,";
		pForIDFout << "Room Number,";
		for(int i=0; i < roomScheduleSize; i++) {
			file_byOccu << i << ",";
			pForIDFout << i << ",";
		}
		file_byOccu << "-1, \n";
		pForIDFout << "-1, \n";
		file_byOccu << "Room ID,";
		pForIDFout << "Room ID,";


		for (size_t i = 0; i < m_buildings.size(); i++) {
			for (size_t j = 0; j < m_buildings[i]->getSpaces().size(); j++) {
				file_byOccu << m_buildings[i]->getSpaces()[j]->getID() << ",";
				pForIDFout << m_buildings[i]->getSpaces()[j]->getID() << ",";
			}
		}
		file_byOccu << "Outside building, \n";
		pForIDFout << "Outside building, \n";
		file_byOccu << "Occupant Number,";
		pForIDFout << "Occupant Number,";

		for(size_t i=0; i < m_occupants.size(); i++){
			file_byOccu << i << ",";
			pForIDFout << i << ",";
		}
		file_byOccu << "\n";
		pForIDFout << "\n";
		file_byOccu << "Occupant ID,";
		pForIDFout << "Occupant ID,";
		for(size_t i=0; i < m_occupants.size(); i++){
			file_byOccu << m_occupants[i]->getID() << ",";
			pForIDFout << m_occupants[i]->getID() << ",";
		}
		file_byOccu << "\n";
		pForIDFout << "\n";

		//Setup the header for the by oocupant csv file
		file_byOccu << "Step,Time,";
		for (size_t k = 0; k < m_occupants.size(); k++) {
			file_byOccu << k << ",";
		}
		file_byOccu << "\n";

		/// Setup the header for the by room csv file
		file_byRoom << "Step,Time,";
		
		for (size_t i = 0; i < m_buildings.size(); i++) {
			for (size_t j = 1; j < m_buildings[i]->getSpaces().size(); j++) {
				file_byRoom << m_buildings[i]->getSpaces()[j]->getID() << ",";
			}
		}
		file_byRoom << "Whole building,\n";

		/// Setup the header for the by room csv file for IDF
		pForIDFout << "Step,Time,";
		for (size_t i = 0; i < m_buildings.size(); i++) {
			for (size_t j = 1; j < m_buildings[i]->getSpaces().size(); j++) {
				pForIDFout << m_buildings[i]->getSpaces()[j]->getID() << ",";
			}
		}
	
	}

	/// Perform Movement calculation
	bool OccupantBehavior::performMovementCalculation(string fileName, std::string& error) {
		bool isDebugMode = m_simulationSettings.isDebugMode();
		clock_t begin = clock();
		clock_t end = clock();
		double elapsed_secs;
		if (fileName.size() > 4 && fileName.substr(fileName.size() - 4).compare(".csv") == 0) {
			fileName = fileName.substr(0, fileName.size() - 4);
		}
		if (isDebugMode) {
			begin = clock();
			std::cout << "  Init movement calculation...\n";
		}

		//Init the overall calculation only
		if(!initMovementCalculation(error)){
			error += "  Fail to initMovementCalculation.\n";
			return false;
		}

		if (isDebugMode) {
			end = clock();
			elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
			std::cout<< "  Done with " << elapsed_secs <<"s.\n";
			begin = end;
		}

		/// Get the list of holidays to skip
		vector<int> indexOfDaysToSkip;
		for(size_t i=0; i < m_holidays.size(); i++){
			tm timeinfo = m_holidays[i];
			timeinfo.tm_year = 2010 - 1900;
			mktime ( &timeinfo );
			/// The tm yday start from 0; (The day of year of 2015-1-1 is 0 in tm)
			if((timeinfo.tm_yday + 1 >= m_simulationSettings.getStartDayofYear()) && (timeinfo.tm_yday + 1 <= m_simulationSettings.getEndDayofYear())){
				indexOfDaysToSkip.push_back(timeinfo.tm_yday + 1 - m_simulationSettings.getStartDayofYear());
			}
		}


		/// Initalize the output file, write the header line
		string fileNameByOccu = fileName + "_by_Occupant.csv";
		string fileNameByRoom = fileName + ".csv";
		string fileNameIDFCSV = fileName + "_IDF.csv";
		string fileNameIDF = fileName + "_IDF.idf";
		vector<int> roomSchedule;
		vector<int> roomMaxSchedule;
		int roomScheduleSize = 0;
		for (size_t i = 0; i < m_buildings.size(); i++) {
			roomScheduleSize += m_buildings[i]->getSpaces().size();
		}
		for (int i = 0; i < roomScheduleSize; i++) {
			roomSchedule.push_back(-1);
			roomMaxSchedule.push_back(0);
		}
		///Schedule file by room
		ofstream file_byOccu;                          
		file_byOccu.open (fileNameByOccu, std::ofstream::out | std::ofstream::trunc);
		///Schedule file by room
		ofstream file_byRoom;
		file_byRoom.open (fileNameByRoom, std::ofstream::out | std::ofstream::trunc);
		/// The file for IDF
		ofstream pForIDFout;
		pForIDFout.open (fileNameIDFCSV, std::ofstream::out | std::ofstream::trunc);
		/// The IDF file
		ofstream pIDFout;
		pIDFout.open (fileNameIDF, std::ofstream::out | std::ofstream::trunc);
		writeOutPutFiles(file_byOccu, file_byRoom, pForIDFout, pIDFout);
		ofstream meetingOut;
		vector<MeetingEvent*> meetingEvents;
		for (size_t i = 0; i < m_buildings.size(); i++) {
			for (size_t j = 0; j < m_buildings[i]->getSpaces().size(); j++) {
				vector<MeetingEvent*> newMeetingEvents = m_buildings[i]->getSpaces()[j]->getMeetingEvents();             
				for (size_t k = 0; k < newMeetingEvents.size(); k++) {
					meetingEvents.push_back(newMeetingEvents[k]);
				}
			}
		}
		

		for (size_t i = 0; i < meetingEvents.size(); i++) {
			/// Generate meetings based on day of week and season
			if (isDebugMode) {
				string fileName = "Meeting_stats_"; 
				char c = '0' + i;
				fileName += string(1, c);
				fileName += ".csv";
				meetingOut.open (fileName, std::ofstream::out | std::ofstream::trunc);
				meetingOut << "Day, number of meetings, start time, duration, number of occupants\n";
			}
		}

		

		/// For each day, determine the date type
		int numOfStepPerHour = m_simulationSettings.getNumberofTimestepsPerHour();
		int totalSteps = numOfStepPerHour * 24;
		int totalDayNum = m_simulationSettings.getTotalNumberofDays();
		int curDayNum = m_simulationSettings.getStartDayofYear();
		for (int day = 0; day < totalDayNum; day++) {
		    std::cout << "  Start simulation day: " << day <<  ".\n";

			//Skip if holiday
			bool isHoliday = false;
			for (size_t i = 0; i < indexOfDaysToSkip.size(); i++) {
				if (day == indexOfDaysToSkip[i]) {
					isHoliday = true;
				}
			}

			//Init daily schedules based on last status
			for (size_t i = 0; i < m_occupants.size(); i++) {
				m_occupants[i]->initOccupantSchedules(totalSteps);
			}
			
			if (!isHoliday) {
				begin = clock();
				/// Update occupancy status (1/0) of each occupant
				for (size_t i = 0; i < m_occupants.size(); i++) {
					if (!m_occupants[i]->simOccupancyStatus(day, error)) {
						error += "Fail to simulate occupancy schedule of ";
						error += m_occupants[i]->getID();
						error += " .\n";
						return false;
					}
				}
				end = clock();
				if (isDebugMode) {
					elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
					std::cout<< "     Simulating status done with " << elapsed_secs <<"s.\n";
					begin = end;
				}
				begin = end;
				///simulate meetings
				if (!simulateMeetings(day, meetingOut)) {
					error += "Fail to generate meetings.\n";
					return false;
				}
				if (isDebugMode) {
					elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
					std::cout<< "     Simulating meeting done with " << elapsed_secs <<"s.\n";
					begin = end;
				}

				begin = end;
				/// Update occupancy location of each occupant
				for (size_t i = 0; i < m_occupants.size(); i++) {
					if (!m_occupants[i]->simOccupantLocation(day, error)) {
						error += "Fail to simulate occupancy schedule of ";
						error += m_occupants[i]->getID();
						error += " .\n";
						return false;
					}
				}
				end = clock();
				if (isDebugMode) {
					elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
					std::cout<< "     Simulating location done with " << elapsed_secs <<"s.\n";
					begin = end;
				}
			}

			///Write schedule for each day
			int curMon, curDay, hour, min;
			if (file_byOccu.is_open() && file_byRoom.is_open()) {
				for (int i = 0; i < totalSteps; i++) {
					int curTotal = 0;
					getMonDay(&curMon, &curDay, curDayNum);
					getHourMin(i, numOfStepPerHour, hour, min);
                    
					file_byOccu << day * totalSteps + i << ", " << curMon << "/" << curDay << " " << hour << ":" <<  min << ":00, ";
					file_byRoom << day * totalSteps + i << ", " << curMon << "/" << curDay << " " <<  hour << ":" <<  min << ":00, ";
                    
					roomSchedule.clear();
					roomSchedule.resize(roomScheduleSize);
					
                    for (size_t k = 0; k < m_occupants.size(); k++) {
						int curLocation =  m_occupants[k]->getLocationAt(i);
						if (curLocation >= 0) {
							roomSchedule[curLocation]++;
							curTotal++;
						}
						file_byOccu << curLocation << ",";
					}

					file_byOccu << "\n";
					for (size_t k = 1; k < roomSchedule.size(); k++) {
						file_byRoom << roomSchedule[k] << ",";
						if (roomSchedule[k] >= roomMaxSchedule[k]) {
							roomMaxSchedule[k] = roomSchedule[k];
						}
					}

					file_byRoom << curTotal << ",";
					file_byRoom << "\n";
				}
			}

			else cout << "Unable to open output files";
			end = clock();
			elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
			std::cout<< "     Writing files done with " << elapsed_secs <<"s.\n";

			curDayNum++;
			
		}
		file_byOccu.close();
		file_byRoom.close();
		
		
		writeForIDFout(pForIDFout, fileNameByRoom, fileNameIDFCSV, roomMaxSchedule);
		writeIDF(pIDFout, fileNameIDFCSV, roomMaxSchedule);
		
		return true;
	}

	///Write CSV for IDF
	bool OccupantBehavior::writeForIDFout(ofstream& pForIDFout, string fileNameByRoom, string fileNameIDFCSV, vector<int> maxSchedule) {

		std::ifstream file(fileNameByRoom);

		CSVRow row;
		int line = 0;
		while(file >> row)
		{
			if (line > 0) {
				pForIDFout << row[0];
				pForIDFout << ",";
				pForIDFout << row[1];
				pForIDFout << ",";
				for (size_t i = 2; i <row.size() - 1; i++) {
					int occupantNum = stoi(row[i]);
					if (maxSchedule[i - 1] == 0) {
						pForIDFout << 0;
					} else {
						pForIDFout << (double)occupantNum / maxSchedule[i - 1];
					}
					pForIDFout << ",";
				}
				pForIDFout << "\n";
			}
			line++;
		}
		pForIDFout.close();
		file.close();
		return true;

	}

	///Write IDF
	bool OccupantBehavior::writeIDF(ofstream& pIDFout, string csvName, vector<int> maxSchedule) {

		vector<Space*> m_roomSetVec;

		for (size_t i = 0; i < m_buildings.size(); i++) {
			for (size_t j = 0; j < m_buildings[i]->getSpaces().size(); j++) {
				m_roomSetVec.push_back(m_buildings[i]->getSpaces()[j]);
			}
		}
		int totalStep = m_simulationSettings.getNumberofTimestepsPerHour();
		int minPerStep = 60 / totalStep;
		int totalHour = m_simulationSettings.getTotalNumberofTimeSteps() / totalStep;
		string defaultActivity = "";
		defaultActivity += "Schedule:Compact,\n";
		defaultActivity += "obFMU Activity Schedule, !- Name\n";
		defaultActivity += "obFMU Any Number,        !- Schedule Type Limits Name\n";
		defaultActivity += "Through: 12/31,          !- Field 1\n";
		defaultActivity += "For: AllDays,            !- Field 2\n";
		defaultActivity += "Until: 24:00,            !- Field 3\n";
		defaultActivity += "110.7;                   !- Field 4\n\n";

		defaultActivity += "ScheduleTypeLimits,\n";
		defaultActivity += "obFMU Fraction,          !- Name\n";
		defaultActivity += "0.0,                     !- Lower Limit Value\n";
		defaultActivity += "1.0,                     !- Upper Limit Value\n";
		defaultActivity += "CONTINUOUS;              !- Numeric Type\n\n";

		defaultActivity += "ScheduleTypeLimits,\n";
		defaultActivity += "obFMU Any Number;        !- Name\n\n";
	
		pIDFout << defaultActivity;
		for(size_t i = 0; i < m_roomSetVec.size(); i++){
			Space::SpaceType type = m_roomSetVec[i]->getSpaceType();
			if(type != Space::Space_Outdoor){
				/// Write the People object
				string item= "";
				item += "Zone,\n";
				item += m_roomSetVec[i]->getID();
				item += ";                   !- Name\n";
				item += "People,\n";
				item += m_roomSetVec[i]->getID();
				item += " People,            !- Name\n";
				item += m_roomSetVec[i]->getID();
				item += ",                   !- Zone or ZoneList Name\n";
				item += m_roomSetVec[i]->getID();
				item += " People Schedule,   !- Number of People Schedule Name\n";
				item += "People,                  !- Number of People Calculation Method\n";
				item += std::to_string((uint64_t)(maxSchedule[i]));
				item += ",                       !- Number of People\n";
				item += ",                        !- People per Zone Floor Area {person/m2}\n";
				item += ",                        !- Zone Floor Area per Person {m2/person}\n";
				item += "0.3,                     !- Fraction Radiant\n";
				item += "AUTOCALCULATE,           !- Sensible Heat Fraction\n";
				item += "obFMU Activity Schedule; !- Activity Level Schedule Name\n\n";

				item += "Schedule:File,\n";
				item += m_roomSetVec[i]->getID();
				item += " People Schedule,   !- Name\n";
				item += "obFMU Fraction,          !- Schedule Type Limits Name\n";
				item += csvName;
				item += ",    !- File Name\n";
				item += std::to_string((uint64_t)(i+3));
				item += ",                       !- Column Number\n";
				item += "7,                       !- Rows to Skip at Top\n";
				item += std::to_string((uint64_t)(totalHour));
				item += ",                     !- Number of Hours of Data\n";
				item += "Comma,                   !- Column Separator\n";
				item += "No,                      !- Interpolate to Timestep\n";
				item += std::to_string((uint64_t)(minPerStep));
				item += ";                      !- Minutes per Item\n\n";

				pIDFout << item;
			}
		}
		pIDFout.close();
		return true;
	}

	bool OccupantBehavior::simulateMeetings(int curDayNum, ofstream& meetingOut) {
		bool isDebugMode = m_simulationSettings.isDebugMode();
		vector<MeetingEvent*> meetingEvents;
		for (size_t i = 0; i < m_buildings.size(); i++) {
			for (size_t j = 0; j < m_buildings[i]->getSpaces().size(); j++) {
				vector<MeetingEvent*> newMeetingEvents = m_buildings[i]->getSpaces()[j]->getMeetingEvents();             
				for (size_t k = 0; k < newMeetingEvents.size(); k++) {
					meetingEvents.push_back(newMeetingEvents[k]);
				}
			}
		}
		

		for (size_t i = 0; i < meetingEvents.size(); i++) {
			/// Generate meetings based on day of week and season
			if (isDebugMode) {
				string fileName = "Meeting_stats_"; 
				char c = '0' + i;
				fileName += string(1, c);
				fileName += ".csv";
				meetingOut.open (fileName, std::ofstream::out | std::ofstream::app);
			}
			if (meetingEvents[i]->checkDayFlag(curDayNum)) {
				int numOfMeetings = meetingEvents[i]->getNumOfMeetingsPerDay();
				if (isDebugMode) meetingOut << curDayNum << ","  << numOfMeetings << " ,";
				for (int j = 0; j < numOfMeetings; j++) {
					int locationId = meetingEvents[i]->getLocationId();
					int numOfOccupants = meetingEvents[i]->getNumOfOccupants();
					int duration = meetingEvents[i]->getDuration();
					int startTime;
					if (!drawMeeting(numOfOccupants, duration, locationId, curDayNum, startTime)) {
						if (isDebugMode) {
							meetingOut << "Unable to draw meeting of space " << locationId << " on day " << curDayNum <<"\n" ;
							cout << "Unable to draw meeting of space " << locationId << " on day " << curDayNum <<"\n" ;
						}
					} else {
						if (isDebugMode) {
							if (j != 0) {
								meetingOut << curDayNum << ","  << ",";
							}
							meetingOut << startTime << ","  << duration << "," << numOfOccupants <<"\n" ;
						}
					}
				}
			}
			meetingOut.close();
		}
		
		return true;

	}

	///Draw meetings
	///Paras:
	/// - number of people, duration required; meeting room location id to locate the people; curDay: day number to decide occupant's meeting percentage
	bool OccupantBehavior::drawMeeting(int numOfOccupants, int duration, int locationId, int curDay, int& startTime) {
		int durationInStep = int(((double)duration / 60) * m_simulationSettings.getNumberofTimestepsPerHour());
		int totalSteps = m_simulationSettings.getNumberofTimestepsPerHour() * 24;
		int checksStep = int(0.5 * m_simulationSettings.getNumberofTimestepsPerHour()); // check every half hour
		vector<string> occupants;
		int count;
		vector<int> availableStartTime;
		// loop through every half hour
		for (int t = 0; t < totalSteps - durationInStep; t += checksStep) {
			count = 0;
			bool onMeeting = false;

			for (size_t i = 0; i < m_occupants.size(); i++) {
				bool available = true;
				for (int j = t; j < t + durationInStep; j++) {
					///Not available
					if (m_occupants[i]->getStatusAt(j) == 0) {
						available = false;
					}
					//Exceed occupant's maximum meeting percentage
					double maxMeetingTime = m_occupants[i]->getMaxMeetingTimePercent(curDay);
					if (((double)m_occupants[i]->getCurMeetingTimeInDay() / totalSteps) > maxMeetingTime) {
						//cout << "Occupant " << m_occupants[i]->getID() << " attends too many meetings today!" <<"\n" ;
						available = false;
					}
					if (m_occupants[i]->getLocationAt(j) == locationId) {
						onMeeting = true;
						break;
					}
				}
				if (available) {
					count++;
				}
			}
			if (count >= numOfOccupants && (!onMeeting)) {
				availableStartTime.push_back(t);
			}
		}
		//Random draw a start time from all available time periods
		if (availableStartTime.size() > 0) {
			int randomIndex = rand() % availableStartTime.size();
			startTime = availableStartTime.at(randomIndex);
		

			occupants.clear();
			count = 0;
			for (size_t i = 0; i < m_occupants.size(); i++) {
				bool available = true;
				for (int j = startTime; j < startTime + durationInStep; j++) {
					///Not available
					if (m_occupants[i]->getStatusAt(j) == 0) {
						available = false;
						break;
					}
				}
				if (available) {
					occupants.push_back(m_occupants[i]->getID());
					count++;
				}
			}
			//Random draw occupants and locate the occupants to meeting room
			std::random_shuffle(occupants.begin(), occupants.end());
			for (int i = 0; i < numOfOccupants; i++) {
				for (size_t j = 0; j < m_occupants.size(); j++) {
					if (m_occupants[j]->getID().compare(occupants[i]) == 0) {
						m_occupants[j]->setMeetingTime(m_occupants[j]->getCurMeetingTimeInDay() + durationInStep);
						for (int k = startTime; k < startTime + durationInStep; k++) {
							m_occupants[j]->setLocationAt(k, locationId);
							m_occupants[j]->setStatusAt(k, 0);
						}
						break;
					}
				}
			}
			return true;
		} 
		return false;
	}

	// Create flag
	bool OccupantBehavior::setMoveCalcFileFlag() {
        
        string szWorkDir = getCurrentDir();
		string fName = szWorkDir;
		fName += m_fFlagName ;
		std::ifstream file(fName);
		file.open (fName, std::ofstream::out | std::ofstream::trunc);
		if(!file.is_open()) {
			return false;
		}
		file.close();
		return true;
	}

	//Delete flag
	bool OccupantBehavior::deleteMoveCalcFileFlag()
	{
		bool bSuc = false;
        
        string szWorkDir = getCurrentDir();
		string fName = szWorkDir;
		fName += m_fFlagName ;
		std::ifstream file(fName);
		if(file.good()) {
			file.close();
#ifdef _WIN
			DeleteFile(fName.c_str());
#else
            remove(fName.c_str());
#endif
			bSuc = true;
			if(file.good()) { 
				std::cout << "File not deleted...\n"; 
			}
		}
		return bSuc;
	}

	//Find flag
	bool OccupantBehavior::isMoveCalcFileFlag()
	{
        
        string szWorkDir = getCurrentDir();
		string fName = szWorkDir;
		fName += m_fFlagName ; 
		std::ifstream file(fName);
		return file.good();
	}


};
