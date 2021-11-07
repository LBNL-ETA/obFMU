#include "ObFmu.h"
#include <cstring>
using namespace OB;

/// The result file in CSV format.
static FILE *fp		 = NULL;
static FILE *logFile = NULL;

/// The number of inputs and outputs.
int iInputNum	= 6;
int iOutputNum	= 7;

/// Each obFMU is associated with one zone. 
/// The following flags are used to make sure the zone information is loaded once per simulation.
static bool bGetObXML = false;
static bool	bGetMovement = false;

/// Flag for logger;
/// For debug purpose, only show the log information once
static bool	bLogfmi2Instantiate = true;
static bool	bLogfmiFreeSlaveInstance = true;
static bool	bLogfmiDoStep = true;
static bool	bLogfmiGetReal= true;
static bool	bLogfmiSetReal= true;

static bool	bLogfmi2InstantiateOnce = true;
static bool	bLogfmiFreeSlaveInstanceOnce = true;
static bool	bLogfmiDoStepOnce = true;
static bool	bLogfmiGetRealOnce = true;
static bool	bLogfmiSetRealOnce = true;

static OB::OccupantBehavior* obInstance = new OB::OccupantBehavior();
static int iCurrentTimeStep = 0;

static bool	showLog = true;

static void logger(std::string log){
#ifdef _DEBUG
	if(showLog){
		std::cout << log;
	}
#endif
};

bool is_file_exist(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}


#ifdef FMI_V2

/// Create an instance and allocate the memory
/* The function returns a new instance of an FMU. If a null pointer is returned, then instantiation
failed. In that case, ¡°functions->logger¡± was called with detailed information about the
reason. An FMU can be instantiated many times (provided capability flag
canBeInstantiatedOnlyOncePerProcess = false). */
FMI2_Export fmi2Component fmi2Instantiate(fmi2String instanceName,fmi2Type fmuType,fmi2String fmuGUID,fmi2String fmuResourceLocation,
	const fmi2CallbackFunctions* functions,fmi2Boolean visible,fmi2Boolean loggingOn){
	showLog = bLogfmi2Instantiate;
	/// functions.logger(NULL, instanceName, fmiOK, "Information", "  Perform checks");	
	/// Perform checks
	if (!functions->logger)
		return NULL;

	if (!functions->allocateMemory){
		functions->logger(NULL, instanceName, fmi2Error, "error",
			"fmi2Instantiate: Missing callback function: allocateMemory");
		return NULL;
	}
	if (!functions->freeMemory){
		functions->logger(NULL, instanceName, fmi2Error, "error",
			"fmi2Instantiate: Missing callback function: freeMemory");
		return NULL;
	}
	if (!instanceName || strlen(instanceName)==0) {
		functions->logger(NULL, instanceName, fmi2Error, "error",
			"fmi2Instantiate: Missing instance name.");
		return NULL;
	}
	if (strcmp(fmuGUID, MODEL_GUID)) {
		functions->logger(NULL, instanceName, fmi2Error, "error",
			"fmi2Instantiate: Wrong FMU GUID %s. Expected %s.", fmuGUID, MODEL_GUID);
		return NULL;
	}

	if( fmuType != fmi2CoSimulation){
		functions->logger(NULL, instanceName, fmi2Error, "Error", "Wrong FMU type. Expected fmi2CoSimulation.");
		return NULL;
	}
	
	functions->logger(NULL, instanceName, fmi2OK, "Information", "fmi2Instantiate for instance name: %s, FMU resource location: %s",instanceName,fmuResourceLocation);

	ModelInstance* component;

	/// functions.logger(NULL, instanceName, fmiOK, "Information", "  Allocate memory");
	/// allocate the memory
	component = (ModelInstance *)functions->allocateMemory(1, sizeof(ModelInstance));
	component->inputs = (fmi2Real*)functions->allocateMemory(iInputNum,    sizeof(fmi2Real));
	component->outputs = (fmi2Real*)functions->allocateMemory(iOutputNum,    sizeof(fmi2Real));
	component->functions = functions;
	component->instanceName = instanceName;

	string fmuResourceLocationStr = std::string(fmuResourceLocation);

#if defined(_WIN32) || defined(_WIN64)
	string fmuLocationStart = std::string(fmuResourceLocation).substr(0,8);
	if(fmuLocationStart.compare("file:///") ==0 ){
		int len = strlen(fmuResourceLocation);
		fmuResourceLocationStr = std::string(fmuResourceLocation).substr(8,len-8);
	}else{
		fmuLocationStart = std::string(fmuResourceLocation).substr(0,7);
		if(fmuLocationStart.compare("file://") ==0 ){
			int len = strlen(fmuResourceLocation);
			fmuResourceLocationStr = std::string(fmuResourceLocation).substr(7,len-7);
		}else{
			fmuLocationStart = std::string(fmuResourceLocation).substr(0,6);
			if(fmuLocationStart.compare("file:/") ==0 ){
				int len = strlen(fmuResourceLocation);
				fmuResourceLocationStr = std::string(fmuResourceLocation).substr(6,len-6);
			}
		}
	}
#endif

	/// Get current working directory
    string szWorkDir = getCurrentDir();	
	string result_filename = szWorkDir;

	result_filename += "/";
	result_filename += instanceName;
	string log_filename = result_filename;
	log_filename += "_log.csv";
	result_filename += ".csv";

	/// Set default values for inputs and outputs
	component->inputs[0] = 24.0;	/// temp Val, from e+
	component->inputs[1] = 0.0;		/// ill Val,  from e+
	component->inputs[2] = 400.0;	/// co2 Val,  from e+
	component->inputs[3] = 500.0;	/// light power Val,  from e+
	component->inputs[4] = 24.0;	/// Outdoor air drybulb temperature val,  from e+
	component->inputs[5] = 0;	    /// Outdoor rain indicator val,  from e+

	component->outputs[0] = 0.0;	// HVAC Schedule, 
	component->outputs[1] = 0.0;	// light Schedule, 
	component->outputs[2] = 0.0;   // infiltration Schedule, 
	component->outputs[3] = 0.0;	// PlugLoad Schedule 
	component->outputs[4] = 24.0;	// Thermostat Schedule 
	component->outputs[5] = 0.0;	// Shade and Blind Schedule
	component->outputs[6] = 1.0;	// occupant Schedule
								
	iCurrentTimeStep = 0;

	//get OBM parameters only one time
  	if(bGetObXML == false)
	{	
		string obXMLFilename = szWorkDir;
		obXMLFilename += "/obXML.xml";

		if(is_file_exist(obXMLFilename.c_str())){		
			functions->logger(NULL, instanceName, fmi2OK, "information", "Find and use the obXML.xml in the working directory.");
		}else{
			functions->logger(NULL, instanceName, fmi2Warning, "warning", "Cannot find the obXML.xml in the working directory.");
			obXMLFilename = fmuResourceLocationStr + "/obXML.xml";
			if(is_file_exist(obXMLFilename.c_str())){
				functions->logger(NULL, instanceName, fmi2OK, "information", "Find and use the obXML.xml in the resources subfolder of obFMU fmuLocation.");
			}else{
				functions->logger(NULL, instanceName, fmi2Warning, "warning", "Cannot find the obXML.xml in the obFMU fmuLocation.");
				functions->logger(NULL, instanceName, fmi2Error, "error", "Cannot find the obXML.xml in all locations.");
				return NULL;
			}
		}

		string error;
		if(!obInstance->readObXML(obXMLFilename.c_str(),error)){
			functions->logger(NULL, instanceName, fmi2Error, "error", "Fail to load obXML.xml file from %s. Error: %s",obXMLFilename.c_str(), error);
			return NULL;
		}

		string obCoSimFilename = szWorkDir;
		obCoSimFilename += "/obCoSim.xml";

		if(is_file_exist(obCoSimFilename.c_str())){		
			functions->logger(NULL, instanceName, fmi2OK, "information", "Find and use the obCoSim.xml in the working directory.");
		}else{
			functions->logger(NULL, instanceName, fmi2Warning, "warning", "Cannot find the obCoSim.xml in the working directory.");
			obCoSimFilename = fmuResourceLocationStr + "/obCoSim.xml";
			if(is_file_exist(obCoSimFilename.c_str())){
				functions->logger(NULL, instanceName, fmi2OK, "information", "Find and use the obCoSim.xml in the resources subfolder of obFMU fmuLocation.");
			}else{
				functions->logger(NULL, instanceName, fmi2Warning, "warning", "Cannot find the obCoSim.xml in the obFMU fmuLocation.");
				functions->logger(NULL, instanceName, fmi2Error, "error", "Cannot find the obCoSim.xml in all locations.");
				return NULL;
			}
		}

		if(!obInstance->readObCoSim(obCoSimFilename.c_str(), error)){
			functions->logger(NULL, instanceName, fmi2Error, "error", "Fail to load obCoSim.xml file from %s. Error: %s",obXMLFilename.c_str(), error.c_str());
			return NULL;
		}else{
			functions->logger(NULL, instanceName, fmi2OK, "Information", "  Finish reading the obCoSim file.");
		}

		obInstance->init();
		functions->logger(NULL, instanceName, fmi2OK, "Information", "  Finish init the obFMU instance.");

		if(!obInstance->initCurrentFMUSpace(instanceName, error)){
			functions->logger(NULL, instanceName, fmi2Error, "error", "Fail to get the space for FMU Instance: %s. Error: %s.",instanceName, error.c_str());
			return NULL;
		}else{
		  functions->logger(NULL, instanceName, fmi2OK, "Information", "  Finish connection the space with the instance name.");
		}

		bGetObXML = true;
	}

	/// should setup the logfile or 
	if(logFile==NULL && obInstance->getSimulationSettings()->isDebugMode()){	
		/// functions->logger(NULL, instanceName, fmiOK, "Information", "  Init Logger File");
		fopen_s(&logFile, log_filename.c_str(),"w"); 
		if(logFile==NULL) 
		{
			functions->logger(NULL, instanceName, fmi2Error, "error", "Can't open %s for logging",log_filename.c_str());
			return NULL;
		}
		obInstance->setupLogFile(logFile);
	}

	//get OBM parameters only one time
  	if(bGetMovement == false){
        string prWorkDir = szWorkDir;

        string movementResultFile = prWorkDir;
        movementResultFile += "/output.csv";
        std::string error;

        /// Check whether user has the movment results
        bool doMovementCalculation = obInstance->getSimulationSettings()->doMovementCalculation();                       

        /// The movement calculation is only calculated once per simulation.
        /// Once the movement calculation is done, a file flag will be generated.
        /// Check the movement file flag to determine running movement calculation or not.
        string movementResultByOccupantFile = prWorkDir;
        string movementResultByRoomFile = prWorkDir;
        string movementResultForIDFFile = prWorkDir;
        string movementResultIDFFile = prWorkDir;
        if (doMovementCalculation) {
            movementResultByOccupantFile += "/output_by_Occupant.csv";
            movementResultByRoomFile += "/output.csv";
            movementResultForIDFFile += "/output_IDF.csv";
            movementResultIDFFile += "/output_IDF.idf";
        
            if (obInstance->isMoveCalcFileFlag() == false)  {
                logger(" Perform movement calculation...\n");
                /// Perform movement calculation
                if (!obInstance->performMovementCalculation(movementResultFile, error)) {
					functions->logger(NULL, instanceName, fmi2Error, "error", "Fail to perform occupancy schedule simulation. Error: %s",error.c_str());
                } else {
                    std::cout << "Finish occupancy simulation.\n";
                }
                obInstance->setMoveCalcFileFlag();
            }
        }else{
            movementResultByOccupantFile += "/";
            movementResultByOccupantFile += obInstance->getSimulationSettings()->getUserMovementResultFilename();
        }
        
		if(!obInstance->readMovementCSV(movementResultByOccupantFile,error)){
			functions->logger(NULL, instanceName, fmi2Error, "error", "Fail to read the movement results from %s. Error: %s",movementResultByOccupantFile.c_str(),error.c_str());
			return NULL;
		}

 		bGetMovement=true;
	}
	
	if(bLogfmi2InstantiateOnce)
		bLogfmi2Instantiate = false;

	
	if(fp==NULL && obInstance->getSimulationSettings()->shouldExportCSVResults()){
		/// functions->logger(NULL, instanceName, fmiOK, "Information", "  Init Result File at %s.",result_filename.c_str());
		fopen_s(&fp, result_filename.c_str(),"w"); 
		if(fp==NULL) {
			functions->logger(NULL, instanceName, fmi2Error, "error", "Can't open %s for writing results.",result_filename.c_str());
			return NULL;
		}else{
			fprintf(fp,"Time,Temperature (C),Illuminance(lux),CO2 (ppm),LPD (w),Ourdoor air temperature (C), Outdoor rain indicator, AC Schedule, Lighting Schedule,Window Schedule,Plug Load Schedule,Thermostat Setpoint (C),Shade and Blind Schedule, Occupant Amount\n");
		}
	}	
	
	functions->logger(NULL, instanceName, fmi2OK, "Information", "fmi2Instantiate done");
	return component;
}

/// Is called by the environment to reset the FMU after a simulation run. The FMU goes into the
/// same state as if fmi2Instantiate would have been called. All variables have their default
/// values. Before starting a new run, fmi2SetupExperiment and
/// fmi2EnterInitializationMode have to be called.
FMI2_Export fmi2Status fmi2Reset(fmi2Component c){  
	ModelInstance* component = (ModelInstance *) c;
	/// Set default values for inputs and outputs
	component->inputs[0] = 24.0;	/// temp Val, from e+
	component->inputs[1] = 0.0;		/// ill Val,  from e+
	component->inputs[2] = 400.0;	/// co2 Val,  from e+
	component->inputs[3] = 500.0;	/// light power Val,  from e+
	component->inputs[4] = 24.0;	/// Outdoor air drybulb temperature val,  from e+
	component->inputs[5] = 0;	    /// Outdoor rain indicator val,  from e+

	component->outputs[0] = 0.0;	// HVAC Schedule, 
	component->outputs[1] = 0.0;	// light Schedule, 
	component->outputs[2] = 0.0;   // infiltration Schedule, 
	component->outputs[3] = 0.0;	// PlugLoad Schedule 
	component->outputs[4] = 24.0;	// Thermostat Schedule 
	component->outputs[5] = 0.0;	// Shade and Blind Schedule
	component->outputs[6] = 1.0;	// occupant Schedule
								
	iCurrentTimeStep = 0;
	return fmi2OK;
}

/* Disposes the given instance, unloads the loaded model, and frees all the allocated memory
and other resources that have been allocated by the functions of the FMU interface. If a null
pointer is provided for ¡°c¡±, the function call is ignored (does not have an effect). */
/// Free the instance 
/// --- Clean the movement flag and close the results file
FMI2_Export void  fmi2FreeInstance(fmi2Component c) {
	showLog = bLogfmiFreeSlaveInstance;
	logger("fmiFreeSlaveInstance start\n");
	ModelInstance* component = (ModelInstance *) c;
	component->functions->freeMemory(component->inputs);
	component->functions->freeMemory(component->outputs);
	component->functions->freeMemory(component);
#ifdef _WIN
	/// Clean the movement file flag
	obInstance->deleteMoveCalcFileFlag();
#endif
	
	iCurrentTimeStep = 0;

	// Free the results file
	if(fp != NULL) {
		fclose(fp);
		fp=NULL;
	}
	if(logFile != NULL){
		fclose(logFile);
		logFile=NULL;
	}
	logger("fmiFreeSlaveInstance done\n");	
	if(bLogfmiFreeSlaveInstanceOnce)
		bLogfmiFreeSlaveInstance = false;
}

/// Perform the time step interaction solver
/// Attribute:
/// --- currentCommunicationPoint:seconds
/// --- communicationStepSize: seconds
FMI2_Export fmi2Status fmi2DoStep(fmi2Component c, fmi2Real currentCommunicationPoint,
	fmi2Real communicationStepSize, fmi2Boolean noSetFMUStatePriorToCurrentPoint){
	showLog = bLogfmiDoStep;
	logger("fmiDoStep start\n");
	ModelInstance* component = (ModelInstance *) c;

	int curMon, curDay, hour, min;
    int numOfStepPerHour = obInstance->getSimulationSettings()->getNumberofTimestepsPerHour();
	int totalStepsPerDay = numOfStepPerHour * 24;
	int startDayNum = obInstance->getSimulationSettings()->getStartDayofYear();
	int curDayNum = startDayNum + (iCurrentTimeStep+1) / totalStepsPerDay;
	getMonDay(&curMon, &curDay, curDayNum);
	getHourMin((iCurrentTimeStep+1) % totalStepsPerDay, numOfStepPerHour, hour, min);

	string error;
	if(logFile != NULL)
		fprintf(logFile,"%i\n", iCurrentTimeStep);

	map<Parameter::ParameterType,double> parameterValues;
	parameterValues[Parameter::Parameter_RoomTemperature] = component->inputs[0];
	parameterValues[Parameter::Parameter_RoomWorkPlaneDaylightIlluminance] = component->inputs[1];
	parameterValues[Parameter::Parameter_RoomCO2Concentration] = component->inputs[2];
	parameterValues[Parameter::Parameter_RoomLightsPowerDensity] = component->inputs[3];
	parameterValues[Parameter::Parameter_OutdoorDryBulbTemperature] = component->inputs[4];
	parameterValues[Parameter::Parameter_OutdoorRainIndicator] = component->inputs[5];

	map<Space::OutputParameterType, double> outputValues;
	outputValues[Space::Output_HVAC_Schedule] = component->outputs[0];
	outputValues[Space::Output_Light_Schedule] = component->outputs[1];
	outputValues[Space::Output_Infiltration_Schedule] = component->outputs[2];
	outputValues[Space::Output_PlugLoad_Schedule] = component->outputs[3];
	outputValues[Space::Output_Thermostat_Schedule] = component->outputs[4];
	outputValues[Space::Output_ShadeAndBlind_Schedule] = component->outputs[5];
	outputValues[Space::Output_Occupant_Schedule] = component->outputs[6];

	OB::Space* space = obInstance->getCurrentFMUSpace();
	
	if(!space->performCalculation(iCurrentTimeStep, parameterValues, outputValues, error, logFile)){
		error += "Fail to perform calculation at time step " + std::to_string((uint64_t)iCurrentTimeStep) + ".\n";
		std::cout  << error << endl;
		return fmi2Error;
	}
	component->outputs[0] = outputValues[Space::Output_HVAC_Schedule];
	component->outputs[1] = outputValues[Space::Output_Light_Schedule];
	component->outputs[2] = outputValues[Space::Output_Infiltration_Schedule];
	component->outputs[3] = outputValues[Space::Output_PlugLoad_Schedule];
	component->outputs[4] = outputValues[Space::Output_Thermostat_Schedule];
	component->outputs[5] = outputValues[Space::Output_ShadeAndBlind_Schedule];
	component->outputs[6] = outputValues[Space::Output_Occupant_Schedule];

	if(logFile != NULL)
		fprintf(logFile,"%i,%d/%d %d:%d:00,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", iCurrentTimeStep, curMon, curDay, hour, min,component->inputs[0], component->inputs[1], component->inputs[2], component->inputs[3], component->inputs[4],component->inputs[5],component->outputs[0],component->outputs[1],component->outputs[2],component->outputs[3],component->outputs[4],component->outputs[5],component->outputs[6]);
	
	// Write the results to results file
	if(fp != NULL) 
		fprintf(fp,"%d/%d %d:%d:00,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", curMon, curDay, hour, min,component->inputs[0], component->inputs[1], component->inputs[2], component->inputs[3], component->inputs[4],component->inputs[5],component->outputs[0],component->outputs[1],component->outputs[2],component->outputs[3],component->outputs[4],component->outputs[5],component->outputs[6]);

	iCurrentTimeStep++;
	logger("fmiDoStep done\n");
	if(bLogfmiDoStepOnce) bLogfmiDoStep= false;

	return fmi2OK;
}

/// Exchange data with co-simulation manager
/// --- Write outputs from obFMU to co-simulation manager (EnergyPlus)
FMI2_Export fmi2Status fmi2GetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[]) {
	showLog = bLogfmiGetReal;
	logger("fmiGetReal start\n");
	ModelInstance* component = (ModelInstance *) c;
	for(int i=0; i< nvr; i++){
		value[i] = component->outputs[vr[i]-101];
	}
	logger("fmiGetReal done\n");

	if(bLogfmiGetRealOnce)
		bLogfmiGetReal= false;
	return fmi2OK;
}

/// Exchange data with co-simulation manager
/// --- Read inputs from co-simulation manager (EnergyPlus) to obFMU
FMI2_Export fmi2Status fmi2SetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[]){  
	showLog = bLogfmiSetReal;
	logger("fmiSetReal start\n");
	ModelInstance* component = (ModelInstance *) c;
	for(int i=0; i< nvr; i++){
		component->inputs[vr[i]-201] = value[i];
	}
	logger("fmiSetReal done\n");

	if(bLogfmiSetRealOnce)
		bLogfmiSetReal= false;
	return fmi2OK;
}

/* 
fmi2GetFMUstate makes a copy of the internal FMU state and returns a pointer to this copy
(FMUstate). If on entry *FMUstate == NULL, a new allocation is required. If *FMUstate !=
NULL, then *FMUstate points to a previously returned FMUstate that has not been modified
since. In particular, fmi2FreeFMUstate had not been called with this FMUstate as an argument.
[Function fmi2GetFMUstate typically reuses the memory of this FMUstate in this case and
returns the same pointer to it, but with the actual FMUstate.]
*/
FMI2_Export fmi2Status fmi2GetFMUstate (fmi2Component c, fmi2FMUstate* FMUstate){
	ModelInstance* component = (ModelInstance *) c;
	if(FMUstate == NULL){
		StateInstance* new_state;
		new_state = (StateInstance *)component->functions->allocateMemory(1, sizeof(StateInstance));
		new_state->inputs = (fmi2Real*)component->functions->allocateMemory(iInputNum,sizeof(fmi2Real));
		new_state->outputs = (fmi2Real*)component->functions->allocateMemory(iOutputNum,sizeof(fmi2Real));
		FMUstate = (fmi2FMUstate*) new_state;
	}
	StateInstance* state = (StateInstance *) FMUstate;
	for(int i=0; i < iInputNum; i++){
		state->inputs[i] = component->inputs[i];
	}

	for(int i=0; i < iOutputNum; i++){
		state->outputs[i] = component->outputs[i];
	}
	state->iCurrentTimeStep = iCurrentTimeStep;
	return fmi2OK;
};

/* 
fmi2SetFMUstate copies the content of the previously copied FMUstate back and uses it as
actual new FMU state. The FMUstate copy does still exist.
*/
FMI2_Export fmi2Status fmi2SetFMUstate (fmi2Component c, fmi2FMUstate FMUstate){
	ModelInstance* component = (ModelInstance *) c;
	if(FMUstate == NULL){
		component->functions->logger(NULL, component->instanceName, fmi2Error, "error", "fmi2SetFMUstate: The fmi2FMUstate is NULL.");
		return fmi2Error;
	}
	StateInstance* state = (StateInstance *) FMUstate;
	for(int i=0; i < iInputNum; i++){
		component->inputs[i] = state->inputs[i];
	}

	for(int i=0; i < iOutputNum; i++){
		component->outputs[i] = state->outputs[i];
	}
	iCurrentTimeStep = state->iCurrentTimeStep;
	return fmi2OK;
};

/* 
fmi2FreeFMUstate frees all memory and other resources allocated with the fmi2GetFMUstate
call for this FMUstate. The input argument to this function is the FMUstate to be freed. If a null
pointer is provided, the call is ignored. The function returns a null pointer in argument FMUstate.
*/
FMI2_Export fmi2Status fmi2FreeFMUstate(fmi2Component c, fmi2FMUstate* FMUstate){
	if(FMUstate == NULL){
		return fmi2OK;
	}
	StateInstance* state = (StateInstance *) FMUstate;
	ModelInstance* component = (ModelInstance *) c;
	component->functions->freeMemory(state->inputs);
	component->functions->freeMemory(state->outputs);
	component->functions->freeMemory(state);
	return fmi2OK;
};

#else 

/// Create an instance and allocate the memory
fmiComponent DllExport fmi2Instantiate(fmiString  instanceName, fmiString  GUID,
	fmiString  fmuLocation, fmiString  mimeType, fmiReal timeout, fmiBoolean visible,
	fmiBoolean interactive, fmiCallbackFunctions functions, fmiBoolean loggingOn) {
	showLog = bLogfmi2Instantiate;
	functions.logger(NULL, instanceName, fmiOK, "Information", "fmi2Instantiate for instance name: %s, FMU location: %s",instanceName,fmuLocation);
	ModelInstance* component;

	// functions.logger(NULL, instanceName, fmiOK, "Information", "  Perform checks");	
	// Perform checks
	if (!functions.logger)
		return NULL;
	if (!functions.allocateMemory || !functions.freeMemory){
		functions.logger(NULL, instanceName, fmiError, "error",
			"fmi2Instantiate: Missing callback function: freeMemory");
		return NULL;
	}
	if (!instanceName || strlen(instanceName)==0) {
		functions.logger(NULL, instanceName, fmiError, "error",
			"fmi2Instantiate: Missing instance name.");
		return NULL;
	}
	if (strcmp(GUID, MODEL_GUID)) {
		functions.logger(NULL, instanceName, fmiError, "error",
			"fmi2Instantiate: Wrong GUID %s. Expected %s.", GUID, MODEL_GUID);
		return NULL;
	}

	/// functions.logger(NULL, instanceName, fmiOK, "Information", "  Allocate memory");
	/// allocate the memory
	component = (ModelInstance *)functions.allocateMemory(1, sizeof(ModelInstance));
	component->inputs = (fmiReal*)functions.allocateMemory(iInputNum,    sizeof(fmiReal));
	component->outputs = (fmiReal*)functions.allocateMemory(iOutputNum,    sizeof(fmiReal));
	component->functions = functions;
	component->instanceName = instanceName;

	string fmuLocationStr = std::string(fmuLocation);

#if defined(_WIN32) || defined(_WIN64)
	string fmuLocationStart = std::string(fmuLocation).substr(0,8);
	if(fmuLocationStart.compare("file:///") ==0 ){
		int len = strlen(fmuLocation);
		fmuLocationStr = std::string(fmuLocation).substr(8,len-8);
	}else{
		fmuLocationStart = std::string(fmuLocation).substr(0,7);
		if(fmuLocationStart.compare("file://") ==0 ){
			int len = strlen(fmuLocation);
			fmuLocationStr = std::string(fmuLocation).substr(7,len-7);
		}
		/// functions.logger(NULL, instanceName, fmiOK, "Information", "fmuLocation for windows: %s", fmuLocationStr.c_str());
	}
#endif

	/// Get current working directory
    string szWorkDir = getCurrentDir();
	
	/// functions.logger(NULL, instanceName, fmiOK, "Information", "Currnet working directory: %s", szWorkDir.c_str());
	/// Generate an unique name for the result file.
	/// Current the instanceName from EnergyPlus is the FMU name (ie. obFMU) rather than the instance name. 
	/// The fmuLocation = tmp-fmus\obFMU.fmu_<instance name>
	/// TODO: update this after EnergyPlus fix the instanceName bug. 
	string fmuName = "obFMU";
	string instanceNameCorrect;
	if(std::strcmp(instanceName,fmuName.c_str())==0){
		int len = strlen(fmuLocation);
		string instanceNameStart = std::string(instanceName).substr(0,19);
		if(instanceNameStart.compare("tmp-fmus\\obFMU.fmu_")==0){
			/// For EnergyPlus 8.2.
			instanceNameCorrect = std::string(fmuLocation).substr(19, len-19);
		}else{
			/// Cannot find the instance name
			functions.logger(NULL, instanceName, fmiError, "error", "The instanceName (obFMU) is the same as the FMU name.");
			return NULL;
		}
	}else{
		if(std::strcmp(instanceName,"Slave")==0){
			instanceNameCorrect = "obFMU_01";
		}else{
			/// For ESP-r and EnergyPlus V8.3 and later
			instanceNameCorrect = std::string(instanceName);
		}
	}
	
	string result_filename = szWorkDir;

	result_filename += "/";
	result_filename += instanceNameCorrect;
	string log_filename = result_filename;
	log_filename += "_log.csv";
	result_filename += ".csv";

	/// Set default values for inputs and outputs
	component->inputs[0] = 24.0;	/// temp Val, from e+
	component->inputs[1] = 0.0;		/// ill Val,  from e+
	component->inputs[2] = 400.0;	/// co2 Val,  from e+
	component->inputs[3] = 500.0;	/// light power Val,  from e+
	component->inputs[4] = 24.0;	/// Outdoor air drybulb temperature val,  from e+
	component->inputs[5] = 0;	    /// Outdoor rain indicator val,  from e+

	component->outputs[0] = 0.0;	// HVAC Schedule, 
	component->outputs[1] = 0.0;	// light Schedule, 
	component->outputs[2] = 0.0;   // infiltration Schedule, 
	component->outputs[3] = 0.0;	// PlugLoad Schedule 
	component->outputs[4] = 24.0;	// Thermostat Schedule 
	component->outputs[5] = 0.0;	// Shade and Blind Schedule
	component->outputs[6] = 1.0;	// occupant Schedule
								
	iCurrentTimeStep = 0;

	//get OBM parameters only one time
  	if(bGetObXML == false)
	{	
		string obXMLFilename = szWorkDir;
		obXMLFilename += "/obXML.xml";

		if(is_file_exist(obXMLFilename.c_str())){		
			functions.logger(NULL, instanceName, fmiOK, "information", "Find and use the obXML.xml in the working directory.");
		}else{
			functions.logger(NULL, instanceName, fmiWarning, "warning", "Cannot find the obXML.xml in the working directory.");
			obXMLFilename = fmuLocationStr + "/resources/obXML.xml";
			if(is_file_exist(obXMLFilename.c_str())){
				functions.logger(NULL, instanceName, fmiOK, "information", "Find and use the obXML.xml in the resources subfolder of obFMU fmuLocation.");
			}else{
			    functions.logger(NULL, instanceName, fmiWarning, "warning", "Cannot find the obXML.xml in the resources subfolder of obFMU fmuLocation.");
				obXMLFilename = fmuLocationStr + "/obXML.xml";
				if(is_file_exist(obXMLFilename.c_str())){
					functions.logger(NULL, instanceName, fmiOK, "information", "Find and use the obXML.xml in the obFMU fmuLocation.");
				}else{
					functions.logger(NULL, instanceName, fmiWarning, "warning", "Cannot find the obXML.xml in the obFMU fmuLocation.");
					functions.logger(NULL, instanceName, fmiError, "error", "Cannot find the obXML.xml in all locations.");
					return NULL;
				}
			}
		}

		string error;
		if(!obInstance->readObXML(obXMLFilename.c_str(),error)){
			functions.logger(NULL, instanceName, fmiError, "error", "Fail to load obXML.xml file from %s. Error: %s",obXMLFilename.c_str(), error);
			return NULL;
		}

		string obCoSimFilename = szWorkDir;
		obCoSimFilename += "/obCoSim.xml";

		if(is_file_exist(obCoSimFilename.c_str())){		
			functions.logger(NULL, instanceName, fmiOK, "information", "Find and use the obCoSim.xml in the working directory.");
		}else{
			functions.logger(NULL, instanceName, fmiWarning, "warning", "Cannot find the obCoSim.xml in the working directory.");
			obCoSimFilename = fmuLocationStr + "/resources/obCoSim.xml";
			if(is_file_exist(obCoSimFilename.c_str())){
				functions.logger(NULL, instanceName, fmiOK, "information", "Find and use the obCoSim.xml in the resources subfolder of obFMU fmuLocation.");
			}else{
			    functions.logger(NULL, instanceName, fmiWarning, "warning", "Cannot find the obCoSim.xml in the resources subfolder of obFMU fmuLocation.");
				obCoSimFilename = fmuLocationStr + "/obCoSim.xml";
				if(is_file_exist(obCoSimFilename.c_str())){
					functions.logger(NULL, instanceName, fmiOK, "information", "Find and use the obCoSim.xml in the obFMU fmuLocation.");
				}else{
					functions.logger(NULL, instanceName, fmiWarning, "warning", "Cannot find the obCoSim.xml in the obFMU fmuLocation.");
					functions.logger(NULL, instanceName, fmiError, "error", "Cannot find the obCoSim.xml in all locations.");
					return NULL;
				}
			}
		}

		if(!obInstance->readObCoSim(obCoSimFilename.c_str(), error)){
			functions.logger(NULL, instanceName, fmiError, "error", "Fail to load obCoSim.xml file from %s. Error: %s",obXMLFilename.c_str(), error.c_str());
			return NULL;
		}else{
			functions.logger(NULL, instanceName, fmiOK, "Information", "  Finish reading the obCoSim file.");
		}

		obInstance->init();
		functions.logger(NULL, instanceName, fmiOK, "Information", "  Finish init the obFMU instance.");

		if(!obInstance->initCurrentFMUSpace(instanceNameCorrect.c_str(), error)){
			functions.logger(NULL, instanceName, fmiError, "error", "Fail to get the space for FMU Instance: %s. Error: %s.",instanceNameCorrect.c_str(), error.c_str());
			return NULL;
		}else{
		  functions.logger(NULL, instanceName, fmiOK, "Information", "  Finish connection the space with the instance name.");
		}

		bGetObXML = true;
	}

	/// should setup the logfile or 
	if(logFile==NULL && obInstance->getSimulationSettings()->isDebugMode()){	
		/// functions.logger(NULL, instanceName, fmiOK, "Information", "  Init Logger File");
		fopen_s(&logFile, log_filename.c_str(),"w"); 
		if(logFile==NULL) 
		{
			functions.logger(NULL, instanceName, fmiError, "error", "Can't open %s for logging",log_filename.c_str());
			return NULL;
		}
		obInstance->setupLogFile(logFile);
	}

	//get OBM parameters only one time
  	if(bGetMovement == false){
        string prWorkDir = szWorkDir;

        string movementResultFile = prWorkDir;
        movementResultFile += "/output.csv";
        std::string error;

        /// Check whether user has the movment results
        bool doMovementCalculation = obInstance->getSimulationSettings()->doMovementCalculation();                       

        /// The movement calculation is only calculated once per simulation.
        /// Once the movement calculation is done, a file flag will be generated.
        /// Check the movement file flag to determine running movement calculation or not.
        string movementResultByOccupantFile = prWorkDir;
        string movementResultByRoomFile = prWorkDir;
        string movementResultForIDFFile = prWorkDir;
        string movementResultIDFFile = prWorkDir;
        if (doMovementCalculation) {
            movementResultByOccupantFile += "/output_by_Occupant.csv";
            movementResultByRoomFile += "/output.csv";
            movementResultForIDFFile += "/output_IDF.csv";
            movementResultIDFFile += "/output_IDF.idf";
        
            if (obInstance->isMoveCalcFileFlag() == false)  {
                logger(" Perform movement calculation...\n");
                /// Perform movement calculation
                if (!obInstance->performMovementCalculation(movementResultFile, error)) {
					functions.logger(NULL, instanceName, fmiError, "error", "Fail to perform occupancy schedule simulation. Error: %s",error.c_str());
                } else {
                    std::cout << "Finish occupancy simulation.\n";
                }
                obInstance->setMoveCalcFileFlag();
            }
        }else{
            movementResultByOccupantFile += "/";
            movementResultByOccupantFile += obInstance->getSimulationSettings()->getUserMovementResultFilename();
        }
        
		if(!obInstance->readMovementCSV(movementResultByOccupantFile,error)){
			functions.logger(NULL, instanceName, fmiError, "error", "Fail to read the movement results from %s. Error: %s",movementResultByOccupantFile.c_str(),error.c_str());
			return NULL;
		}

 		bGetMovement=true;
	}
	
	if(bLogfmi2InstantiateOnce)
		bLogfmi2Instantiate = false;

	
	if(fp==NULL && obInstance->getSimulationSettings()->shouldExportCSVResults()){
		/// functions.logger(NULL, instanceName, fmiOK, "Information", "  Init Result File at %s.",result_filename.c_str());
		fopen_s(&fp, result_filename.c_str(),"w"); 
		if(fp==NULL) {
			functions.logger(NULL, instanceName, fmiError, "error", "Can't open %s for writing results.",result_filename.c_str());
			return NULL;
		}else{
			fprintf(fp,"Time,Temperature (C),Illuminance(lux),CO2 (ppm),LPD (w),Ourdoor air temperature (C), Outdoor rain indicator, AC Schedule, Lighting Schedule,Window Schedule,Plug Load Schedule,Thermostat Setpoint (C),Shade and Blind Schedule, Occupant Amount\n");
		}
	}	
	
	functions.logger(NULL, instanceName, fmiOK, "Information", "fmi2Instantiate done");
	return component;
}

/// Initialize an slave
/// -- Set the initial input and output values
/// -- Perform movement calculation or read movement calculation results
/// -- Process the obXML file
fmiStatus DllExport fmiInitializeSlave(fmiComponent c, fmiReal tStart, fmiBoolean StopTimeDefined, fmiReal tStop) {
	iCurrentTimeStep = 0;
	return fmiOK;
}

/// Free the instance 
/// --- Clean the movement flag and close the results file
void DllExport fmiFreeSlaveInstance(fmiComponent c) {
	showLog = bLogfmiFreeSlaveInstance;
	logger("fmiFreeSlaveInstance start\n");
	ModelInstance* component = (ModelInstance *) c;
	component->functions.freeMemory(component->inputs);
	component->functions.freeMemory(component->outputs);
	component->functions.freeMemory(component);
#ifdef _WIN
	/// Clean the movement file flag
	obInstance->deleteMoveCalcFileFlag();
#endif
	
	iCurrentTimeStep = 0;

	// Free the results file
	if(fp != NULL) {
		fclose(fp);
		fp=NULL;
	}
	if(logFile != NULL){
		fclose(logFile);
		logFile=NULL;
	}
	logger("fmiFreeSlaveInstance done\n");	
	if(bLogfmiFreeSlaveInstanceOnce)
		bLogfmiFreeSlaveInstance = false;
}

/// Perform the time step interaction solver
/// --- AC Control
/// --- Lighting control
/// --- Window Action
/// Attribute:
/// --- currentCommunicationPoint:seconds
/// --- communicationStepSize: seconds
fmiStatus DllExport fmiDoStep(fmiComponent c, fmiReal currentCommunicationPoint,
	fmiReal communicationStepSize, fmiBoolean newStep){
	showLog = bLogfmiDoStep;
	logger("fmiDoStep start\n");
	ModelInstance* component = (ModelInstance *) c;

	int curMon, curDay, hour, min;
    int numOfStepPerHour = obInstance->getSimulationSettings()->getNumberofTimestepsPerHour();
	int totalStepsPerDay = numOfStepPerHour * 24;
	int startDayNum = obInstance->getSimulationSettings()->getStartDayofYear();
	int curDayNum = startDayNum + (iCurrentTimeStep+1) / totalStepsPerDay;
	getMonDay(&curMon, &curDay, curDayNum);
	getHourMin((iCurrentTimeStep+1) % totalStepsPerDay, numOfStepPerHour, hour, min);

	string error;
	if(logFile != NULL)
		fprintf(logFile,"%i\n", iCurrentTimeStep);

	map<Parameter::ParameterType,double> parameterValues;
	parameterValues[Parameter::Parameter_RoomTemperature] = component->inputs[0];
	parameterValues[Parameter::Parameter_RoomWorkPlaneDaylightIlluminance] = component->inputs[1];
	parameterValues[Parameter::Parameter_RoomCO2Concentration] = component->inputs[2];
	parameterValues[Parameter::Parameter_RoomLightsPowerDensity] = component->inputs[3];
	parameterValues[Parameter::Parameter_OutdoorDryBulbTemperature] = component->inputs[4];
	parameterValues[Parameter::Parameter_OutdoorRainIndicator] = component->inputs[5];

	map<Space::OutputParameterType, double> outputValues;
	outputValues[Space::Output_HVAC_Schedule] = component->outputs[0];
	outputValues[Space::Output_Light_Schedule] = component->outputs[1];
	outputValues[Space::Output_Infiltration_Schedule] = component->outputs[2];
	outputValues[Space::Output_PlugLoad_Schedule] = component->outputs[3];
	outputValues[Space::Output_Thermostat_Schedule] = component->outputs[4];
	outputValues[Space::Output_ShadeAndBlind_Schedule] = component->outputs[5];
	outputValues[Space::Output_Occupant_Schedule] = component->outputs[6];

	OB::Space* space = obInstance->getCurrentFMUSpace();
	
	if(!space->performCalculation(iCurrentTimeStep, parameterValues, outputValues, error, logFile)){
		error += "Fail to perform calculation at time step " + std::to_string((uint64_t)iCurrentTimeStep) + ".\n";
		std::cout  << error << endl;
		return fmiError;
	}
	component->outputs[0] = outputValues[Space::Output_HVAC_Schedule];
	component->outputs[1] = outputValues[Space::Output_Light_Schedule];
	component->outputs[2] = outputValues[Space::Output_Infiltration_Schedule];
	component->outputs[3] = outputValues[Space::Output_PlugLoad_Schedule];
	component->outputs[4] = outputValues[Space::Output_Thermostat_Schedule];
	component->outputs[5] = outputValues[Space::Output_ShadeAndBlind_Schedule];
	component->outputs[6] = outputValues[Space::Output_Occupant_Schedule];

	if(logFile != NULL)
		fprintf(logFile,"%i,%d/%d %d:%d:00,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", iCurrentTimeStep, curMon, curDay, hour, min,component->inputs[0], component->inputs[1], component->inputs[2], component->inputs[3], component->inputs[4],component->inputs[5],component->outputs[0],component->outputs[1],component->outputs[2],component->outputs[3],component->outputs[4],component->outputs[5],component->outputs[6]);
	
	// Write the results to results file
	if(fp != NULL) 
		fprintf(fp,"%d/%d %d:%d:00,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", curMon, curDay, hour, min,component->inputs[0], component->inputs[1], component->inputs[2], component->inputs[3], component->inputs[4],component->inputs[5],component->outputs[0],component->outputs[1],component->outputs[2],component->outputs[3],component->outputs[4],component->outputs[5],component->outputs[6]);

	iCurrentTimeStep++;
	logger("fmiDoStep done\n");
	if(bLogfmiDoStepOnce) bLogfmiDoStep= false;

	return fmiOK;
}

/// Exchange data with co-simulation manager
/// --- Write outputs from obFMU to co-simulation manager (EnergyPlus)
fmiStatus DllExport fmiGetReal(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiReal value[]) {
	showLog = bLogfmiGetReal;
	logger("fmiGetReal start\n");
	ModelInstance* component = (ModelInstance *) c;
	for(int i=0; i< nvr; i++){
		value[i] = component->outputs[vr[i]-101];
	}
	logger("fmiGetReal done\n");

	if(bLogfmiGetRealOnce)
		bLogfmiGetReal= false;
	return fmiOK;
}

/// Exchange data with co-simulation manager
/// --- Read inputs from co-simulation manager (EnergyPlus) to obFMU
fmiStatus DllExport fmiSetReal(fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiReal value[]){  
	showLog = bLogfmiSetReal;
	logger("fmiSetReal start\n");
	ModelInstance* component = (ModelInstance *) c;
	for(int i=0; i< nvr; i++){
		component->inputs[vr[i]-201] = value[i];
	}
	logger("fmiSetReal done\n");

	if(bLogfmiSetRealOnce)
		bLogfmiSetReal= false;
	return fmiOK;
}

#endif /// FMI_V2