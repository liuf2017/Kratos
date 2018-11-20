//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//                   Ruben Zorrilla
//                   Vicente Mataix Ferrandiz
//
//

/* System includes */

/* External includes */

/* Project includes */
#include "utilities/openmp_utils.h"
#include "utilities/variable_utils.h"
#include "utilities/math_utils.h"

namespace Kratos
{

void VariableUtils::SetVectorVar(
    const ArrayVarType& rVariable,
    const array_1d<double, 3 >& Value,
    NodesContainerType& rNodes
    )
{
    KRATOS_TRY

    #pragma omp parallel for
    for (int k = 0; k< static_cast<int> (rNodes.size()); ++k) {
        NodesContainerType::iterator it_node = rNodes.begin() + k;
        noalias(it_node->FastGetSolutionStepValue(rVariable)) = Value;
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

void VariableUtils::SetVectorVarForFlag(
    const ArrayVarType& rVariable,
    const array_1d<double, 3 >& Value,
    NodesContainerType& rNodes,
    const Flags Flag,
    const bool Check
    )
{
    KRATOS_TRY

    #pragma omp parallel for
    for (int k = 0; k< static_cast<int> (rNodes.size()); ++k) {
        NodesContainerType::iterator it_node = rNodes.begin() + k;
        if (it_node->Is(Flag) == Check) noalias(it_node->FastGetSolutionStepValue(rVariable)) = Value;
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

void VariableUtils::SetNonHistoricalVectorVar(
    const ArrayVarType& rVariable,
    const array_1d<double, 3 >& Value,
    NodesContainerType& rNodes
    )
{
    KRATOS_TRY

    #pragma omp parallel for
    for (int k = 0; k< static_cast<int> (rNodes.size()); ++k) {
        NodesContainerType::iterator it_node = rNodes.begin() + k;
        it_node->SetValue(rVariable, Value);
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

void VariableUtils::SaveVectorVar(
    const ArrayVarType& OriginVariable,
    const ArrayVarType& SavedVariable,
    NodesContainerType& rNodes
    )
{
    KRATOS_TRY

    #pragma omp parallel for
    for (int k = 0; k< static_cast<int> (rNodes.size()); ++k) {
        NodesContainerType::iterator it_node = rNodes.begin() + k;
        it_node->SetValue(SavedVariable, it_node->FastGetSolutionStepValue(OriginVariable));
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

void VariableUtils::SaveScalarVar(
    const DoubleVarType& OriginVariable,
    DoubleVarType& SavedVariable,
    NodesContainerType& rNodes
    )
{
    KRATOS_TRY

    #pragma omp parallel for
    for (int k = 0; k < static_cast<int> (rNodes.size()); ++k) {
        NodesContainerType::iterator it_node = rNodes.begin() + k;
        it_node->SetValue(SavedVariable,it_node->FastGetSolutionStepValue(OriginVariable));
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

void VariableUtils::SaveVectorNonHistoricalVar(
    const ArrayVarType& OriginVariable,
    const ArrayVarType& SavedVariable,
    NodesContainerType& rNodes
    )
{
    KRATOS_TRY

    #pragma omp parallel for
    for (int k = 0; k< static_cast<int> (rNodes.size()); ++k) {
        NodesContainerType::iterator it_node = rNodes.begin() + k;
        it_node->SetValue(SavedVariable, it_node->GetValue(OriginVariable));
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

void VariableUtils::SaveScalarNonHistoricalVar(
    const DoubleVarType& OriginVariable,
    DoubleVarType& SavedVariable,
    NodesContainerType& rNodes
    )
{
    KRATOS_TRY

    #pragma omp parallel for
    for (int k = 0; k < static_cast<int> (rNodes.size()); ++k) {
        NodesContainerType::iterator it_node = rNodes.begin() + k;
        it_node->SetValue(SavedVariable,it_node->GetValue(OriginVariable));
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

void VariableUtils::CopyVectorVar(
    const ArrayVarType& OriginVariable,
    ArrayVarType& DestinationVariable,
    NodesContainerType& rNodes
    )
{
    KRATOS_TRY

    #pragma omp parallel for
    for (int k = 0; k < static_cast<int> (rNodes.size()); ++k) {
        NodesContainerType::iterator it_node = rNodes.begin() + k;
        noalias(it_node->FastGetSolutionStepValue(DestinationVariable)) = it_node->FastGetSolutionStepValue(OriginVariable);
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

void VariableUtils::CopyComponentVar(
    const ComponentVarType& OriginVariable,
    ComponentVarType& DestinationVariable,
    NodesContainerType& rNodes
    )
{
    KRATOS_TRY

    #pragma omp parallel for
    for (int k = 0; k< static_cast<int> (rNodes.size()); ++k) {
        NodesContainerType::iterator it_node = rNodes.begin() + k;
        it_node->FastGetSolutionStepValue(DestinationVariable) = it_node->FastGetSolutionStepValue(OriginVariable);
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

void VariableUtils::CopyScalarVar(
    const DoubleVarType& OriginVariable,
    DoubleVarType& DestinationVariable,
    NodesContainerType& rNodes
    )
{
    KRATOS_TRY

    #pragma omp parallel for
    for (int k = 0; k< static_cast<int> (rNodes.size()); ++k) {
        NodesContainerType::iterator it_node = rNodes.begin() + k;
        it_node->FastGetSolutionStepValue(DestinationVariable) = it_node->FastGetSolutionStepValue(OriginVariable);
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

void VariableUtils::SetToZero_VectorVar(
    const ArrayVarType& Variable,
    NodesContainerType& rNodes
    )
{
    KRATOS_TRY

    #pragma omp parallel for
    for (int k = 0; k < static_cast<int> (rNodes.size()); ++k) {
        NodesContainerType::iterator i = rNodes.begin() + k;
        noalias(i->FastGetSolutionStepValue(Variable)) = ZeroVector(3);
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

void VariableUtils::SetToZero_ScalarVar(
    const DoubleVarType& Variable,
    NodesContainerType& rNodes
    )
{
    KRATOS_TRY

    #pragma omp parallel for
    for (int k = 0; k < static_cast<int> (rNodes.size()); ++k) {
        NodesContainerType::iterator i = rNodes.begin() + k;
        i->FastGetSolutionStepValue(Variable) = 0.0;
    }

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

ModelPart::NodesContainerType VariableUtils::SelectNodeList(
    const DoubleVarType& Variable,
    const double Value,
    NodesContainerType& rOriginNodes
    )
{
    KRATOS_TRY

    NodesContainerType selected_nodes;
    for (NodesContainerType::iterator it_node = rOriginNodes.begin(); it_node != rOriginNodes.end(); ++it_node) {
        if (it_node->FastGetSolutionStepValue(Variable) == Value)
            selected_nodes.push_back(*(it_node.base()));
    }

    return selected_nodes;

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

array_1d<double, 3> VariableUtils::SumNonHistoricalNodeVectorVariable(
    const Variable<array_1d<double, 3> >& rVar,
    ModelPart& rModelPart
    )
{
    KRATOS_TRY

    array_1d<double, 3> sum_value = ZeroVector(3);

    #pragma omp parallel
    {
        array_1d<double, 3> private_sum_value = ZeroVector(3);

        #pragma omp for
        for (int k = 0; k < static_cast<int>(rModelPart.GetCommunicator().LocalMesh().NumberOfNodes()); ++k) {
            NodesContainerType::iterator it_node = rModelPart.GetCommunicator().LocalMesh().NodesBegin() + k;
            private_sum_value += it_node->GetValue(rVar);
        }

        for (int j = 0; j < static_cast<int>(sum_value.size()); ++j) {
            #pragma omp atomic
            sum_value[j] += private_sum_value[j];
        }
    }

    rModelPart.GetCommunicator().SumAll(sum_value);

    return sum_value;

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

array_1d<double, 3> VariableUtils::SumHistoricalNodeVectorVariable(
    const Variable<array_1d<double, 3> >& rVar,
    ModelPart& rModelPart,
    const unsigned int rBuffStep
    )
{
    KRATOS_TRY

    array_1d<double, 3> sum_value = ZeroVector(3);

    #pragma omp parallel
    {
        array_1d<double, 3> private_sum_value = ZeroVector(3);

        #pragma omp for
        for (int k = 0; k < static_cast<int>(rModelPart.GetCommunicator().LocalMesh().NumberOfNodes()); ++k) {
            NodesContainerType::iterator it_node = rModelPart.GetCommunicator().LocalMesh().NodesBegin() + k;
            private_sum_value += it_node->GetSolutionStepValue(rVar, rBuffStep);
        }

        for (int j = 0; j < static_cast<int>(sum_value.size()); ++j) {
            #pragma omp atomic
            sum_value[j] += private_sum_value[j];
        }
    }

    rModelPart.GetCommunicator().SumAll(sum_value);

    return sum_value;

    KRATOS_CATCH("")
}

// this function can dot product given nodal variable with its normal or surface component. Present implementation does it with surface/parallel component
// This require calculation of NORMAL on the respective structure part before, for the Chimera its done in the initial chimera step of apply chimera process monolithic
double VariableUtils::SumHistoricalNodeVectorVariableDotWithNormal(
    const Variable<array_1d<double, 3> >& rVar,
    ModelPart& rModelPart,
    double centreX,
    double centreY,
    double centreZ,
    const unsigned int rBuffStep
    )
{
    KRATOS_TRY
    KRATOS_INFO("printing so that we reached here");
    
    array_1d<double, 3> centre ;
    centre[0]=centreX;
    centre[1]=centreY;
    centre[2]=centreZ;
    KRATOS_INFO("printing so that we reached here 1");
    double sum_value = 0;
    KRATOS_INFO("printing so that we reached here 2");
    double private_sum_value = 0;
    for (int k = 0; k < static_cast<int>(rModelPart.NumberOfNodes()); ++k) {
        KRATOS_INFO("printing so that we reached here 3");
        NodesContainerType::iterator it_node = rModelPart.NodesBegin() + k;
        KRATOS_INFO("printing so that we reached here 4");
        double temp = MathUtils<double>::Dot(it_node->GetSolutionStepValue(rVar, rBuffStep),it_node->GetSolutionStepValue(NORMAL));
        KRATOS_INFO("printing so that we reached here 5");
        array_1d<double, 3> normalForce = temp* (it_node->GetSolutionStepValue(NORMAL));
        //it_node->GetSolutionStepValue(PRESSURE_FORCE)= temp* it_node->GetSolutionStepValue(NORMAL);
        KRATOS_INFO("printing so that we reached here 6");
        Vector ShearForce = it_node->GetSolutionStepValue(rVar, rBuffStep)-normalForce;
        //it_node->GetSolutionStepValue(SHEAR_FORCE)= it_node->GetSolutionStepValue(rVar, rBuffStep)-normalForce;;
        array_1d<double, 3> NodalCordinates = it_node->Coordinates();
        KRATOS_INFO("printing so that we reached here 7");
        Vector rVector;
        rVector[0] = NodalCordinates[0]-centre[0];
        rVector[1] = NodalCordinates[1]-centre[1];
        rVector[2] = NodalCordinates[2]-centre[2];

        array_1d<double, 3> MomentOfShearForce =  MathUtils<double>::CrossProduct(rVector,ShearForce);
        temp = rVector[2];
        private_sum_value += temp;
    }

    sum_value += private_sum_value;

    rModelPart.GetCommunicator().SumAll(sum_value);

    return sum_value;

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

array_1d<double, 3> VariableUtils::SumConditionVectorVariable(
    const Variable<array_1d<double, 3> >& rVar,
    ModelPart& rModelPart
    )
{
    KRATOS_TRY

    array_1d<double, 3> sum_value = ZeroVector(3);

    #pragma omp parallel
    {
        array_1d<double, 3> private_sum_value = ZeroVector(3);

        #pragma omp for
        for (int k = 0; k < static_cast<int>(rModelPart.GetCommunicator().LocalMesh().NumberOfConditions()); ++k) {
            ConditionsContainerType::iterator it_cond = rModelPart.GetCommunicator().LocalMesh().ConditionsBegin() + k;
            private_sum_value += it_cond->GetValue(rVar);
        }

        for (int j = 0; j < static_cast<int>(sum_value.size()); ++j) {
            #pragma omp atomic
            sum_value[j] += private_sum_value[j];
        }
    }

    rModelPart.GetCommunicator().SumAll(sum_value);

    return sum_value;

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

array_1d<double, 3> VariableUtils::SumElementVectorVariable(
    const Variable<array_1d<double, 3> >& rVar,
    ModelPart& rModelPart
    )
{
    KRATOS_TRY

    array_1d<double, 3> sum_value = ZeroVector(3);

    #pragma omp parallel
    {
        array_1d<double, 3> private_sum_value = ZeroVector(3);

        #pragma omp for
        for (int k = 0; k < static_cast<int>(rModelPart.GetCommunicator().LocalMesh().NumberOfElements()); ++k) {
            ElementsContainerType::iterator it_elem = rModelPart.GetCommunicator().LocalMesh().ElementsBegin() + k;
            private_sum_value += it_elem->GetValue(rVar);
        }

        for (int j = 0; j < static_cast<int>(sum_value.size()); ++j) {
            #pragma omp atomic
            sum_value[j] += private_sum_value[j];
        }
    }

    rModelPart.GetCommunicator().SumAll(sum_value);

    return sum_value;

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

bool VariableUtils::CheckVariableKeys()
{
    KRATOS_TRY

    CheckVariableKeysHelper< Variable<double> >();
    CheckVariableKeysHelper< Variable<array_1d<double,3> > >();
    CheckVariableKeysHelper< Variable<array_1d<double,4> > >();
    CheckVariableKeysHelper< Variable<array_1d<double,6> > >();
    CheckVariableKeysHelper< Variable<array_1d<double,9> > >();
    CheckVariableKeysHelper< Variable<bool> >();
    CheckVariableKeysHelper< Variable<int> >();
    CheckVariableKeysHelper< Variable<unsigned int> >();
    CheckVariableKeysHelper< Variable<Vector> >();
    CheckVariableKeysHelper< Variable<Matrix> >();
    CheckVariableKeysHelper< VariableComponent<VectorComponentAdaptor<array_1d<double, 3> > > >();
    CheckVariableKeysHelper< VariableComponent<VectorComponentAdaptor<array_1d<double, 4> > > >();
    CheckVariableKeysHelper< VariableComponent<VectorComponentAdaptor<array_1d<double, 6> > > >();
    CheckVariableKeysHelper< VariableComponent<VectorComponentAdaptor<array_1d<double, 9> > > >();

    return true;

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

bool VariableUtils::CheckDofs(ModelPart& rModelPart)
{
    KRATOS_TRY

    for(auto& node : rModelPart.Nodes()) {
        for (auto& dof : node.GetDofs()) {
//                 KRATOS_ERROR_IF_NOT(node.SolutionStepsDataHas(dof.GetVariable())) << "Node : " << node << " does not have allocated space for the variable " << dof << std::endl;
            KRATOS_CHECK_VARIABLE_KEY(dof.GetVariable());

        }
    }

    return true;

    KRATOS_CATCH("")
}

/***********************************************************************************/
/***********************************************************************************/

void VariableUtils::UpdateCurrentToInitialConfiguration(const ModelPart::NodesContainerType& rNodes) {
    KRATOS_TRY;

    const int num_nodes = rNodes.size();
    const auto nodes_begin = rNodes.begin();

    #pragma omp parallel for
    for (int i=0; i<num_nodes; i++) {
        const auto it_node  = nodes_begin + i;
        noalias(it_node->Coordinates()) = it_node->GetInitialPosition();
    }

    KRATOS_CATCH("");
}

/***********************************************************************************/
/***********************************************************************************/

void VariableUtils::UpdateInitialToCurrentConfiguration(const ModelPart::NodesContainerType& rNodes) {
    KRATOS_TRY;

    const int num_nodes = rNodes.size();
    const auto nodes_begin = rNodes.begin();

    #pragma omp parallel for
    for (int i=0; i<num_nodes; i++) {
        const auto it_node  = nodes_begin + i;
        noalias(it_node->GetInitialPosition()) = it_node->Coordinates();
    }

    KRATOS_CATCH("");
}

} /* namespace Kratos.*/
