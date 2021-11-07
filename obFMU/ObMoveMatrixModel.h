#ifndef OBMOVEMATRIXMODEL_H
#define OBMOVEMATRIXMODEL_H

#include <string>
#include <vector>
#include <iostream>
#include "ObMoveMatrixObject.h"

using namespace std;

class ObMoveMatrixModel{
public:
	ObMoveMatrixModel(const std::vector<double>& VecT, const std::vector<double>& VecP, 
		int matSize, const std::vector<int>& objSizeList);
	~ObMoveMatrixModel();

	/// There are not copy, assign or comparision in this class.
	//MoveMatrixModel(const MoveMatrixModel& other);
	//MoveMatrixModel& operator= (const MoveMatrixModel&);
	//bool operator==(const MoveMatrixModel&);

	/// Generate the init value for the unkonwn variables;
	void	initData(double* x);

	/// Update the value based on the given x;
	void	updateData(const double* x); 

	/// Get the number of unknown variables, including:
	/// 1. The probabilities in upper right and lower left
	/// 2. The calculated Pai
	size_t	getVariableSize();

	/// Get the size of the simplified matrix
	size_t getMatrixSize();

	/// Get the lower bound of the unknown variables
	double* getLowerBound();
	
	/// Get the upper bound of the unknown variables
	double* getUpperBound();

	/// Get the objective function results
	double getObjectFunctions();

	/// Update the current row or column number
	void	updateCurrentRowOrColumn(int index);

	/// get the row constraint
	/// 1.0 - sum(Pij) for row i
	double	rowConstraint();
	
	/// get the column constraint
	/// Pai(j) - sum(Pij*Pai(ij)) for column j
	double	colConstraint();

	/// Get the pai constraint
	double paiConstraint();

	/// update the matrix
	void	updateMatrix(vector<vector<double>>& mat);

	/// Check whether this model is simiar to the given model
	/// If similar, also assgin the results from this model to the given model
	bool	similar(ObMoveMatrixModel& other);


private:
	vector<ObMoveMatrixObject*>	m_objects;		/// The list of all the parameters
	int		m_matSize;
	int		m_currentRowOrColumn;				/// For calculating the row or column constraints
	vector<int>					m_nullIndex;	/// The index where probability is 0.	

};

#endif
