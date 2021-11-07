/* ----------------------------------------------------------------------------
** The ObOccupantBehavior holds all the information readed from obXML.   
** Author: Yixing Chen
** History:
** 6/30/2015: first release
** -----------------------------------------------------------------------------*/
#ifndef ObOccupantBehavior_h
#define ObOccupantBehavior_h

#include "ObObject.h"
#include "ObBehavior.h"
#include "ObMovementBehavior.h"
#include "ObRandomMovementEvent.h"
#include "ObOccupant.h"
#include "ObSeason.h"
#include "ObTimeofDay.h"
#include "ObBuilding.h"
#include "ObDayofWeek.h"
#include "ObSimulationSettings.h"
//#include <windows.h>
#include <string> 
#include <iostream>
#include <fstream>


using namespace std;

namespace OB {
	class OccupantBehavior: public Object {
	public:
		OccupantBehavior();
		~OccupantBehavior();

		/// initialize by reading an obXML file
		/// params:
		/// - filename: the filename of the obXML file including the path
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		bool	readObXML(std::string xmlName, std::string& error);
		
		/// Prase the obCoSim file
		/// params:
		/// - filename: the filename of the obCoSim file including the path
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObCoSim successful or not
		bool	readObCoSim(std::string xmlName, std::string& error);
		
		/// Obtain the movement results from the movement result csv file		
		/// params:
		/// - filename: the filename of the movement result file including the path
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObCoSim successful or not
		bool	readMovementCSV(string filename, std::string& error);

		/// Get Current FMU Space
		bool	initCurrentFMUSpace(string instanceName, std::string& error);

		/// Get Current FMU Space
		Space*	getCurrentFMUSpace();

		SimulationSettings*	getSimulationSettings();

		void	setupLogFile(FILE* file);

		void	init();

		/// Perform Movement calculation
		bool	performMovementCalculation(string fileName, std::string& error);   
		
		// Create flag
		bool    setMoveCalcFileFlag();
		
		//Delete flag
		bool    deleteMoveCalcFileFlag();
		
		//Find flag
		bool    isMoveCalcFileFlag();

	private:
		/// Read the Buildings element
		bool	readBuildings(TiXmlElement *element, std::string& error);

		/// Read the Occupants element
		bool	readOccupants(TiXmlElement *element, std::string& error);

		/// Read the Behaviors element
		bool	readBehaviors(TiXmlElement *element, std::string& error);

		/// Read the Seasons element
		bool	readSeasons(TiXmlElement *element, std::string& error);

		/// Read the TimeofDays element
		bool	readTimeofDays(TiXmlElement *element, std::string& error);

		bool    readHolidays(TiXmlElement * parentElem, std::string& error);
		bool    readHoliday(TiXmlElement * parentElem, std::string& error) ;

		
		/// Read the SpaceNameMapping element
		bool	readSpaceNameMapping(TiXmlElement *element, std::string& error);

		/// Write the current OB information to an obXML file
		/// Params:
		/// - filename: the filename of the obXML file including the path
		//void	writeObXML(string filename);
	

		/// Get the occupant with the ID
		Occupant*			getOccupant(string ID);

		/// Get the behavior with the ID
		Behavior*			getBehavior(string ID);

		/// Get the behavior with the ID
		MovementBehavior*	getMovementBehavior(string ID);

		/// Get the Space with the ID
		Space*				getSpace(string ID);

		vector<Space*>		getSpaces();

		/// Initialize Movement calculation settings and accollations
		bool    initMovementCalculation(string& error);                                            /// Init Movement calculation

		/// connect all the reference IDs with their objects.
		/// Return
		/// - return false when can't find objects with the given ID.
		bool	connectIDswithObjects(std::string& error);
		bool	getLineItems(std::ifstream& moveFile, string& line, vector<string>& items, long long& line_number,  size_t item_min_number, string& error);


	private:
		map<Season::SeasonType, Season*>			m_seasons;
		map<TimeofDay::TimeofDayType, TimeofDay*>	m_timeofDays;
		map<DayofWeek::DayofWeekType, DayofWeek*>	m_dayofWeeks;
		vector<Building*>							m_buildings;
		vector<Behavior*>							m_behaviors;
		vector<MovementBehavior*>					m_movementBehaviors;
		vector<Occupant*>							m_occupants;
		string										m_version;
		vector<tm>	                                m_holidays;

		map<string, Space*>							m_fmuInstanceNameToSpaces;
		SimulationSettings							m_simulationSettings;
		Space*										m_currentFMUSpace;
		string                                      m_fFlagName;
		bool                                        simulateMeetings(int curDayNum, ofstream& meetingOut);
		bool										drawMeeting(int numOfOccupants, int duration, int locationId, int curDayNum, int& startTime);
		///Write output files
		void    writeOutPutFiles(ofstream& file_byOccu, ofstream& file_byRoom, ofstream& pForIDFout, ofstream& pIDFout);
		bool    writeIDF(ofstream& pIDFout, string csvName, vector<int> maxSchedule); 
		bool    writeForIDFout(ofstream& pForIDFout, string fileNameByRoom, string fileNameIDFCSV, vector<int> maxSchedule);

		

	};
};

#endif
