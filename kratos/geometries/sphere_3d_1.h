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
//                   Janosch Stascheit
//                   Felix Nagel
//  contributors:    Hoang Giang Bui
//                   Josep Maria Carbonell
//

#if !defined(KRATOS_SPHERE_3D_1_H_INCLUDED )
#define  KRATOS_SPHERE_3D_1_H_INCLUDED

// System includes

// External includes

// Project includes
#include "geometries/geometry.h"
#include "integration/line_gauss_legendre_integration_points.h"


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

/**
*/
template<class TPointType>

class Sphere3D1 : public Geometry<TPointType>
{

public:
    ///@}
    ///@name Type Definitions
    ///@{

    /// Geometry as base class.
    typedef Geometry<TPointType> BaseType;

    /// Pointer definition of Sphere3D1
    KRATOS_CLASS_POINTER_DEFINITION( Sphere3D1 );

    /** Integration methods implemented in geometry.
    */
    typedef GeometryData::IntegrationMethod IntegrationMethod;

    /** A Vector of counted pointers to Geometries. Used for
    returning edges of the geometry.
     */
    typedef typename BaseType::GeometriesArrayType GeometriesArrayType;

    /** Redefinition of template parameter TPointType.
     */
    typedef TPointType PointType;

    /** Type used for indexing in geometry class.std::size_t used for indexing
    point or integration point access methods and also all other
    methods which need point or integration point index.
    */
    typedef typename BaseType::IndexType IndexType;


    /** This typed used to return size or dimension in
    geometry. Dimension, WorkingDimension, PointsNumber and
    ... return this type as their results.
    */
    typedef typename BaseType::SizeType SizeType;

    /** Array of counted pointers to point. This type used to hold
    geometry's points.
    */
    typedef typename BaseType::PointsArrayType PointsArrayType;

    /** This type used for representing an integration point in
    geometry. This integration point is a point with an
    additional weight component.
    */
    typedef typename BaseType::IntegrationPointType IntegrationPointType;

    /** A Vector of IntegrationPointType which used to hold
    integration points related to an integration
    method. IntegrationPoints functions used this type to return
    their results.
    */
    typedef typename BaseType::IntegrationPointsArrayType IntegrationPointsArrayType;

    /** A Vector of IntegrationPointsArrayType which used to hold
    integration points related to different integration method
    implemented in geometry.
    */
    typedef typename BaseType::IntegrationPointsContainerType IntegrationPointsContainerType;

    /** A third order tensor used as shape functions' values
    continer.
    */
    typedef typename BaseType::ShapeFunctionsValuesContainerType ShapeFunctionsValuesContainerType;

    /** A fourth order tensor used as shape functions' local
    gradients container in geometry.
    */
    typedef typename BaseType::ShapeFunctionsLocalGradientsContainerType ShapeFunctionsLocalGradientsContainerType;

    /** A third order tensor to hold jacobian matrices evaluated at
    integration points. Jacobian and InverseOfJacobian functions
    return this type as their result.
    */
    typedef typename BaseType::JacobiansType JacobiansType;

    /** A third order tensor to hold shape functions' local
    gradients. ShapefunctionsLocalGradients function return this
    type as its result.
    */
    typedef typename BaseType::ShapeFunctionsGradientsType ShapeFunctionsGradientsType;

    /** Type of the normal vector used for normal to edges in geomety.
     */
    typedef typename BaseType::NormalType NormalType;

    /**
     * Type of coordinates array
     */
    typedef typename BaseType::CoordinatesArrayType CoordinatesArrayType;

    ///@}
    ///@name Life Cycle
    ///@{

//     Sphere3D1( const PointType& FirstPoint)
//         : BaseType( PointsArrayType(), &msGeometryData )
//     {
//         BaseType::Points().push_back( typename PointType::Pointer( new PointType( FirstPoint ) ) );
//     }

    Sphere3D1( typename PointType::Pointer pFirstPoint )
        : BaseType( PointsArrayType(), &msGeometryData ), mpRadius(NULL)
    {
        BaseType::Points().push_back( pFirstPoint );
    }

    Sphere3D1( const PointsArrayType& ThisPoints )
        : BaseType( ThisPoints, &msGeometryData ) , mpRadius(NULL)
    {
        if ( BaseType::PointsNumber() != 1 )
            KRATOS_ERROR << "Invalid points number. Expected 1, given " << BaseType::PointsNumber() << std::endl;
    }

    /** Copy constructor.
    Construct this geometry as a copy of given geometry.

    @note This copy constructor don't copy the points and new
    geometry shares points with given source geometry. It's
    obvious that any change to this new geometry's point affect
    source geometry's points too.
    */
    Sphere3D1( Sphere3D1 const& rOther )
        : BaseType( rOther )
    {}


    /** Copy constructor from a geometry with other point type.
    Construct this geometry as a copy of given geometry which
    has different type of points. The given geometry's
    TOtherPointType* must be implicitly convertible to this
    geometry PointType.

    @note This copy constructor don't copy the points and new
    geometry shares points with given source geometry. It's
    obvious that any change to this new geometry's point affect
    source geometry's points too.
    */
    template<class TOtherPointType> Sphere3D1( Sphere3D1<TOtherPointType> const& rOther )
        : BaseType( rOther )
    {}

    /// Destructor. Do nothing!!!
    ~Sphere3D1() override {}

    GeometryData::KratosGeometryFamily GetGeometryFamily() const override
    {
        return GeometryData::Kratos_Point;
    }

    GeometryData::KratosGeometryType GetGeometryType() const override
    {
        return GeometryData::Kratos_Sphere3D1;
    }

    ///@}
    ///@name Operators
    ///@{

    /** Assignment operator.

    @note This operator don't copy the points and this
    geometry shares points with given source geometry. It's
    obvious that any change to this geometry's point affect
    source geometry's points too. However the radius will need
    to be set separately.

    @see Clone
    @see ClonePoints
    */
    Sphere3D1& operator=( const Sphere3D1& rOther )
    {
        BaseType::operator=( rOther );

        return *this;
    }

    /** Assignment operator for geometries with different point type.

    @note This operator don't copy the points and this
    geometry shares points with given source geometry. It's
    obvious that any change to this geometry's point affect
    source geometry's points too. However the radius will need
    to be set separately.

    @see Clone
    @see ClonePoints
    */
    template<class TOtherPointType>
    Sphere3D1& operator=( Sphere3D1<TOtherPointType> const & rOther )
    {
        BaseType::operator=( rOther );

        return *this;
    }

    ///@}
    ///@name Operations
    ///@{

    typename BaseType::Pointer Create( PointsArrayType const& ThisPoints ) const override
    {
        return typename BaseType::Pointer( new Sphere3D1( ThisPoints ) );
    }

    // Geometry< Point<3> >::Pointer Clone() const override
    // {
    //     Geometry< Point<3> >::PointsArrayType NewPoints;

    //     //making a copy of the nodes TO POINTS (not Nodes!!!)
    //     for ( IndexType i = 0 ; i < this->size() ; i++ )
    //     {
    //             NewPoints.push_back(Kratos::make_shared< Point<3> >(( *this )[i]));
    //     }

    //     //creating a geometry with the new points
    //     Geometry< Point<3> >::Pointer p_clone( new Sphere3D1< Point<3> >( NewPoints ) );

    //     return p_clone;
    // }

    /**
     * @brief Lumping factors for the calculation of the lumped mass matrix
     * @param rResult Vector containing the lumping factors
     * @param LumpingMethod The lumping method considered. The three methods available are:
     *      - The row sum method
     *      - Diagonal scaling
     *      - Evaluation of M using a quadrature involving only the nodal points and thus automatically yielding a diagonal matrix for standard element shape function
     */
    Vector& LumpingFactors(
        Vector& rResult,
        const typename BaseType::LumpingMethods LumpingMethod = BaseType::LumpingMethods::ROW_SUM
        )  const override
    {
	if(rResult.size() != 1)
           rResult.resize( 1, false );
        rResult[0] = 1.0;
        return rResult;
    }

    ///@}
    ///@name Informations
    ///@{

    /** This method calculate and return Length or charactereistic
    length of this geometry depending to it's dimension. For one
    dimensional geometry for example Line it returns length of it
    and for the other geometries it gives Characteristic length
    otherwise.

    @return double value contains length or Characteristic
    length
    @see Area()
    @see Volume()
    @see DomainSize()
    */
    double Length() const override
    {
        KRATOS_WARNING("Sphere3D1") << "This method (Length) has no meaning for this type of geometry (Sphere)."<<std::endl;

        return 0.0;
    }

    /** This method calculate and return area or surface area of
    this geometry depending to it's dimension. For one dimensional
    geometry it returns zero, for two dimensional it gives area
    and for three dimensional geometries it gives surface area.

    @return double value contains area or surface
    area.
    @see Length()
    @see Volume()
    @see DomainSize()
    */
    double Area() const override
    {
        return 4.0 * Globals::Pi * std::pow(GetRadius(), 2);
    }

    double Volume() const override
    {
        return 4.0/3.0 * Globals::Pi * std::pow(GetRadius(), 3);
    }

    /** This method calculate and return length, area or volume of
    this geometry depending to it's dimension. For one dimensional
    geometry it returns its length, for two dimensional it gives area
    and for three dimensional geometries it gives its volume.

    @return double value contains length, area or volume.
    @see Length()
    @see Area()
    @see Volume()
    */
    double DomainSize() const override
    {
        return Volume();
    }

//      virtual void Bounding_Box(BoundingBox<TPointType, BaseType>& rResult) const
//              {
//                 //rResult.Geometry() = *(this);
//                 BaseType::Bounding_Box(rResult.LowPoint(), rResult.HighPoint());
//              }

    ///@}
    ///@name Jacobian
    ///@{

    /**
     * Check if an axis-aliged bounding box (AABB) intersects a triangle
     *
     * Based on an answer by Ben Voigt : https://stackoverflow.com/questions/4578967/cube-sphere-intersection-test
     *
     * @return bool if the sphere overlaps over a box that is aligned with the global coordinate axes.
     * @param rLowPoint first corner of the box
     * @param rHighPoint second corner of the box
     */
    bool HasIntersection( const Point& rLowPoint, const Point& rHighPoint ) override
    {
        const array_1d<double, 3>& center = this->operator[](0).Coordinates();
        double radius_squared = std::pow(this->GetRadius(), 2);

        if (center[0] < rLowPoint[0]){
            radius_squared -= std::pow(center[0] - rLowPoint[0], 2);
        }

        else if (center[0] > rHighPoint[0]){
            radius_squared -= std::pow(center[0] - rHighPoint[0], 2);
        }

        if (center[1] < rLowPoint[1]){
            radius_squared -= std::pow(center[1] - rLowPoint[1], 2);
        }

        else if (center[1] > rHighPoint[1]){
            radius_squared -= std::pow(center[1] - rHighPoint[1], 2);
        }

        if (center[2] < rLowPoint[2]){
            radius_squared -= std::pow(center[2] - rLowPoint[2], 2);
        }

        else if (center[2] > rHighPoint[2]){
            radius_squared -= std::pow(center[2] - rHighPoint[2], 2);
        }

        return radius_squared > 0;
    }

    /** Jacobians for given  method. This method
    calculate jacobians matrices in all integrations points of
    given integration method.

    @param ThisMethod integration method which jacobians has to
    be calculated in its integration points.

    @return JacobiansType a Vector of jacobian
    matrices \f$ J_i \f$ where \f$ i=1,2,...,n \f$ is the integration
    point index of given integration method.

    @see DeterminantOfJacobian
    @see InverseOfJacobian
    */
    JacobiansType& Jacobian( JacobiansType& rResult, IntegrationMethod ThisMethod ) const override
    {
        std::cout<<"This method (Jacobian) has no meaning for this type of geometry (Sphere)."<<std::endl;
        return rResult;
    }

    /** Jacobians for given  method. This method
    calculate jacobians matrices in all integrations points of
    given integration method.

    @param ThisMethod integration method which jacobians has to
    be calculated in its integration points.

    @return JacobiansType a Vector of jacobian
    matrices \f$ J_i \f$ where \f$ i=1,2,...,n \f$ is the integration
    point index of given integration method.

    @param DeltaPosition Matrix with the nodes position increment which describes
    the configuration where the jacobian has to be calculated.

    @see DeterminantOfJacobian
    @see InverseOfJacobian
    */
    JacobiansType& Jacobian( JacobiansType& rResult, IntegrationMethod ThisMethod, Matrix & DeltaPosition ) const override
    {
        std::cout<<"This method (Jacobian) has no meaning for this type of geometry (Sphere)."<<std::endl;
        return rResult;
    }

    /** Jacobian in specific integration point of given integration
    method. This method calculate jacobian matrix in given
    integration point of given integration method.

    @param IntegrationPointIndex index of integration point which jacobians has to
    be calculated in it.

    @param ThisMethod integration method which jacobians has to
    be calculated in its integration points.

    @return Matrix<double> Jacobian matrix \f$ J_i \f$ where \f$
    i \f$ is the given integration point index of given
    integration method.

    @see DeterminantOfJacobian
    @see InverseOfJacobian
    */
    Matrix& Jacobian( Matrix& rResult, IndexType IntegrationPointIndex, IntegrationMethod ThisMethod ) const override
    {
        std::cout<<"This method (Jacobian) has no meaning for this type of geometry (Sphere)."<<std::endl;
        return rResult;
    }

    /** Jacobian in given point. This method calculate jacobian
    matrix in given point.

    @param rPoint point which jacobians has to
    be calculated in it.

    @return Matrix of double which is jacobian matrix \f$ J \f$ in given point.

    @see DeterminantOfJacobian
    @see InverseOfJacobian
    */
    Matrix& Jacobian( Matrix& rResult, const CoordinatesArrayType& rPoint ) const override
    {
        std::cout<<"This method (Jacobian) has no meaning for this type of geometry (Sphere)."<<std::endl;
        return rResult;
    }

    /** Determinant of jacobians for given integration method. This
    method calculate determinant of jacobian in all
    integrations points of given integration method.

    @return Vector of double which is vector of determinants of
    jacobians \f$ |J|_i \f$ where \f$ i=1,2,...,n \f$ is the
    integration point index of given integration method.

    @see Jacobian
    @see InverseOfJacobian
    */
    Vector& DeterminantOfJacobian( Vector& rResult, IntegrationMethod ThisMethod ) const override
    {
        std::cout<<"This method (DeterminantOfJacobian) has no meaning for this type of geometry (Sphere)."<<std::endl;
        return rResult;
    }

    /** Determinant of jacobian in specific integration point of
    given integration method. This method calculate determinant
    of jacobian in given integration point of given integration
    method.

    @param IntegrationPointIndex index of integration point which jacobians has to
    be calculated in it.

    @param IntegrationPointIndex index of integration point
    which determinant of jacobians has to be calculated in it.

    @return Determinamt of jacobian matrix \f$ |J|_i \f$ where \f$
    i \f$ is the given integration point index of given
    integration method.

    @see Jacobian
    @see InverseOfJacobian
    */
    double DeterminantOfJacobian( IndexType IntegrationPointIndex, IntegrationMethod ThisMethod ) const override
    {
        std::cout<<"This method (DeterminantOfJacobian) has no meaning for this type of geometry (Sphere)."<<std::endl;
        return 0.0;
    }

    /** Determinant of jacobian in given point. This method calculate determinant of jacobian
    matrix in given point.

    @param rPoint point which determinant of jacobians has to
    be calculated in it.

    @return Determinamt of jacobian matrix \f$ |J| \f$ in given
    point.

    @see DeterminantOfJacobian
    @see InverseOfJacobian
    */
    double DeterminantOfJacobian( const CoordinatesArrayType& rPoint ) const override
    {
        std::cout<<"This method (DeterminantOfJacobian) has no meaning for this type of geometry (Sphere)."<<std::endl;
        return 0.0;
    }


    /** Inverse of jacobians for given integration method. This method
    calculate inverse of jacobians matrices in all integrations points of
    given integration method.

    @param ThisMethod integration method which inverse of jacobians has to
    be calculated in its integration points.

    @return Inverse of jacobian
    matrices \f$ J^{-1}_i \f$ where \f$ i=1,2,...,n \f$ is the integration
    point index of given integration method.

    @see Jacobian
    @see DeterminantOfJacobian
    */
    JacobiansType& InverseOfJacobian( JacobiansType& rResult, IntegrationMethod ThisMethod ) const override
    {
        std::cout<<"This method (InverseOfJacobian) has no meaning for this type of geometry (Sphere)."<<std::endl;
        return rResult;
    }

    /** Inverse of jacobian in specific integration point of given integration
    method. This method calculate Inverse of jacobian matrix in given
    integration point of given integration method.

    @param IntegrationPointIndex index of integration point which inverse of jacobians has to
    be calculated in it.

    @param ThisMethod integration method which inverse of jacobians has to
    be calculated in its integration points.

    @return Inverse of jacobian matrix \f$ J^{-1}_i \f$ where \f$
    i \f$ is the given integration point index of given
    integration method.

    @see Jacobian
    @see DeterminantOfJacobian
    */
    Matrix& InverseOfJacobian( Matrix& rResult, IndexType IntegrationPointIndex, IntegrationMethod ThisMethod ) const override
    {
        std::cout<<"This method (InverseOfJacobian) has no meaning for this type of geometry (Sphere)."<<std::endl;
        return rResult;
    }

    /** Inverse of jacobian in given point. This method calculate inverse of jacobian
    matrix in given point.

    @param rPoint point which inverse of jacobians has to
    be calculated in it.

    @return Inverse of jacobian matrix \f$ J^{-1} \f$ in given point.

    @see DeterminantOfJacobian
    @see InverseOfJacobian
    */
    Matrix& InverseOfJacobian( Matrix& rResult, const CoordinatesArrayType& rPoint ) const override
    {
        std::cout<<"This method (InverseOfJacobian) has no meaning for this type of geometry (Sphere)."<<std::endl;
        return rResult;
    }


    /** EdgesNumber
    @return SizeType returns the number of edges of the geometry.
    */
    SizeType EdgesNumber() const override
    {
        return 0;
    }

    SizeType FacesNumber() const override
    {
        return 1;
    }

    ///@}
    ///@name Shape Function
    ///@{

    double ShapeFunctionValue( IndexType ShapeFunctionIndex,
                                       const CoordinatesArrayType& rPoint ) const override
    {
        std::cout<<"This method (ShapeFunctionValue) has no meaning for this type of geometry (Sphere)."<<std::endl;
        return 0;
    }

    Matrix& ShapeFunctionsLocalGradients( Matrix& rResult,
            const CoordinatesArrayType& rPoint ) const override
    {
        std::cout<<"This method (ShapeFunctionsLocalGradients) has no meaning for this type of geometry (Sphere)."<<std::endl;
        return( rResult );
    }



    ShapeFunctionsGradientsType& ShapeFunctionsIntegrationPointsGradients( ShapeFunctionsGradientsType& rResult, IntegrationMethod ThisMethod ) const override
    {
        std::cout<<"This method (ShapeFunctionsLocalGradients) has no meaning for this type of geometry (Sphere)."<<std::endl;
        return( rResult );
    }

    double GetRadius() const override{
        KRATOS_ERROR_IF(mpRadius == NULL) << "The Sphere3D1's radius address has not been properly set before calling the method 'GetRadius'. Use 'SetRadius' for that." << std::endl;

        return *mpRadius;
    }

    /** Counterpart of GetRadius(). It provides an interface
     * to set the radius when it cannot be inferred from other
     * information in the object (e.g., in a sphere)
     */

    void SetRadius(const double& rRadius) override {
        mpRadius = &rRadius;
    }

    /** Turn back information as a string.

    @return String contains information about this geometry.
    @see PrintData()
    @see PrintInfo()
    */
    std::string Info() const override
    {
        return "A sphere with 1 node in its center, in 3D space";
    }

    /** Print information about this object.

    @param rOStream Stream to print into it.
    @see PrintData()
    @see Info()
    */
    void PrintInfo( std::ostream& rOStream ) const override
    {
        rOStream << "A sphere with 1 node in its center, in 3D space";
    }

    /** Print geometry's data into given stream. Prints it's points
    by the order they stored in the geometry and then center
    point of geometry.

    @param rOStream Stream to print into it.
    @see PrintInfo()
    @see Info()
    */
    void PrintData( std::ostream& rOStream ) const override
    {

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

    static const GeometryData msGeometryData;

    ///@}
    ///@name Member Variables
    ///@{
    const double* mpRadius;
    ///@}
    ///@name Serialization
    ///@{

    friend class Serializer;

    void save( Serializer& rSerializer ) const override
    {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS( rSerializer, BaseType );
    }

    void load( Serializer& rSerializer ) override
    {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS( rSerializer, BaseType );
    }

    Sphere3D1(): BaseType( PointsArrayType(), &msGeometryData ) {}


    ///@}
    ///@name Private Operators
    ///@{


    ///@}
    ///@name Private Operations
    ///@{

    static Matrix CalculateShapeFunctionsIntegrationPointsValues( typename BaseType::IntegrationMethod ThisMethod )
    {
        const IntegrationPointsContainerType& all_integration_points = AllIntegrationPoints();
        const IntegrationPointsArrayType& IntegrationPoints = all_integration_points[ThisMethod];
        const int integration_points_number = IntegrationPoints.size();
        Matrix N( integration_points_number, 1 );

        //std::cout<<"This method (CalculateShapeFunctionsIntegrationPointsValues) has no meaning for this type of geometry (Sphere)."<<std::endl;

        return N;
    }

    static ShapeFunctionsGradientsType CalculateShapeFunctionsIntegrationPointsLocalGradients( typename BaseType::IntegrationMethod ThisMethod )
    {
        const IntegrationPointsContainerType& all_integration_points = AllIntegrationPoints();
        const IntegrationPointsArrayType& IntegrationPoints = all_integration_points[ThisMethod];
        ShapeFunctionsGradientsType DN_De( IntegrationPoints.size() );
        std::fill( DN_De.begin(), DN_De.end(), Matrix( 2, 1 ) );

        //std::cout<<"This method (CalculateShapeFunctionsIntegrationPointsLocalGradients) has no meaning for this type of geometry (Sphere)."<<std::endl;

        return DN_De;

    }

    static const IntegrationPointsContainerType AllIntegrationPoints()
    {
        IntegrationPointsContainerType integration_points = {{
                Quadrature<LineGaussLegendreIntegrationPoints1, 1, IntegrationPoint<3> >::GenerateIntegrationPoints(),
                Quadrature<LineGaussLegendreIntegrationPoints2, 1, IntegrationPoint<3> >::GenerateIntegrationPoints(),
                Quadrature<LineGaussLegendreIntegrationPoints3, 1, IntegrationPoint<3> >::GenerateIntegrationPoints(),
                Quadrature<LineGaussLegendreIntegrationPoints4, 1, IntegrationPoint<3> >::GenerateIntegrationPoints(),
                Quadrature<LineGaussLegendreIntegrationPoints5, 1, IntegrationPoint<3> >::GenerateIntegrationPoints()
            }
        };
        return integration_points;
    }

    static const ShapeFunctionsValuesContainerType AllShapeFunctionsValues()
    {
        ShapeFunctionsValuesContainerType shape_functions_values = {{
                Sphere3D1<TPointType>::CalculateShapeFunctionsIntegrationPointsValues( GeometryData::GI_GAUSS_1 ),
                Sphere3D1<TPointType>::CalculateShapeFunctionsIntegrationPointsValues( GeometryData::GI_GAUSS_2 ),
                Sphere3D1<TPointType>::CalculateShapeFunctionsIntegrationPointsValues( GeometryData::GI_GAUSS_3 ),
                Sphere3D1<TPointType>::CalculateShapeFunctionsIntegrationPointsValues( GeometryData::GI_GAUSS_4 ),
                Sphere3D1<TPointType>::CalculateShapeFunctionsIntegrationPointsValues( GeometryData::GI_GAUSS_5 )
            }
        };
        return shape_functions_values;
    }

    static const ShapeFunctionsLocalGradientsContainerType AllShapeFunctionsLocalGradients()
    {
        ShapeFunctionsLocalGradientsContainerType shape_functions_local_gradients = {{
                Sphere3D1<TPointType>::CalculateShapeFunctionsIntegrationPointsLocalGradients( GeometryData::GI_GAUSS_1 ),
                Sphere3D1<TPointType>::CalculateShapeFunctionsIntegrationPointsLocalGradients( GeometryData::GI_GAUSS_2 ),
                Sphere3D1<TPointType>::CalculateShapeFunctionsIntegrationPointsLocalGradients( GeometryData::GI_GAUSS_3 ),
                Sphere3D1<TPointType>::CalculateShapeFunctionsIntegrationPointsLocalGradients( GeometryData::GI_GAUSS_4 ),
                Sphere3D1<TPointType>::CalculateShapeFunctionsIntegrationPointsLocalGradients( GeometryData::GI_GAUSS_5 ),

            }
        };
        return shape_functions_local_gradients;
    }

    ///@}
    ///@name Private  Access
    ///@{


    ///@}
    ///@name Private Inquiry
    ///@{


    ///@}
    ///@name Private Friends
    ///@{

    template<class TOtherPointType> friend class Sphere3D1;

    ///@}
    ///@name Un accessible methods
    ///@{

    ///@}

}; // Class Sphere3D1

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{


/// input stream function
template<class TPointType>
inline std::istream& operator >> ( std::istream& rIStream,
                                   Sphere3D1<TPointType>& rThis );

/// output stream function
template<class TPointType>
inline std::ostream& operator << ( std::ostream& rOStream,
                                   const Sphere3D1<TPointType>& rThis )
{
    rThis.PrintInfo( rOStream );
    rOStream << std::endl;
    rThis.PrintData( rOStream );

    return rOStream;
}



template<class TPointType>
const GeometryData Sphere3D1<TPointType>::msGeometryData( 3,
        3,
        1,
        GeometryData::GI_GAUSS_1,
        Sphere3D1<TPointType>::AllIntegrationPoints(),
        Sphere3D1<TPointType>::AllShapeFunctionsValues(),
        AllShapeFunctionsLocalGradients() );

}  // namespace Kratos.

#endif // KRATOS_SPHERE_3D_1_H_INCLUDED  defined

