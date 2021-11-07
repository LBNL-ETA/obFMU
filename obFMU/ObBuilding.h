/* ----------------------------------------------------------------------------
** The object represents the Building node in obXML schema.   
** Author: Yixing Chen
** History:
** 6/30/2015: first release
** -----------------------------------------------------------------------------*/
#ifndef ObBuilding_h
#define ObBuilding_h

#include "ObObject.h"
#include "ObOccupant.h"
#include "ObSpace.h"
using namespace std;

namespace OB {
	class Building: public Object {
	public:
		enum BuildingType {
			Building_Office
		};

		Building(Object* parent = NULL);
		~Building();
		Building(const Building& other);
		Building& operator= (const Building&);

		/// initialize by reading an TiXmlElement
		/// params:
		/// - element: the TiXMLElement
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		bool	readObXML(TiXmlElement* element, std::string& error);

		
		/// Read the Spaces element
		bool	readSpaces(TiXmlElement *element, std::string& error);

		vector<Space*>  getSpaces();
		Space*			getSpace(string ID);

		void			setSimulationNumberofTimestepsPerHour(int value);

	private:
		string				m_address;
		BuildingType		m_type;
		vector<Space*>		m_spaces;

	};
};

#endif