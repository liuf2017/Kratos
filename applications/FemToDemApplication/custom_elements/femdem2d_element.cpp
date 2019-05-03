//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics FemDem Application
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Alejandro Cornejo Velazquez
//

#include "femdem2d_element.hpp"
#include "fem_to_dem_application_variables.h"
#include "solid_mechanics_application_variables.h"

namespace Kratos
{
//***********************DEFAULT CONSTRUCTOR******************************************
//************************************************************************************

FemDem2DElement::FemDem2DElement(IndexType NewId, GeometryType::Pointer pGeometry)
	: SmallDisplacementElement(NewId, pGeometry)
{
}
//******************************CONSTRUCTOR*******************************************
//************************************************************************************

FemDem2DElement::FemDem2DElement(IndexType NewId, GeometryType::Pointer pGeometry, PropertiesType::Pointer pProperties)
	: SmallDisplacementElement(NewId, pGeometry, pProperties)
{
	mThisIntegrationMethod = GetGeometry().GetDefaultIntegrationMethod();

	// Each component == Each edge
	mNumberOfEdges = 3;
	mNonConvergedThresholds = ZeroVector(mNumberOfEdges);   // Equivalent stress
	mThresholds = ZeroVector(mNumberOfEdges); // Stress mThreshold on edge
	mDamages = ZeroVector(mNumberOfEdges); // Converged mDamage on each edge
	mNonConvergedDamages = ZeroVector(mNumberOfEdges); // mDamages on edges of "i" iteration
}

//******************************COPY CONSTRUCTOR**************************************
//************************************************************************************

FemDem2DElement::FemDem2DElement(FemDem2DElement const &rOther)
	: SmallDisplacementElement(rOther)
{
}

//*******************************ASSIGMENT OPERATOR***********************************
//************************************************************************************

FemDem2DElement &FemDem2DElement::operator=(FemDem2DElement const &rOther)
{
	SmallDisplacementElement::operator=(rOther);
	return *this;
}

//*********************************OPERATIONS*****************************************
//************************************************************************************

Element::Pointer FemDem2DElement::Create(IndexType NewId, NodesArrayType const &rThisNodes, PropertiesType::Pointer pProperties) const
{
	return Element::Pointer(new FemDem2DElement(NewId, GetGeometry().Create(rThisNodes), pProperties));
}

//************************************CLONE*******************************************
//************************************************************************************

Element::Pointer FemDem2DElement::Clone(IndexType NewId, NodesArrayType const &rThisNodes) const
{
	FemDem2DElement NewElement(NewId, GetGeometry().Create(rThisNodes), pGetProperties());
	return Element::Pointer(new FemDem2DElement(NewElement));
}

//*******************************DESTRUCTOR*******************************************
//************************************************************************************

FemDem2DElement::~FemDem2DElement()
{
}

void FemDem2DElement::InitializeSolutionStep(ProcessInfo &rCurrentProcessInfo) 
{
	this->InitializeInternalVariablesAfterMapping();
}

void FemDem2DElement::InitializeInternalVariablesAfterMapping()
{
	// After the mapping, the thresholds of the edges (are equal to 0.0) are imposed equal to the IP threshold
	const double element_threshold = mThreshold;
	if (mThresholds[0] + mThresholds[1] + mThresholds[2] < tolerance) {
		mThresholds[0] = element_threshold;
		mThresholds[1] = element_threshold;
		mThresholds[2] = element_threshold;
	}

	// IDEM with the edge damages
	const double damage_element = mDamage;
	if (mDamages[0] + mDamages[1] + mDamages[2] < tolerance) {
		mDamages[0] = damage_element;
		mDamages[1] = damage_element;
		mDamages[2] = damage_element;
	}
}

void FemDem2DElement::UpdateDataBase()
{
	for (unsigned int edge = 0; edge < mNumberOfEdges; edge++) {
		mDamages[edge] = mNonConvergedDamages[edge];
		mThresholds[edge] = mNonConvergedThresholds[edge];
	}
	double converged_damage, converged_threshold;
	converged_damage = this->CalculateElementalDamage(mDamages);
	if (converged_damage > mDamage) mDamage = converged_damage;
	converged_threshold = this->CalculateElementalDamage(mThresholds);
	if (converged_threshold > mThreshold) mThreshold = converged_threshold;
}

void FemDem2DElement::FinalizeSolutionStep(ProcessInfo &rCurrentProcessInfo)
{
	this->UpdateDataBase();

	if (mDamage >= 0.98) {
		this->Set(ACTIVE, false);
	}
}

void FemDem2DElement::InitializeNonLinearIteration(ProcessInfo &rCurrentProcessInfo)
{
	KRATOS_TRY

	//create and initialize element variables:
	ElementDataType variables;
	this->InitializeElementData(variables,rCurrentProcessInfo);

	//create constitutive law parameters:
	ConstitutiveLaw::Parameters values(GetGeometry(),GetProperties(),rCurrentProcessInfo);

	//set constitutive law flags:
	Flags& ConstitutiveLawOptions = values.GetOptions();

	ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_STRESS);
	ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR);

	//reading integration points
	const GeometryType::IntegrationPointsArrayType& r_integration_points = GetGeometry().IntegrationPoints( mThisIntegrationMethod );

	for (SizeType point_number = 0; point_number < r_integration_points.size(); point_number++) {
		//compute element kinematic variables B, F, DN_DX ...
		this->CalculateKinematics(variables,point_number);

		//calculate material response
		this->CalculateMaterialResponse(variables, values, point_number);
	}
	this->SetValue(STRESS_VECTOR, values.GetStressVector());
	this->SetValue(STRAIN_VECTOR, values.GetStrainVector());

	KRATOS_CATCH("")
}

void FemDem2DElement::CalculateLocalSystem(MatrixType &rLeftHandSideMatrix, VectorType &rRightHandSideVector, ProcessInfo &rCurrentProcessInfo)
{
	KRATOS_TRY

	const unsigned int number_of_nodes = GetGeometry().PointsNumber();
	const unsigned int dimension       = GetGeometry().WorkingSpaceDimension();

	//resizing as needed the LHS
	unsigned int mat_size = number_of_nodes * (dimension) ; // water pressure

	if ( rLeftHandSideMatrix.size1() != mat_size)
	rLeftHandSideMatrix.resize(mat_size, mat_size, false);

	noalias(rLeftHandSideMatrix) = ZeroMatrix(mat_size, mat_size); //resetting LHS
	if (rRightHandSideVector.size() != mat_size )
	rRightHandSideVector.resize(mat_size, false);

	noalias(rRightHandSideVector) = ZeroVector(mat_size); //resetting RHS
	
	//create and initialize element variables:
	ElementDataType variables;
	this->InitializeElementData(variables, rCurrentProcessInfo);

	//create constitutive law parameters:
	ConstitutiveLaw::Parameters values(this->GetGeometry(), this->GetProperties(), rCurrentProcessInfo);

	//set constitutive law flags:
	Flags& ConstitutiveLawOptions = values.GetOptions();

	ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_STRESS);
	ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR);

	//reading integration points
	const GeometryType::IntegrationPointsArrayType& r_integration_points = GetGeometry().IntegrationPoints(mThisIntegrationMethod );

	//auxiliary terms
	Vector VolumeForce(dimension);

	auto& r_elem_neigb = this->GetValue(NEIGHBOUR_ELEMENTS);
	KRATOS_ERROR_IF(r_elem_neigb.size() == 0) << " Neighbour Elements not calculated" << std::endl;

	for (SizeType point_number = 0; point_number < r_integration_points.size(); point_number++ ) {
		//compute element kinematic variables B, F, DN_DX ...
		this->CalculateKinematics(variables, point_number);

		//calculate material response
		this->CalculateMaterialResponse(variables, values, point_number); // FEMDEM

		//calculating weights for integration on the "reference configuration"
		variables.IntegrationWeight = r_integration_points[point_number].Weight() * variables.detJ;
		variables.IntegrationWeight = this->CalculateIntegrationWeight(variables.IntegrationWeight );

		// Loop over edges of the element...
		Vector average_stress_edge(3);
		Vector average_strain_edge(3);
		bool is_damaging = false;

		for (unsigned int edge = 0; edge < mNumberOfEdges; edge++) {
			this->CalculateAverageStressOnEdge(&r_elem_neigb[edge], this, average_stress_edge);
			this->CalculateAverageStrainOnEdge(&r_elem_neigb[edge], this, average_strain_edge);

			// We recover converged values
			double threshold = mThresholds[edge];
			double damage = mDamages[edge];
			
			const double length = this->CalculateCharacteristicLength(this, r_elem_neigb[edge], edge);
			this->IntegrateStressDamageMechanics(threshold, damage, average_strain_edge,
													average_stress_edge, edge, length, is_damaging);

			mNonConvergedDamages[edge] = damage;
			mNonConvergedThresholds[edge] = threshold;
		} // Loop over edges

		// Calculate the elemental Damage...
		const double damage_element = this->CalculateElementalDamage(mNonConvergedDamages);
		const Vector& r_predictive_stress_vector = values.GetStressVector();
		const Vector& r_integrated_stress_vector = (1.0 - damage_element) * r_predictive_stress_vector;

		//contributions to stiffness matrix calculated on the reference config
		const Matrix& C =  values.GetConstitutiveMatrix();
		Matrix tangent_tensor;
		const Vector& r_strain_vector = values.GetStrainVector();

		if (is_damaging == true && std::abs(r_strain_vector[0] + r_strain_vector[1] + r_strain_vector[2]) > tolerance) {
			if (this->GetProperties()[TANGENT_CONSTITUTIVE_TENSOR] == true) {
				const Vector& r_internal_forces = variables.IntegrationWeight * prod(trans(variables.B), r_integrated_stress_vector);
				this->CalculateTangentTensorUPerturbed(tangent_tensor, r_strain_vector, r_integrated_stress_vector, r_internal_forces, C);
				noalias(rLeftHandSideMatrix) += tangent_tensor;
			} else {
				this->CalculateTangentTensor(tangent_tensor, r_strain_vector, r_integrated_stress_vector, C);
				noalias(rLeftHandSideMatrix) += variables.IntegrationWeight * prod(trans(variables.B), Matrix(prod(tangent_tensor, variables.B)));
			}
		} else {
			noalias(rLeftHandSideMatrix) += variables.IntegrationWeight * (1.0 - damage_element) * prod(trans(variables.B), Matrix(prod(C, variables.B)));
		}

		//contribution to external forces
		VolumeForce = this->CalculateVolumeForce(VolumeForce, variables.N);

		//contribution of the internal and external forces
		// operation performed: rRightHandSideVector += ExtForce*IntToReferenceWeight
		this->CalculateAndAddExternalForces(rRightHandSideVector, variables, VolumeForce, variables.IntegrationWeight );

		// operation performed: rRightHandSideVector -= IntForce*IntToReferenceWeight
		rRightHandSideVector -= variables.IntegrationWeight * prod(trans(variables.B), r_integrated_stress_vector);
	}
	KRATOS_CATCH("")
}

void FemDem2DElement::CalculateLeftHandSide(MatrixType& rLeftHandSideMatrix, ProcessInfo& rCurrentProcessInfo)
{
	KRATOS_TRY

	const unsigned int number_of_nodes = GetGeometry().PointsNumber();
	const unsigned int dimension       = GetGeometry().WorkingSpaceDimension();

	//resizing as needed the LHS
	unsigned int mat_size = number_of_nodes * (dimension);

	if ( rLeftHandSideMatrix.size1() != mat_size )
	rLeftHandSideMatrix.resize( mat_size, mat_size, false );

	noalias(rLeftHandSideMatrix) = ZeroMatrix(mat_size, mat_size); // resetting LHS
	
	//create and initialize element variables:
	ElementDataType variables;
	this->InitializeElementData(variables, rCurrentProcessInfo);

	//create constitutive law parameters:
	ConstitutiveLaw::Parameters values(this->GetGeometry(), this->GetProperties(), rCurrentProcessInfo);

	//set constitutive law flags:
	Flags& ConstitutiveLawOptions = values.GetOptions();

	//ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_STRESS);
	ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR);

	//reading integration points
	const GeometryType::IntegrationPointsArrayType& r_integration_points = GetGeometry().IntegrationPoints(mThisIntegrationMethod );

	//auxiliary terms
	Vector VolumeForce(dimension);

	auto &r_elem_neigb = this->GetValue(NEIGHBOUR_ELEMENTS);
	KRATOS_ERROR_IF(r_elem_neigb.size() == 0) << " Neighbour Elements not calculated" << std::endl;

	for (SizeType point_number = 0; point_number < r_integration_points.size(); point_number++ ) {
		//compute element kinematic variables B, F, DN_DX ...
		this->CalculateKinematics(variables, point_number);

		//calculate material response
		this->CalculateMaterialResponse(variables, values, point_number);

		//calculating weights for integration on the "reference configuration"
		variables.IntegrationWeight = r_integration_points[point_number].Weight() * variables.detJ;
		variables.IntegrationWeight = this->CalculateIntegrationWeight(variables.IntegrationWeight );

		// Loop over edges of the element...
		Vector average_stress_edge(3);
		Vector average_strain_edge(3);
		bool is_damaging = false;
		Vector damages(mNumberOfEdges);

		for (unsigned int edge = 0; edge < mNumberOfEdges; edge++) {
			this->CalculateAverageStressOnEdge(&r_elem_neigb[edge], this, average_stress_edge);
			this->CalculateAverageStrainOnEdge(&r_elem_neigb[edge], this, average_strain_edge);

			// We recover converged values
			double threshold = mThresholds[edge];
			double damage = mDamages[edge];
			
			const double length = this->CalculateCharacteristicLength(this, r_elem_neigb[edge], edge);
			this->IntegrateStressDamageMechanics(threshold, damage, average_strain_edge,
													average_stress_edge, edge, length, is_damaging);
			damages[edge] = damage;
		} // Loop over edges

		// Calculate the elemental Damage...
		const double damage_element = this->CalculateElementalDamage(damages);
		const Vector& r_predictive_stress_vector = values.GetStressVector();
		const Vector& r_integrated_stress_vector = (1.0 - damage_element) * r_predictive_stress_vector;

		//contributions to stiffness matrix calculated on the reference config
		const Matrix& C =  values.GetConstitutiveMatrix();
		Matrix tangent_tensor;
		const Vector& r_strain_vector = values.GetStrainVector();
		if (is_damaging == true && std::abs(r_strain_vector[0] + r_strain_vector[1] + r_strain_vector[2]) > tolerance) {
			if (this->GetProperties()[TANGENT_CONSTITUTIVE_TENSOR] == true) {
				const Vector& r_internal_forces = variables.IntegrationWeight * prod(trans(variables.B), r_integrated_stress_vector);
				this->CalculateTangentTensorUPerturbed(tangent_tensor, r_strain_vector, r_integrated_stress_vector, r_internal_forces, C);
			} else {
				this->CalculateTangentTensor(tangent_tensor, r_strain_vector, r_integrated_stress_vector, C);
				noalias(rLeftHandSideMatrix) += variables.IntegrationWeight * prod(trans(variables.B), Matrix(prod(tangent_tensor, variables.B)));
			}
		} else {
			noalias(rLeftHandSideMatrix) += variables.IntegrationWeight * (1.0 - damage_element) * prod(trans(variables.B), Matrix(prod(C, variables.B)));
		}
	}
	KRATOS_CATCH("")
}

void FemDem2DElement::CalculateRightHandSide(VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo)
{
	KRATOS_TRY

	const unsigned int number_of_nodes = GetGeometry().PointsNumber();
	const unsigned int dimension       = GetGeometry().WorkingSpaceDimension();

	//resizing as needed the LHS
	unsigned int mat_size = number_of_nodes * (dimension);

	if (rRightHandSideVector.size() != mat_size )
	rRightHandSideVector.resize(mat_size, false);

	noalias(rRightHandSideVector) = ZeroVector(mat_size); //resetting RHS

	//create and initialize element variables:
	ElementDataType variables;
	this->InitializeElementData(variables, rCurrentProcessInfo);

	//create constitutive law parameters:
	ConstitutiveLaw::Parameters values(this->GetGeometry(), this->GetProperties(), rCurrentProcessInfo);

	//set constitutive law flags:
	Flags& ConstitutiveLawOptions = values.GetOptions();

	ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_STRESS);
	//ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR);

	//reading integration points
	const GeometryType::IntegrationPointsArrayType& r_integration_points = GetGeometry().IntegrationPoints(mThisIntegrationMethod);

	//auxiliary terms
	Vector VolumeForce(dimension);

	auto& r_elem_neigb = this->GetValue(NEIGHBOUR_ELEMENTS);
	KRATOS_ERROR_IF(r_elem_neigb.size() == 0) << " Neighbour Elements not calculated" << std::endl;

	for (SizeType point_number = 0; point_number < r_integration_points.size(); point_number++ ) {
		//compute element kinematic variables B, F, DN_DX ...
		this->CalculateKinematics(variables, point_number);

		//calculate material response
		this->CalculateMaterialResponse(variables, values, point_number); // FEMDEM

		//calculating weights for integration on the "reference configuration"
		variables.IntegrationWeight = r_integration_points[point_number].Weight() * variables.detJ;
		variables.IntegrationWeight = this->CalculateIntegrationWeight(variables.IntegrationWeight);

		// Loop over edges of the element...
		Vector average_stress_edge(3);
		Vector average_strain_edge(3);
		bool is_damaging = false;
		Vector damages(mNumberOfEdges);

		for (unsigned int edge = 0; edge < mNumberOfEdges; edge++) {
			this->CalculateAverageStressOnEdge(&r_elem_neigb[edge], this, average_stress_edge);
			this->CalculateAverageStrainOnEdge(&r_elem_neigb[edge], this, average_strain_edge);

			// We recover converged values
			double threshold = mThresholds[edge];
			double damage = mDamages[edge];
			
			const double length = this->CalculateCharacteristicLength(this, r_elem_neigb[edge], edge);
			this->IntegrateStressDamageMechanics(threshold, damage, average_strain_edge,
													average_stress_edge, edge, length, is_damaging);
			damages[edge] = damage;
		} // Loop over edges

		// Calculate the elemental Damage...
		const double damage_element = this->CalculateElementalDamage(damages);
		const Vector& r_predictive_stress_vector = values.GetStressVector();
		const Vector& r_integrated_stress_vector = (1.0 - damage_element) * r_predictive_stress_vector;

		this->SetValue(STRESS_VECTOR, r_predictive_stress_vector);
		this->SetValue(STRESS_VECTOR_INTEGRATED, r_integrated_stress_vector);
		const Vector& r_strain_vector = values.GetStrainVector();
		this->SetValue(STRAIN_VECTOR, r_strain_vector);

		//contribution to external forces
		VolumeForce = this->CalculateVolumeForce(VolumeForce, variables.N);

		//contribution of the internal and external forces
		// operation performed: rRightHandSideVector += ExtForce*IntToReferenceWeight
		this->CalculateAndAddExternalForces(rRightHandSideVector, variables, VolumeForce, variables.IntegrationWeight);

		// operation performed: rRightHandSideVector -= IntForce*IntToReferenceWeight
		rRightHandSideVector -= variables.IntegrationWeight * prod(trans(variables.B), r_integrated_stress_vector);
	}
	KRATOS_CATCH("")
}


double FemDem2DElement::CalculateElementalDamage(const Vector& rEdgeDamages)
{
	Vector two_max_values = ZeroVector(2);
	this->Get2MaxValues(two_max_values, rEdgeDamages[0], rEdgeDamages[1], rEdgeDamages[2]);
	return 0.5 * (two_max_values[0] + two_max_values[1]);
}

void FemDem2DElement::CalculateAverageStressOnEdge(
	const Element* Neighbour, 
	const Element* CurrentElement, 
	Vector& rAverageStress
	)
{
	rAverageStress = 0.5*(Neighbour->GetValue(STRESS_VECTOR) + CurrentElement->GetValue(STRESS_VECTOR));
}

void FemDem2DElement::CalculateAverageStrainOnEdge(
	const Element* Neighbour, 
	const Element* CurrentElement, 
	Vector& rAverageStrain
	)
{
	rAverageStrain = 0.5*(Neighbour->GetValue(STRAIN_VECTOR) + CurrentElement->GetValue(STRAIN_VECTOR));
}

void FemDem2DElement::CalculateDeformationMatrix(Matrix &rB, const Matrix &rDN_DX)
{
	const unsigned int number_of_nodes = GetGeometry().PointsNumber();
	const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
	unsigned int voigt_size = dimension * (dimension + 1) / 2;

	if (rB.size1() != voigt_size || rB.size2() != dimension * number_of_nodes)
		rB.resize(voigt_size, dimension * number_of_nodes, false);

	for (unsigned int i = 0; i < number_of_nodes; i++) {
		unsigned int index = 2 * i;
		rB(0, index + 0) = rDN_DX(i, 0);
		rB(0, index + 1) = 0.0;
		rB(1, index + 0) = 0.0;
		rB(1, index + 1) = rDN_DX(i, 1);
		rB(2, index + 0) = rDN_DX(i, 1);
		rB(2, index + 1) = rDN_DX(i, 0);
	}
}

void FemDem2DElement::CalculateConstitutiveMatrix(Matrix &rConstitutiveMatrix, const double &rYoungModulus,
												  const double &rPoissonCoefficient)
{
	rConstitutiveMatrix.clear();

	if (this->GetProperties()[THICKNESS] == 1) {
		// Plane strain constitutive matrix
		rConstitutiveMatrix(0, 0) = (rYoungModulus * (1.0 - rPoissonCoefficient) / ((1.0 + rPoissonCoefficient) * (1.0 - 2.0 * rPoissonCoefficient)));
		rConstitutiveMatrix(1, 1) = rConstitutiveMatrix(0, 0);
		rConstitutiveMatrix(2, 2) = rConstitutiveMatrix(0, 0) * (1.0 - 2.0 * rPoissonCoefficient) / (2.0 * (1.0 - rPoissonCoefficient));
		rConstitutiveMatrix(0, 1) = rConstitutiveMatrix(0, 0) * rPoissonCoefficient / (1.0 - rPoissonCoefficient);
		rConstitutiveMatrix(1, 0) = rConstitutiveMatrix(0, 1);
	} else {
		// Plane stress constitutive matrix
		rConstitutiveMatrix(0, 0) = (rYoungModulus) / (1.0 - rPoissonCoefficient * rPoissonCoefficient);
		rConstitutiveMatrix(1, 1) = rConstitutiveMatrix(0, 0);
		rConstitutiveMatrix(2, 2) = rConstitutiveMatrix(0, 0) * (1.0 - rPoissonCoefficient) * 0.5;
		rConstitutiveMatrix(0, 1) = rConstitutiveMatrix(0, 0) * rPoissonCoefficient;
		rConstitutiveMatrix(1, 0) = rConstitutiveMatrix(0, 1);
	}
}

void FemDem2DElement::CalculateDN_DX(Matrix &rDN_DX, int PointNumber)
{
	const unsigned int number_of_nodes = GetGeometry().size();
	const unsigned int dimension = GetGeometry().WorkingSpaceDimension();

	//get the shape functions parent coordinates derivative [dN/d�] (for the order of the default integration method)
	const GeometryType::ShapeFunctionsGradientsType &DN_De = GetGeometry().ShapeFunctionsLocalGradients(mThisIntegrationMethod);

	//calculate delta position (here coincides with the current displacement)
	Matrix delta_position = ZeroMatrix(number_of_nodes, dimension);
	delta_position = this->CalculateDeltaPosition(delta_position);

	//calculating the reference jacobian from cartesian coordinates to parent coordinates for all integration points [dx_n/d�]
	GeometryType::JacobiansType J;
	J.resize(1, false);
	J[0] = ZeroMatrix(1, 1);
	J = GetGeometry().Jacobian(J, mThisIntegrationMethod, delta_position);
	//a.-compute element kinematics

	//calculating the inverse of the jacobian for this integration point[d�/dx_n]
	Matrix InvJ = ZeroMatrix(dimension, dimension);
	double detJ = 0;
	MathUtils<double>::InvertMatrix(J[PointNumber], InvJ, detJ);

	KRATOS_ERROR_IF(detJ < 0) << "SMALL DISPLACEMENT ELEMENT INVERTED: |J|<0" << std::endl;

	//compute cartesian derivatives for this integration point  [dN/dx_n]
	rDN_DX = prod(DN_De[PointNumber], InvJ);
}

void FemDem2DElement::CalculateInfinitesimalStrain(Vector &rStrainVector, const Matrix &rDN_DX)
{
	const unsigned int number_of_nodes = GetGeometry().PointsNumber();
	const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
	Matrix H = zero_matrix<double>(dimension); //[dU/dx_n]

	for (unsigned int i = 0; i < number_of_nodes; i++) {
		array_1d<double, 3> &r_displacement = GetGeometry()[i].FastGetSolutionStepValue(DISPLACEMENT);

		H(0, 0) += r_displacement[0] * rDN_DX(i, 0);
		H(0, 1) += r_displacement[0] * rDN_DX(i, 1);
		H(1, 0) += r_displacement[1] * rDN_DX(i, 0);
		H(1, 1) += r_displacement[1] * rDN_DX(i, 1);
	}
	//Infinitesimal Strain Calculation
	if (rStrainVector.size() != 3)
		rStrainVector.resize(3, false);

	rStrainVector[0] = H(0, 0);
	rStrainVector[1] = H(1, 1);
	rStrainVector[2] = (H(0, 1) + H(1, 0)); // xy
}

void FemDem2DElement::CalculateStressVector(Vector &rStressVector, const Matrix &rConstitutiveMAtrix, const Vector &rInfinitesimalStrainVector)
{
	noalias(rStressVector) = prod(rConstitutiveMAtrix, rInfinitesimalStrainVector);
}

void FemDem2DElement::CalculatePrincipalStress(Vector &rPrincipalStressVector, const Vector StressVector)
{
	rPrincipalStressVector.resize(2);
	rPrincipalStressVector[0] = 0.5 * (StressVector[0] + StressVector[1]) + 
		std::sqrt(std::pow(0.5 * (StressVector[0] - StressVector[1]), 2) + std::pow(StressVector[2], 2));
		
	rPrincipalStressVector[1] = 0.5 * (StressVector[0] + StressVector[1]) - 
		std::sqrt(std::pow(0.5 * (StressVector[0] - StressVector[1]), 2) + std::pow(StressVector[2], 2));
}

void FemDem2DElement::FinalizeNonLinearIteration(ProcessInfo &CurrentProcessInfo)
{
}

void FemDem2DElement::AverageVector(Vector &rAverageVector, const Vector &v, const Vector &w)
{
	const int n = v.size();
	const int m = w.size();
	KRATOS_ERROR_IF(n != m) << "The dimension of the vectors are different or null" << std::endl;
	rAverageVector.resize(n);
	for (unsigned int cont = 0; cont < n; cont++) {
		rAverageVector[cont] = (v[cont] + w[cont]) * 0.5;
	}
}

void FemDem2DElement::GetValueOnIntegrationPoints(
	const Variable<double> &rVariable,
	std::vector<double> &rValues,
	const ProcessInfo &rCurrentProcessInfo)
{
	if (rVariable == DAMAGE_ELEMENT || rVariable == IS_DAMAGED || rVariable == STRESS_THRESHOLD) {
		CalculateOnIntegrationPoints(rVariable, rValues, rCurrentProcessInfo);
	}
}

void FemDem2DElement::GetValueOnIntegrationPoints(
	const Variable<Vector> &rVariable,
	std::vector<Vector> &rValues,
	const ProcessInfo &rCurrentProcessInfo)
{

	if (rVariable == STRAIN_VECTOR) {
		CalculateOnIntegrationPoints(rVariable, rValues, rCurrentProcessInfo);
	} else if (rVariable == STRESS_VECTOR) {
		CalculateOnIntegrationPoints(rVariable, rValues, rCurrentProcessInfo);
	} else if (rVariable == STRESS_VECTOR_INTEGRATED) {
		CalculateOnIntegrationPoints(rVariable, rValues, rCurrentProcessInfo);
	}

	const unsigned int& integration_points_number = mConstitutiveLawVector.size();
	if (rValues.size() != integration_points_number)
	rValues.resize(integration_points_number);

	if (rVariable == PK2_STRESS_TENSOR || rVariable == CAUCHY_STRESS_TENSOR) {
		CalculateOnIntegrationPoints(rVariable, rValues, rCurrentProcessInfo);
	} else if (rVariable == PK2_STRESS_VECTOR || rVariable == CAUCHY_STRESS_VECTOR) {
		CalculateOnIntegrationPoints(rVariable, rValues, rCurrentProcessInfo);
	} else if (rVariable == GREEN_LAGRANGE_STRAIN_TENSOR || rVariable == ALMANSI_STRAIN_TENSOR) {
		CalculateOnIntegrationPoints(rVariable, rValues, rCurrentProcessInfo);
	} else {
		for (unsigned int point_number = 0; point_number < integration_points_number; point_number++) {
			rValues[point_number] = mConstitutiveLawVector[point_number]->GetValue(rVariable, rValues[point_number]);
		}
	}
}

// DOUBLE variables
void FemDem2DElement::CalculateOnIntegrationPoints(
	const Variable<double> &rVariable,
	std::vector<double> &rOutput,
	const ProcessInfo &rCurrentProcessInfo)
{
	if (rVariable == DAMAGE_ELEMENT) {
		rOutput.resize(1);
		for (unsigned int point_number = 0; point_number < 1; point_number++) {
			rOutput[point_number] = mDamage;
		}
	} else if (rVariable == STRESS_THRESHOLD) {
		rOutput.resize(1);
		for (unsigned int point_number = 0; point_number < 1; point_number++) {
			rOutput[point_number] = mThreshold;
		}
	}
}

// VECTOR variables
void FemDem2DElement::CalculateOnIntegrationPoints(
	const Variable<Vector> &rVariable,
	std::vector<Vector> &rOutput,
	const ProcessInfo &rCurrentProcessInfo)
{
	const unsigned int &integration_points_number = GetGeometry().IntegrationPointsNumber(mThisIntegrationMethod);
	if (rOutput.size() != integration_points_number)
		rOutput.resize(integration_points_number);

	if (rVariable == STRESS_VECTOR) {
		rOutput[0] = this->GetValue(STRESS_VECTOR);
	} else if (rVariable == STRAIN_VECTOR) {
		rOutput[0] = this->GetValue(STRAIN_VECTOR);
	} else if (rVariable == STRESS_VECTOR_INTEGRATED) {
		rOutput[0] = (1.0- mDamage) * (this->GetValue(STRESS_VECTOR));
	} else if (rVariable == GREEN_LAGRANGE_STRAIN_VECTOR || rVariable == ALMANSI_STRAIN_VECTOR) {
          // create and initialize element variables:
          ElementDataType variables;
          this->InitializeElementData(variables, rCurrentProcessInfo);

          // reading integration points
        for (unsigned int point_number = 0; point_number < mConstitutiveLawVector.size(); point_number++) {
            this->CalculateKinematics(variables, point_number);
            if (rOutput[point_number].size() != variables.StrainVector.size())
              rOutput[point_number].resize(variables.StrainVector.size(), false);
            rOutput[point_number] = variables.StrainVector;
        }
	} else {
		for (unsigned int ii = 0; ii < mConstitutiveLawVector.size(); ii++) {
			rOutput[ii] = mConstitutiveLawVector[ii]->GetValue(rVariable, rOutput[ii]);
		}
	}
}

double FemDem2DElement::CalculateCharacteristicLength(FemDem2DElement *pCurrentElement, const Element &rNeibElement, int cont)
{
	Geometry<Node<3>>& r_nodes_current_element = pCurrentElement->GetGeometry(); // 3 nodes of the Element 1

	Vector node_1_coordinates(3);
	Vector node_2_coordinates(3);
	Vector node_3_coordinates(3);
	node_1_coordinates[0] = r_nodes_current_element[0].X0();
	node_1_coordinates[1] = r_nodes_current_element[0].Y0();
	node_1_coordinates[2] = r_nodes_current_element[0].Z0();
	node_2_coordinates[0] = r_nodes_current_element[1].X0();
	node_2_coordinates[1] = r_nodes_current_element[1].Y0();
	node_2_coordinates[2] = r_nodes_current_element[1].Z0();
	node_3_coordinates[0] = r_nodes_current_element[2].X0();
	node_3_coordinates[1] = r_nodes_current_element[2].Y0();
	node_3_coordinates[2] = r_nodes_current_element[2].Z0();

	const double length1 = MathUtils<double>::Norm3(node_1_coordinates-node_2_coordinates);
	const double length2 = MathUtils<double>::Norm3(node_2_coordinates-node_3_coordinates);
	const double length3 = MathUtils<double>::Norm3(node_3_coordinates-node_1_coordinates);
	return (length1 + length2 + length3) / 3.0;
}

void FemDem2DElement::Get2MaxValues(Vector &MaxValues, double a, double b, double c)
{
	MaxValues.resize(2);
	Vector V;
	V.resize(3);
	V[0] = a;
	V[1] = b;
	V[2] = c;
	const int n = 3;

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n - 1; j++) {
			if (V[j] > V[j + 1]) {
				double aux = V[j];
				V[j] = V[j + 1];
				V[j + 1] = aux;
			}
		}
	}
	MaxValues[0] = V[2];
	MaxValues[1] = V[1];
}

void FemDem2DElement::Get2MinValues(Vector &MaxValues, double a, double b, double c)
{
	MaxValues.resize(2);
	Vector V;
	V.resize(3);
	V[0] = a;
	V[1] = b;
	V[2] = c;
	const int n = 3;

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n - 1; j++) {
			if (V[j] > V[j + 1]) {
				const double aux = V[j];
				V[j] = V[j + 1];
				V[j + 1] = aux;
			}
		}
	}
	MaxValues[0] = V[1];
	MaxValues[1] = V[0];
}
double FemDem2DElement::CalculateI1Invariant(double sigma1, double sigma2) { return sigma1 + sigma2; }
double FemDem2DElement::CalculateJ2Invariant(double sigma1, double sigma2)
{
	return (std::pow((sigma1 - sigma2), 2) + std::pow(sigma1, 2) + std::pow(sigma2, 2)) / 6.0;
}
double FemDem2DElement::CalculateJ3Invariant(double sigma1, double sigma2, double I1)
{
	return (sigma1 - I1 / 3.0) * ((sigma2 - I1 / 3.0)) * (-I1 / 3.0);
}
double FemDem2DElement::CalculateLodeAngle(double J2, double J3)
{
    if (std::abs(J2) > tolerance) {
        double sint3 = (-3.0 * std::sqrt(3.0) * J3) / (2.0 * J2 * std::sqrt(J2));
        if (sint3 < -0.95)
            sint3 = -1.0;
        else if (sint3 > 0.95)
            sint3 = 1.0;
        return std::asin(sint3) / 3.0;
    } else {
        return 0.0;
    }
}

void FemDem2DElement::CalculateMassMatrix(MatrixType &rMassMatrix, ProcessInfo &rCurrentProcessInfo)
{
	bool ComputeLumpedMassMatrix = false;
	if (rCurrentProcessInfo.Has(COMPUTE_LUMPED_MASS_MATRIX))
		if (rCurrentProcessInfo[COMPUTE_LUMPED_MASS_MATRIX] == true)
			ComputeLumpedMassMatrix = true;

	if (ComputeLumpedMassMatrix == false) {

		//create local system components
		LocalSystemComponents LocalSystem;

		//calculation flags
		LocalSystem.CalculationFlags.Set(SmallDisplacementElement::COMPUTE_LHS_MATRIX);

		VectorType RightHandSideVector = Vector();

		//Initialize sizes for the system components:
		this->InitializeSystemMatrices(rMassMatrix, RightHandSideVector, LocalSystem.CalculationFlags);

		//Set variables to Local system components
		LocalSystem.SetLeftHandSideMatrix(rMassMatrix);
		LocalSystem.SetRightHandSideVector(RightHandSideVector);

		//Calculate elemental system
		CalculateDynamicSystem(LocalSystem, rCurrentProcessInfo);
	} else {

		//lumped
		unsigned int dimension = GetGeometry().WorkingSpaceDimension();
		const unsigned int number_of_nodes = GetGeometry().PointsNumber();
		unsigned int MatSize = dimension * number_of_nodes;

		if (rMassMatrix.size1() != MatSize)
			rMassMatrix.resize(MatSize, MatSize, false);

		noalias(rMassMatrix) = ZeroMatrix(MatSize, MatSize);

		double TotalMass = 0;
		TotalMass = this->CalculateTotalMass(TotalMass, rCurrentProcessInfo);

		Vector LumpFact(number_of_nodes);
		noalias(LumpFact) = ZeroVector(number_of_nodes);

		LumpFact = GetGeometry().LumpingFactors(LumpFact);

		for (unsigned int i = 0; i < number_of_nodes; i++) {
			const double temp = LumpFact[i] * TotalMass;
			for (unsigned int j = 0; j < dimension; j++) {
				unsigned int index = i * dimension + j;
				rMassMatrix(index, index) = temp;
			}
		}
	}
}
Vector &FemDem2DElement::CalculateVolumeForce(Vector &rVolumeForce, const Vector &rN)
{
	const unsigned int number_of_nodes = GetGeometry().PointsNumber();
	const unsigned int dimension = GetGeometry().WorkingSpaceDimension();

	if (rVolumeForce.size() != dimension)
		rVolumeForce.resize(dimension, false);

	noalias(rVolumeForce) = ZeroVector(dimension);

	for (unsigned int j = 0; j < number_of_nodes; j++) {
		if (GetGeometry()[j].SolutionStepsDataHas(VOLUME_ACCELERATION)) { // it must be checked once at the begining only
			array_1d<double, 3> &VolumeAcceleration = GetGeometry()[j].FastGetSolutionStepValue(VOLUME_ACCELERATION);
			for (unsigned int i = 0; i < dimension; i++)
				rVolumeForce[i] += rN[j] * VolumeAcceleration[i];
		}
	}

	rVolumeForce *= GetProperties()[DENSITY];
	return rVolumeForce;
}

double FemDem2DElement::GetMaxValue(const Vector& rValues)
{
	Vector V;
	const int n = rValues.size();
	V.resize(n);

	for (int cont = 0; cont < n; cont++) {
		V[cont] = rValues[cont];
	}
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n - 1; j++) {
			if (V[j] > V[j + 1]) {
				const double aux = V[j];
				V[j] = V[j + 1];
				V[j + 1] = aux;
			}
		}
	}
	return V[n - 1];
}

double FemDem2DElement::GetMaxAbsValue(const Vector& rArrayValues)
{
	const SizeType dimension = rArrayValues.size();

	IndexType counter = 0;
	double aux = 0.0;
	for (IndexType i = 0; i < dimension; ++i) {
		if (std::abs(rArrayValues[i]) > aux) {
			aux = std::abs(rArrayValues[i]);
			++counter;
		}
	}
	return aux;
}

double FemDem2DElement::GetMinAbsValue(const Vector& rArrayValues)
{
	const SizeType dimension = rArrayValues.size();

	IndexType counter = 0;
	double aux = std::numeric_limits<double>::max();
	for (IndexType i = 0; i < dimension; ++i) {
		if (std::abs(rArrayValues[i]) < aux) {
			aux = std::abs(rArrayValues[i]);
			++counter;
		}
	}
	return aux;
}


// ******* DAMAGE MECHANICS YIELD SURFACES AND EXPONENTIAL SOFTENING ********
void FemDem2DElement::IntegrateStressDamageMechanics(
	double& rThreshold,
	double &rDamage,
	const Vector& rStrainVector,
	const Vector& rStressVector,
	const int Edge,
	const double Length,
	bool& rIsDamaging)
{
	const std::string& yield_surface = this->GetProperties()[YIELD_SURFACE];

	if (yield_surface == "ModifiedMohrCoulomb") {
		this->ModifiedMohrCoulombCriterion(
			rThreshold, rDamage, rStressVector, Edge, Length, rIsDamaging);
	} else if (yield_surface == "SimoJu") {
		this->SimoJuCriterion(
			rThreshold, rDamage, rStrainVector, rStressVector, Edge, Length, rIsDamaging);
	} else if (yield_surface == "Rankine") {
		this->RankineCriterion(
			rThreshold, rDamage, rStressVector, Edge, Length, rIsDamaging);
	} else if (yield_surface == "DruckerPrager") {
		this->DruckerPragerCriterion(
			rThreshold, rDamage, rStressVector, Edge, Length, rIsDamaging);
	} else if (yield_surface == "RankineFragile") {
		this->RankineFragileLaw(
			rThreshold, rDamage, rStressVector, Edge, Length, rIsDamaging);
	} else if (yield_surface == "Elastic") {
		this->ElasticLaw(
			rThreshold, rDamage, rStressVector, Edge, Length, rIsDamaging);
	} else {
		KRATOS_ERROR << " Yield Surface not defined " << std::endl;
	}
}

void FemDem2DElement::ModifiedMohrCoulombCriterion(
	double& rThreshold,
	double &rDamage, 
	const Vector &rStressVector, 
	const int Edge, 
	const double Length,
	bool& rIsDamaging
	)
{
	Vector PrincipalStressVector(2);
	this->CalculatePrincipalStress(PrincipalStressVector, rStressVector);

	const auto& properties = this->GetProperties();
	const double sigma_c = properties[YIELD_STRESS_C];
	const double sigma_t = properties[YIELD_STRESS_T];
	double friction_angle = properties[INTERNAL_FRICTION_ANGLE] * Globals::Pi / 180.0; // In radians!
	const double E = properties[YOUNG_MODULUS];
	const double Gt = properties[FRAC_ENERGY_T];

	KRATOS_WARNING_IF("friction_angle", friction_angle < tolerance) << "Friction Angle not defined, assumed equal to 32deg" << std::endl;
	KRATOS_ERROR_IF(sigma_c < tolerance) << "Yield stress in compression not defined, include YIELD_STRESS_C in .mdpa " << std::endl;
	KRATOS_ERROR_IF(sigma_t < tolerance) << "Yield stress in tension not defined, include YIELD_STRESS_T in .mdpa" << std::endl;
	KRATOS_ERROR_IF(Gt < tolerance) << " ERROR: Fracture Energy not defined in the model part, include FRAC_ENERGY_T in .mdpa " << std::endl;
	if (friction_angle < tolerance) {
		friction_angle = 32.0 * Globals::Pi / 180.0;
		KRATOS_WARNING_IF("friction_angle", friction_angle < tolerance) << "Friction Angle not defined, assumed equal to 32 " << std::endl;
	}

	const double R = std::abs(sigma_c / sigma_t);
	const double Rmorh = std::pow(tan((Globals::Pi / 4.0) + friction_angle / 2.0), 2.0);
	const double alpha_r = R / Rmorh;
	const double c_max = std::abs(sigma_c);
	const double I1 = CalculateI1Invariant(PrincipalStressVector[0], PrincipalStressVector[1]);
	const double J2 = CalculateJ2Invariant(PrincipalStressVector[0], PrincipalStressVector[1]);
	const double J3 = CalculateJ3Invariant(PrincipalStressVector[0], PrincipalStressVector[1], I1);
	const double K1 = 0.5 * (1.0 + alpha_r) - 0.5 * (1.0 - alpha_r) * std::sin(friction_angle);
	const double K2 = 0.5 * (1.0 + alpha_r) - 0.5 * (1.0 - alpha_r) / std::sin(friction_angle);
	const double K3 = 0.5 * (1.0 + alpha_r) * std::sin(friction_angle) - 0.5 * (1.0 - alpha_r);
	const double n = sigma_c / sigma_t;
	const double A = 1.00 / (n * n * Gt * E / (Length * std::pow(sigma_c, 2)) - 0.5);
	KRATOS_ERROR_IF(A < tolerance) << " 'A' damage parameter lower than zero --> Increase FRAC_ENERGY_T" << std::endl;

	double uniaxial_stress; /// F = f-c = 0 classical definition of yield surface
	// Check Modified Mohr-Coulomb criterion
	if (PrincipalStressVector[0] + PrincipalStressVector[1] < tolerance) {
		uniaxial_stress = 0.0;
	} else {
		const double theta = CalculateLodeAngle(J2, J3);
		uniaxial_stress = (2.00 * std::tan(Globals::Pi * 0.25 + friction_angle * 0.5) / std::cos(friction_angle)) * ((I1 * K3 / 3.0) + 
							std::sqrt(J2) * (K1 * std::cos(theta) - K2 * std::sin(theta) * std::sin(friction_angle) / std::sqrt(3.0)));
	}
	if (rThreshold < tolerance) {
		rThreshold = c_max;
	} // 1st iteration sets threshold as c_max

	const double F = uniaxial_stress - rThreshold;
	if (F <= 0.0) {// Elastic region --> Damage is constant
		rDamage = this->GetConvergedDamages(Edge);
	} else {
		this->CalculateExponentialDamage(rDamage, A, uniaxial_stress, c_max);
		rThreshold = uniaxial_stress;
		rIsDamaging = true;
	}
}

void FemDem2DElement::RankineCriterion(
	double& rThreshold,
	double &rDamage, 
	const Vector &rStressVector, 
	const int Edge, 
	const double Length,
	bool& rIsDamaging
	)
{
	Vector PrincipalStressVector(3);
	this->CalculatePrincipalStress(PrincipalStressVector, rStressVector);

	const auto& properties = this->GetProperties();	
	const double sigma_c = properties[YIELD_STRESS_C];
	const double sigma_t = properties[YIELD_STRESS_T];
	const double E = properties[YOUNG_MODULUS];
	const double Gt = properties[FRAC_ENERGY_T];
	const double c_max = std::abs(sigma_t);
	const double A = 1.00 / (Gt * E / (Length * std::pow(sigma_c, 2)) - 0.5);
	KRATOS_ERROR_IF(A < tolerance)<< " 'A' damage parameter lower than zero --> Increase FRAC_ENERGY_T" << std::endl;

	const double uniaxial_stress = GetMaxValue(PrincipalStressVector);

	if (rThreshold < tolerance) {
		rThreshold = c_max;
	} // 1st iteration sets threshold as c_max

	const double F = uniaxial_stress - rThreshold;
	if (F <= 0.0) { // Elastic region --> Damage is constant
		rDamage = this->GetConvergedDamages(Edge);
	} else {
		this->CalculateExponentialDamage(rDamage, A, uniaxial_stress, c_max);
		rThreshold = uniaxial_stress;
		rIsDamaging = true;
	}
}

void FemDem2DElement::DruckerPragerCriterion(
	double& rThreshold,
	double &rDamage, 
	const Vector &rStressVector, 
	const int Edge, 
	const double Length,
	bool& rIsDamaging
	)
{
	Vector PrincipalStressVector(3);
	this->CalculatePrincipalStress(PrincipalStressVector, rStressVector);

	const auto& properties = this->GetProperties();		
	const double sigma_c = properties[YIELD_STRESS_C];
	const double sigma_t = properties[YIELD_STRESS_T];
	double friction_angle = properties[INTERNAL_FRICTION_ANGLE] * Globals::Pi / 180; // In radians!
	const double E = properties[YOUNG_MODULUS];
	const double Gt = properties[FRAC_ENERGY_T];

	KRATOS_WARNING_IF("friction_angle", friction_angle < tolerance) << "Friction Angle not defined, assumed equal to 32deg" << std::endl;
	KRATOS_ERROR_IF(sigma_c < tolerance) << "Yield stress in compression not defined, include YIELD_STRESS_C in .mdpa " << std::endl;
	KRATOS_ERROR_IF(sigma_t < tolerance) << "Yield stress in tension not defined, include YIELD_STRESS_T in .mdpa" << std::endl;
	KRATOS_ERROR_IF(Gt < tolerance) << " ERROR: Fracture Energy not defined in the model part, include FRAC_ENERGY_T in .mdpa " << std::endl;

	// Check input variables
	if (friction_angle < tolerance) {
		friction_angle = 32 * Globals::Pi / 180;
		KRATOS_WARNING_IF("friction_angle", friction_angle < tolerance) << "Friction Angle not defined, assumed equal to 32 " << std::endl;
	}

	const double c_max = std::abs(sigma_t * (3.0 + std::sin(friction_angle)) / (3.0 * std::sin(friction_angle) - 3.0));
	const double I1 = CalculateI1Invariant(PrincipalStressVector[0], PrincipalStressVector[1]);
	const double J2 = CalculateJ2Invariant(PrincipalStressVector[0], PrincipalStressVector[1]);
	const double A = 1.00 / (Gt * E / (Length * std::pow(sigma_c, 2)) - 0.5);
	KRATOS_ERROR_IF(A < tolerance) << "'A' damage parameter lower than zero --> Increase FRAC_ENERGY_T" << std::endl;

	double uniaxial_stress;
	// Check DruckerPrager criterion
	if (PrincipalStressVector[0] + PrincipalStressVector[1] < tolerance) {
		uniaxial_stress = 0.0;
	} else {
		const double CFL = -std::sqrt(3.0) * (3.0 - std::sin(friction_angle)) / (3.0 * std::sin(friction_angle) - 3.0);
		const double TEN0 = 2.0 * I1 * std::sin(friction_angle) / (std::sqrt(3.0) * (3.0 - std::sin(friction_angle))) + std::sqrt(J2);
		uniaxial_stress = std::abs(CFL * TEN0);
	}

	if (rThreshold < tolerance) {
		rThreshold = c_max;
	} // 1st iteration sets threshold as c_max

	const double F = uniaxial_stress - rThreshold;
	if (F <= 0.0) {// Elastic region --> Damage is constant
		rDamage = this->GetConvergedDamages(Edge);
	} else {
		this->CalculateExponentialDamage(rDamage, A, uniaxial_stress, c_max);
		rThreshold = uniaxial_stress;
		rIsDamaging = true;
	}
}

void FemDem2DElement::SimoJuCriterion(
	double& rThreshold,
	double &rDamage,
	const Vector &rStrainVector,
	const Vector &rStressVector,
	const int Edge,
	const double Length,
	bool& rIsDamaging
	)
{
	Vector PrincipalStressVector(3);
	this->CalculatePrincipalStress(PrincipalStressVector, rStressVector);
	const auto& properties = this->GetProperties();
	const double sigma_t = properties[YIELD_STRESS_T];
	const double sigma_c = properties[YIELD_STRESS_C];
	const double E = properties[YOUNG_MODULUS];
	const double Gt = properties[FRAC_ENERGY_T];
	const double n = std::abs(sigma_c / sigma_t);
	const double c_max = std::abs(sigma_c) / std::sqrt(E);

	double SumA = 0.0, SumB = 0.0, SumC = 0.0;
	for (unsigned int cont = 0; cont < 2; cont++) {
		SumA += std::abs(PrincipalStressVector[cont]);
		SumB += 0.5 * (PrincipalStressVector[cont]  + std::abs(PrincipalStressVector[cont]));
		SumC += 0.5 * (-PrincipalStressVector[cont] + std::abs(PrincipalStressVector[cont]));
	}
	const double ere0 = SumB / SumA;
	const double ere1 = SumC / SumA;
	double uniaxial_stress;
	// Check SimoJu criterion
	if (rStrainVector[0] + rStrainVector[1] < tolerance) {
		uniaxial_stress = 0;
	} else {
		double auxf = 0.0;
		for (unsigned int cont = 0; cont < 3; cont++) {
			auxf += rStrainVector[cont] * rStressVector[cont]; // E*S
		}
		uniaxial_stress = std::sqrt(auxf);
		uniaxial_stress *= (ere0 * n + ere1);
	}

	if (rThreshold < tolerance) {
		rThreshold = c_max;
	} // 1st iteration sets threshold as c_max

	const double F = uniaxial_stress - rThreshold;

	const double A = 1.00 / (Gt * n * n * E / (Length * std::pow(sigma_c, 2)) - 0.5);
	KRATOS_ERROR_IF(A < tolerance) << "'A' damage parameter lower than zero --> Increase FRAC_ENERGY_T" << std::endl;

	if (F <= 0) {// Elastic region --> Damage is constant
		rDamage = this->GetConvergedDamages(Edge);
	} else {
		this->CalculateExponentialDamage(rDamage, A, uniaxial_stress, c_max);
		rThreshold = uniaxial_stress;
		rIsDamaging = true;
	}
}

void FemDem2DElement::RankineFragileLaw(
	double& rThreshold,
	double &rDamage, 
	const Vector &rStressVector, 
	const int Edge, 
	const double Length,
	bool& rIsDamaging
	)
{
	Vector PrincipalStressVector(3);
	this->CalculatePrincipalStress(PrincipalStressVector, rStressVector);
	const auto& properties = this->GetProperties();

	const double sigma_t = properties[YIELD_STRESS_T];
	const double c_max = std::abs(sigma_t);

	const double uniaxial_stress = GetMaxValue(PrincipalStressVector);

	if (rThreshold < tolerance) {
		rThreshold = c_max;
	} // 1st iteration sets threshold as c_max

	const double F = uniaxial_stress - rThreshold;

	if (F <= 0.0) {// Elastic region --> Damage is constant
		rDamage = this->GetConvergedDamage();
	} else {
		rDamage = 0.98; // Fragile  law
		rIsDamaging = true;
		rThreshold = uniaxial_stress;
	}
}

void FemDem2DElement::ElasticLaw(
	double& rThreshold,
	double &rDamage, 
	const Vector &rStressVector, 
	const int Edge, 
	const double Length,
	bool& rIsDamaging
	)
{
	const auto& properties = this->GetProperties();
	const double sigma_t = properties[YIELD_STRESS_T];
	const double c_max = std::abs(sigma_t);
	rDamage = 0.0;
	if (rThreshold < tolerance) {
		rThreshold = c_max;
	} // 1st iteration sets threshold as c_max
}

void FemDem2DElement::SetValueOnIntegrationPoints(
	const Variable<double> &rVariable,
	std::vector<double> &rValues,
	const ProcessInfo &rCurrentProcessInfo)
{
	if (rVariable == DAMAGE_ELEMENT) {
		for (unsigned int point_number = 0; point_number < 1; point_number++) {
			mDamage = rValues[point_number];
		}
	} else if (rVariable == STRESS_THRESHOLD) {
		for (unsigned int point_number = 0; point_number < 1; point_number++) {
			mThreshold = rValues[point_number];
		}
	} else {
		for (unsigned int point_number = 0; point_number < GetGeometry().IntegrationPoints().size(); ++point_number) {
			this->SetValue(rVariable, rValues[point_number]);
		}
	}
}

void FemDem2DElement::SetValueOnIntegrationPoints(
	const Variable<Vector> &rVariable,
	std::vector<Vector> &rValues,
	const ProcessInfo &rCurrentProcessInfo)
{
	for (unsigned int point_number = 0; point_number < GetGeometry().IntegrationPoints().size(); ++point_number) {
		this->SetValue(rVariable, rValues[point_number]);
	}
}

void FemDem2DElement::CalculateExponentialDamage(
	double& rDamage,
	const double DamageParameter,
	const double UniaxialStress,
	const double InitialThrehsold
	)
{
	rDamage = 1.0 - (InitialThrehsold / UniaxialStress) * std::exp(DamageParameter *
			 (1.0 - UniaxialStress / InitialThrehsold)); // Exponential softening law
	if (rDamage > 0.99) rDamage = 0.99;
}

// Methods to compute the tangent tensor by numerical derivation
void FemDem2DElement::CalculateTangentTensor(
	Matrix& rTangentTensor,
	const Vector& rStrainVectorGP,
	const Vector& rStressVectorGP,
	const Matrix& rElasticMatrix
	)
{
	const double number_components = rStrainVectorGP.size();
	rTangentTensor.resize(number_components, number_components);
	Vector perturbed_stress, perturbed_strain;
	perturbed_strain.resize(number_components);
	perturbed_stress.resize(number_components);

	for (unsigned int component = 0; component < number_components; component++) {
		double perturbation;
		this->CalculatePerturbation(rStrainVectorGP, perturbation, component);
		this->PerturbateStrainVector(perturbed_strain, rStrainVectorGP, perturbation, component);
		this->IntegratePerturbedStrain(perturbed_stress, perturbed_strain, rElasticMatrix);
		const Vector& delta_stress = perturbed_stress - rStressVectorGP;
		this->AssignComponentsToTangentTensor(rTangentTensor, delta_stress, perturbation, component);
	}
}

void FemDem2DElement::CalculateSecondOrderTangentTensor(
	Matrix& rTangentTensor,
	const Vector& rStrainVectorGP,
	const Vector& rStressVectorGP,
	const Matrix& rElasticMatrix
	)
{
	const double number_components = rStrainVectorGP.size();
	rTangentTensor.resize(number_components, number_components);
	Vector perturbed_stress, perturbed_strain, twice_perturbed_stress, twice_perturbed_strain;
	perturbed_strain.resize(number_components);
	perturbed_stress.resize(number_components);
	twice_perturbed_strain.resize(number_components);
	twice_perturbed_stress.resize(number_components);

	for (unsigned int component = 0; component < number_components; component++) {
		double perturbation;
		this->CalculatePerturbation(rStrainVectorGP, perturbation, component);

		// 1st the f(x+h)
		this->PerturbateStrainVector(perturbed_strain, rStrainVectorGP, perturbation, component);
		this->IntegratePerturbedStrain(perturbed_stress, perturbed_strain, rElasticMatrix);

		// Then the f(x+2h)
		this->PerturbateStrainVector(twice_perturbed_strain, rStrainVectorGP, 2*perturbation, component);
		this->IntegratePerturbedStrain(twice_perturbed_stress, twice_perturbed_strain, rElasticMatrix);

		this->AssignComponentsToSecondOrderTangentTensor(
			rTangentTensor, 
			rStressVectorGP, 
			perturbed_stress, 
			twice_perturbed_stress, 
			perturbation, 
			component);
	}
}

void FemDem2DElement::CalculateSecondOrderCentralDifferencesTangentTensor(
	Matrix& rTangentTensor,
	const Vector& rStrainVectorGP,
	const Vector& rStressVectorGP,
	const Matrix& rElasticMatrix
	)
{
	const double number_components = rStrainVectorGP.size();
	rTangentTensor.resize(number_components, number_components);
	Vector perturbed_stress, perturbed_strain, minus_perturbed_stress, minus_perturbed_strain;
	perturbed_strain.resize(number_components);
	perturbed_stress.resize(number_components);
	minus_perturbed_strain.resize(number_components);
	minus_perturbed_stress.resize(number_components);

	for (unsigned int component = 0; component < number_components; component++) {
		double perturbation;
		this->CalculatePerturbation(rStrainVectorGP, perturbation, component);

		// 1st the f(x+h)
		this->PerturbateStrainVector(perturbed_strain, rStrainVectorGP, perturbation, component);
		this->IntegratePerturbedStrain(perturbed_stress, perturbed_strain, rElasticMatrix);

		// Then the f(x-h)
		this->PerturbateStrainVector(minus_perturbed_strain, rStrainVectorGP, -perturbation, component);
		this->IntegratePerturbedStrain(minus_perturbed_stress, minus_perturbed_strain, rElasticMatrix);

		this->AssignComponentsToSecondOrderCentralDifferencesTangentTensor(
			rTangentTensor, 
			rStressVectorGP, 
			perturbed_stress, 
			minus_perturbed_stress, 
			perturbation, 
			component);
	}
}
void FemDem2DElement::CalculatePerturbation(
	const Vector& rStrainVectorGP,
	double& rPerturbation,
	const int Component
	)
{
	double perturbation_1, perturbation_2;
	if (std::abs(rStrainVectorGP[Component]) > tolerance) {
		perturbation_1 = 1.0e-5 * rStrainVectorGP[Component];
	} else {
		double min_strain_component = this->GetMinAbsValue(rStrainVectorGP);
		perturbation_1 = 1.0e-5 * min_strain_component;
	}
	const double max_strain_component = this->GetMaxAbsValue(rStrainVectorGP);
	perturbation_2 = 1.0e-10 * max_strain_component;
	rPerturbation = std::max(perturbation_1, perturbation_2);

	const double min_pert = std::sqrt(tolerance) * (1.0 + max_strain_component);
	if (rPerturbation < min_pert) rPerturbation = min_pert;
}

void FemDem2DElement::PerturbateStrainVector(
	Vector& rPerturbedStrainVector,
	const Vector& rStrainVectorGP,
	const double Perturbation,
	const int Component
	)
{
    noalias(rPerturbedStrainVector) = rStrainVectorGP;
    rPerturbedStrainVector[Component] += Perturbation;
}

void FemDem2DElement::IntegratePerturbedStrain(
	Vector& rPerturbedStressVector,
	const Vector& rPerturbedStrainVector,
	const Matrix& rElasticMatrix
	)
{
	const Vector& r_perturbed_predictive_stress = prod(rElasticMatrix, rPerturbedStrainVector);
	Vector damages_edges = ZeroVector(mNumberOfEdges);

	auto& r_elem_neigb = this->GetValue(NEIGHBOUR_ELEMENTS);
	KRATOS_ERROR_IF(r_elem_neigb.size() == 0) << " Neighbour Elements not calculated" << std::endl;

	for (unsigned int edge = 0; edge < mNumberOfEdges; edge++) {
		const Vector& r_average_stress_edge = 0.5*(r_elem_neigb[edge].GetValue(STRESS_VECTOR) + r_perturbed_predictive_stress);
		const Vector& r_average_strain_edge = 0.5*(r_elem_neigb[edge].GetValue(STRAIN_VECTOR) + rPerturbedStrainVector);

		double damage_edge = mDamages[edge];
		double threshold = mThresholds[edge];

		const double length = this->CalculateCharacteristicLength(this, r_elem_neigb[edge], edge);
		bool dummy = false;
		this->IntegrateStressDamageMechanics(threshold,
											 damage_edge,
											 r_average_strain_edge, 
											 r_average_stress_edge, 
											 edge, 
											 length,
											 dummy);
		damages_edges[edge] = damage_edge;
	} // End loop edges
	const double damage_element = this->CalculateElementalDamage(damages_edges);
	noalias(rPerturbedStressVector) = (1.0 - damage_element) * r_perturbed_predictive_stress;
}

void FemDem2DElement::AssignComponentsToTangentTensor(
	Matrix& rTangentTensor,
	const Vector& rDeltaStress,
	const double Perturbation,
	const int Component
	)
{
	const int voigt_size = rDeltaStress.size();
	for (IndexType row = 0; row < voigt_size; ++row) {
		rTangentTensor(row, Component) = rDeltaStress[row] / Perturbation;
	}
}

void FemDem2DElement::AssignComponentsToSecondOrderTangentTensor(
	Matrix& rTangentTensor,
	const Vector& rGaussPointStress,
	const Vector& rPerturbedStress,
	const Vector& rTwicePerturbedStress,
	const double Perturbation,
	const int Component
	)
{
	const int voigt_size = rGaussPointStress.size();
	for (IndexType row = 0; row < voigt_size; ++row) {
		rTangentTensor(row, Component) = (4.0 * rPerturbedStress[row] - rTwicePerturbedStress[row] - 3.0 * rGaussPointStress[row]) / Perturbation;
	}
}

void FemDem2DElement::AssignComponentsToSecondOrderCentralDifferencesTangentTensor(
	Matrix& rTangentTensor,
	const Vector& rGaussPointStress,
	const Vector& rPerturbedStress,
	const Vector& rMinusPerturbedStress,
	const double Perturbation,
	const int Component
	)
{
	const int voigt_size = rGaussPointStress.size();
	for (IndexType row = 0; row < voigt_size; ++row) {
		rTangentTensor(row, Component) = (rPerturbedStress[row] - rMinusPerturbedStress[row]) / Perturbation;
	}
}

void FemDem2DElement::CalculateTangentTensorUPerturbed(
	Matrix& rTangentTensor,
	const Vector& rStrainVectorGP,
	const Vector& rStressVectorGP,
	const Vector& rInternalForcesVectorGP,
	const Matrix& rElasticMatrix
	)
{
	auto& rGeometry = this->GetGeometry();
	const unsigned int number_of_nodes = rGeometry.PointsNumber();
	const unsigned int dimension       = rGeometry.WorkingSpaceDimension();
	const int mat_size = number_of_nodes*dimension;
	Vector current_displacements(mat_size);
	Vector previous_displacements(mat_size);
	Vector displacements_copy(mat_size);

	rTangentTensor.resize(mat_size, mat_size);

	// we fill the Vectors
	for (unsigned int i = 0; i < number_of_nodes; i++) {
		array_1d<double,3> displacement = rGeometry[i].FastGetSolutionStepValue(DISPLACEMENT);
		array_1d<double,3> displacement_old = rGeometry[i].FastGetSolutionStepValue(DISPLACEMENT, 1);

		current_displacements[2*i]     = displacement[0];
		current_displacements[2*i + 1] = displacement[1];

		previous_displacements[2*i]     = displacement_old[0];
		previous_displacements[2*i + 1] = displacement_old[1];
	}
	
	const double displ_norm = norm_2(current_displacements);
	// const double delta_displ_norm = norm_2(r_delta_displ);
	// const double numerical_tolerance = std::sqrt(tolerance) * (1.0 + displ_norm / delta_displ_norm);
	// const double numerical_tolerance = std::sqrt(tolerance) * (1.0 + displ_norm);
	// const double numerical_tolerance = 1e-12 * displ_norm;
	// const double numerical_tolerance = std::sqrt(tolerance) * (1.0 + displ_norm);
	const double numerical_tolerance = 1e-15;

	// We impose a certain displ perturbation = numerical_tolerance*delta_u
	Vector displacement_perturbation(mat_size);

	// const double min = -1.0;
	const double min = 0.0;
	const double max = 1.0;
	
	// here the perturbations at each DoF are computed
	for (unsigned int i = 0; i < mat_size; i++) {
		const double random_contr = ((double) rand()*(max - min) / (double)RAND_MAX + min);
		displacement_perturbation[i] = random_contr;
	}

	noalias(displacements_copy) = current_displacements;

	// loop over DoF in order to perturbate at each one
	for (unsigned int comp = 0; comp < current_displacements.size(); comp++) {
		// We perturbate the displacement field and save the u field
		current_displacements[comp] += numerical_tolerance * displacement_perturbation[comp];

		// Now we recompute everything
		for (unsigned int i = 0; i < number_of_nodes; i++) {
			array_1d<double,3>& r_displacement = this->GetGeometry()[i].FastGetSolutionStepValue(DISPLACEMENT);

			// We substitute the perturbed displ
			r_displacement[0] = current_displacements[2*i];
			r_displacement[1] = current_displacements[2*i + 1];
		}

		//create and initialize element variables:
		ElementDataType variables;
		ProcessInfo dummy_process_info;
		this->InitializeElementData(variables, dummy_process_info);

		//create constitutive law parameters:
		ConstitutiveLaw::Parameters values(this->GetGeometry(), this->GetProperties(), dummy_process_info);

		//set constitutive law flags:
		Flags& ConstitutiveLawOptions = values.GetOptions();

		ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_STRESS);
		ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR);

		//reading integration points
		const GeometryType::IntegrationPointsArrayType& r_integration_points = GetGeometry().IntegrationPoints(mThisIntegrationMethod );

		//auxiliary terms
		Vector VolumeForce(dimension);

		auto& r_elem_neigb = this->GetValue(NEIGHBOUR_ELEMENTS);
		KRATOS_ERROR_IF(r_elem_neigb.size() == 0) << " Neighbour Elements not calculated" << std::endl;
		Vector perturbed_internal_forces(number_of_nodes*dimension);
		perturbed_internal_forces = ZeroVector(number_of_nodes*dimension);

		for (SizeType point_number = 0; point_number < r_integration_points.size(); point_number++ ) {
			//compute element kinematic variables B, F, DN_DX ...
			this->CalculateKinematics(variables, point_number);

			//calculate material response
			this->CalculateMaterialResponse(variables, values, point_number); // FEMDEM

			//calculating weights for integration on the "reference configuration"
			variables.IntegrationWeight = r_integration_points[point_number].Weight() * variables.detJ;
			variables.IntegrationWeight = this->CalculateIntegrationWeight(variables.IntegrationWeight );

			// Loop over edges of the element...
			Vector average_stress_edge(3);
			Vector average_strain_edge(3);
			bool is_damaging = false;
			Vector damages(mNumberOfEdges);

			for (unsigned int edge = 0; edge < mNumberOfEdges; edge++) {
				this->CalculateAverageStressOnEdge(&r_elem_neigb[edge], this, average_stress_edge);
				this->CalculateAverageStrainOnEdge(&r_elem_neigb[edge], this, average_strain_edge);

				// We recover converged values
				double threshold = mThresholds[edge];
				double damage = mDamages[edge];
				
				const double length = this->CalculateCharacteristicLength(this, r_elem_neigb[edge], edge);
				this->IntegrateStressDamageMechanics(threshold, damage, average_strain_edge,
														average_stress_edge, edge, length, is_damaging);

				damages[edge] = damage;
			} // Loop over edges

			// Calculate the elemental Damage...
			const double damage_element = this->CalculateElementalDamage(damages);
			const Vector& r_predictive_stress_vector = values.GetStressVector();
			const Vector& r_integrated_stress_vector = (1.0 - damage_element) * r_predictive_stress_vector;

			// std::cout << "**************************" << std::endl;
			// KRATOS_WATCH(r_integrated_stress_vector)
			// KRATOS_WATCH(rStressVectorGP)
			// KRATOS_WATCH(numerical_tolerance)
			// KRATOS_WATCH(displacement_perturbation[comp])
			// KRATOS_WATCH(displacement_perturbation)
			// KRATOS_WATCH(numerical_tolerance)
			// KRATOS_WATCH(displ_norm)

			// Now we Reset the displacement field
			for (unsigned int i = 0; i < number_of_nodes; i++) {
				array_1d<double,3>& r_displacement = this->GetGeometry()[i].FastGetSolutionStepValue(DISPLACEMENT);

				// We substitute the perturbed displ
				r_displacement[0] = displacements_copy[2*i];
				r_displacement[1] = displacements_copy[2*i + 1];
			}
			noalias(current_displacements) = displacements_copy;

			// And compute the new Fint (only one GP)
			perturbed_internal_forces = variables.IntegrationWeight * prod(trans(variables.B), r_integrated_stress_vector);

			// KRATOS_WATCH(perturbed_internal_forces)
			// KRATOS_WATCH(rInternalForcesVectorGP)

			// Assign components to tangent tensor
			for (unsigned int row = 0; row < mat_size; ++row) {
				rTangentTensor(row, comp) = (perturbed_internal_forces[row] - rInternalForcesVectorGP[row]) 
					/ (numerical_tolerance * displacement_perturbation[comp]);
			}
		}
	}
	rTangentTensor = (trans(rTangentTensor) + rTangentTensor)*0.5;
	// KRATOS_WATCH(rTangentTensor)
}
} // namespace Kratos