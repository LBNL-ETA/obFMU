#include "ObMoveMatrixModel.h"

ObMoveMatrixModel::ObMoveMatrixModel(const std::vector<double>& VecT, const std::vector<double>& VecP, int matSize,
	const std::vector<int>& objSizeList) {
	m_matSize = matSize;
	int modelSize = objSizeList.size();
	int matIndex = 0;
	ObMoveMatrixObject::ObjectType type;
	for (int i = 0; i < modelSize; i++) {
		if (VecP[i] <= -1e-10) {
			m_nullIndex.push_back(i);
		} else {
			if (objSizeList[i] > 1) {
				type = ObMoveMatrixObject::Type_Multiple;
			} else {
				type = ObMoveMatrixObject::Type_Single;
			}
			ObMoveMatrixObject* newObj = new ObMoveMatrixObject(VecT[i], VecP[i], matIndex, i, type);
			m_objects.push_back(newObj);
			matIndex += objSizeList[i];
		}
	}
	for(size_t i=0; i< m_objects.size(); i++){
		m_objects[i]->setModelSize(modelSize);
	}
}

ObMoveMatrixModel::~ObMoveMatrixModel(){
	for(size_t i=0; i < m_objects.size(); i++){
		delete m_objects[i];
	}
};

/// Generate the init value for the unkonwn variables;
void	ObMoveMatrixModel::initData(double* x){
	int currentIndex = 0;
	for(size_t i=0; i < m_objects.size(); i++){
		m_objects[i]->initData(x, currentIndex);
	}
};

/// Update the value based on the given x;
void	ObMoveMatrixModel::updateData(const double* x){
	int currentIndex = 0;
	for(size_t i=0; i < m_objects.size(); i++){
		m_objects[i]->updateData(x, currentIndex);
	}
}; 


/// Get the number of unknown variables, including:
/// 1. The probabilities in upper right and lower left
/// 2. The calculated Pai
size_t	ObMoveMatrixModel::getVariableSize(){
	size_t variableSize = 0;
	for(size_t i=0; i < m_objects.size(); i++){
		variableSize += m_objects[i]->getVariableSize();
	}
	return variableSize;
}

/// Get the size of the simplified matrix
size_t ObMoveMatrixModel::getMatrixSize(){
	return m_objects.size();
};

/// Get the lower bound of the unknown variables
double* ObMoveMatrixModel::getLowerBound(){
	size_t variableSize = getVariableSize();
	double* lb = (double*)malloc(variableSize * sizeof(double));
	for(size_t i=0; i < variableSize; i++){
		lb[i] = 0.0;
	}
	return lb;
};
	
/// Get the upper bound of the unknown variables
double* ObMoveMatrixModel::getUpperBound(){	
	size_t variableSize = getVariableSize();
	double* lb = (double*)malloc(variableSize * sizeof(double));
	for(size_t i=0; i < variableSize; i++){
		lb[i] = 1.0;
	}
	return lb;
};

/// Get the objective function results
double ObMoveMatrixModel::getObjectFunctions(){
	double obj = 0.0;
	for( size_t i=0; i < m_objects.size(); i++){
		obj += m_objects[i]->getObjectFunction();
	}

	obj *= m_matSize * m_matSize;
	//std::cout << "obj: " << obj  << endl;
	return obj;
};

/// Update the current row or column number
void	ObMoveMatrixModel::updateCurrentRowOrColumn(int index){
	m_currentRowOrColumn = index;
};


/// get the row constraint
/// 1.0 - sum(Pij) for row i
double	ObMoveMatrixModel::rowConstraint(){
	double result = 1.0 - m_objects[m_currentRowOrColumn]->getPii_PUpperRight();

	for(size_t i=0; i < m_objects.size(); i++){
		if(i != m_currentRowOrColumn){
			result -= m_objects[m_currentRowOrColumn]->getPij(i) * m_objects[i]->gerMatIndexSize();
		}
	}
	return result;
};
	
/// get the column constraint
/// Pai(j) - sum(Pij*Pai(ij)) for column j
double	ObMoveMatrixModel::colConstraint(){
	double result = m_objects[m_currentRowOrColumn]->getPai() * (1 - m_objects[m_currentRowOrColumn]->getPii_PLowerLeft());

	for(size_t i=0; i < m_objects.size(); i++){
		if(i != m_currentRowOrColumn){
			result -= m_objects[i]->getPijPai(m_currentRowOrColumn);
		}
	}
	return result;
};


/// Get the pai constraint
double ObMoveMatrixModel::paiConstraint(){
	double result = 1.0;
	for(size_t i=0; i < m_objects.size(); i++){
		double test = m_objects[i]->getPai();
		result -= m_objects[i]->getPai() * m_objects[i]->gerMatIndexSize();

	}
	return result;
};

/// update the matrix
void	ObMoveMatrixModel::updateMatrix(vector<vector<double>>& mat){
	vector<vector<int>>		matIndexes;
	for(size_t i=0; i < m_objects.size();i++){
		matIndexes.push_back(m_objects[i]->gerMatIndex());
	}

	for(size_t i=0; i < m_objects.size();i++){
		m_objects[i]->updateMatrix(mat, matIndexes);
	}

	/// Update the null index row and column
	for(int i=0; i < m_matSize; i++){
		for(size_t j=0; j < m_nullIndex.size(); j++){
			mat[m_nullIndex[j]][i] = 0.0;
			mat[i][m_nullIndex[j]] = 0.0;
		}
	}
};

/// Check whether this model is simiar to the given model
/// If similar, also assgin the results from this model to the given model
bool	ObMoveMatrixModel::similar(ObMoveMatrixModel& other){
	/// Check the size of the matrix
	if(m_matSize != other.m_matSize)
		return false;
	
	/// Check the sime of the simplified matrix
	if(getMatrixSize() != other.getMatrixSize())
		return false;

	/// Check the number of variables
	if(getVariableSize() != other.getVariableSize())
		return false;

	/// Check the number of null index
	if(m_nullIndex.size() != other.m_nullIndex.size())
		return false;

	vector<int>	mappingIndex;

	/// Check each objects
	for(size_t i=0; i< other.m_objects.size(); i++){
		bool foundSimilar = false;
		for(size_t j=0; j < m_objects.size(); j++){
			if(m_objects[j]->similar(*other.m_objects[i])){
				mappingIndex.push_back(j);
				foundSimilar = true;
				break;
			}
		}
		if(!foundSimilar)
			return false;
	}

	/// Assign the value for each objects
	for(size_t i=0; i< other.m_objects.size(); i++){
		m_objects[mappingIndex[i]]->assignTo(*other.m_objects[i], mappingIndex);
	}
	return true;
};
