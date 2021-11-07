/* ----------------------------------------------------------------------------
** The object represents the MeetingEvent node in obXML schema.   
** Author: Xuan Luo
** -----------------------------------------------------------------------------*/
#ifndef ObMeetingEvent_h
#define ObMeetingEvent_h

#include "ObObject.h"
#include "ObSeason.h"
#include "ObDayofWeek.h"
using namespace std;

namespace OB{
	class MeetingEvent: public Object {
	public:
		MeetingEvent(Object* parent = NULL);
		~MeetingEvent();
		MeetingEvent(const MeetingEvent& other);
		MeetingEvent& operator= (const MeetingEvent&);
		/// initialize by reading an obXML file
		/// params:
		/// - filename: the filename of the obXML file including the path
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		bool readObXML(TiXmlElement* element, std::string& error);
		vector<Season::SeasonType>				getSeasonTypes();
		void									addSeasion(Season* obj);
		vector<DayofWeek::DayofWeekType>		getDayofWeekTypes();
		void									addDayofWeek(DayofWeek* obj);

		/// Simulate daily number of meetings 
		int										getNumOfMeetingsPerDay();
		/// Simulate number of occupant for a meeting 
		int										getNumOfOccupants();
		/// Simulate duration for a meeting 
		int										getDuration();
		int										getLocationId();
		void									setLocationId(int id);

		bool									checkDayFlag(int dayNum);

	private:
		int									m_locationId;
		bool								readMeetingProb(TiXmlElement* element, std::string& error);

		vector<Season::SeasonType>			m_seasonTypes;			/// The season Types
		vector<Season*>						m_seasons;				/// The season constrain

		vector<DayofWeek::DayofWeekType>	m_dayofWeekTypes;		/// The day of week constrain
		vector<DayofWeek*>					m_dayofWeeks;			/// The day of weeks

		int									m_maxNumOfMeetingsPerDay;
		int									m_minNumOfMeetingsPerDay;
		map<double, double>					m_meetingDurationProb;
		int									m_maxNumOfPeople;
		int									m_minNumOfPeople;


	};
};

#endif