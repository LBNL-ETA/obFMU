#include "ObMoveMatrixObject.h"
#include <math.h>     /// fabs header file

ObMoveMatrixObject::ObMoveMatrixObject(double estimateT, double probability, int matIndex, int objIndex, ObjectType type) {
	m_type = type;
	m_estimateT = estimateT;
	m_Pai = probability;
	m_matIndexes.push_back(matIndex);
	m_index = objIndex;
	if (probability >= 1.0 - 1e-10) {  // = 1 when very close to 1
		m_Pii = 1.0;
	} else if (probability <= 1e-10) { // = 0 when very close to 0
		m_Pii= 0.0;
	} else {                    
		m_Pii = 1 - 1.0 / estimateT;
	}
};


ObMoveMatrixObject::~ObMoveMatrixObject(){
};

void ObMoveMatrixObject::setModelSize(int size){
	m_modelSize = size;
};

/// Add a pair into the list
/// Return true if it is added to this object
/// Return false if the estimateT and the probability is not the same.
bool ObMoveMatrixObject::addPair(double estimateT, double probability, int matIndex) {
	if (fabs(estimateT - m_estimateT) < PROBABILITY_TOL && fabs(probability - m_Pai) < PROBABILITY_TOL) {
		m_matIndexes.push_back(matIndex);
		m_type = Type_Multiple;
		return true;
	}
	return false;
};

	/// Init the data
void	ObMoveMatrixObject::initData(double* x, int& currentIndex){
	switch(m_type){
	case Type_Multiple:
		x[currentIndex++] = 0.0;
		for(int i=0; i< m_modelSize; i++){
				x[currentIndex++] = 0.0;
		}
		break;
	case Type_Single:
	default:
		x[currentIndex++] = 0.0;
		for(int i=0; i< m_modelSize-1; i++){
				x[currentIndex++] = 0.0;
		}
		break;
	}
	
	m_calulatedPai = 0.0;
	
	for(int i=0; i< m_modelSize; i++){
		m_PijValues.push_back(0.0);
	}
	m_Pii_corner = 0.0;
};

/// Update the data
void ObMoveMatrixObject::updateData(const double* x, int& currentIndex){
	switch(m_type){
	case Type_Multiple:
		m_calulatedPai = x[currentIndex++];
		for(int i=0; i< m_modelSize; i++){
			if( i != m_index){
				m_PijValues[i] = x[currentIndex++];
			}
		}
		m_Pii_corner = x[currentIndex++];
		break;
	case Type_Single:
	default:
		m_calulatedPai = x[currentIndex++];
		for(int i=0; i< m_modelSize; i++){
			if( i != m_index){
				m_PijValues[i] = x[currentIndex++];
			}
		}
		break;
	}
};

/// Get the variable size of this object
size_t ObMoveMatrixObject::getVariableSize(){
	switch(m_type){
	case Type_Multiple:
		return m_modelSize + 1;
	case Type_Single:
	default:
		return m_modelSize;
	}
};

/// Get the object function
double ObMoveMatrixObject::getObjectFunction(){
	double obj = (m_Pai - m_calulatedPai) * (m_Pai - m_calulatedPai) * m_matIndexes.size();
	return obj;
};

/// Get the Pii, and PUpperRight for Type_Multiple
double ObMoveMatrixObject::getPii_PUpperRight(){
	double value = m_Pii;
	switch(m_type){
	case Type_Multiple:
		value += (m_Pii_corner) * (m_matIndexes.size()-1);
		break;
	case Type_Single:
	default:
		break;
	} 
	return value;
};
	/// Get the original mat indexes size 
int ObMoveMatrixObject::gerMatIndexSize(){
	return m_matIndexes.size();
};


/// Get the Pii, and PLowerLeft for Type_Multiple
double ObMoveMatrixObject::getPii_PLowerLeft(){
	double value = m_Pii;
	switch(m_type){
	case Type_Multiple:
		value += (m_Pii_corner) * (m_matIndexes.size()-1);
		break;
	case Type_Single:
	default:
		break;
	} 
	return value;
};

/// Get the Pij based on the given row (i)
double ObMoveMatrixObject::getPij(int row){
	return m_PijValues[row];
};

/// Get the Pai
double ObMoveMatrixObject::getPai(){
	return m_Pai;
};

/// Get the Pij*Pai on the given column (j)
double ObMoveMatrixObject::getPijPai(int column){
	double result = m_matIndexes.size() * m_Pai * (m_PijValues[column]);
	return result;
};
	
/// Get the mat indexes
vector<int>	ObMoveMatrixObject::gerMatIndex(){
	return m_matIndexes;
};

/// Update the results matrix
void ObMoveMatrixObject::updateMatrix(vector<vector<double>>& mat,vector<vector<int>> matIndexSizes){
	/// Update the Pij
	for( size_t i=0; i < m_matIndexes.size(); i++){
		/// for each row
		int row = m_matIndexes[i];
		for(int j=0; j < m_modelSize; j++){
			vector<int> columns = matIndexSizes[j];
			for( size_t k =0; k < columns.size(); k++){
				int column = columns[k];
				if(j != m_index){
					mat[row][column] = m_PijValues[j];
				}else{
					if( row == column){
						mat[row][column] = m_Pii;
					}else{
						mat[row][column] = m_Pii_corner;
					}
				}
			}
		}

	}
};

/// Check whether is similar, don't assgin the value yet
bool ObMoveMatrixObject::similar(ObMoveMatrixObject& other) {
	/// Check the estimiate time
	if (fabs(m_estimateT - other.m_estimateT) > PROBABILITY_TOL)
		return false;

	/// Check the probability
	if (fabs(m_Pai - other.m_Pai) > PROBABILITY_TOL)
		return false;

	/// Check the size of represented elements.
	if (m_matIndexes.size() != other.m_matIndexes.size())
		return false;

	return true;
};

/// Assign the value to other object
void ObMoveMatrixObject::assignTo(ObMoveMatrixObject& other, vector<int>	mappingIndex){
	other.m_calulatedPai = m_calulatedPai;
	other.m_PijValues.clear();
	for(size_t i=0; i < m_PijValues.size(); i++){
		other.m_PijValues.push_back(m_PijValues[mappingIndex[i]]);
	}
	other.m_Pii_corner  = m_Pii_corner;
};
