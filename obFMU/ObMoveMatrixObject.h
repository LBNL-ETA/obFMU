#ifndef OBMOVEMATRIXOBJECT_H
#define OBMOVEMATRIXOBJECT_H
#include <string>
#include <vector>
using namespace std;
#define PROBABILITY_TOL 0.0001

class ObMoveMatrixObject {
public:
	enum ObjectType {
		Type_Single,		/// Only one object in the matrix
		Type_Multiple		/// Multiple objects in the matrix
	};

	ObMoveMatrixObject(double estimateT, double probability, int matIndex, int objIndex, ObjectType type);
	~ObMoveMatrixObject();

	void setModelSize(int size);

	/// There are not copy, assign or comparision in this class.
	//MoveMatrixObject(const MoveMatrixObject& other);
	//MoveMatrixObject& operator= (const MoveMatrixObject&);
	//bool operator==(const MoveMatrixObject&);

	/// Add a pair into the list
	/// Return true if it is added to this object
	/// Return false if the estimateT and the probability is not the same.
	bool	addPair(double estimateT, double probability, int matIndex);
	
	/// Init the data
	void	initData(double* x, int& currentIndex);

	/// Update the data
	void	updateData(const double* x, int& currentIndex);

	/// Get the variable size of this object
	size_t		getVariableSize();

	/// Get the object function
	double		getObjectFunction();

	/// Get the Pii, and PUpperRight for Type_Multiple
	double		getPii_PUpperRight();
	
	/// Get the Pii, and PLowerLeft for Type_Multiple
	double		getPii_PLowerLeft();

	/// Get the Pij based on the given row (i)
	double		getPij(int row);

	/// Get the original mat indexes size 
	int			gerMatIndexSize();

	/// Get the Pai
	double		getPai();

	/// Get the Pij*Pai on the given column (j)
	double		getPijPai(int column);

	/// Get the mat indexes
	vector<int>	gerMatIndex();

	/// Update the results matrix
	void		updateMatrix(vector<vector<double>>& mat, vector<vector<int>> matIndexSizes);

	/// Check whether is similar, don't assgin the value yet
	bool		similar(ObMoveMatrixObject& other);

	/// Assign the value to other object
	void		assignTo(ObMoveMatrixObject& other, vector<int>	mappingIndex);

private:
	ObjectType		m_type;				/// The type
	int				m_index;			/// The index of the parameter.
	int				m_modelSize;		/// The size of objects in the model;
	double			m_estimateT;		/// The estimate time
	double			m_Pai;				/// The true Pai;
	double			m_Pii;				/// The probability of Pii
	vector<int>		m_matIndexes;		/// The list of original matrix indexes;

	/// Unknows:
	double			m_calulatedPai;			/// The calucated Pai, reference to X

	/// The probability of moving from this object (i) to other objects (j), reference to X; 
	//	m_PijValues[m_index] = NUll, so the index of other Pij can be directly used.
	vector<double>	m_PijValues;			

	/// Only for Type_Multiple
	double		m_Pii_corner;		/// The probability of moving within this object from i to i+1; uppper right or lower left of the box, reference to X
};
#endif
