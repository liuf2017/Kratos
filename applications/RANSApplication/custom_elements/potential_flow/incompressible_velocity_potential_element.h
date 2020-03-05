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

#if !defined(KRATOS_INCOMPRESSIBLE_VELOCITY_POTENTIAL_ELEMENT_H_INCLUDED)
#define KRATOS_INCOMPRESSIBLE_VELOCITY_POTENTIAL_ELEMENT_H_INCLUDED

// System includes

// External includes

// Project includes
#include "custom_elements/potential_flow/laplace_element.h"
#include "includes/checks.h"
#include "includes/define.h"

// Application includes
#include "includes/variables.h"
#include "rans_application_variables.h"

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

template <unsigned int TDim, unsigned int TNumNodes>
class IncompressibleVelocityPotentialElement : public LaplaceElement<TDim, TNumNodes>
{
public:
    ///@name Type Definitions
    ///@{

    using BaseType = LaplaceElement<TDim, TNumNodes>;

    using NodeType = Node<3>;

    /// Geometry type (using with given NodeType)
    using GeometryType = Geometry<NodeType>;

    using PropertiesType = Properties;

    /// Definition of nodes container type, redefined from GeometryType
    using NodesArrayType = GeometryType::PointsArrayType;

    using ShapeFunctionDerivativesArrayType = GeometryType::ShapeFunctionsGradientsType;

    using IndexType = std::size_t;

    ///@}
    ///@name Pointer Definitions
    /// Pointer definition of IncompressibleVelocityPotentialElement
    KRATOS_CLASS_POINTER_DEFINITION(IncompressibleVelocityPotentialElement);

    ///@}
    ///@name Life Cycle
    ///@{

    /**
     * Constructor.
     */
    explicit IncompressibleVelocityPotentialElement(IndexType NewId = 0)
        : BaseType(NewId)
    {
    }

    /**
     * Constructor using an array of nodes
     */
    IncompressibleVelocityPotentialElement(IndexType NewId, const NodesArrayType& ThisNodes)
        : BaseType(NewId, ThisNodes)
    {
    }

    /**
     * Constructor using Geometry
     */
    IncompressibleVelocityPotentialElement(IndexType NewId, GeometryType::Pointer pGeometry)
        : BaseType(NewId, pGeometry)
    {
    }

    /**
     * Constructor using Properties
     */
    IncompressibleVelocityPotentialElement(IndexType NewId,
                                           GeometryType::Pointer pGeometry,
                                           PropertiesType::Pointer pProperties)
        : BaseType(NewId, pGeometry, pProperties)
    {
    }

    /**
     * Copy Constructor
     */
    IncompressibleVelocityPotentialElement(IncompressibleVelocityPotentialElement const& rOther)
        : BaseType(rOther)
    {
    }

    /**
     * Destructor
     */
    ~IncompressibleVelocityPotentialElement() override = default;

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    /**
     * ELEMENTS inherited from this class have to implement next
     * Create and Clone methods: MANDATORY
     */

    /**
     * creates a new element pointer
     * @param NewId: the ID of the new element
     * @param ThisNodes: the nodes of the new element
     * @param pProperties: the properties assigned to the new element
     * @return a Pointer to the new element
     */
    Element::Pointer Create(IndexType NewId,
                            NodesArrayType const& ThisNodes,
                            PropertiesType::Pointer pProperties) const override
    {
        KRATOS_TRY
        return Kratos::make_intrusive<IncompressibleVelocityPotentialElement>(
            NewId, Element::GetGeometry().Create(ThisNodes), pProperties);
        KRATOS_CATCH("");
    }

    /**
     * creates a new element pointer
     * @param NewId: the ID of the new element
     * @param pGeom: the geometry to be employed
     * @param pProperties: the properties assigned to the new element
     * @return a Pointer to the new element
     */
    Element::Pointer Create(IndexType NewId,
                            GeometryType::Pointer pGeom,
                            PropertiesType::Pointer pProperties) const override
    {
        KRATOS_TRY
        return Kratos::make_intrusive<IncompressibleVelocityPotentialElement>(
            NewId, pGeom, pProperties);
        KRATOS_CATCH("");
    }

    /**
     * creates a new element pointer and clones the previous element data
     * @param NewId: the ID of the new element
     * @param ThisNodes: the nodes of the new element
     * @param pProperties: the properties assigned to the new element
     * @return a Pointer to the new element
     */
    Element::Pointer Clone(IndexType NewId, NodesArrayType const& ThisNodes) const override
    {
        KRATOS_TRY
        return Kratos::make_intrusive<IncompressibleVelocityPotentialElement>(
            NewId, Element::GetGeometry().Create(ThisNodes), Element::pGetProperties());
        KRATOS_CATCH("");
    }

    const Variable<double> GetVariable() const override
    {
        return VELOCITY_POTENTIAL;
    }

    void GetValueOnIntegrationPoints(const Variable<array_1d<double, 3>>& rVariable,
                                     std::vector<array_1d<double, 3>>& rValues,
                                     const ProcessInfo& rCurrentProcessInfo) override
    {
        KRATOS_TRY
        if (rVariable == VELOCITY)
        {
            Vector gauss_weights;
            Matrix shape_functions;
            ShapeFunctionDerivativesArrayType shape_derivatives;
            this->CalculateGeometryData(gauss_weights, shape_functions, shape_derivatives);
            const IndexType num_gauss_points = gauss_weights.size();

            if (rValues.size() != num_gauss_points)
            {
                rValues.resize(num_gauss_points);
            }

            const GeometryType& r_geometry = this->GetGeometry();

            for (IndexType g = 0; g < num_gauss_points; ++g)
            {
                const Matrix& r_shape_derivatives = shape_derivatives[g];

                array_1d<double, 3> velocity;
                RansCalculationUtilities::CalculateGradient(
                    velocity, r_geometry, VELOCITY_POTENTIAL, r_shape_derivatives);
                rValues[g] = velocity;
            }
        }
        else
        {
            KRATOS_ERROR << "GetValueOnIntegrationPoints for variable "
                         << rVariable.Name() << " not defined for " << this->Info();
        }

        KRATOS_CATCH("");
    }

    int Check(const ProcessInfo& rCurrentProcessInfo) override
    {
        KRATOS_TRY

        int check = BaseType::Check(rCurrentProcessInfo);

        for (IndexType iNode = 0; iNode < this->GetGeometry().size(); ++iNode)
        {
            KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(VELOCITY, this->GetGeometry()[iNode]);
        }

        return check;

        KRATOS_CATCH("");
    }

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
        std::stringstream buffer;
        buffer << "IncompressibleVelocityPotentialElement #" << this->Id();
        return buffer.str();
    }
    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << "IncompressibleVelocityPotentialElement #" << this->Id();
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

    ///@}
    ///@name Member Variables
    ///@{

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    ///@}
    ///@name Serialization
    ///@{

    friend class Serializer;

    void save(Serializer& rSerializer) const override
    {
        KRATOS_TRY

        KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, Element);

        KRATOS_CATCH("");
    }
    void load(Serializer& rSerializer) override
    {
        KRATOS_TRY

        KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, Element);

        KRATOS_CATCH("");
    }

    ///@}
    ///@name Private  Access
    ///@{

    ///@}
    ///@name Private Inquiry
    ///@{

    ///@}
    ///@name Un accessible methods
    ///@{

    ///@}

}; // Class IncompressibleVelocityPotentialElement

///@}

///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

template <unsigned int TDim, unsigned int TNumNodes>
inline std::istream& operator>>(std::istream& rIStream,
                                IncompressibleVelocityPotentialElement<TDim, TNumNodes>& rThis);

/// output stream function
template <unsigned int TDim, unsigned int TNumNodes>
inline std::ostream& operator<<(std::ostream& rOStream,
                                const IncompressibleVelocityPotentialElement<TDim, TNumNodes>& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << " : " << std::endl;
    rThis.PrintData(rOStream);
    return rOStream;
}

///@}

} // namespace Kratos.

#endif // KRATOS_INCOMPRESSIBLE_VELOCITY_POTENTIAL_ELEMENT_H_INCLUDED defined
