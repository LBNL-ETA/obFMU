
#ifndef ObRandomMovementEvent_h
#define ObRandomMovementEvent_h

#include "ObObject.h"
#include "ObSeason.h"
#include "ObDayofWeek.h"

using namespace std;

typedef std::vector<double>::size_type SZT;


namespace OB {
	class RandomMovementEvent: public Object {
	public:
		enum SpaceCategory {                          
			SPACE_OWN,
			SPACE_OTHER,
			SPACE_AUX,
			SPACE_MEETING,
			SPACE_OUT,
			SPACE_NONE
		};
		RandomMovementEvent();

		/// initialize by reading an TiXmlElement
		/// params:
		/// - element: the TiXMLElement
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		bool   readObXML(TiXmlElement* element, std::string& error);                  

		/// Initialize random movement events calculation inputs
		bool   initEvent(int stepPerHour, std::string& error);

		/// Initialize space category mapping for the simulation building
		void   initLocationCategory(vector<vector<int>> locationIdMap);
		
		/// Calculate meeting percent per day
		double getMeetingPercent();              
		
		 /// Simulate location for the next time step
		int    updateLocation(int lastLocation, int ownLocationId);


	private:
		typedef struct{
			size_t size;
			std::vector<double> p;
		} opt_data;
		typedef struct{
			 size_t nth;	//  the nth iterator
			 size_t size;	//  the size of the matrix
			 std::vector<double>::size_type bg;  // the beginning point
			 std::vector<double>::size_type ed;	 // the end point
			 size_t step;						// the iterator step
			 std::vector<double> p;				// the pobability in this room
		} constraint_data;


		map<SpaceCategory, double>	 m_spaceCategoryDuration; ///In case new category, update vecT mapping
		map<SpaceCategory, double>	 m_spaceCategoryPercent;  ///In case new category, update vecP
		vector<vector<int>>          m_locationCategoryIdMap; /// 0: office, 1: meeting, 2: other rooms, 3: outdoor
		vector<double>               m_vecT;                  /// 0: out, 1:own, 2: other office; 3:aux
		vector<double>               m_vecP;                  /// 0: out, 1:own, 2: other office; 3:aux
		double                       m_meetingPercent;        /// for meeting percent per day calculation for each occupancy
		vector<vector<double>>       m_movementMat;

		bool                         calculateMatrix(std::string& error);
		void                         initMatrix();
		bool                         optMatrix(vector<vector<double>>& mat, const std::vector<double>& VecT, const std::vector<double>& VecP, int matSize, vector<int> sizeList);  // To calculate the P matrix from Est and pi
	};
};

#endif