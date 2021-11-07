

#include "ObRandomMovementEvent.h"
#include "ObMoveMatrixModel.h"

#include <cmath>

#include "nlopt.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cassert>

static int func_count = 0; 
static int func_CurrentRowOrColumn = 0;
static int func_MatSize = 0;
static vector<ObMoveMatrixModel*> func_models;

namespace OB {

	RandomMovementEvent::RandomMovementEvent() {

	}

	bool RandomMovementEvent::readObXML(TiXmlElement *element, std::string& error){
		bool hasOccupancy = false;
		for(const TiXmlNode* child = element->FirstChild(); child; child = child->NextSibling()) { 
			if (child->Type() != TiXmlNode::ELEMENT)	continue;
			TiXmlElement *childElement = (TiXmlElement*)child; 
			if (!strcmp(child->Value(), "SpaceOccupancy")) {
				hasOccupancy = true;
				string spaceCategory;
				double percentValue;
				double durationMinValue;
				for(const TiXmlNode* secondChild = childElement->FirstChild(); secondChild; secondChild = secondChild->NextSibling() ) 
				{ 
					if (secondChild->Type() != TiXmlNode::ELEMENT)	continue;
					TiXmlElement *secondChildElement = (TiXmlElement*)secondChild; 
					
					if (!strcmp(secondChild->Value(), "SpaceCategory")) {
						spaceCategory = secondChildElement->GetText();
					} else if (!strcmp(secondChild->Value(), "PercentTimePresence")) {
						percentValue = atof(secondChildElement->GetText()) / 100;
	
					} else if (!strcmp(secondChild->Value(), "Duration")) {
						if(!getXsDurationInMinutes(secondChildElement->GetText(), durationMinValue)){
							error += "Fail to get Duration.\n";
							return false;
						}
					} else {
						error += "Find unexpected Type: (";
						error += secondChild->Value();
						error += ") in StatusTransitionEvent element.\n";
						return false;
					} 
				}
				if (spaceCategory.compare("OwnOffice") == 0) {
					m_spaceCategoryPercent.insert(std::pair<SpaceCategory, double>(SPACE_OWN, percentValue));
					m_spaceCategoryDuration.insert(std::pair<SpaceCategory, double>(SPACE_OWN, durationMinValue));
				} else if (spaceCategory.compare("OtherOffice") == 0) {
					m_spaceCategoryPercent.insert(std::pair<SpaceCategory, double>(SPACE_OTHER, percentValue));
					m_spaceCategoryDuration.insert(std::pair<SpaceCategory, double>(SPACE_OTHER, durationMinValue));
				} else if (spaceCategory.compare("MeetingRoom") == 0) {
					m_meetingPercent = percentValue;
				} else if (spaceCategory.compare("AuxRoom") == 0) {
					m_spaceCategoryPercent.insert(std::pair<SpaceCategory, double>(SPACE_AUX, percentValue));
					m_spaceCategoryDuration.insert(std::pair<SpaceCategory, double>(SPACE_AUX, durationMinValue));
				} else if (spaceCategory.compare("Outdoor") == 0) {
					m_spaceCategoryPercent.insert(std::pair<SpaceCategory, double>(SPACE_OUT, percentValue));
					m_spaceCategoryDuration.insert(std::pair<SpaceCategory, double>(SPACE_OUT, durationMinValue));
				} else {
					error += "Find unexpected Type: (";
					error += spaceCategory;
					error += ") in StatusTransitionEvent element.\n";
					return false;
				}
			} else {
				error += "Find unexpected Type: (";
				error += child->Value();
				error += ") in StatusTransitionEvent element.\n";
				return false;
			} 
		}
		if (!hasOccupancy) {
			error+= "Fail to find the Type element.\n";
			return false;
		}
		return true;
	}

	/// Initialize space category mapping for the simulation building
	void RandomMovementEvent::initLocationCategory(vector<vector<int>> locationIdMap) {
		m_locationCategoryIdMap = locationIdMap;
	}

	///Initialize random movement events calculation inputs
	bool RandomMovementEvent::initEvent(int stepPerHour, std::string& error) {
		double totalPercent = 1 - m_meetingPercent;
		double minPerStep = 60 / (double)stepPerHour;
		m_vecT.push_back(m_spaceCategoryDuration.at(SPACE_OUT) / minPerStep);
		m_vecT.push_back(m_spaceCategoryDuration.at(SPACE_OWN) / minPerStep);
		m_vecT.push_back(m_spaceCategoryDuration.at(SPACE_OTHER) / minPerStep);
		m_vecT.push_back(m_spaceCategoryDuration.at(SPACE_AUX) / minPerStep);
		m_vecP.push_back(m_spaceCategoryPercent.at(SPACE_OUT) / totalPercent);
		m_vecP.push_back(m_spaceCategoryPercent.at(SPACE_OWN) / totalPercent);
		m_vecP.push_back(m_spaceCategoryPercent.at(SPACE_OTHER) / totalPercent);
		m_vecP.push_back(m_spaceCategoryPercent.at(SPACE_AUX) / totalPercent);
		if (!calculateMatrix(error)) {
			return false;
		} else {
			return true;
		}
	}


	double RandomMovementEvent::getMeetingPercent() {
		return m_meetingPercent;
	}
	
	///Calculate homogeneous markov matrix
	bool RandomMovementEvent::calculateMatrix(std::string& error) {
		/// Get matrix size and objects size based on m_locationCategoryIdMap, check mapping with VecT, VecP
		/// m_locationCategoryIdMap: 0: office, 1: meeting, 2: other rooms, 3: outdoor
		/// out||own||other other||aux aux
		int matSize = 0;
		vector<int> objectSizeList;
		if (m_locationCategoryIdMap[3].size() > 0) {
			matSize += 1;
			objectSizeList.push_back(1); //out
		} else {
			error += "Missing outdoor space element. \n";
			cout << error;
		}
		if (m_locationCategoryIdMap[0].size() > 0) {
			matSize += 1;
			objectSizeList.push_back(1); //own
			if (m_locationCategoryIdMap[0].size() > 2) {
				matSize += 2;
				objectSizeList.push_back(2); //other
			} else {
				///TODO: check prob of other = 0, else false
				matSize += 1;
				objectSizeList.push_back(1); //other
			}
		} else {
			error += "Missing office space element. \n";
			cout << error;
		}
		if (m_locationCategoryIdMap[2].size() > 1) {
			matSize += 2;
			objectSizeList.push_back(2); //aux
		} else if (m_locationCategoryIdMap[2].size() == 1) {
			matSize += 1;
			objectSizeList.push_back(1); //aux
		} else {

			error += "Missing auxiliary space element. \n";
			cout << error;
		}

		/// Optimization
		//Matrix Mat(matSize, matSize);
		vector<vector<double>> mat;
		mat.resize(matSize);
		for (size_t i = 0; i < mat.size(); i++) {
			mat[i].resize(matSize);
		}
		optMatrix(mat, m_vecT, m_vecP, matSize, objectSizeList);
		//Matrix& mat = Mat;
		m_movementMat.resize(matSize);
		for (size_t i = 0; i < m_movementMat.size(); i++) {
			m_movementMat[i].resize(matSize);
		}
		for (size_t i = 0; i < m_movementMat.size(); i++) {
			double temp = 0; 
			for (size_t j = 0; j < m_movementMat.size(); j++) {
				temp += mat[i][j];
				m_movementMat[i][j] = temp; 
			}
		}

		return true;
	}

	int RandomMovementEvent::updateLocation(int lastLocation, int ownLocationId) {
		int result = lastLocation;
		int matRow = 0;
		int matCol = 0;
		vector<vector<int>> locationIdMap;  /// 0: out, 1:own, 2: other office; 3:aux
		vector<int> ownId;
		ownId.push_back(ownLocationId);
		vector<int> otherId;
		for (size_t i = 0; i < m_locationCategoryIdMap[0].size(); i++) {
			if (m_locationCategoryIdMap[0][i] != ownLocationId) {
				otherId.push_back(m_locationCategoryIdMap[0][i]);
			}
		}
		locationIdMap.push_back(m_locationCategoryIdMap[3]);
		locationIdMap.push_back(ownId);
		locationIdMap.push_back(otherId);
		locationIdMap.push_back(m_locationCategoryIdMap[2]);
		


		//Find corresponding row in matrix given lastLocation
		
		bool found = false;
		for (size_t i = 0; i < locationIdMap.size(); i++) {
			found = false;
			for (size_t j = 0; j < locationIdMap[i].size(); j++) {
				if (locationIdMap[i][j] == lastLocation) {
					found = true;
					break;
				}
			}
			if (found) {
				break;
			} else { 
				if (locationIdMap[i].size() > 1) {
					matRow += 2;
				} else if (locationIdMap[i].size() == 1) {
					matRow += 1;
				}
			}
		}
		if (matRow >= m_movementMat.size()) {
			cout << "Mat row oversized.. "<< matRow << " \n";
			return result;
		}
		//Find corresponding col in matrix given generated random number
		double rNum = (double)rand() / (double)RAND_MAX;
		for (size_t i = 0; i < locationIdMap.size(); i++) {
			if (locationIdMap[i].size() == 1) {
				// Single 1 * 1 object
				if (rNum < m_movementMat[matRow][matCol]) {
					//transit
					result = locationIdMap[i][0];
					break;
				}
				matCol += 1;
			} else if (locationIdMap[i].size() >= 2) {
				// Multiple 2 * 2 object - > 1*2?
				if (matCol >= m_movementMat[matRow].size()) {
					cout << "Mat col oversized..\n ";
					return result;
				}
				if (rNum < m_movementMat[matRow][matCol]) {
					//fall into upper left
					for (size_t j = 0; j < locationIdMap[i].size(); j++) {
						if (locationIdMap[i][j] == lastLocation) {
							result = lastLocation;
							break;
						}
					}
					int index = rand() % locationIdMap[i].size();
					result = locationIdMap[i][index];
					break;
				}
				matCol += 1;
				if (matCol >= m_movementMat[matRow].size()) {
					cout << "Mat col oversized..\n ";
					return result;
				}
				if (rNum < m_movementMat[matRow][matCol]) {
					//fall into upper right - same category excluding last location
					result = lastLocation;
					while (result == lastLocation) {
						int index = rand() % locationIdMap[i].size();
						result = locationIdMap[i][index];
					}
					break;
				}
				matCol += 1;
			}
		}
		return result;
	}

	/// In this case, we NLOPT_LN_COBYLA algorithm.
	/// NLOPT_LN_COBYLA constant refers to the COBYLA algorithm (described below), 
	///    which is a local (L) derivative-free (N) optimization algorithm. 
	/// The gradient (double* grad) is only needed for gradient-based algorithms; 
	///   if you use a derivative-free optimization algorithm, grad will always be NULL and you need never compute any derivatives.
	double objFun(unsigned n, const double* x, double* grad, void* data)  
	{	
		++func_count;
		ObMoveMatrixModel* model = (ObMoveMatrixModel*)data;
		model->updateData(x);
		double cons = model->getObjectFunctions();
		//std::cout << "  obj cons: " << cons << endl;
		return cons;

	}


	double row_constraint(unsigned n, const double* x, double* grad, void* data)
	{
		ObMoveMatrixModel* model = reinterpret_cast<ObMoveMatrixModel*>(data);
		model->updateCurrentRowOrColumn(func_CurrentRowOrColumn);
		model->updateData(x);
		double cons =  model->rowConstraint();
		//std::cout << "  row cons: " << cons << endl;
		return cons;

	}

	double col_constraint(unsigned n, const double* x, double* grad, void* data)
	{
		ObMoveMatrixModel* model = reinterpret_cast<ObMoveMatrixModel*>(data);
		model->updateCurrentRowOrColumn(func_CurrentRowOrColumn);
		model->updateData(x);
		double cons =  model->colConstraint();
		//std::cout << "  col cons: " << cons << endl;
		func_CurrentRowOrColumn = (func_CurrentRowOrColumn+1)%func_MatSize;
		return cons;
	}

	double pai_constraint(unsigned n, const double* x, double* grad, void* data)
	{
		ObMoveMatrixModel* model = reinterpret_cast<ObMoveMatrixModel*>(data);
		model->updateData(x);	
		double cons =  model->paiConstraint();
		//std::cout << "pai cons: " << cons << endl;
		return cons;

	}


	//first: Est ; second: Pi
	 bool RandomMovementEvent::optMatrix(vector<vector<double>>& mat, const std::vector<double>& VecT, 
		 const std::vector<double>& VecP, int matSize, vector<int> objSizeList) {
		/// Create an data model
		ObMoveMatrixModel* model = new ObMoveMatrixModel(VecT, VecP, matSize, objSizeList);

		/// Check whether similar to any existing model
		for(size_t i=0; i < func_models.size(); i++){
			if(func_models[i]->similar(*model)){
				model->updateMatrix(mat);
				free(model);
				return true;
			}
		}

		/// The size of the matrix
		size_t matrix_size = model->getMatrixSize();
		/// The size of all the unknowns
		size_t size = model->getVariableSize();
		//double *lb(get_lower_bound(VecT, VecP)), *ub(get_upper_bound(VecT, VecP));
		double* lb = model->getLowerBound();
		double* ub = model->getUpperBound();
		double minf;
		double* revec = (double*)malloc(size * sizeof(double));
		model->initData(revec);
		model->updateData(revec);
	
		/// Create the nlopt with the size of unknown parameters
		/// NLOPT_LN_COBYLA constant refers to the COBYLA algorithm (described below), which is a local (L) derivative-free (N) optimization algorithm. 
		/// The gradient is only needed for gradient-based algorithms; if you use a derivative-free optimization algorithm, grad will always be NULL and you need never compute any derivatives.
		nlopt_opt optProb = nlopt_create(NLOPT_LN_COBYLA, size);
		/// Set the lower and upper bounds of the unknown parameters
		nlopt_set_lower_bounds(optProb, lb);
		nlopt_set_upper_bounds(optProb, ub);

		/// set the object function
		nlopt_set_min_objective(optProb, objFun, model);

		/// Set the constraints
		func_CurrentRowOrColumn = 0;
		func_MatSize = matrix_size;
		for(size_t i=0; i < matrix_size; i++){
			//std::cout << "index: " << i << endl;
			model->updateCurrentRowOrColumn(i);
			nlopt_add_equality_constraint(optProb, row_constraint, model, 0.001);
			nlopt_add_equality_constraint(optProb, col_constraint, model, 0.001);
		}
		nlopt_add_equality_constraint(optProb, pai_constraint, model, 0.001);


		/// Set the tolrence
		nlopt_set_stopval(optProb, 0.001);

		/// Run the calculation
		nlopt_result Re = nlopt_optimize(optProb, revec, &minf);
		model->updateData(revec);
		model->updateMatrix(mat);
		//writeMatrix(mat, VecT.size(), "C:/Users/YixingChen/Desktop/test02.csv");

		free(revec);
		free(lb);
		free(ub);
		nlopt_destroy(optProb);
		lb = NULL; 
		ub = NULL; 
		revec = NULL;	

		func_models.push_back(model);
		return true;
	}



}