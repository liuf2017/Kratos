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

#if !defined(KRATOS_RANS_COMPUTE_REACTIONS_PROCESS_H_INCLUDED)
#define KRATOS_RANS_COMPUTE_REACTIONS_PROCESS_H_INCLUDED

// System includes
#include <string>

// External includes

// Project includes
#include "containers/model.h"
#include "processes/process.h"

namespace Kratos
{
///@addtogroup RANSApplication
///@{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/**
 * @brief Computes the reaction forces for slip modelpart, can be
 *        further used for drag calculation
 *
 * This process sets epsilon values based on the following formula
 *
 * \[
 *
 *  \REACTION = p.n + tau
 *
 * \]
 *
 *
 */

class KRATOS_API(RANS_APPLICATION) RansComputeReactionsProcess : public Process
{
public:
    ///@name Type Definitions
    ///@{

    using NodeType = ModelPart::NodeType;

    using NodesContainerType = ModelPart::NodesContainerType;

    /// Pointer definition of RansComputeReactionsProcess
    KRATOS_CLASS_POINTER_DEFINITION(RansComputeReactionsProcess);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Constructor
    RansComputeReactionsProcess(Model& rModel, Parameters rParameters);

    /// Destructor.
    ~RansComputeReactionsProcess() override = default;

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{
    // void ExecuteInitialize() override;
    void ExecuteInitialize() override;
    void ExecuteFinalizeSolutionStep() override;

    int Check() override;

    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override;

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override;

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override;

    ///@}
    ///@name Friends
    ///@{

    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    ///@}
    ///@name Protected  Access
    ///@{

    ///@}
    ///@name Protected Inquiry
    ///@{

    ///@}
    ///@name Protected LifeCycle
    ///@{

    ///@}

private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    Model& mrModel;
    Parameters mrParameters;
    std::string mModelPartName;

    int mEchoLevel;
    bool mPeriodic;
    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    void CalculateReactionValues(ModelPart::ConditionType& rCondition);

    void CorrectPeriodicNodes(ModelPart& rModelPart, const Variable<array_1d<double, 3>>& rVariable);

    ///@}
    ///@name Private  Access
    ///@{

    ///@}
    ///@name Private Inquiry
    ///@{

    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    RansComputeReactionsProcess& operator=(
        RansComputeReactionsProcess const& rOther);

    /// Copy constructor.
    RansComputeReactionsProcess(RansComputeReactionsProcess const& rOther);

    ///@}

}; // Class RansComputeReactionsProcess

///@}

///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

/// output stream function
inline std::ostream& operator<<(std::ostream& rOStream,
                                const RansComputeReactionsProcess& rThis);

///@}

///@} addtogroup block

} // namespace Kratos.

#endif // KRATOS_RANS_COMPUTE_REACTIONS_PROCESS_H_INCLUDED defined