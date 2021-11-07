/* ----------------------------------------------------------------------------
** This file is the wrapper that provide the interfaces using FMI standards.   
** It is the starting point of the obFMU.
** Author: Hongsan Sun, Yixing Chen
** History:
** 6/26/2015: first release
** -----------------------------------------------------------------------------*/

#ifndef OBFMU_H
#define OBFMU_H
#include "ObOccupantBehavior.h"

/// The model identifier string.
#define MODEL_IDENTIFIER obFMU

// Globally unique ID used to make sure the XML file and the DLL match.
// The following was generated at http://guid.us
#define MODEL_GUID "{7b2d6d3f-ac4d-4aa8-93eb-d53357dc58ec}"


#ifdef FMI_V2


/// The model identifier string.
#define MODEL_IDENTIFIER obFMU2

// Globally unique ID used to make sure the XML file and the DLL match.
// The following was generated at http://guid.us
#define MODEL_GUID "{fed5964c-e9f4-4d18-8c6f-fdf3fd96d4bf}"


/// The FMI is coding in C, so need the extern "C" to include the FMI 
#ifdef __cplusplus  
	extern "C" {
#endif
	/// The FMI standard can be used for co-simulation as well as Model exchange
	#include "fmi2Functions.h"
#ifdef __cplusplus  
	}
#endif

// Data structure for a fmi2Component of this FMU.
typedef struct {
	// Use a pointer to a fmiReal so that we can allocate space for it.
	fmi2Real						*inputs;
	fmi2Real						*outputs;
	const fmi2CallbackFunctions* 	functions;
	fmi2String						instanceName;
} ModelInstance;

// Data structure for a fmi2FMUstate of this FMU.
typedef struct {
	// Use a pointer to a fmiReal so that we can allocate space for it.
	fmi2Real						*inputs;
	fmi2Real						*outputs;
	int								iCurrentTimeStep;
} StateInstance;



FMI2_Export fmi2String fmi2GetTypesPlatform(){
	return fmi2TypesPlatform;
};

fmi2String FMI2_Export fmi2GetVersion(){
	return fmi2Version;
}

FMI2_Export fmi2Status fmi2SetDebugLogging(fmi2Component c, fmi2Boolean loggingOn, size_t nCategories, const fmi2String categories[]){
	return fmi2OK;
}

FMI2_Export fmi2Status fmi2SetupExperiment(fmi2Component c,fmi2Boolean toleranceDefined,fmi2Real tolerance,fmi2Real startTime,fmi2Boolean stopTimeDefined,fmi2Real stopTime){	
	return fmi2OK;
}

FMI2_Export fmi2Status fmi2EnterInitializationMode(fmi2Component c){
	return fmi2OK;
}

FMI2_Export fmi2Status fmi2ExitInitializationMode(fmi2Component c){
	return fmi2OK;
}

/// This function is used to report error for accessing functions not used in obFMU
fmi2Status reportError(fmi2Component c, string functionName){
	ModelInstance* component = (ModelInstance *) c;
	component->functions->logger(NULL, functionName.c_str(), fmi2Error, "Error", "The function (%s) is not used in obFMU. Please check!",functionName.c_str());
	return fmi2Error;
}

FMI2_Export fmi2Status fmi2GetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[]){
	return reportError(c, "fmi2GetInteger");
}

FMI2_Export fmi2Status fmi2GetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[]){
	return reportError(c, "fmi2GetBoolean");
}

FMI2_Export fmi2Status fmi2GetString (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2String  value[]){
	return reportError(c, "fmi2GetString");
}

FMI2_Export fmi2Status fmi2SetInteger (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[]){
	return reportError(c, "fmi2SetInteger");
}

FMI2_Export fmi2Status fmi2SetBoolean (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[]){
	return reportError(c, "fmi2SetBoolean");
}

FMI2_Export fmi2Status fmi2SetString  (fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2String  value[]){
	return reportError(c, "fmi2SetString");
}

FMI2_Export fmi2Status fmi2GetStatus       (fmi2Component c, const fmi2StatusKind s, fmi2Status*  value){
	return reportError(c, "fmi2GetStatus");
};

FMI2_Export fmi2Status fmi2GetRealStatus   (fmi2Component c, const fmi2StatusKind s, fmi2Real*    value){
	return reportError(c, "fmi2GetRealStatus");
};

FMI2_Export fmi2Status fmi2GetIntegerStatus(fmi2Component c, const fmi2StatusKind s, fmi2Integer* value){
	return reportError(c, "fmi2GetIntegerStatus");
};

FMI2_Export fmi2Status fmi2GetBooleanStatus(fmi2Component c, const fmi2StatusKind s, fmi2Boolean* value){
	return reportError(c, "fmi2GetBooleanStatus");
};

FMI2_Export fmi2Status fmi2GetStringStatus (fmi2Component c, const fmi2StatusKind s, fmi2String*  value){
	return reportError(c, "fmi2GetStringStatus");
};

FMI2_Export fmi2Status fmi2SerializedFMUstateSize(fmi2Component c, fmi2FMUstate FMUstate,size_t *size){
	return reportError(c, "fmi2SerializedFMUstateSize");
};
FMI2_Export fmi2Status fmi2SerializeFMUstate (fmi2Component c, fmi2FMUstate FMUstate, fmi2Byte serializedState[], size_t size){
	return reportError(c, "fmi2SerializeFMUstate");
};
FMI2_Export fmi2Status fmi2DeSerializeFMUstate (fmi2Component c,const fmi2Byte serializedState[],size_t size, fmi2FMUstate* FMUstate){
	return reportError(c, "fmi2DeSerializeFMUstate");
};

FMI2_Export fmi2Status fmi2GetDirectionalDerivative(fmi2Component c,const fmi2ValueReference vUnknown_ref[], size_t nUnknown,const fmi2ValueReference vKnown_ref[] , size_t nKnown,const fmi2Real dvKnown[],fmi2Real dvUnknown[]){
	return reportError(c, "fmi2GetDirectionalDerivative");
}

FMI2_Export fmi2Status fmi2CancelStep(fmi2Component c){
	return fmi2OK;
};

FMI2_Export fmi2Status fmi2Terminate(fmi2Component c){
	return fmi2OK;
}

FMI2_Export fmi2Status fmi2SetRealInputDerivatives(fmi2Component c,const  fmi2ValueReference vr[], size_t nvr,const  fmi2Integer order[], const  fmi2Real value[]){
	/// Just return fmi2Error as the derivatives are not used in OB models
	return reportError(c, "fmi2SetRealInputDerivatives");
};

FMI2_Export fmi2Status fmi2GetRealOutputDerivatives(fmi2Component c, const fmi2ValueReference vr[], size_t  nvr, const fmi2Integer order[],fmi2Real value[]){
	/// Just return fmi2Error as the derivatives are not used in OB models	
	return reportError(c, "fmi2GetRealOutputDerivatives");
};

#else


/// The model identifier string.
#define MODEL_IDENTIFIER obFMU

// Globally unique ID used to make sure the XML file and the DLL match.
// The following was generated at http://guid.us
#define MODEL_GUID "{7b2d6d3f-ac4d-4aa8-93eb-d53357dc58ec}"


/// The FMI is coding in C, so need the extern "C" to include the FMI 
#ifdef __cplusplus  
	extern "C" {
#endif
	/// The FMI standard can be used for co-simulation as well as Model exchange
	/// Define FMI_COSIMULATION in "Project Properties->C/C++->Preprocesser->Preprocesser Defination 
	#ifdef FMI_COSIMULATION
		#include "fmiFunctions.h"
	#else
		#include "fmiModelFunctions.h"
	#endif
#ifdef __cplusplus  
	}
#endif

// Data structure for an instance of this FMU.
typedef struct {
	// Use a pointer to a fmiReal so that we can allocate space for it.
	fmiReal					*inputs;
	fmiReal					*outputs;
	fmiCallbackFunctions	functions;
	fmiString				instanceName;
} ModelInstance;


DllExport fmiString fmiGetTypesPlatform()
{
	return "standard32";
};

/// The FMI interfaces that are not used in this project
fmiStatus DllExport fmiTerminateSlave(fmiComponent c) 
{
	return fmiOK;
}
// Added a function which returns the FMI version
fmiString DllExport fmiGetVersion()
{   // This function always returns 1.0
	return "1.0";
}
DllExport fmiStatus fmiGetInteger(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiInteger value[])
{
	return fmiOK;
}
DllExport fmiStatus fmiGetBoolean(fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiBoolean value[])
{
	return fmiOK;
}
DllExport fmiStatus fmiGetString (fmiComponent c, const fmiValueReference vr[], size_t nvr, fmiString  value[])
{
	return fmiOK;
}
DllExport fmiStatus fmiSetInteger (fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiInteger value[])
{
	return fmiOK;
}
DllExport fmiStatus fmiSetBoolean (fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiBoolean value[])
{
	return fmiOK;
}
DllExport fmiStatus fmiSetString  (fmiComponent c, const fmiValueReference vr[], size_t nvr, const fmiString  value[])
{
	return fmiOK;
}
DllExport fmiStatus fmiResetSlave(fmiComponent c)
{  
	return fmiOK;
}
DllExport fmiStatus fmiSetDebugLogging  (fmiComponent c, fmiBoolean loggingOn)
{
	return fmiOK;
};

DllExport fmiStatus fmiGetStatus       (fmiComponent c, const fmiStatusKind s, fmiStatus*  value){
	return fmiOK;
};
   DllExport fmiStatus fmiGetRealStatus   (fmiComponent c, const fmiStatusKind s, fmiReal*    value){
	return fmiOK;
};

   DllExport fmiStatus fmiGetIntegerStatus(fmiComponent c, const fmiStatusKind s, fmiInteger* value){
	return fmiOK;
};

   DllExport fmiStatus fmiGetBooleanStatus(fmiComponent c, const fmiStatusKind s, fmiBoolean* value){
	return fmiOK;
};

   DllExport fmiStatus fmiGetStringStatus (fmiComponent c, const fmiStatusKind s, fmiString*  value){
	return fmiOK;
};

DllExport fmiStatus fmiCancelStep(fmiComponent c){
	return fmiOK;
};
   DllExport fmiStatus fmiSetRealInputDerivatives(fmiComponent c,const  fmiValueReference vr[], size_t nvr,const  fmiInteger order[], const  fmiReal value[]){
	   /// Just return fmiOK as the derivatives are not used in OB models
	   return fmiOK;
   };

   DllExport fmiStatus fmiGetRealOutputDerivatives(fmiComponent c, const fmiValueReference vr[], size_t  nvr, const fmiInteger order[],fmiReal value[]){
	   ModelInstance* component = (ModelInstance *)c;
	   /// We also can't provide the Derivatives as most of the outputs are strongly depended on logic operation rather than numerical operation.
	   for(int i=0; i < nvr; i++){
		   if(order[i]>0){
			   std::cout << "fmiGetRealOutputDerivatives: the "<< i <<"th order (" << order[i] << ") is higher than the maxOutputDerivativeOrder (0).\n";
			   return fmiError;
		   }
			value[i] = 0;
	   }
		return fmiOK;
   };
#endif /// FMI_V2
#endif // OBFMU_H