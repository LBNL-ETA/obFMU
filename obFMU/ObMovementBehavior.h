/* ----------------------------------------------------------------------------
** The object represents the Movement Behavior node in obXML schema.   
** Author: Xuan Luo
** -----------------------------------------------------------------------------*/
#ifndef OBMOVEMENTBEHAVIOR_H
#define OBMOVEMENTBEHAVIOR_H

#include "ObObject.h"
#include "ObSeason.h"
#include "ObDayofWeek.h"
#include "ObTimeofDay.h"
#include "ObStatusTransitionEvent.h"
#include "ObRandomMovementEvent.h"
#include "ObMoveMatrixModel.h"

using namespace std;

namespace OB{
	class MovementBehavior: public Object {
	public:
		
		MovementBehavior(Object* parent =NULL);
		~MovementBehavior();
		MovementBehavior(const MovementBehavior& other);
		MovementBehavior& operator= (const MovementBehavior&);
	   
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

		vector<DayofWeek::DayofWeekType>		getDayofWeekTypes();
		void									addDayofWeek(DayofWeek* obj);

		void                                    initEvents(SimulationSettings* settings, std::string& error);        
		void                                    initLocationCategory(vector<vector<int>> locationIdMap);
		

		/// Generate events and simulate present time
		/// Return:
		/// - if successfully generated
		bool	                                simStatusSchedule(std::vector<int> &m_statusSchedule);       

		int                                     movementNumOnDay(int dayNum); 
		double                                  getMeetingPercent(); 
		RandomMovementEvent*                    getRandomMoveModel();

		
		void    calculateMatrice();

		void	setTimeStep(int dt);


	private:

		bool	checkSeason(int numberofTimeStep);

		bool	checkTimeofDay(int numberofTimeStep);

		    

		
                            

		bool	getTimeInDay(string time, int& hour, int& min, string& error);



		
	private:
		
		/// Obtained from Drivers node
		vector<Season::SeasonType>			m_seasonTypes;			/// The season Types
		vector<Season*>						m_seasons;				/// The season constrain

		vector<DayofWeek::DayofWeekType>	m_dayofWeekTypes;		/// The day of week constrain
		vector<DayofWeek*>					m_dayofWeeks;			/// The day of weeks
		int							        m_dt; //// number of minutes per time step

		/// The simulation
		int							        m_originalLocationId;
		vector<StatusTransitionEvent*>		m_transEvents; 			/// Movement events

		
		
		RandomMovementEvent*	            m_randomMovementModel;
		vector<int>				            m_movementModelVec;
	};
};
#endif 