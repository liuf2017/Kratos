// KRATOS  __  __ _____ ____  _   _ ___ _   _  ____
//        |  \/  | ____/ ___|| | | |_ _| \ | |/ ___|
//        | |\/| |  _| \___ \| |_| || ||  \| | |  _
//        | |  | | |___ ___) |  _  || || |\  | |_| |
//        |_|  |_|_____|____/|_| |_|___|_| \_|\____| APPLICATION
//
//  License:		 BSD License
//                       license: MeshingApplication/license.txt
//
//  Main authors:    Vicente Mataix Ferrandiz
//

#if !defined(KRATOS_MMG_UTILITIES)
#define KRATOS_MMG_UTILITIES

// System includes

// External includes

// Project includes
#include "meshing_application.h"
#include "includes/key_hash.h"
#include "includes/model_part.h"
#include "includes/kratos_parameters.h"

// NOTE: The following contains the license of the MMG library
/* =============================================================================
**  Copyright (c) Bx INP/Inria/UBordeaux/UPMC, 2004- .
**
**  mmg is free software: you can redistribute it and/or modify it
**  under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  mmg is distributed in the hope that it will be useful, but WITHOUT
**  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
**  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
**  License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License and of the GNU General Public License along with mmg (in
**  files COPYING.LESSER and COPYING). If not, see
**  <http://www.gnu.org/licenses/>. Please read their terms carefully and
**  use this copy of the mmg distribution only if you accept them.
** =============================================================================
*/

namespace Kratos
{
///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

    /// Index definition
    typedef std::size_t                                               IndexType;

    /// Size definition
    typedef std::size_t                                                SizeType;

    /// Index vector
    typedef std::vector<IndexType>                              IndexVectorType;

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
 * @struct MMGMeshInfo
 * @ingroup MeshingApplication
 * @brief This struct stores the Mmg mesh information
 * @author Vicente Mataix Ferrandiz
 */
template<MMGLibrary TMMGLibrary>
struct MMGMeshInfo
{
    // Info stored
    SizeType NumberOfNodes;
    SizeType NumberOfLines;
    SizeType NumberOfTriangles;
    SizeType NumberOfQuadrilaterals;
    SizeType NumberOfPrism;
    SizeType NumberOfTetrahedra;

    /**
     * @brief It returns the number of the first type of conditions
     */
    SizeType NumberFirstTypeConditions();

    /**
     * @brief It returns the number of the second type of conditions
     */
    SizeType NumberSecondTypeConditions();

    /**
     * @brief It returns the number of the first type of elements
     */
    SizeType NumberFirstTypeElements();

    /**
     * @brief It returns the number of the second type of elements
     */
    SizeType NumberSecondTypeElements();
};

/**
 * @class MmgUtilities
 * @ingroup MeshingApplication
 * @brief This class are different utilities that uses the MMG library
 * @details This class are different utilities that uses the MMG library
 * @author Vicente Mataix Ferrandiz
 */
template<MMGLibrary TMMGLibrary>
class KRATOS_API(MESHING_APPLICATION) MmgUtilities
{
public:

    ///@name Type Definitions
    ///@{

    /// Pointer definition of MmgUtilities
    KRATOS_CLASS_POINTER_DEFINITION(MmgUtilities);

    /// Node definition
    typedef Node <3>                                                   NodeType;
    // Geometry definition
    typedef Geometry<NodeType>                                     GeometryType;

    /// Conditions array size
    static constexpr SizeType Dimension = (TMMGLibrary == MMGLibrary::MMG2D) ? 2 : 3;

    /// The type of array considered for the tensor
    typedef typename std::conditional<Dimension == 2, array_1d<double, 3>, array_1d<double, 6>>::type TensorArrayType;

    /// Double vector
    typedef std::vector<double> DoubleVectorType;

    /// Double vector map
    typedef std::unordered_map<DoubleVectorType, IndexType, KeyHasherRange<DoubleVectorType>, KeyComparorRange<DoubleVectorType> > DoubleVectorMapType;

    /// Index vector map
    typedef std::unordered_map<IndexVectorType, IndexType, KeyHasherRange<IndexVectorType>, KeyComparorRange<IndexVectorType> > IndexVectorMapType;

    ///@}
    ///@name  Enum's
    ///@{

    ///@}
    ///@name Life Cycle
    ///@{

    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Input and output
    ///@{

    ///@}
    ///@name Friends
    ///@{

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    /**
     * @brief This method sets the echo level
     * @param EchoLevel Sets the echo level
     */
    void SetEchoLevel(const SizeType EchoLevel);

    /**
     * @brief It prints info about the current mesh
     * @param rMMGMeshInfo The number of nodes, conditions and elements
     */
    void PrintAndGetMmgMeshInfo(MMGMeshInfo<TMMGLibrary>& rMMGMeshInfo);

    /**
     * @brief It checks if the nodes are repeated and remove the repeated ones
     * @param rModelPart The model part of interest to study
     */
    IndexVectorType FindDuplicateNodeIds(ModelPart& rModelPart);

    /**
     * @brief It checks if the conditions are repeated and returns a list of indices of the nodes
     */
    IndexVectorType CheckFirstTypeConditions();

    /**
     * @brief It checks if the conditions are repeated and remove the repeated ones
     */
    IndexVectorType CheckSecondTypeConditions();

    /**
     * @brief It checks if the elemenst are removed and remove the repeated ones
     */
    IndexVectorType CheckFirstTypeElements();

    /**
     * @brief It checks if the elemenst are removed and remove the repeated ones
     */
    IndexVectorType CheckSecondTypeElements();

    /**
     * @brief It blocks certain nodes before remesh the model
     * @param iNode The index of the node
     */
    void BlockNode(const IndexType iNode);

    /**
     * @brief It blocks certain conditions before remesh the model
     * @param iCondition The index of the condition
     */
    void BlockCondition(const IndexType iCondition);

    /**
     * @brief It blocks certain elements before remesh the model
     * @param iElement The index of the element
     */
    void BlockElement(const IndexType iElement);

    /**
     * @brief It creates the new node
     * @param rModelPart The model part of interest to study
     * @param iNode The index of the new noode
     * @param Ref The submodelpart id
     * @param IsRequired MMG value (I don't know that it does)
     * @return pNode The pointer to the new node created
     */
    NodeType::Pointer CreateNode(
        ModelPart& rModelPart,
        IndexType iNode,
        int& Ref,
        int& IsRequired
        );

    /**
     * @brief It creates the new condition (first type, depends if the library work in 2D/3D/Surfaces)
     * @param rModelPart The model part of interest to study
     * @param rMapPointersRefCondition The pointer to the condition of reference
     * @param CondId The id of the condition
     * @param PropId The submodelpart id
     * @param IsRequired MMG value (I don't know that it does)
     * @param RemoveRegions Cuttig-out specified regions during surface remeshing
     * @param Discretization The discretization option
     * @return pCondition The pointer to the new condition created
     */
    Condition::Pointer CreateFirstTypeCondition(
        ModelPart& rModelPart,
        std::unordered_map<IndexType,Condition::Pointer>& rMapPointersRefCondition,
        const IndexType CondId,
        int& PropId,
        int& IsRequired,
        bool SkipCreation,
        const bool RemoveRegions = false,
        const DiscretizationOption Discretization = DiscretizationOption::STANDARD
        );

    /**
     * @brief It creates the new condition (second type, depends if the library work in 2D/3D/Surfaces)
     * @param rModelPart The model part of interest to study
     * @param rMapPointersRefCondition The pointer to the condition of reference
     * @param CondId The id of the condition
     * @param PropId The submodelpart id
     * @param IsRequired MMG value (I don't know that it does)
     * @param RemoveRegions Cuttig-out specified regions during surface remeshing
     * @param Discretization The discretization option
     * @return pCondition The pointer to the new condition created
     */
    Condition::Pointer CreateSecondTypeCondition(
        ModelPart& rModelPart,
        std::unordered_map<IndexType,Condition::Pointer>& rMapPointersRefCondition,
        const IndexType CondId,
        int& PropId,
        int& IsRequired,
        bool SkipCreation,
        const bool RemoveRegions = false,
        const DiscretizationOption Discretization = DiscretizationOption::STANDARD
        );

    /**
     * @brief It creates the new element (first type, depends if the library work in 2D/3D/Surfaces)
     * @param rModelPart The model part of interest to study
     * @param rMapPointersRefElement The pointer to the element of reference
     * @param ElemId The id of the element
     * @param PropId The submodelpart id
     * @param IsRequired MMG value (I don't know that it does)
     * @param RemoveRegions Cuttig-out specified regions during surface remeshing
     * @param Discretization The discretization option
     * @return pElement The pointer to the new condition created
     */
    Element::Pointer CreateFirstTypeElement(
        ModelPart& rModelPart,
        std::unordered_map<IndexType,Element::Pointer>& rMapPointersRefElement,
        const IndexType ElemId,
        int& PropId,
        int& IsRequired,
        bool SkipCreation,
        const bool RemoveRegions = false,
        const DiscretizationOption Discretization = DiscretizationOption::STANDARD
        );

    /**
     * @brief It creates the new element (second type, depends if the library work in 2D/3D/Surfaces)
     * @param rModelPart The model part of interest to study
     * @param rMapPointersRefElement The pointer to the element of reference
     * @param ElemId The id of the element
     * @param PropId The submodelpart id
     * @param IsRequired MMG value (I don't know that it does)
     * @param RemoveRegions Cuttig-out specified regions during surface remeshing
     * @param Discretization The discretization option
     * @return pElement The pointer to the new condition created
     */
    Element::Pointer CreateSecondTypeElement(
        ModelPart& rModelPart,
        std::unordered_map<IndexType,Element::Pointer>& rMapPointersRefElement,
        const IndexType ElemId,
        int& PropId,
        int& IsRequired,
        bool SkipCreation,
        const bool RemoveRegions = false,
        const DiscretizationOption Discretization = DiscretizationOption::STANDARD
        );

    /**
     * @brief Initialisation of mesh and sol structures
     * @details Initialisation of mesh and sol structures args of InitMesh:
     * -# MMG5_ARG_start we start to give the args of a variadic func
     * -# MMG5_ARG_ppMesh next arg will be a pointer over a MMG5_pMesh
     * -# &mmgMesh pointer toward your MMG5_pMesh (that store your mesh)
     * -# MMG5_ARG_ppMet next arg will be a pointer over a MMG5_pSol storing a metric
     * -# &mmgSol pointer toward your MMG5_pSol (that store your metric)
     * @param Discretization The discretization type
     */
    void InitMesh(DiscretizationOption Discretization = DiscretizationOption::STANDARD);

    /**
     * @brief Here the verbosity is set
     */
    void InitVerbosity();

    /**
     * @brief Here the verbosity is set using the API
     * @param VerbosityMMG The equivalent verbosity level in the MMG API
     */
    void InitVerbosityParameter(const IndexType VerbosityMMG);

    /**
     * @brief This sets the size of the mesh
     * @param rMMGMeshInfo The number of nodes, conditions and elements
     */
    void SetMeshSize(MMGMeshInfo<TMMGLibrary>& rMMGMeshInfo);

    /**
     * @brief This sets the size of the solution for the scalar case
     * @param NumNodes Number of nodes
     */
    void SetSolSizeScalar(const SizeType NumNodes);

    /**
     * @brief This sets the size of the solution for the vector case
     * @param NumNodes Number of nodes
     */
    void SetSolSizeVector(const SizeType NumNodes);

    /**
     * @brief This sets the size of the solution for the tensor case
     * @param NumNodes Number of nodes
     */
    void SetSolSizeTensor(const SizeType NumNodes);

    /**
     * @brief This checks the mesh data and prints if it is OK
     */
    void CheckMeshData();

    /**
     * @brief This sets the output mesh
     * @param rInputName The input name
     * @param PostOutput If the ouput file is the solution after take into account the metric or not
     * @param Step The step to postprocess
     */
    void InputMesh(const std::string& rInputName);

    /**
     * @brief This sets the output sol
     * @param rInputName The input name
     */
    void InputSol(const std::string& rInputName);

    /**
     * @brief This sets the output mesh
     * @param rOutputName The output name
     * @param PostOutput If the ouput file is the solution after take into account the metric or not
     * @param Step The step to postprocess
     */
    void OutputMesh(
        const std::string& rOutputName,
        const bool PostOutput,
        const IndexType Step
        );

    /**
     * @brief This sets the output sol
     * @param rOutputName The output name
     * @param PostOutput If the ouput file is the solution after take into account the metric or not
     * @param Step The step to postprocess
     */
    void OutputSol(
        const std::string& rOutputName,
        const bool PostOutput,
        const IndexType Step
        );

    /**
     * @brief This frees the MMG structures
     */
    void FreeAll();

    /**
     * @brief This loads the solution
     */
    void MMGLibCallMetric(Parameters ConfigurationParameters);

    /**
     * @brief This loads the solution
     */
    void MMGLibCallIsoSurface(Parameters ConfigurationParameters);

    /**
     * @brief This sets the nodes of the mesh
     * @param X Coordinate X
     * @param Y Coordinate Y
     * @param Z Coordinate Z
     * @param Color Reference of the node(submodelpart)
     * @param Index The index number of the node
     */
    void SetNodes(
        const double X,
        const double Y,
        const double Z,
        const IndexType Color,
        const IndexType Index
        );

    /**
     * @brief This sets the conditions of the mesh
     * @param rGeometry The geometry of the condition
     * @param Color Reference of the node(submodelpart)
     * @param Index The index number of the node
     */
    void SetConditions(
        GeometryType& rGeometry,
        const IndexType Color,
        const IndexType Index
        );

    /**
     * @brief This sets elements of the mesh
     * @param rGeometry The geometry of the element
     * @param Color Reference of the node(submodelpart)
     * @param Index The index number of the node
     */
    void SetElements(
        GeometryType& rGeometry,
        const IndexType Color,
        const IndexType Index
        );

    /**
     * @brief This function is used to compute the metric scalar
     * @param Metric The inverse of the size node
     * @param NodeId The id of the node
     */
    void SetMetricScalar(
        const double Metric,
        const IndexType NodeId
        );

    /**
     * @brief This function is used to compute the metric vector (x, y, z)
     * @param Metric This array contains the components of the metric vector
     * @param NodeId The id of the node
     */
    void SetMetricVector(
        const array_1d<double, Dimension>& Metric,
        const IndexType NodeId
        );

    /**
     * @brief This function is used to compute the Hessian metric tensor, note that when using the Hessian, more than one metric can be defined simultaneously, so in consecuence we need to define the elipsoid which defines the volume of maximal intersection
     * @param Metric This array contains the components of the metric tensor in the MMG defined order
     * @param NodeId The id of the node
     */
    void SetMetricTensor(
        const TensorArrayType& Metric,
        const IndexType NodeId
        );

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
    std::string Info() const // override
    {
        return "MmgUtilities";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const // override
    {
        rOStream << "MmgUtilities";
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const // override
    {
    }

protected:

    ///@name Protected Member Variables
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

    SizeType mEchoLevel = 0; /// The echo level of the utilities

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

//     /// Assignment operator.
//     MmgUtilities& operator=(MmgUtilities const& rOther);

//     /// Copy constructor.
//     MmgUtilities(MmgUtilities const& rOther);

    ///@}

};// class MmgUtilities
///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{

/// input stream function
template<MMGLibrary TMMGLibrary>
inline std::istream& operator >> (std::istream& rIStream,
                                  MmgUtilities<TMMGLibrary>& rThis);

/// output stream function
template<MMGLibrary TMMGLibrary>
inline std::ostream& operator << (std::ostream& rOStream,
                                  const MmgUtilities<TMMGLibrary>& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}

}// namespace Kratos.
#endif /* KRATOS_MMG_UTILITIES defined */
