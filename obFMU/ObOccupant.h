/* ----------------------------------------------------------------------------
** The object represents the Occupant node in obXML schema.   
** Author: Yixing Chen
** History:
** 6/30/2015: first release
** -----------------------------------------------------------------------------*/
#ifndef ObOccupant_h
#define ObOccupant_h

#include "ObObject.h"
#include "ObBehavior.h"
#include "ObMovementBehavior.h"
#include "ObRandomMovementEvent.h"


namespace OB{
	class Occupant: public Object {
	public:
		

		enum GenderType{
			Gender_Male,
			Gender_Female
		};

		Occupant(Object* parent =NULL);
		~Occupant();
		Occupant(const Occupant& other);
		Occupant& operator= (const Occupant&);

		/// initialize by reading an TiXmlElement
		/// params:
		/// - element: the TiXMLElement
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		bool	readObXML(TiXmlElement* element, std::string& error);

		/// Get the behavior IDs
		vector<string>	getBehaviorIDs(); 

		/// Get the behavior IDs
		vector<string>	getMovementBehaviorIDs(); 

		/// Add Behavior to the list
		void	addBehavior(Behavior* obj);                                    /// Suggested: addBehavior(int behaviorId) - decoupling   

		/// Add Movement Behavior to the list                                  /// TODO: READ MOVEBEHAVIOR
		void	addMoveBehavior(MovementBehavior* obj);                                /// Suggested: addMovementBehavior(int behaviorId);     

		/// Perform interaction
		/// Params:
		/// - numberofTimeStep: the number of current time step
		/// - systemType: which system, window, HVAC, or lights
		/// - interactionType: turn on or turn off
		/// - results: the output; 1 for perform the action (turn on or turn off), 0 for not perform the action.
		/// - error: the error information if return false
		/// Return:
		/// - Return false when fail the performn the interaction
		bool	performInteraction(int numberofTimeStep, Behavior::SystemType systemType, 
			Behavior::InteractionType interactionType, int spaceSystemControlType, 
			vector<Behavior::EventType> eventTypes, bool isSpaceEmptyNextTimeStep, 
			map<Parameter::ParameterType,double> parameterValues, double& result, 
			string& error, double& controlValue,bool isSpecialBehavior);

		/// Get the priority, the larger value means higher priority
		int		getPriority();


		/// Allocate memories for daily occupant schedule
		/// Params:
		/// - simulationSteps: total number of time steps
		void    initOccupantSchedules(int simulationSteps);

		/// Simulate occupancy status of a day
		/// Params:
		/// - dayNum: day number
		/// Return:
		/// - if successfully perform simulation
		bool    simOccupancyStatus(int dayNum, string error);

		/// Simulate occupancy location of a day
		/// Params:
		/// - dayNum: day number
		/// Return:
		/// - if successfully perform simulation
		bool    simOccupantLocation(int dayNum, string error);

		
		/// Calculate proposed meeting time of a day
		/// Params:
		/// - numberOfDays: day number
		/// Return:
		/// - if successfully perform calculation
		bool    initDailyMeetingPercent(int numberOfDay, string error);

		/// Set last status for the previous day
		/// Params:
		/// - status: 1/0
		void setlLastStepStatus(int status);
		

		/// Params:
		/// - i: time step in a day
		int getStatusAt(int i);

		int getLocationAt(int i);

		void setStatusAt(int i, int status);

		void setLocationAt(int i, int locationId);

		/// Set own office space ID
		/// Params:
		/// - i: time step in a day
		void setOwnLocationId(int id);

		/// Get own office space ID
		int getLocationId();

		/// Set scheduled meeting time
		void setMeetingTime(int meetingTime);

		/// Get scheduled meeting time for checking
		int getCurMeetingTimeInDay();
		
		/// Get proposed meeting time percentage for checking
		double getMaxMeetingTimePercent(int day);

	private:
		string				m_name;
		double				m_age;
		GenderType			m_gender;
		string				m_lifeStyle;
		string				m_jobType;
		int					m_priority;

		int					m_ownLocationID;

		vector<string>		m_movementBehaviorIDs;
		vector<string>		m_behaviorIDs;
		vector<Behavior*>	m_behaviors; 
		vector<MovementBehavior*>   m_moveBehaviors;                                  

		/// TODO: use -1 to represent Home/Default outdoor
		std::vector<int>    m_statusSchedule;         /// Home or Office                           
		std::vector<int>    m_locationSchedule;		  /// Office location schedule 	
		
		/// TODO: all the class parameter should start with m_
		int					m_lastStepStatus;	/// TODO: Initlaized at the global initalization
		int					curMeetingTimeInDay;

		vector<double>		m_dailyMeetingPercentVec;
	};
};

#endif