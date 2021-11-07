/* ----------------------------------------------------------------------------
** The object represents the Space node in obXML schema.   
** Author: Yixing Chen
** History:
** 6/30/2015: first release
** -----------------------------------------------------------------------------*/
#ifndef ObSpace_h
#define ObSpace_h

#include "ObObject.h"
#include "ObMeetingEvent.h"
#include "ObOccupant.h"
#include "ObBehavior.h"
using namespace std;

namespace OB{
	class Space: public Object {
	public:
		enum SpaceType{
			Space_ResidentialOwn,
			Space_ResidentialRent,
			Space_OfficeShared,
			Space_OfficePrivate,
			Space_Office,
			Space_MeetingRoom,
			Space_Corridor,
			Space_Outdoor,
			Space_Other
		};

		enum WindowType{
			Window_Operable,
			Window_Fixed,
			Window_NONE
		};

		enum LightType{
			Light_OnOff,
			Light_ContinuousControl,
			Light_NONE
		};

		enum HVACType{
			HVAC_ZoneOnOff,
			HVAC_NONE
		};

		enum PlugLoadType{
			PlugLoad_OnOff,
			PlugLoad_ContinuousControl,
			PlugLoad_NONE
		};

		enum ThermostatType{
			Thermostat_Adjustable,
			Thermostat_NONE
		};

		enum ShadeAndBlindType{
			ShadeAndBlind_Operable,
			ShadeAndBlind_NONE
		};

		enum GroupPriorityType{
			GroupPriority_Majority
		};

		enum OutputParameterType {
			Output_HVAC_Schedule,
			Output_Light_Schedule,
			Output_Infiltration_Schedule,
			Output_PlugLoad_Schedule,
			Output_Thermostat_Schedule,
			Output_ShadeAndBlind_Schedule,
			Output_Occupant_Schedule      /// Number of Occupnats
		};

		Space(Object* parent = NULL);
		~Space();
		Space(const Space& other);
		Space& operator= (const Space&);

		/// initialize by reading an TiXmlElement
		/// params:
		/// - element: the TiXMLElement
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		bool	readObXML(TiXmlElement* element, std::string& error);
		
		/// Read the Systems element
		bool	readSystems(TiXmlElement *element, std::string& error);

		
		/// Read the Window element
		bool	readWindow(TiXmlElement *element, std::string& error);

		
		/// Read the Light element
		bool	readLight(TiXmlElement *element, std::string& error);

		
		/// Read the HVAC element
		bool	readHVAC(TiXmlElement *element, std::string& error);

		/// Read the PlugLoad element
		bool	readPlugLoad(TiXmlElement *element, std::string& error);

		/// Read the Thermostat element
		bool	readThermostat(TiXmlElement *element, std::string& error);

		/// Read the Shade and Blind element
		bool	readShadeAndBlind(TiXmlElement *element, std::string& error);

		bool    performnOnOff(size_t numberofTimeStep, OutputParameterType outputType, map<Parameter::ParameterType, 
			double> parameterValues, map<Space::OutputParameterType,double>& outputValues, string& error, FILE* logFile);

		bool    performnSetToControlValue(size_t numberofTimeStep, OutputParameterType outputType, map<Parameter::ParameterType, 
			double> parameterValues, map<Space::OutputParameterType,double>& outputValues, string& error, FILE* logFile);

		/// Calculate the output
		bool	performCalculation(size_t numberofTimeStep, map<Parameter::ParameterType,double> parameterValues, 
			map<Space::OutputParameterType,double>& outputValues, string& error, FILE* logFile);

		/// Perform interaction
		/// Params:
		/// - numberofTimeStep: the number of current time step
		/// - systemType: which system, window, HVAC, or lights
		/// - interactionType: turn on or turn off
		/// - results: the output; 1 for perform the action (turn on or turn off), 0 for not perform the action.
		/// - error: the error information if return false
		/// Return:
		/// - Return false when fail the performn the interaction
		bool	performInteraction(size_t numberofTimeStep, Behavior::SystemType systemType, Behavior::InteractionType interactionType, 
			map<Parameter::ParameterType,double> parameterValues, double& result, string& error, double& controlValue, 
			bool isSpecialBehavior, bool& foundSpecialBehavior);

		SpaceType				getSpaceType();
		/// Get the list of occupant IDs
		vector<string>			getOccupantIDs();

		/// Add the occupant to the list
		void					addOccupant(Occupant* obj);

		void					clearRunTimeOccupants();
		void					initRunTimeOccupnatsForOneTimeStep();
		void					addRunTimeOccupnat(Occupant* occupant);
		int						getRunTimeOccupantNumber(int timeStep);
		vector<Occupant*>		getOccupants();

		vector<MeetingEvent*>   getMeetingEvents();

		void					initSpaceSchedules(int simulationSteps);

		void					setLocationId(int id);
		int						getLocationId();                                                                 

		void					setSimulationNumberofTimestepsPerHour(int value);
		void                    setMaxNumOccupants(int num);
		int                     getMaxNumOccupants();

	private:
		int						m_locationIdInBuilding;
		SpaceType				m_type;
		int						m_maxNumOccupants;
		int						m_minNumOccupants;
		WindowType				m_windowType;
		LightType				m_lightType;
		HVACType				m_hvacType;
		PlugLoadType			m_plugLoadType;
		ThermostatType			m_thermostatType;
		ShadeAndBlindType		m_shadeAndBlindType;

		GroupPriorityType		m_groupPriority;  /// Use for group control event

		vector<MeetingEvent*>	m_meetingEvents;
		vector<string>			m_occupantIDs;
		vector<Occupant*>		m_occupants;

		map<int, string>		m_locationIdMap;

		/// The list of occupants at each time step
		vector<vector<Occupant*> >	m_runTimeOccupants;
		int							m_currentMoveTimeStep;
		int							m_simulationNumberofTimestepsPerHour;
		int                         m_maxNumOfOccupants;

	    vector<string>					OUTPUT_VARIABLE_NAMES;
	    vector<Behavior::SystemType>	OUTPUT_SYSTEM_NAMES;

		std::vector<int>		m_meetingSchedule;                                     /// TODO: SIZE BY TIMESTEP
	};
};

#endif
