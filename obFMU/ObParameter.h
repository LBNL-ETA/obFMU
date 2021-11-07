/* ----------------------------------------------------------------------------
** The object represents the Parameter node in obXML schema.   
** Author: Yixing Chen
** History:
** 6/30/2015: first release
** -----------------------------------------------------------------------------*/
#ifndef ObParameter_h
#define ObParameter_h

#include "ObObject.h"
using namespace std;

namespace OB{
	class Parameter: public Object {
	public:
		enum ParameterType{
			Parameter_RoomTemperature,
			Parameter_RoomWorkPlaneDaylightIlluminance,
			Parameter_RoomCO2Concentration,
			Parameter_RoomLightsPowerDensity,
			Parameter_OutdoorDryBulbTemperature,
			Parameter_OutdoorRainIndicator
		};

		Parameter(Object* parent=NULL);
		~Parameter();
		Parameter(const Parameter& other);
		Parameter& operator= (const Parameter&);		
		
		/// initialize by reading an TiXmlElement
		/// params:
		/// - element: the TiXMLElement
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		bool	readObXML(TiXmlElement* element, std::string& error);

		ParameterType	getParameterType();
	private:
		ParameterType	m_type;
		string			m_unit;
		string			m_name;
	};
};
#endif