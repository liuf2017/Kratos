//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//
//


#if !defined(KRATOS_MODELER_H_INCLUDED )
#define  KRATOS_MODELER_H_INCLUDED


// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "containers/model.h"
#include "spatial_containers/spatial_containers.h"


namespace Kratos
{

///@name Kratos Classes
///@{

/// Modeler to interact with ModelParts.
/* The modeler is designed to interact, create and update
   the ModelPart of the analyses after and at certain steps.
*/
class Modeler
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of Modeler
    KRATOS_CLASS_POINTER_DEFINITION(Modeler);

    typedef std::size_t SizeType;
    typedef std::size_t IndexType;

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    Modeler(
        Model& rModel,
        const Parameters ModelerParameters = Parameters())
        : mParameters(ModelerParameters)
    {}

    /// Destructor.
    virtual ~Modeler() = default;

    ///@}
    ///@name Modeler Stages at Initialize
    ///@{

    /// Import geometry models from external input.
    virtual void ImportGeometryModel()
    {}

    /// Prepare or update the geometry model_part.
    virtual void PrepareGeometryModel()
    {}

    /// Convert the geometry model to analysis suitable models.
    virtual void GenerateModelPart()
    {}

    /// Import the model_part from external input.
    virtual void ImportModelPart()
    {}

    /// Prepare the analysis model_part for the simulation.
    virtual void PrepareModelPart()
    {}

    ///@}
    ///@name Operators
    ///@{

    virtual void GenerateModelPart(ModelPart& rOriginModelPart, ModelPart& rDestinationModelPart, Element const& rReferenceElement, Condition const& rReferenceBoundaryCondition)
    {
        KRATOS_ERROR << "This modeler CAN NOT be used for mesh generation." << std::endl;
    }

    virtual void GenerateMesh(ModelPart& ThisModelPart, Element const& rReferenceElement, Condition const& rReferenceBoundaryCondition)
    {
        KRATOS_ERROR << "This modeler CAN NOT be used for mesh generation." << std::endl;
    }

    virtual void GenerateNodes(ModelPart& ThisModelPart)
    {
        KRATOS_ERROR << "This modeler CAN NOT be used for node generation." << std::endl;
    }

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    virtual std::string Info() const
    {
        return "Modeler";
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream& rOStream) const
    {
        rOStream << Info();
    }

    /// Print object's data.
    virtual void PrintData(std::ostream& rOStream) const
    {
    }

    ///@}

protected:
    ///@name Protected members
    ///@{

    const Parameters mParameters;

    ///@}

}; // Class Modeler

///@}
///@name Input and output
///@{

/// input stream function
inline std::istream& operator >> (std::istream& rIStream,
                                  Modeler& rThis);

/// output stream function
inline std::ostream& operator << (std::ostream& rOStream,
                                  const Modeler& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}
///@}

}  // namespace Kratos.

#endif // KRATOS_MODELER_H_INCLUDED  defined


