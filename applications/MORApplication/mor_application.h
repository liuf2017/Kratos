//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    @{KRATOS_APP_AUTHOR}
//


#if !defined(KRATOS_MOR_APPLICATION_H_INCLUDED )
#define  KRATOS_MOR_APPLICATION_H_INCLUDED


// System includes
#include <string>
#include <iostream>

// External includes


// Project includes
#include "includes/define.h"
#include "includes/kratos_application.h"

/* ELEMENTS */

/* Adding acoustic element */
#include "custom_elements/acoustic_element.h"

/* CONSTITUTIVE LAWS */
#include "custom_constitutive/acoustic_material.h"

/* CONDITIONS */
#include "custom_conditions/base_displacement_condition.h"
#include "custom_conditions/point_displacement_condition.h"

namespace Kratos {

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
 * @class KratosMORApplication
 * @ingroup MORApplication
 * @brief This application features Elements for acoustic problems
 * @author RR
 */
class KratosMORApplication : public KratosApplication {
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of KratosMORApplication
    KRATOS_CLASS_POINTER_DEFINITION(KratosMORApplication);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    KratosMORApplication();

    /// Destructor.
    ~KratosMORApplication() override {}

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
        return "KratosMORApplication";
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
          KRATOS_WATCH(KratosComponents<VariableData>::GetComponents().size() );

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

    const AcousticElement mAcousticElement2D3N;
    const AcousticElement mAcousticElement2D4N;
    const AcousticElement mAcousticElement3D4N;
    const AcousticElement mAcousticElement3D8N;
     
    // const Elem3D   mElem3D;


    /* CONSTITUTIVE LAWS */
    const AcousticMaterial mAcousticMaterial;


    /* CONDITIONS*/
    // Point load
    const PointDisplacementCondition mPointDisplacementCondition3D1N;

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
    KratosMORApplication& operator=(KratosMORApplication const& rOther);

    /// Copy constructor.
    KratosMORApplication(KratosMORApplication const& rOther);


    ///@}

}; // Class KratosMORApplication

///@}


///@name Type Definitions
///@{


///@}
///@name Input and output
///@{

///@}


}  // namespace Kratos.

#endif // KRATOS_MOR_APPLICATION_H_INCLUDED  defined
