/* ----------------------------------------------------------------------------
** The object represents the ParameterRange node in obXML schema.   
** Author: Yixing Chen
** History:
** 6/30/2015: first release
** -----------------------------------------------------------------------------*/
#ifndef ObParameterRange_h
#define ObParameterRange_h

#include "ObObject.h"
#include "ObParameter.h"
using namespace std;

namespace OB{
	class ParameterRange: public Object {
	public:
		ParameterRange(Object* parent=NULL);
		~ParameterRange();
		ParameterRange(const ParameterRange& other);
		ParameterRange& operator= (const ParameterRange&);		
		
		/// initialize by reading an TiXmlElement
		/// params:
		/// - element: the TiXMLElement
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		bool	readObXML(TiXmlElement* element, std::string& error);

		string	getParameterID();
		void	setParameter(Parameter* obj);

		Parameter::ParameterType	getParameterType();

		/// Check whether the value is within the limit or not.
		/// return true if the value is within the min and max values;
		/// return false if the value is outside the ranage.
		bool	checkParameterValue(double value);

	private:
		string			m_parameterID;
		Parameter*		m_parameter;
		double*			m_minValue;
		double*			m_maxValue;
	};
};
#endif