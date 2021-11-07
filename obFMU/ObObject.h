/* ----------------------------------------------------------------------------
** The OB object.   
** Author: Yixing Chen
** History:
** 6/30/2015: first release
** -----------------------------------------------------------------------------*/
#ifndef ObObject_h
#define ObObject_h

#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>
#include <algorithm>
#include <ctime>
#include "tinyxml.h"
#include <errno.h>

#ifdef _WIN
  #include <direct.h>
  #include <windows.h>
  typedef unsigned long long uint64_t;
#else
    #include <unistd.h>
    #define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),(mode)))==NULL
#endif

using namespace std;

static inline std::string getCurrentDir(){
    
#ifdef _WIN
    char szWorkDirChar[MAX_PATH + 1];
    DWORD dwcNameSize = MAX_PATH + 1;
    GetCurrentDirectory(dwcNameSize, szWorkDirChar);
    string szWorkDir(szWorkDirChar);
#else
    char szWorkDirChar[512];
    getcwd(szWorkDirChar, sizeof(szWorkDirChar));
    string szWorkDir(szWorkDirChar);
#endif
    szWorkDir = szWorkDir.substr(0, szWorkDir.find_last_of("\\"));
    return szWorkDir;
}



static inline std::string trim(std::string const& str)
{
    if(str.empty())
        return str;

    std::size_t firstScan = str.find_first_not_of(" \n\r\t");
    std::size_t first     = firstScan == std::string::npos? str.length():firstScan;
    std::size_t last      = str.find_last_not_of(" \n\r\t");
    return str.substr(first, last-first+1);
}

static inline void getMonDay(int *month, int *day, int orderDay) {
	int days = orderDay;
	*month = 0;
	*day = 0;
	if(days < 0) return;

	while(days > 0 && *month < 12) {
		*month += 1;
		*day = days;
		if(*month == 1) {
			days -= 31;
			*day = orderDay;
		}
		else if(*month == 2)	days -= 28;
		else if(*month == 3)	days -= 31;
		else if(*month == 4)	days -= 30;
		else if(*month == 5)	days -= 31;
		else if(*month == 6)	days -= 30;
		else if(*month == 7)	days -= 31;
		else if(*month == 8)	days -= 31;
		else if(*month == 9)	days -= 30;
		else if(*month == 10)	days -= 31;
		else if(*month == 11)	days -= 30;
		else if(*month == 12)	days -= 31;
	}
}

static inline bool getTimeInDay(string time, int& hour, int& min,  string& error) { 
	int result = -1;
	int colon = time.find(":");
	if (colon >= 0) {
		hour = stoi(time.substr(0, colon));
		string minute = time.substr(colon + 1, time.length());
		int colon_2 = minute.find(":");
		if (colon_2 >= 0) {
			min = stoi(minute.substr(0, colon_2));
		}
	}
	if (hour >= 0 && hour <= 23 && min >= 0 && min <= 59) {
		return true;
	} else {
		return false;
	}
}

static inline bool getHourMin(int timestep, int stepPerHour, int& hour, int& min) { 
	hour = timestep / stepPerHour;
	min = (int)((timestep - hour * stepPerHour) * (double)(60 / stepPerHour));
	return true;
}

static inline bool getXsDurationInMinutes(const char* text, double& value){
	value = 0;
	if( text == NULL)
		return false;
	size_t length = strlen(text);

	/// require at least 3 chars
	if(length < 3 )
		return false;

	/// P indicates the period (required)
	if(text[0] != 'P' && text[0] !='p')
		return false;

	/// Check the "T"
	int tIndex = -1;
	for( size_t i=1; i < length; i++){
		if(text[i] == 'T' || text[i]=='t'){
			tIndex = i;
			break;
		}
	}

	string date = std::string(text + 1, text + length);
	string time = "";

	if(tIndex >0){
		time = std::string(text + tIndex + 1, text + length);
		if(tIndex > 1){
			date = std::string(text + 1, text + tIndex);
		}else{
			date = "";
		}
	}

	if(date != ""){
		transform(date.begin(), date.end(), date.begin(), ::toupper);
		int dateLength = date.length();
		
		int yIndex = -1;
		int mIndex = -1;
		int dIndex = -1;

		int yValue = 0;
		int mValue = 0;
		int dValue = 0;
		string strValue = "";

		/// Find unknown char
		for( int i=0; i < dateLength; i++){
			if(date[i] != 'Y'){
				if(date[i] != 'M'){
					if(date[i] != 'D'){
						if(date[i] < '0' || date[i] >'9'){
							/// Wrong char;
							return false;
						}else{
							strValue += date[i];
						}
					}else{
						/// Found 2 "D"
						if(dIndex != -1)
							return false;
						dIndex = i;
						/// No number in front of D
						if(strValue == "")
							return false;
						dValue = atoi(strValue.c_str());
						strValue = "";
					}
				}else{
					/// Found 2 "M", or D before M
					if(mIndex != -1 || dIndex != -1)
						return false;
					mIndex = i;

					/// No number in front of M
					if(strValue == "")
						return false;
					mValue = atoi(strValue.c_str());
					strValue = "";
				}
			}else{
				/// Found 2 "Y", M before Y, or D before Y
				if(yIndex != -1 || mIndex != -1 || dIndex != -1)
					return false;
				yIndex = i;

				/// No number in front of Y
				if(strValue == "")
					return false;
				yValue = atoi(strValue.c_str());
				strValue = "";
			}
		}

		/// Can't Y, M, or D
		if(yIndex == -1 && mIndex == -1 && dIndex == -1 || strValue != "")
			return false;

		value += (yValue * 365.2425 + mValue * 30.436875 + dValue) * 24 * 60;
	}

	if( time != ""){
		transform(time.begin(), time.end(), time.begin(), ::toupper);
		int timeLength = time.length();
		
		int hIndex = -1;
		int mIndex = -1;
		int sIndex = -1;

		int hValue = 0;
		int mValue = 0;
		int sValue = 0;
		string strValue = "";

		/// Find unknown char
		for( int i=0; i < timeLength; i++){
			if(time[i] != 'H'){
				if(time[i] != 'M'){
					if(time[i] != 'S'){
						if(time[i] < '0' || time[i] >'9'){
							/// Wrong char;
							return false;
						}else{
							strValue += time[i];
						}
					}else{
						/// Found 2 "S"
						if(sIndex != -1)
							return false;
						sIndex = i;
						/// No number in front of S
						if(strValue == "")
							return false;
						sValue = atoi(strValue.c_str());
						strValue = "";
					}
				}else{
					/// Found 2 "M", or S before M
					if(mIndex != -1 || sIndex != -1)
						return false;
					mIndex = i;

					/// No number in front of M
					if(strValue == "")
						return false;
					mValue = atoi(strValue.c_str());
					strValue = "";
				}
			}else{
				/// Found 2 "H", M before H, or D before H
				if(hIndex != -1 || mIndex != -1 || sIndex != -1)
					return false;
				hIndex = i;

				/// No number in front of Y
				if(strValue == "")
					return false;
				hValue = atoi(strValue.c_str());
				strValue = "";
			}
		}
		/// Can't H, M, or S
		if(hIndex == -1 && mIndex == -1 && sIndex == -1 || strValue != "")
			return false;

		value += hValue * 60 + mValue + sValue/60;
	}
	return true;
} 

// You could also take an existing vector as a parameter.
static inline std::vector<std::string> split(std::string str, char delimiter) {
  vector<string> result;
  stringstream ss(str); // Turn the string into a stream.
  string tok;
  
  while(getline(ss, tok, delimiter)) {
	result.push_back(tok);
  }
  
  return result;
}

namespace OB{
	class Object{
	public:
		Object(Object* parent = NULL);
		~Object();
		Object(const Object& other);
		Object& operator= (const Object&);

		bool	setID(string ID, string& error);
		void	setDescription(string description);

		string	getID();
		string	getDescription();

		void	setLogFile(FILE* logFile);
		void	writeToLogFile(string info);

		/// Obtain the attribute from Attribute, and set it to the result
		/// Params:
		/// - element: the TiXmlElement
		/// - attributeName: the name of the attribute
		/// - isOptional: whether the attribute is optional or not
		/// - result: the output string
		/// - error: the error information
		/// return:
		/// - bool: true when get the string attribute, else return false
		static bool	getStringAttribute(TiXmlElement* element, string attributeName, string& result, string& error, bool isOptional=false);

		/// Return the day of the year, 0 means the month and day is out of range.
		static int		getDayofYear(int month, int day, bool isLeapYear=false);

	private:
		/// Check whether the ID is unique or not
		/// return false when the length of the string is 0 or the string is already in the EXISTING_ID_LIST
		/// return true when the ID is not in the EXISTING_ID_LIST, and add it to the EXISTING_ID_LIST
		static bool	isUniqueID(string ID, string& error);

		string	m_ID;
		string	m_description;
		static vector<string> EXISTING_ID_LIST;
		Object*				m_parent;
		FILE*				m_logFile;
	};
};

#endif
