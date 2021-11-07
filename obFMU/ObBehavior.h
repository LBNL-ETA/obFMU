/* ----------------------------------------------------------------------------
** The object represents the Behavior node in obXML schema.   
** Author: Yixing Chen
** History:
** 6/30/2015: first release
** -----------------------------------------------------------------------------*/
#ifndef OBBEHAVIOR_H
#define OBBEHAVIOR_H

#include "ObObject.h"
#include "ObSeason.h"
#include "ObTimeofDay.h"
#include "ObParameter.h"
#include "ObParameterRange.h"
#include "ObDayofWeek.h"
#include "ObStatusTransitionEvent.h"

using namespace std;

namespace OB{
	class Behavior: public Object {
	public:
		enum SystemType{
			System_HVAC,
			System_Lights,
			System_Windows,
			System_PlugLoad,
			System_Thermostat,
			System_ShadeAndBlind,
			System_NONE
		};

		enum InteractionType{
			Interaction_TurnOn,
			Interaction_TurnOff,
			Interaction_SetToControlValue,
			Interaction_None
		};

		enum ActionType{
			Action_Interaction,
			Action_Movement,
			Action_None
		};

		enum FormulaType{
			Formula_ConstantValue,
			Formula_Linear1D,
			Formula_Linear2D,
			Formula_Linear3D,
			Formula_Quadratic1D,
			Formula_Logit1D,
			Formula_Logit2D,
			Formula_Logit3D,
			Formula_Weibull1D,
			Formula_Logit1DQuadratic
		};

		enum EventType{
			Event_LeavingRoom,
			Event_EnteringRoom,
			Event_StayingInRoom,
			Event_LeavingRoomMoreThan1Hour,
			Event_LeavingRoomMoreThan6Hours
		};

		enum OtherConstraintType{
			OtherConstraint_NoOccupantsInRoom
		};

		Behavior(Object* parent =NULL);
		~Behavior();
		Behavior(const Behavior& other);
		Behavior& operator= (const Behavior&);
	   
		/// initialize by reading an TiXmlElement
		/// params:
		/// - element: the TiXMLElement
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		bool	readObXML(TiXmlElement* element, std::string& error);

		/// connect all the reference IDs with their objects.
		/// Return
		/// - return false when can't find objects with the given ID.
		bool	connectIDswithObjects(string& error);		

		vector<Season::SeasonType>				getSeasonTypes();
		void									addSeasion(Season* obj);

		vector<TimeofDay::TimeofDayType>		getTimeofDayTypes();
		void									addTimeofDay(TimeofDay* obj);

		vector<DayofWeek::DayofWeekType>		getDayofWeekTypes();
		void									addDayofWeek(DayofWeek* obj);

		SystemType								getSystemType();
		int										getAvailableSystemType();

		ActionType								getActionType();

		/// The available system IDs are not used yet.
		vector<string>							getAvailableSystemIDs();
		void									addAvailableSystem(Object* obj);

		/// Perform interaction
		/// Params:
		/// - numberofTimeStep: the number of current time step
		/// - systemType: which system, window, HVAC, or lights
		/// - interactionType: turn on or turn off
		/// - results: the output; 1 for perform the action (turn on or turn off), 0 for not perform the action.
		/// - error: the error information if return false
		/// Return:
		/// - Return false when fail the performn the interaction
		bool	performInteraction(int numberofTimeStep, Behavior::SystemType systemType, Behavior::InteractionType interactionType, 
			int spaceSystemControlType,  vector<Behavior::EventType> eventTypes, bool isSpaceEmptyNextTimeStep, 
			map<Parameter::ParameterType,double> parameterValues, double& result, string& error, bool isSpecialBehavior);
                                                                    

		double	getControlValue();

		void	setTimeStep(int dt);

	private:

		/// Read the Drivers element
		bool	readDrivers(TiXmlElement *element, std::string& error);

		/// Read the Needs element
		bool	readNeeds(TiXmlElement *element, std::string& error);

		/// Read the Actions element
		bool	readActions(TiXmlElement *element, std::string& error);

		/// Read the Systems element
		bool	readSystems(TiXmlElement *element, std::string& error);
		
		/// Read the Time element
		bool	readTime(TiXmlElement *element, std::string& error);

		/// Read the Environment element
		bool	readEnvironment(TiXmlElement *element, std::string& error);

		/// Read the Physical element
		bool	readPhysical(TiXmlElement *element, std::string& error);

		/// Read the Thermal element
		bool	readThermal(TiXmlElement *element, std::string& error);

		/// Read the parent element of ParameterRange
		bool	readParameterRanges(TiXmlElement *element, std::string& error);

		/// Read the Windows elemeent
		bool	readWindows(TiXmlElement *element, std::string& error);

		/// Read the Lights element
		bool	readLights(TiXmlElement *element, std::string& error);
		
		/// Read the HVAC element
		bool	readHVAC(TiXmlElement *element, std::string& error);

		/// Read the PlugLoad element
		bool	readPlugLoad(TiXmlElement *element, std::string& error);

		/// Read the Thermostat element
		bool	readThermostat(TiXmlElement *element, std::string& error);

		/// Read the ShadeAndBlind element
		bool	readShadeAndBlind(TiXmlElement *element, std::string& error);
		
		/// Read the Interaction element
		bool	readInteraction(TiXmlElement *element, std::string& error);

		/// Read the Formula element
		bool	readFormula(TiXmlElement *element, std::string& error);

		/// Read the child of Formula element
		bool	readFormulaChild(TiXmlElement *element, std::string& error, FormulaType type);

		Parameter* getParameter(string ID);

		bool	checkSeason(int numberofTimeStep);
		bool	checkTimeofDay(int numberofTimeStep);                               


	private:
		/// Obtained from Drivers node
		vector<Season::SeasonType>			m_seasonTypes;			/// The season Types
		vector<Season*>						m_seasons;				/// The season constrain
		vector<TimeofDay::TimeofDayType>	m_timeofDayTypes;		/// The time of day type
		vector<TimeofDay*>					m_timeofDays;			/// The time of week constrain
		vector<DayofWeek::DayofWeekType>	m_dayofWeekTypes;		/// The day of week constrain
		vector<DayofWeek*>					m_dayofWeeks;			/// The day of weeks

		vector<EventType>					m_eventTypes;			/// The event types constrain
		vector<OtherConstraintType>			m_otherConstraintTypes;	/// The other constraint types

		vector<Parameter*>					m_environmentParameters;			/// The list of parameters

		/// Obtained from Needs node
		vector<ParameterRange*>             m_needsParameterRanges;				/// The needs


		/// Obtained from Actions node
		ActionType					m_actionType;			/// The type of action
		InteractionType				m_interactionType;		/// The type of interaction;
		double						m_interactionControlValue;	/// The control value used in SetToControlValue Type

		int							m_dt; //// number of minutes per time step

		FormulaType					m_formulaType;			/// The function type
		double						m_coefficientA;			/// The coefficient A
		double						m_coefficientB;			/// The coefficient B
		double						m_coefficientC;			/// The coefficient C
		double						m_coefficientD;			/// The coefficient D
		string						m_parameter1ID;			/// Use the parameter1IDRef to search the parameter in driver
		string						m_parameter2ID;			/// Use the parameter2IDRef to search the parameter in driver
		string						m_parameter3ID;			/// Use the parameter3IDRef to search the parameter in driver
		Parameter*					m_parameter1;
		Parameter*					m_parameter2;
		Parameter*					m_parameter3;

		/// is special behavior
		bool				m_isSpecialBehavior;
		
		/// Obtained from systems node
		SystemType			m_systemType;
		int					m_availableSystemType;  /// Window type, light type, or hvac type; -1 means not system type constrains

		vector<string>		m_availableSystemIDs;   /// The list of available conpoments
		vector<Object*>		m_availableSystems;		/// The available System objects

		/// The simulation 
		vector<StatusTransitionEvent*>		m_moveEvents;			/// Movement events

	};
};
#endif //OBBEHAVIOR_H