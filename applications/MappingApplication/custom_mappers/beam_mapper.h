//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Philipp Bucher, Jordi Cotela
//
// See Master-Thesis P.Bucher
// "Development and Implementation of a Parallel
//  Framework for Non-Matching Grid Mapping"

#if !defined(KRATOS_BEAM_MAPPER_H_INCLUDED )
#define  KRATOS_BEAM_MAPPER_H_INCLUDED

// System includes
//#include <tuple>

// External includes

// Project includes
#include "mapper.h"
#include "custom_searching/interface_communicator.h"
#include "custom_utilities/interface_vector_container.h"
#include "custom_utilities/mapper_flags.h"
#include "custom_utilities/mapper_local_system.h"

#include "custom_utilities/projection_utilities.h"
#include "utilities/geometrical_projection_utilities.h"

namespace Kratos
{
///@name Kratos Classes
///@{

// ***************************** BeamMapperInterfaceInfo
class BeamMapperInterfaceInfo : public MapperInterfaceInfo
{
public:

    /// Default constructor.
    explicit BeamMapperInterfaceInfo(const double LocalCoordTol=0.0) : mLocalCoordTol(LocalCoordTol) {}

    explicit BeamMapperInterfaceInfo(const CoordinatesArrayType& rCoordinates,
                                 const IndexType SourceLocalSystemIndex,
                                 const IndexType SourceRank,
                                 const double LocalCoordTol=0.0)
        : MapperInterfaceInfo(rCoordinates, SourceLocalSystemIndex, SourceRank), mLocalCoordTol(LocalCoordTol) {}

    MapperInterfaceInfo::Pointer Create() const override
    {
        return Kratos::make_shared<BeamMapperInterfaceInfo>();
    }

    MapperInterfaceInfo::Pointer Create(const CoordinatesArrayType& rCoordinates,
                                        const IndexType SourceLocalSystemIndex,
                                        const IndexType SourceRank) const override
    {
        return Kratos::make_shared<BeamMapperInterfaceInfo>(
            rCoordinates,
            SourceLocalSystemIndex,
            SourceRank,
            mLocalCoordTol);
    }

    InterfaceObject::ConstructionType GetInterfaceObjectType() const override
    {
        return InterfaceObject::ConstructionType::Geometry_Center;
    }

    void ProcessSearchResult(const InterfaceObject& rInterfaceObject,
                             const double NeighborDistance) override;
    
    void ProcessSearchResultForApproximation(const InterfaceObject& rInterfaceObject,
                                             const double NeighborDistance) override;

    void GetValue(std::vector<int>& rValue,
                  const InfoType ValueType) const override
    {
        rValue = mNodeIds;
    }

    void GetValue(std::vector<double>& rValue,
                  const InfoType ValueType) const override
    {
        std::vector<double> vector = mLinearShapeFunctionValues;
        vector.insert(vector.end(), mHermitianShapeFunctionValues.begin(), mHermitianShapeFunctionValues.end());
        vector.insert(vector.end(), mHermitianShapeFunctionValuesDerivatives.begin(), mHermitianShapeFunctionValuesDerivatives.end());

        rValue = vector;
    }

    void GetValue(int& rValue,
                  const InfoType ValueType) const override
    {   
        //rValue = mClosestProjectionDistance;
        rValue = (int)mPairingIndex;
    }

    void GetValue(double& rValue,
                  const InfoType ValueType) const override
    {   std::cout << "Geting closest projection Distance" << std::endl;
        //rValue = (int)mPairingIndex;
        rValue = mClosestProjectionDistance;
    }

private:

    std::vector<int> mNodeIds;

    Point mCoordinates;

    std::vector<double> mLinearShapeFunctionValues;
    std::vector<double> mHermitianShapeFunctionValues;
    std::vector<double> mHermitianShapeFunctionValuesDerivatives;

    Point mProjectionOfPoint; // Point that results form the projection of the surface node on the beam

    double mClosestProjectionDistance = std::numeric_limits<double>::max(); // Distance between the surface node and the beam
    ProjectionUtilities::PairingIndex mPairingIndex = ProjectionUtilities::PairingIndex::Unspecified;
    double mLocalCoordTol; // this is not needed after searching, hence no need to serialize it // Do I really need this?

    void SaveSearchResult(const InterfaceObject& rInterfaceObject,
                          const bool ComputeApproximation);

    friend class Serializer;

    void save(Serializer& rSerializer) const override
    {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS( rSerializer, MapperInterfaceInfo );
        rSerializer.save("NodeIds", mNodeIds);
        rSerializer.save("LSFValues", mLinearShapeFunctionValues);
        rSerializer.save("HSFValues", mHermitianShapeFunctionValues);
        rSerializer.save("HSFDValues", mHermitianShapeFunctionValuesDerivatives);
        rSerializer.save("ClosestProjectionDistance", mClosestProjectionDistance);
        rSerializer.save("PairingIndex", (int)mPairingIndex);
    }

    void load(Serializer& rSerializer) override
    {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS( rSerializer, MapperInterfaceInfo );
        rSerializer.load("NodeIds", mNodeIds);
        rSerializer.load("LSFValues", mLinearShapeFunctionValues);
        rSerializer.load("HSFValues", mHermitianShapeFunctionValues);
        rSerializer.load("HSFDValues", mHermitianShapeFunctionValuesDerivatives);
        rSerializer.load("ClosestProjectionDistance", mClosestProjectionDistance);
        int temp;
        rSerializer.load("PairingIndex", temp);
        mPairingIndex = (ProjectionUtilities::PairingIndex)temp;
    }
};

// ***************************** BeamMapperLocalSystem
class BeamMapperLocalSystem : public MapperLocalSystem
{
public:

    explicit BeamMapperLocalSystem(NodePointerType pNode) : mpNode(pNode) {}

    void CalculateAll(MatrixType& rLocalMappingMatrix,
                      EquationIdVectorType& rOriginIds,
                      EquationIdVectorType& rDestinationIds,
                      MapperLocalSystem::PairingStatus& rPairingStatus) const override;

    CoordinatesArrayType& Coordinates() const override
    {
        KRATOS_DEBUG_ERROR_IF_NOT(mpNode) << "Members are not intitialized!" << std::endl;
        return mpNode->Coordinates();
    }

    /// Turn back information as a string.
    std::string PairingInfo(const int EchoLevel) const override;

private:
    NodePointerType mpNode;
    mutable ProjectionUtilities::PairingIndex mPairingIndex = ProjectionUtilities::PairingIndex::Unspecified;
    double mTheta;
};

/// Beam Mapper
/** This class implements the Beam Mapper technique.
* Each node on the destination side gets assigned is's closest neighbor on the other side of the interface.
* In the mapping phase every node gets assigned the value of it's neighbor
* For information abt the available echo_levels and the JSON default-parameters
* look into the class description of the MapperCommunicator
*/
template<class TSparseSpace, class TDenseSpace>
class BeamMapper : public Mapper<TSparseSpace, TDenseSpace>
{
public:

    ///@name Type Definitions
    ///@{

    ///@}
    ///@name Pointer Definitions
    /// Pointer definition of BeamMapper
    KRATOS_CLASS_POINTER_DEFINITION(BeamMapper);

    typedef Mapper<TSparseSpace, TDenseSpace> BaseType;

    typedef Kratos::unique_ptr<InterfaceCommunicator> InterfaceCommunicatorPointerType;
    typedef typename InterfaceCommunicator::MapperInterfaceInfoUniquePointerType MapperInterfaceInfoUniquePointerType;

    typedef InterfaceVectorContainer<TSparseSpace, TDenseSpace> InterfaceVectorContainerType;
    typedef Kratos::unique_ptr<InterfaceVectorContainerType> InterfaceVectorContainerPointerType;

    typedef Kratos::unique_ptr<MapperLocalSystem> MapperLocalSystemPointer;
    typedef std::vector<MapperLocalSystemPointer> MapperLocalSystemPointerVector;

    typedef typename BaseType::MapperUniquePointerType MapperUniquePointerType;
    typedef typename BaseType::TMappingMatrixType TMappingMatrixType;
    typedef Kratos::unique_ptr<TMappingMatrixType> TMappingMatrixUniquePointerType;

    // my typedefs
    typedef std::vector<TMappingMatrixType> RotationMatrixVector;

    typedef VariableComponent< VectorComponentAdaptor<array_1d<double, 3> > > ComponentVariableType;
    
    ///@}
    ///@name Life Cycle
    ///@{

    // Default constructor, needed for registration
    BeamMapper(ModelPart& rModelPartOrigin,
               ModelPart& rModelPartDestination):
               mrModelPartOrigin(rModelPartOrigin),
               mrModelPartDestination(rModelPartDestination) {}

    BeamMapper(ModelPart& rModelPartOrigin,
               ModelPart& rModelPartDestination,
               Parameters JsonParameters):
               mrModelPartOrigin(rModelPartOrigin),
               mrModelPartDestination(rModelPartDestination),
               mMapperSettings(JsonParameters)
                          
    {
        mpInterfaceVectorContainerOrigin = Kratos::make_unique<InterfaceVectorContainerType>(rModelPartOrigin);
        mpInterfaceVectorContainerDestination = Kratos::make_unique<InterfaceVectorContainerType>(rModelPartDestination);

        ValidateInput();
        
        // Not so sure for what it is, but I'll leave it here
        mLocalCoordTol = JsonParameters["local_coord_tolerance"].GetDouble();
        KRATOS_ERROR_IF(mLocalCoordTol < 0.0) << "The local_coord_tolerance cannot be negative" << std::endl;
        
        //  In this function the search task is done, and the parameters for the local systems are stored
        // MapperInterfaceInfo has:
        // 1. A beam id to relate
        // 2. a t_B_P
        // 3. linear and hermitean interpolation values
        // 4. its own position in GCS space

        Initialize();
    }

    /// Destructor.
    ~BeamMapper() override = default;

    ///@}
    ///@name Operations
    ///@{

    void UpdateInterface(Kratos::Flags MappingOptions, double SearchRadius) override
    {
        KRATOS_ERROR << "Implement me UpdateInterface" << std::endl;
    }

    void Map( const Variable< array_1d<double, 3> >& rOriginVariablesDisplacements, 
              const Variable< array_1d<double, 3> >& rOriginVariablesRotations,
              const Variable< array_1d<double, 3> >& rDestinationVariable, 
              Kratos::Flags MappingOptions)
    {
        InitializeInformationBeams(); // Calculates the rotation matrices of the beam elements
        MapInternal( rOriginVariablesDisplacements, rOriginVariablesRotations, rDestinationVariable, MappingOptions );

    }

    void Map( const Variable<double>& rOriginVariable, const Variable<double>& rDestinationVariable,
              Kratos::Flags MappingOptions) override
    {
        KRATOS_ERROR << "This function is not supported for the Beam-Mapper!" << std::endl;
    }

    void Map( const Variable< array_1d<double, 3> >& rOriginVariable, const Variable< array_1d<double, 3> >& rDestinationVariable,
              Kratos::Flags MappingOptions) override
    {
        KRATOS_ERROR << "This function is not supported for the Beam-Mapper!" << std::endl;
    }

    void InverseMap( const Variable<double>& rOriginVariable, const Variable<double>& rDestinationVariable, 
                     Kratos::Flags MappingOptions) override
    {
        KRATOS_ERROR << "This function is not supported for the Beam-Mapper!" << std::endl;
    }

    void InverseMap( const Variable< array_1d<double, 3> >& rOriginVariable, const Variable< array_1d<double, 3> >& rDestinationVariable,
                     Kratos::Flags MappingOptions) override
    {
        KRATOS_ERROR << "This function is not supported for the Beam-Mapper!" << std::endl;
    }

    void InverseMap( const Variable< array_1d<double, 3> >& rOriginVariablesDisplacements, 
                     const Variable< array_1d<double, 3> >& rOriginVariablesRotations,
                     const Variable< array_1d<double, 3> >& rDestinationVariable,
                     Kratos::Flags MappingOptions)
    {
        KRATOS_ERROR << "Implement Me! (InverseMap)" << std::endl;
    }

    MapperUniquePointerType Clone(ModelPart& rModelPartOrigin,
                                  ModelPart& rModelPartDestination,
                                  Parameters JsonParameters) const override
    {
        return Kratos::make_unique<BeamMapper<TSparseSpace, TDenseSpace>>(
            rModelPartOrigin,
            rModelPartDestination,
            JsonParameters);
    }

    ///@}
    ///@name Access
    ///@{

    TMappingMatrixType* pGetMappingMatrix() override
    {
        KRATOS_ERROR << "This function is not supported by beam-mapper" << std::endl;
        return mpMappingMatrix.get();
    }


    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        return "BeamMapper";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << "BeamMapper";
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
        BaseType::PrintData(rOStream);
    }

    void ValidateInput();

    void Initialize();

    void InitializeInformationBeams();

private:
    ///@name Member Variables
    ///@{
    ModelPart& mrModelPartOrigin;
    ModelPart& mrModelPartDestination;

    Parameters mMapperSettings;

    MapperLocalSystemPointerVector mMapperLocalSystems;

    InterfaceCommunicatorPointerType mpIntefaceCommunicator;

    RotationMatrixVector mRotationMatrixOfBeams;

    double mLocalCoordTol;

    TMappingMatrixUniquePointerType mpMappingMatrix;

    InterfaceVectorContainerPointerType mpInterfaceVectorContainerOrigin;
    InterfaceVectorContainerPointerType mpInterfaceVectorContainerDestination;

    ///@name Private Operations
    ///@{

    void InitializeInterfaceCommunicator();

    void InitializeInterface(Kratos::Flags MappingOptions = Kratos::Flags());

    void BuildMappingMatrix(Kratos::Flags MappingOptions = Kratos::Flags());

    void CreateMapperLocalSystems(
        const Communicator& rModelPartCommunicator,
        std::vector<Kratos::unique_ptr<MapperLocalSystem>>& rLocalSystems)
    {
        MapperUtilities::CreateMapperLocalSystemsFromNodes<BeamMapperLocalSystem>(
            rModelPartCommunicator,
            rLocalSystems);
    }

    void MapInternal(const Variable< array_1d<double, 3> >& rOriginVariablesDisplacements,
                     const Variable< array_1d<double, 3> >& rOriginVariablesRotations,
                     const Variable< array_1d<double, 3> >& rDestinationVariable,
                     Kratos::Flags MappingOptions)
    {
       std::vector< array_1d<double, 3> > element_displacements_vector;
       std::vector< array_1d<double, 3> > element_rotations_vector;

       RotateOriginVariablesLinear(rOriginVariablesDisplacements, rOriginVariablesRotations, element_displacements_vector, element_rotations_vector); 

    }

    void RotateOriginVariablesLinear(const Variable< array_1d<double, 3> >& rOriginVariablesDisplacements,
                                     const Variable< array_1d<double, 3> >& rOriginVariablesRotations,
                                     std::vector< array_1d<double, 3>>& displacements_vector,
                                     std::vector< array_1d<double, 3>>& rotations_vector )
    {
        array_1d<double, 3> displacementNode1;
        array_1d<double, 3> displacementNode2;
        array_1d<double, 3> rotationNode1;
        array_1d<double, 3> rotationNode2;

        const std::vector<std::string> var_comps{"_X", "_Y", "_Z"};

        const std::size_t num_elements = mrModelPartOrigin.GetCommunicator().LocalMesh().NumberOfElements();
        const auto elements_begin = mrModelPartOrigin.GetCommunicator().LocalMesh().Elements().ptr_begin();

        for (std::size_t i = 0; i < num_elements; ++i)
        {   
            auto it_element = elements_begin + i;
            auto it_nodes_line = (*it_element)->GetGeometry();
            
            size_t k = 0;
            for (const auto& var_ext : var_comps)
            {
                const auto& var_origin_disp = KratosComponents<ComponentVariableType>::Get(rOriginVariablesDisplacements.Name() + var_ext);

                displacementNode1[k] = it_nodes_line[0].FastGetSolutionStepValue(var_origin_disp);
                displacementNode2[k] = it_nodes_line[1].FastGetSolutionStepValue(var_origin_disp);
                k++;
            }
            std::cout << "displacement of node 1 is" << displacementNode1 << std::endl;
            std::cout << "displacement of node 2 is" << displacementNode2 << std::endl; 
        }
    }

    MapperInterfaceInfoUniquePointerType GetMapperInterfaceInfo() const 
    {                                                                     
        return Kratos::make_unique<BeamMapperInterfaceInfo>();
    }

    Parameters GetMapperDefaultSettings() const 
    {
        return Parameters( R"({
            "search_radius"            : -1.0,
            "search_iterations"        : 3,
            "local_coord_tolerance"    : 0.25,
            "echo_level"               : 0
        })");
    }

    ///@}

}; // Class BeamMapper

///@} addtogroup block
}  // namespace Kratos.

#endif // KRATOS_BEAM_MAPPER_H_INCLUDED  defined