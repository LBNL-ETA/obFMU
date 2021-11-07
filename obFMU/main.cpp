
#include "ObOccupantBehavior.h"
#include <ctime>
using namespace std;
using namespace OB;

int main(int argc,char *argv[]) {
#ifdef _WIN
	string error;
	string xmlFilename;
	string outputName;
	string obCoSimFilename;
	if (argc == 4) {
		/// Occupancy Simulator App
		//# obFMU exe arguments:
		//# 0: obFMU.exe
		//# 1. xmlFilename: the input obXML file including the path
		//# 2. outputName: the output file including the path and extension
		//# 3. obCoSimFilename: simulation setting file

		///Match path?
		xmlFilename = argv[1];
	    outputName = argv[2];
		obCoSimFilename = argv[3];
	} else if (argc == 1){
		xmlFilename = "obXML.xml";
		outputName = "output.csv";
		obCoSimFilename = "obCoSim.xml";
	} else {
		std::cerr << "Wrong number of arguments.";
	}
	std::cout << "Init OccupantBehavior\n";
	OccupantBehavior* obInstance = new OB::OccupantBehavior();

	/// Read ObXML
	if (!obInstance->readObXML(xmlFilename, error)){
		std::cout << ">>>>Fail to load obXML.xml file from " << xmlFilename << ".\n";
		std::cout << ">>>>Error: " << error << ".\n";
		return 1;
	} else {
		std::cout << "Finish readObXML.\n";
	}

	/// Read ObCoSimXML
	if (!obInstance->readObCoSim(obCoSimFilename, error)) {
		std::cout << ">>>>Fail to load obCoSim.xml file from " << obCoSimFilename << ".\n";
		std::cout << ">>>>Error: " << error << "\n";
		return 1;
	} else {
		std::cout << "Finish readObCoSim.\n";
	}

	/// Perform movement calculation
	clock_t begin = clock();
	std::cout << "Perform movement calculation...\n";
	if (!obInstance->performMovementCalculation(outputName, error)) {
		std::cout << ">>>>Fail to perform occupancy schedule simulation.\n";
		std::cout << ">>>>Error: " << error << "\n";
		return 1;
	} else {
		std::cout << "Finish occupancy simulation.\n";
	}
	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	std::cout<< "Done with " << elapsed_secs <<"s.\n";
	
#else
	/// Test for linux version
	string error;
	string xmlFilename = "../obFMU_Case Study/obXML.xml";
	string obCoSimFilename = "../obFMU_Case Study/obCoSim.xml";
	string moveByOccupantResultFilename = "../obFMU_Case Study/obm_Movement_byOccupants.csv";
	string instanceName = "obm_SupervisorOffice1";
	
	std::cout << "Init OccupantBehavior\n";
	OccupantBehavior* obInstance = new OB::OccupantBehavior();

	if(!obInstance->readObXML(xmlFilename, error)){
		std::cout << ">>>>Fail to load obXML.xml file from " << xmlFilename << ".\n";
		std::cout << ">>>>Error: " << error << ".\n";
		return 1;
	}else{
		std::cout << "Finish readObXML.\n";
	}

		
	if(!obInstance->readObCoSim(obCoSimFilename, error)){
		std::cout << ">>>>Fail to load obCoSim.xml file from " << obCoSimFilename << ".\n";
		std::cout << ">>>>Error: " << error << "\n";
		return 1;
	}else{
		std::cout << "Finish readObCoSim.\n";
	}


	if(!obInstance->initCurrentFMUSpace(instanceName.c_str(), error)){
		std::cout << ">>>>Fail to get the space for FMU Instance: " << instanceName << ".\n";
		std::cout << ">>>>Error: " << error << "\n";
		return 1;
	}else{
		std::cout << "Finish initCurrentFMUSpace.\n";
	}

	if(!obInstance->readMovementCSV(moveByOccupantResultFilename,error)){
		std::cout << ">>>>Fail to read the movement results from:" << moveByOccupantResultFilename << ".\n";
		std::cout << ">>>>Error: " << error << "\n";
		return 1;
	}else{
		std::cout << "Finish readMovementCSV.\n";
	}
#endif
	return 0;
}