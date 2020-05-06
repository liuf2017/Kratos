//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Dharmin Shah (https://github.com/sdharmin)
//

// System includes
#include <cmath>
#include <limits>

// External includes

// Project includes
#include "includes/define.h"
#include "utilities/variable_utils.h"

// Application includes
#include "custom_utilities/rans_calculation_utilities.h"
#include "custom_utilities/rans_check_utilities.h"
#include "fluid_dynamics_application_variables.h"
#include "rans_application_variables.h"

// Include base h
#include "rans_compute_reactions_process.h"

namespace Kratos
{
RansComputeReactionsProcess::RansComputeReactionsProcess(Model& rModel, Parameters rParameters)
    : mrModel(rModel), mrParameters(rParameters)
{
    KRATOS_TRY

    Parameters default_parameters = Parameters(R"(
        {
            "model_part_name"         : "PLEASE_SPECIFY_MODEL_PART_NAME",
            "echo_level"              : 0,
            "consider_periodic"       : false
        })");

    mrParameters.ValidateAndAssignDefaults(default_parameters);

    mEchoLevel = mrParameters["echo_level"].GetInt();
    mModelPartName = mrParameters["model_part_name"].GetString();
    mPeriodic = mrParameters["consider_periodic"].GetBool();

    KRATOS_CATCH("");
}

void RansComputeReactionsProcess::ExecuteInitialize()
{
    KRATOS_TRY

    if (mPeriodic)
    {
        this->CorrectPeriodicNodes(mrModel.GetModelPart(mModelPartName), NORMAL);
    }

    KRATOS_CATCH("");
}

void RansComputeReactionsProcess::CorrectPeriodicNodes(ModelPart& rModelPart,
                                                       const Variable<array_1d<double, 3>>& rVariable)
{
    KRATOS_TRY

    ModelPart::NodesContainerType& r_nodes = rModelPart.Nodes();
    const int number_of_nodes = r_nodes.size();

#pragma omp parallel for
    for (int i_node = 0; i_node < number_of_nodes; ++i_node)
    {
        NodeType& r_node = *(r_nodes.begin() + i_node);
        if (r_node.Is(PERIODIC))
        {
            const int slave_id = r_node.FastGetSolutionStepValue(PATCH_INDEX);
            const int master_id = r_node.Id();
            if (master_id < slave_id)
            {
                array_1d<double, 3>& master_value =
                    r_node.FastGetSolutionStepValue(rVariable);

                NodeType& slave_node = rModelPart.GetNode(slave_id);
                array_1d<double, 3>& slave_value =
                    slave_node.FastGetSolutionStepValue(rVariable);

                const double master_value_norm = norm_2(master_value);
                const double slave_value_norm = norm_2(slave_value);
                const double value_norm = master_value_norm + slave_value_norm;

                if (master_value_norm > 0.0)
                {
                    noalias(master_value) = master_value * (value_norm / master_value_norm);
                }

                if (slave_value_norm > 0.0)
                {
                    noalias(slave_value) = slave_value * (value_norm / slave_value_norm);
                }
            }
        }
    }

    rModelPart.GetCommunicator().SynchronizeVariable(rVariable);

    KRATOS_CATCH("");
}

void RansComputeReactionsProcess::ExecuteFinalizeSolutionStep()
{
    KRATOS_TRY

    ModelPart& r_model_part = mrModel.GetModelPart(mModelPartName);

    ModelPart::NodesContainerType& r_nodes = r_model_part.Nodes();
    VariableUtils().SetHistoricalVariableToZero(REACTION, r_nodes);

    ModelPart::ConditionsContainerType& r_conditions = r_model_part.Conditions();
    const int number_of_conditions = r_conditions.size();

#pragma omp parallel for
    for (int i_cond = 0; i_cond < number_of_conditions; ++i_cond)
    {
        ModelPart::ConditionType& r_condition = *(r_conditions.begin() + i_cond);
        CalculateReactionValues(r_condition);
    }

    r_model_part.GetCommunicator().AssembleCurrentData(REACTION);
    this->CorrectPeriodicNodes(r_model_part, REACTION);

    int number_of_nodes = r_nodes.size();
#pragma omp parallel for
    for (int i_node = 0; i_node < number_of_nodes; ++i_node)
    {
        NodeType& r_node = *(r_nodes.begin() + i_node);
        const array_1d<double, 3>& pressure_force =
            r_node.FastGetSolutionStepValue(NORMAL) *
            (-1.0 * r_node.FastGetSolutionStepValue(PRESSURE));
        r_node.FastGetSolutionStepValue(REACTION) += pressure_force;
    }

    KRATOS_INFO_IF(this->Info(), mEchoLevel > 0)
        << "Computed Reaction terms for " << mModelPartName << " for slip condition.\n";

    KRATOS_CATCH("");
}

int RansComputeReactionsProcess::Check()
{
    KRATOS_TRY
    RansCheckUtilities::CheckIfModelPartExists(mrModel, mModelPartName);

    ModelPart& r_model_part = mrModel.GetModelPart(mModelPartName);

    RansCheckUtilities::CheckIfVariableExistsInModelPart(r_model_part, REACTION);
    RansCheckUtilities::CheckIfVariableExistsInModelPart(r_model_part, DENSITY);
    RansCheckUtilities::CheckIfVariableExistsInModelPart(r_model_part, PRESSURE);
    RansCheckUtilities::CheckIfVariableExistsInModelPart(r_model_part, NORMAL);

    return 0;
    KRATOS_CATCH("");
}

std::string RansComputeReactionsProcess::Info() const
{
    return std::string("RansComputeReactionsProcess");
}

void RansComputeReactionsProcess::PrintInfo(std::ostream& rOStream) const
{
    rOStream << this->Info();
}

void RansComputeReactionsProcess::PrintData(std::ostream& rOStream) const
{
}

void RansComputeReactionsProcess::CalculateReactionValues(ModelPart::ConditionType& rCondition)
{
    if (RansCalculationUtilities::IsWall(rCondition))
    {
        ModelPart::ConditionType::GeometryType& r_geometry = rCondition.GetGeometry();
        for (IndexType i_node = 0; i_node < r_geometry.PointsNumber(); ++i_node)
        {
            NodeType& r_node = r_geometry[i_node];
            const double rho = r_node.FastGetSolutionStepValue(DENSITY);
            const array_1d<double, 3>& r_friction_velocity =
                rCondition.GetValue(FRICTION_VELOCITY);
            const double u_tau = norm_2(r_friction_velocity);

            if (u_tau > 0.0)
            {
                const double shear_force =
                    rho * std::pow(u_tau, 2) * r_geometry.DomainSize();
                r_node.SetLock();
                r_node.FastGetSolutionStepValue(REACTION) +=
                    r_friction_velocity * (shear_force / u_tau);
                r_node.UnSetLock();
            }
        }
    }
}

} // namespace Kratos.
