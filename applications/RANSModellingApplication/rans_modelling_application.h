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

#if !defined(KRATOS_RANS_MODELLING_APPLICATION_H_INCLUDED)
#define KRATOS_RANS_MODELLING_APPLICATION_H_INCLUDED

// System includes

// External includes

// Project includes
#include "includes/kratos_application.h"

// Element includes
#include "custom_elements/evm_k_epsilon/rans_evm_epsilon_element.h"
#include "custom_elements/evm_k_epsilon/rans_evm_k_element.h"
#include "custom_elements/evm_k_epsilon/rans_evm_low_re_epsilon_element.h"
#include "custom_elements/evm_k_epsilon/rans_evm_low_re_k_element.h"

// Condition includes
#include "custom_conditions/evm_k_epsilon/rans_evm_epsilon_wall_condition.h"
#include "custom_conditions/evm_k_epsilon/rans_evm_vms_monolithic_wall_condition.h"

namespace Kratos
{
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

/// Short class definition.
/** Detail class definition.
 */
class KratosRANSModellingApplication : public KratosApplication
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of KratosRANSModellingApplication
    KRATOS_CLASS_POINTER_DEFINITION(KratosRANSModellingApplication);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    KratosRANSModellingApplication();

    /// Destructor.
    ~KratosRANSModellingApplication() override
    {
    }

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    void Register() override;

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
    std::string Info() const override
    {
        return "KratosRANSModellingApplication";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << Info();
        PrintData(rOStream);
    }

    ///// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
        KRATOS_WATCH("in my application");
        KRATOS_WATCH(KratosComponents<VariableData>::GetComponents().size());

        rOStream << "Variables:" << std::endl;
        KratosComponents<VariableData>().PrintData(rOStream);
        rOStream << std::endl;
        rOStream << "Elements:" << std::endl;
        KratosComponents<Element>().PrintData(rOStream);
        rOStream << std::endl;
        rOStream << "Conditions:" << std::endl;
        KratosComponents<Condition>().PrintData(rOStream);
    }

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

    // static const ApplicationCondition  msApplicationCondition;

    ///@}
    ///@name Member Variables
    ///@{

    /// k-epsilon turbulence model elements
    const RansEvmKEpsilonLowReKElement<2, 3> mRansEvmKEpsilonLowReK2D;
    const RansEvmKEpsilonLowReKElement<3, 4> mRansEvmKEpsilonLowReK3D;

    const RansEvmKEpsilonLowReEpsilonElement<2, 3> mRansEvmKEpsilonLowReEpsilon2D;
    const RansEvmKEpsilonLowReEpsilonElement<3, 4> mRansEvmKEpsilonLowReEpsilon3D;

    const RansEvmKEpsilonKElement<2, 3> mRansEvmKEpsilonK2D;
    const RansEvmKEpsilonKElement<3, 4> mRansEvmKEpsilonK3D;

    const RansEvmKEpsilonEpsilon<2, 3> mRansEvmKEpsilonEpsilon2D;
    const RansEvmKEpsilonEpsilon<3, 4> mRansEvmKEpsilonEpsilon3D;

    /// k-epsilon turbulence model conditions
    const RansEvmKEpsilonEpsilonWallCondition<2> mRansEvmKEpsilonEpsilonWallCondition2D2N;
    const RansEvmKEpsilonEpsilonWallCondition<3> mRansEvmKEpsilonEpsilonWallCondition3D3N;

    const RansEvmKEpsilonVmsMonolithicWallCondition<2> mRansEvmKEpsilonVmsMonolithicWallCondition2D2N;
    const RansEvmKEpsilonVmsMonolithicWallCondition<3> mRansEvmKEpsilonVmsMonolithicWallCondition3D3N;

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

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
    KratosRANSModellingApplication& operator=(KratosRANSModellingApplication const& rOther);

    /// Copy constructor.
    KratosRANSModellingApplication(KratosRANSModellingApplication const& rOther);

    ///@}

}; // Class KratosRANSModellingApplication

///@}

///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

///@}

} // namespace Kratos.

#endif // KRATOS_RANS_MODELLING_APPLICATION_H_INCLUDED  defined
