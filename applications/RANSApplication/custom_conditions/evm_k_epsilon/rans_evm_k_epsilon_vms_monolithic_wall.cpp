//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya
//

// System includes
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <string>

// External includes

// Project includes
#include "includes/checks.h"
#include "includes/define.h"
#include "includes/variables.h"

// Application includes
#include "custom_utilities/rans_calculation_utilities.h"
#include "rans_application_variables.h"

// Include base h
#include "rans_evm_k_epsilon_vms_monolithic_wall.h"

namespace Kratos
{
template <unsigned int TDim, unsigned int TNumNodes>
RansEvmKEpsilonVmsMonolithicWall<TDim, TNumNodes>& RansEvmKEpsilonVmsMonolithicWall<TDim, TNumNodes>::operator=(
    RansEvmKEpsilonVmsMonolithicWall<TDim, TNumNodes> const& rOther)
{
    Condition::operator=(rOther);

    return *this;
}

template <unsigned int TDim, unsigned int TNumNodes>
Condition::Pointer RansEvmKEpsilonVmsMonolithicWall<TDim, TNumNodes>::Create(
    IndexType NewId, NodesArrayType const& ThisNodes, PropertiesType::Pointer pProperties) const
{
    return Kratos::make_intrusive<RansEvmKEpsilonVmsMonolithicWall>(
        NewId, this->GetGeometry().Create(ThisNodes), pProperties);
}

template <unsigned int TDim, unsigned int TNumNodes>
Condition::Pointer RansEvmKEpsilonVmsMonolithicWall<TDim, TNumNodes>::Create(
    IndexType NewId, GeometryType::Pointer pGeom, PropertiesType::Pointer pProperties) const
{
    return Kratos::make_intrusive<RansEvmKEpsilonVmsMonolithicWall>(NewId, pGeom, pProperties);
}

template <unsigned int TDim, unsigned int TNumNodes>
Condition::Pointer RansEvmKEpsilonVmsMonolithicWall<TDim, TNumNodes>::Clone(
    IndexType NewId, NodesArrayType const& rThisNodes) const
{
    Condition::Pointer pNewCondition = Create(
        NewId, this->GetGeometry().Create(rThisNodes), this->pGetProperties());

    pNewCondition->SetData(this->GetData());
    pNewCondition->SetFlags(this->GetFlags());

    return pNewCondition;
}

template <unsigned int TDim, unsigned int TNumNodes>
int RansEvmKEpsilonVmsMonolithicWall<TDim, TNumNodes>::Check(const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    int Check = BaseType::Check(rCurrentProcessInfo);

    if (Check != 0)
    {
        return Check;
    }

    const GeometryType& r_geometry = this->GetGeometry();

    for (IndexType i_node = 0; i_node < TNumNodes; ++i_node)
    {
        const NodeType& r_node = r_geometry[i_node];

        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_KINETIC_ENERGY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(DENSITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(VELOCITY, r_node);
    }

    return 0;

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKEpsilonVmsMonolithicWall<TDim, TNumNodes>::Initialize()
{
    KRATOS_TRY;

    if (RansCalculationUtilities::IsWall(*this))
    {
        const array_1d<double, 3>& rNormal = this->GetValue(NORMAL);
        KRATOS_ERROR_IF(norm_2(rNormal) == 0.0)
            << "NORMAL must be calculated before using this " << this->Info() << "\n";

        KRATOS_ERROR_IF(this->GetValue(NEIGHBOUR_ELEMENTS).size() == 0)
            << this->Info() << " cannot find parent element\n";

        mWallHeight = RansCalculationUtilities::CalculateWallHeight(*this, rNormal);
    }

    KRATOS_CATCH("");
}

template <unsigned int TDim, unsigned int TNumNodes>
std::string RansEvmKEpsilonVmsMonolithicWall<TDim, TNumNodes>::Info() const
{
    std::stringstream buffer;
    buffer << "RansEvmKEpsilonVmsMonolithicWall" << TDim << "D";
    return buffer.str();
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKEpsilonVmsMonolithicWall<TDim, TNumNodes>::PrintInfo(std::ostream& rOStream) const
{
    rOStream << "RansEvmKEpsilonVmsMonolithicWall";
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKEpsilonVmsMonolithicWall<TDim, TNumNodes>::PrintData(std::ostream& rOStream) const
{
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKEpsilonVmsMonolithicWall<TDim, TNumNodes>::ApplyWallLaw(
    MatrixType& rLocalMatrix, VectorType& rLocalVector, ProcessInfo& rCurrentProcessInfo)
{
    if (RansCalculationUtilities::IsWall(*this))
    {
        const double eps = std::numeric_limits<double>::epsilon();

        const array_1d<double, 3> wall_cell_center_velocity =
            RansCalculationUtilities::CalculateWallVelocity(*this);
        const double wall_cell_center_velocity_magnitude = norm_2(wall_cell_center_velocity);

        const double y_plus_limit = rCurrentProcessInfo[RANS_Y_PLUS_LIMIT];
        double y_plus = 0.0;

        if (wall_cell_center_velocity_magnitude > eps)
        {
            constexpr unsigned int block_size = TDim + 1;

            // calculate cell centered y_plus value
            const double kappa = rCurrentProcessInfo[WALL_VON_KARMAN];
            const double beta = rCurrentProcessInfo[WALL_SMOOTHNESS_BETA];
            const double nu = RansCalculationUtilities::EvaluateInParentCenter(
                KINEMATIC_VISCOSITY, *this);

            double u_tau;
            RansCalculationUtilities::CalculateYPlusAndUtau(
                y_plus, u_tau, wall_cell_center_velocity_magnitude, mWallHeight,
                nu, kappa, beta);

            GeometryType& r_geometry = this->GetGeometry();

            const GeometryType::IntegrationPointsArrayType& integration_points =
                r_geometry.IntegrationPoints(GeometryData::GI_GAUSS_2);
            const std::size_t number_of_gauss_points = integration_points.size();
            MatrixType shape_functions =
                r_geometry.ShapeFunctionsValues(GeometryData::GI_GAUSS_2);

            const double area = r_geometry.DomainSize();
            // CAUTION: "Jacobian" is 2.0*A for triangles but 0.5*A for lines
            double J = (TDim == 2) ? 0.5 * area : 2.0 * area;

            // In the linear region, force the velocity to be in the log region lowest
            // since k - epsilon is only valid in the log region.
            // In order to avoid issues with stagnation points, tke is also used
            const double c_mu_25 =
                std::pow(rCurrentProcessInfo[TURBULENCE_RANS_C_MU], 0.25);
            const std::function<double(double, double)> linear_region_functional =
                [c_mu_25, y_plus_limit](const double TurbulentKineticEnergy,
                                        const double Velocity) -> double {
                return std::max(c_mu_25 * std::sqrt(std::max(TurbulentKineticEnergy, 0.0)),
                                Velocity / y_plus_limit);
            };

            // log region, apply the u_tau which is calculated based on the cell centered velocity
            const std::function<double(double, double)> log_region_functional =
                [u_tau](const double, const double) -> double { return u_tau; };

            const std::function<double(double, double)> wall_tau_function =
                ((y_plus >= y_plus_limit) ? log_region_functional : linear_region_functional);

            double condition_u_tau = 0.0;

            for (size_t g = 0; g < number_of_gauss_points; ++g)
            {
                const Vector& gauss_shape_functions = row(shape_functions, g);
                const double weight = J * integration_points[g].Weight();

                const array_1d<double, 3>& r_wall_velocity =
                    RansCalculationUtilities::EvaluateInPoint(
                        r_geometry, VELOCITY, gauss_shape_functions);
                const double wall_velocity_magnitude = norm_2(r_wall_velocity);
                const double rho = RansCalculationUtilities::EvaluateInPoint(
                    r_geometry, DENSITY, gauss_shape_functions);

                if (wall_velocity_magnitude > eps)
                {
                    const double tke = RansCalculationUtilities::EvaluateInPoint(
                        r_geometry, TURBULENT_KINETIC_ENERGY, gauss_shape_functions);
                    const double gauss_u_tau =
                        wall_tau_function(tke, wall_velocity_magnitude);

                    condition_u_tau += gauss_u_tau;

                    const double value = rho * std::pow(gauss_u_tau, 2) *
                                         weight / wall_velocity_magnitude;

                    for (IndexType a = 0; a < TNumNodes; ++a)
                    {
                        for (IndexType dim = 0; dim < TDim; ++dim)
                        {
                            for (IndexType b = 0; b < TNumNodes; ++b)
                            {
                                rLocalMatrix(a * block_size + dim, b * block_size + dim) +=
                                    gauss_shape_functions[a] *
                                    gauss_shape_functions[b] * value;
                            }
                            rLocalVector[a * block_size + dim] -=
                                gauss_shape_functions[a] * value * r_wall_velocity[dim];
                        }
                    }
                }
            }

            condition_u_tau /= static_cast<double>(number_of_gauss_points);
            this->SetValue(FRICTION_VELOCITY, condition_u_tau * wall_cell_center_velocity /
                                                  wall_cell_center_velocity_magnitude);
        }

        this->SetValue(RANS_Y_PLUS, std::max(y_plus, y_plus_limit));
    }
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKEpsilonVmsMonolithicWall<TDim, TNumNodes>::save(Serializer& rSerializer) const
{
    KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, Condition);
}

template <unsigned int TDim, unsigned int TNumNodes>
void RansEvmKEpsilonVmsMonolithicWall<TDim, TNumNodes>::load(Serializer& rSerializer)
{
    KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, Condition);
}

// template instantiations

template class RansEvmKEpsilonVmsMonolithicWall<2, 2>;
template class RansEvmKEpsilonVmsMonolithicWall<3, 3>;

} // namespace Kratos.
