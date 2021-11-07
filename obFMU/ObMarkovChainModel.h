/// This is an abstract class
#ifndef ObMarkovChainModel_h
#define ObMarkovChainModel_h
#include "ObEventOccurModel.h"
#include <math.h>
using namespace std;
namespace OB {
	/// TODO: change the class name to match the schema
	class MarkovChainModel : public EventOccurModel{
	public:
		/// initialize by reading an TiXmlElement
		/// params:
		/// - element: the TiXMLElement
		/// - error: the error information, when return false
		/// Return:
		/// - bool: readObXML successful or not
		bool	     readObXML(TiXmlElement * parentElem, string& error);
		///Initialize event model based on simulation settings
		/// params:
		/// - stepPerHour: time step per hour
		void         initEvent(int stepPerHour);
		/// Randomly generate event occur time
		/// return:
		/// - event occur time
		
		int          getOccurTime();


	private:
		int                     m_earliestOccurHour;
		int                     m_earliestOccurMin;
		int                     m_typicalOccurHour;
		int                     m_typicalOccurMin;
		std::vector<double>     m_timeCumuProb; //Size: minutes in a day
	};
};

#endif