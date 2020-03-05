//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya (https://github.com/sunethwarna)
//

// System includes
#include <cmath>

// Application includes
#include "rans_application_variables.h"

// Include base h
#include "rans_calculation_utilities.h"

namespace Kratos
{
namespace RansCalculationUtilities
{
void CalculateGeometryData(const GeometryType& rGeometry,
                           const GeometryData::IntegrationMethod& rIntegrationMethod,
                           Vector& rGaussWeights,
                           Matrix& rNContainer,
                           GeometryType::ShapeFunctionsGradientsType& rDN_DX)
{
    const unsigned int number_of_gauss_points =
        rGeometry.IntegrationPointsNumber(rIntegrationMethod);

    Vector DetJ;
    rGeometry.ShapeFunctionsIntegrationPointsGradients(rDN_DX, DetJ, rIntegrationMethod);

    const std::size_t number_of_nodes = rGeometry.PointsNumber();

    if (rNContainer.size1() != number_of_gauss_points || rNContainer.size2() != number_of_nodes)
    {
        rNContainer.resize(number_of_gauss_points, number_of_nodes, false);
    }
    rNContainer = rGeometry.ShapeFunctionsValues(rIntegrationMethod);

    const GeometryType::IntegrationPointsArrayType& IntegrationPoints =
        rGeometry.IntegrationPoints(rIntegrationMethod);

    if (rGaussWeights.size() != number_of_gauss_points)
    {
        rGaussWeights.resize(number_of_gauss_points, false);
    }

    for (unsigned int g = 0; g < number_of_gauss_points; ++g)
        rGaussWeights[g] = DetJ[g] * IntegrationPoints[g].Weight();
}

void CalculateConditionGeometryData(const GeometryType& rGeometry,
                                    const GeometryData::IntegrationMethod& rIntegrationMethod,
                                    Vector& rGaussWeights,
                                    Matrix& rNContainer)
{
    const GeometryType::IntegrationPointsArrayType& integration_points =
        rGeometry.IntegrationPoints(rIntegrationMethod);

    const std::size_t number_of_integration_points = integration_points.size();
    const int dimension = rGeometry.WorkingSpaceDimension();
    const double domain_size = rGeometry.DomainSize();

    if (rGaussWeights.size() != number_of_integration_points)
    {
        rGaussWeights.resize(number_of_integration_points, false);
    }

    rNContainer = rGeometry.ShapeFunctionsValues(rIntegrationMethod);

    // CAUTION: "Jacobian" is 2.0*A for triangles but 0.5*A for lines
    double det_J = (dimension == 2) ? 0.5 * domain_size : 2.0 * domain_size;

    for (unsigned int g = 0; g < number_of_integration_points; g++)
    {
        rGaussWeights[g] = det_J * integration_points[g].Weight();
    }
}

double EvaluateInPoint(const GeometryType& rGeometry,
                       const Variable<double>& rVariable,
                       const Vector& rShapeFunction,
                       const int Step)
{
    const unsigned int number_of_nodes = rGeometry.PointsNumber();
    double value = 0.0;
    for (unsigned int c = 0; c < number_of_nodes; ++c)
    {
        value += rShapeFunction[c] * rGeometry[c].FastGetSolutionStepValue(rVariable, Step);
    }

    return value;
}

array_1d<double, 3> EvaluateInPoint(const GeometryType& rGeometry,
                                    const Variable<array_1d<double, 3>>& rVariable,
                                    const Vector& rShapeFunction,
                                    const int Step)
{
    const unsigned int number_of_nodes = rGeometry.PointsNumber();
    array_1d<double, 3> value = ZeroVector(3);
    for (unsigned int c = 0; c < number_of_nodes; ++c)
    {
        value += rShapeFunction[c] * rGeometry[c].FastGetSolutionStepValue(rVariable, Step);
    }

    return value;
}

template <unsigned int TDim>
double CalculateMatrixTrace(const BoundedMatrix<double, TDim, TDim>& rMatrix)
{
    double value = 0.0;
    for (unsigned int i = 0; i < TDim; ++i)
        value += rMatrix(i, i);

    return value;
}

GeometryType::ShapeFunctionsGradientsType CalculateGeometryParameterDerivatives(
    const GeometryType& rGeometry, const GeometryData::IntegrationMethod& rIntegrationMethod)
{
    const GeometryType::ShapeFunctionsGradientsType& DN_De =
        rGeometry.ShapeFunctionsLocalGradients(rIntegrationMethod);
    const std::size_t number_of_nodes = rGeometry.PointsNumber();
    const unsigned int number_of_gauss_points =
        rGeometry.IntegrationPointsNumber(rIntegrationMethod);
    const std::size_t dim = rGeometry.WorkingSpaceDimension();

    GeometryType::ShapeFunctionsGradientsType de_dx(number_of_gauss_points);

    Matrix geometry_coordinates(dim, number_of_nodes);

    for (std::size_t i_node = 0; i_node < number_of_nodes; ++i_node)
    {
        const array_1d<double, 3>& r_coordinates =
            rGeometry.Points()[i_node].Coordinates();
        for (std::size_t d = 0; d < dim; ++d)
            geometry_coordinates(d, i_node) = r_coordinates[d];
    }

    for (unsigned int g = 0; g < number_of_gauss_points; ++g)
    {
        const Matrix& r_current_local_gradients = DN_De[g];
        Matrix current_dx_de(dim, dim);
        noalias(current_dx_de) = prod(geometry_coordinates, r_current_local_gradients);
        Matrix inv_current_dx_de(dim, dim);
        double det_J;
        MathUtils<double>::InvertMatrix<Matrix, Matrix>(
            current_dx_de, inv_current_dx_de, det_J);

        de_dx[g] = inv_current_dx_de;
    }
    return de_dx;
}

template <std::size_t TDim>
void CalculateGeometryParameterDerivativesShapeSensitivity(BoundedMatrix<double, TDim, TDim>& rOutput,
                                                           const ShapeParameter& rShapeDerivative,
                                                           const Matrix& rDnDe,
                                                           const Matrix& rDeDx)
{
    const Vector& r_dnc_de = row(rDnDe, rShapeDerivative.NodeIndex);

    for (std::size_t j = 0; j < TDim; ++j)
    {
        const Vector& r_de_dxj = column(rDeDx, j);
        for (std::size_t i = 0; i < TDim; ++i)
        {
            rOutput(i, j) = -1.0 * rDeDx(i, rShapeDerivative.Direction) *
                            inner_prod(r_dnc_de, r_de_dxj);
        }
    }
}

template <unsigned int TDim>
void CalculateGradient(BoundedMatrix<double, TDim, TDim>& rOutput,
                       const Geometry<ModelPart::NodeType>& rGeometry,
                       const Variable<array_1d<double, 3>>& rVariable,
                       const Matrix& rShapeDerivatives,
                       const int Step)
{
    rOutput.clear();
    std::size_t number_of_nodes = rGeometry.PointsNumber();

    for (unsigned int a = 0; a < number_of_nodes; ++a)
    {
        const array_1d<double, 3>& r_value =
            rGeometry[a].FastGetSolutionStepValue(rVariable, Step);
        for (unsigned int i = 0; i < TDim; ++i)
        {
            for (unsigned int j = 0; j < TDim; ++j)
            {
                rOutput(i, j) += rShapeDerivatives(a, j) * r_value[i];
            }
        }
    }
}

void CalculateGradient(array_1d<double, 3>& rOutput,
                       const Geometry<ModelPart::NodeType>& rGeometry,
                       const Variable<double>& rVariable,
                       const Matrix& rShapeDerivatives,
                       const int Step)
{
    rOutput.clear();
    std::size_t number_of_nodes = rGeometry.PointsNumber();
    unsigned int domain_size = rShapeDerivatives.size2();

    for (std::size_t a = 0; a < number_of_nodes; ++a)
    {
        const double value = rGeometry[a].FastGetSolutionStepValue(rVariable, Step);
        for (unsigned int i = 0; i < domain_size; ++i)
            rOutput[i] += rShapeDerivatives(a, i) * value;
    }
}

template <unsigned int TDim>
Vector GetVector(const array_1d<double, 3>& rVector)
{
    Vector result(TDim);

    for (unsigned int i_dim = 0; i_dim < TDim; ++i_dim)
        result[i_dim] = rVector[i_dim];

    return result;
}

Vector GetVector(const array_1d<double, 3>& rVector, const unsigned int Dim)
{
    Vector result(Dim);

    for (unsigned int i_dim = 0; i_dim < Dim; ++i_dim)
        result[i_dim] = rVector[i_dim];

    return result;
}

double CalculateLogarithmicYPlusLimit(const double Kappa,
                                      const double Beta,
                                      const int MaxIterations,
                                      const double Tolerance)
{
    double y_plus = 11.06;
    const double inv_kappa = 1.0 / Kappa;
    double dx = 0.0;
    for (int i = 0; i < MaxIterations; ++i)
    {
        const double value = inv_kappa * std::log(y_plus) + Beta;
        dx = value - y_plus;

        if (std::abs(dx) < Tolerance)
            return y_plus;

        y_plus = value;
    }

    KRATOS_WARNING("LogarithmicYPlusLimit")
        << "Logarithmic y_plus limit reached max iterations with dx > "
           "Tolerance [ "
        << dx << " > " << Tolerance << ", MaxIterations = " << MaxIterations << " ].\n";
    return y_plus;
}

double CalculateWallHeight(const ConditionType& rCondition, const array_1d<double, 3>& rNormal)
{
    KRATOS_TRY

    array_1d<double, 3> normal = rNormal / norm_2(rNormal);

    const ElementType& r_parent_element = rCondition.GetValue(NEIGHBOUR_ELEMENTS)[0];

    const GeometryType& r_parent_geometry = r_parent_element.GetGeometry();
    const GeometryType& r_condition_geometry = rCondition.GetGeometry();

    auto calculate_cell_center = [](const GeometryType& rGeometry) -> array_1d<double, 3> {
        const int number_of_nodes = rGeometry.PointsNumber();
        array_1d<double, 3> cell_center = ZeroVector(3);
        for (int i_node = 0; i_node < number_of_nodes; ++i_node)
        {
            noalias(cell_center) =
                cell_center + rGeometry[i_node].Coordinates() *
                                  (1.0 / static_cast<double>(number_of_nodes));
        }

        return cell_center;
    };

    const array_1d<double, 3>& parent_center = calculate_cell_center(r_parent_geometry);

    const array_1d<double, 3>& condition_center =
        calculate_cell_center(r_condition_geometry);

    return inner_prod(condition_center - parent_center, normal);

    KRATOS_CATCH("");
}

array_1d<double, 3> CalculateWallVelocity(const ConditionType& rCondition)
{
    array_1d<double, 3> normal = rCondition.GetValue(NORMAL);
    normal /= norm_2(normal);

    const ElementType& r_parent_element = rCondition.GetValue(NEIGHBOUR_ELEMENTS)[0];
    const GeometryType& r_parent_geometry = r_parent_element.GetGeometry();

    Vector parent_gauss_weights;
    Matrix parent_shape_functions;
    GeometryData::ShapeFunctionsGradientsType parent_shape_function_derivatives;
    CalculateGeometryData(r_parent_geometry, GeometryData::IntegrationMethod::GI_GAUSS_1,
                          parent_gauss_weights, parent_shape_functions,
                          parent_shape_function_derivatives);

    const Vector& gauss_parent_shape_functions = row(parent_shape_functions, 0);
    const array_1d<double, 3>& parent_center_velocity =
        EvaluateInPoint(r_parent_geometry, VELOCITY, gauss_parent_shape_functions);
    const array_1d<double, 3>& parent_center_mesh_velocity = EvaluateInPoint(
        r_parent_geometry, MESH_VELOCITY, gauss_parent_shape_functions);

    const array_1d<double, 3>& parent_center_effective_velocity =
        parent_center_velocity - parent_center_mesh_velocity;
    return parent_center_effective_velocity -
           normal * inner_prod(parent_center_effective_velocity, normal);
}

template <typename TDataType>
TDataType EvaluateInParentCenter(const Variable<TDataType>& rVariable,
                                 const ConditionType& rCondition,
                                 const int Step)
{
    const ElementType& r_parent_element = rCondition.GetValue(NEIGHBOUR_ELEMENTS)[0];
    const GeometryType& r_parent_geometry = r_parent_element.GetGeometry();

    Vector parent_gauss_weights;
    Matrix parent_shape_functions;
    GeometryData::ShapeFunctionsGradientsType parent_shape_function_derivatives;
    CalculateGeometryData(r_parent_geometry, GeometryData::IntegrationMethod::GI_GAUSS_1,
                          parent_gauss_weights, parent_shape_functions,
                          parent_shape_function_derivatives);
    return EvaluateInPoint(r_parent_geometry, rVariable,
                           row(parent_shape_functions, 0), Step);
}

void CalculateYPlusAndUtau(double& rYPlus,
                           double& rUTau,
                           const double WallVelocity,
                           const double WallHeight,
                           const double KinematicViscosity,
                           const double Kappa,
                           const double Beta,
                           const int MaxIterations,
                           const double Tolerance)
{
    const double limit_y_plus =
        CalculateLogarithmicYPlusLimit(Kappa, Beta, MaxIterations, Tolerance);

    // linear region
    rUTau = std::sqrt(WallVelocity * KinematicViscosity / WallHeight);
    rYPlus = rUTau * WallHeight / KinematicViscosity;
    const double inv_kappa = 1.0 / Kappa;

    // log region
    if (rYPlus > limit_y_plus)
    {
        int iter = 0;
        double dx = 1e10;
        double u_plus = inv_kappa * log(rYPlus) + Beta;

        while (iter < MaxIterations && fabs(dx) > Tolerance * rUTau)
        {
            // Newton-Raphson iteration
            double f = rUTau * u_plus - WallVelocity;
            double df = u_plus + inv_kappa;
            dx = f / df;

            // Update variables
            rUTau -= dx;
            rYPlus = WallHeight * rUTau / KinematicViscosity;
            u_plus = inv_kappa * log(rYPlus) + Beta;
            ++iter;
        }
        if (iter == MaxIterations)
        {
            std::cout << "Warning: wall condition Newton-Raphson did not "
                         "converge. Residual is "
                      << dx << std::endl;
        }
    }
}

bool IsWall(const ConditionType& rCondition)
{
    KRATOS_TRY

    if (rCondition.Is(SLIP) && rCondition.Is(STRUCTURE))
    {
        return true;
    }
    else
    {
        if (rCondition.GetValue(PARENT_CONDITION_POINTER))
        {
            const ConditionType& r_parent_condition =
                *(rCondition.GetValue(PARENT_CONDITION_POINTER));
            if (r_parent_condition.Is(SLIP) && r_parent_condition.Is(STRUCTURE))
            {
                return true;
            }
        }
    }

    return false;

    KRATOS_CATCH("");
}

bool IsInlet(const ConditionType& rCondition)
{
    KRATOS_TRY

    if (rCondition.Is(INLET))
    {
        return true;
    }
    else
    {
        if (rCondition.GetValue(PARENT_CONDITION_POINTER))
        {
            const ConditionType& r_parent_condition =
                *(rCondition.GetValue(PARENT_CONDITION_POINTER));
            if (r_parent_condition.Is(INLET))
            {
                return true;
            }
        }
    }

    return false;

    KRATOS_CATCH("");
}

// template instantiations

template double CalculateMatrixTrace<2>(const BoundedMatrix<double, 2, 2>&);
template double CalculateMatrixTrace<3>(const BoundedMatrix<double, 3, 3>&);

template void CalculateGradient<2>(BoundedMatrix<double, 2, 2>&,
                                   const Geometry<ModelPart::NodeType>&,
                                   const Variable<array_1d<double, 3>>&,
                                   const Matrix&,
                                   const int);

template void CalculateGradient<3>(BoundedMatrix<double, 3, 3>&,
                                   const Geometry<ModelPart::NodeType>&,
                                   const Variable<array_1d<double, 3>>&,
                                   const Matrix&,
                                   const int);

template void CalculateGeometryParameterDerivativesShapeSensitivity<2>(
    BoundedMatrix<double, 2, 2>&, const ShapeParameter&, const Matrix&, const Matrix&);

template void CalculateGeometryParameterDerivativesShapeSensitivity<3>(
    BoundedMatrix<double, 3, 3>&, const ShapeParameter&, const Matrix&, const Matrix&);

template Vector GetVector<2>(const array_1d<double, 3>&);
template Vector GetVector<3>(const array_1d<double, 3>&);

template double EvaluateInParentCenter(const Variable<double>& rVariable,
                                       const ConditionType& rCondition,
                                       const int Step);

template array_1d<double, 3> EvaluateInParentCenter(const Variable<array_1d<double, 3>>& rVariable,
                                                    const ConditionType& rCondition,
                                                    const int Step);

} // namespace RansCalculationUtilities
///@}

} // namespace Kratos