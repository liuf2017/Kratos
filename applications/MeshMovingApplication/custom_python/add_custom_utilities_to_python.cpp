//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license:
// kratos/license.txt
//
//  Main authors:    Andreas Winterstein (a.winterstein@tum.de)
//

// System includes

// External includes
#include "boost/numeric/ublas/vector.hpp"

// Project includes
#include "linear_solvers/linear_solver.h"
#include "spaces/ublas_space.h"

// Application includes
#include "custom_python/add_custom_utilities_to_python.h"
#include "custom_utilities/ball_vertex_meshmoving.h"
#include "custom_utilities/ball_vertex_meshmoving3D.h"
#include "custom_utilities/explicit_fixed_mesh_ale_utilities.h"
#include "custom_utilities/fixed_mesh_ale_utilities.h"
#include "custom_utilities/mesh_velocity_calculation.h"
#include "custom_utilities/move_mesh_utilities.h"

namespace Kratos {
namespace Python {

void AddCustomUtilitiesToPython(pybind11::module& m) {
    namespace py = pybind11;

    typedef UblasSpace<double, CompressedMatrix, boost::numeric::ublas::vector<double>> SparseSpaceType;
    typedef UblasSpace<double, Matrix, Vector> LocalSpaceType;
    typedef LinearSolver<SparseSpaceType, LocalSpaceType> LinearSolverType;

    py::class_<BallVertexMeshMoving<2, SparseSpaceType, LinearSolverType> >(m,"BallVertexMeshMoving2D")
        .def(py::init<>())
        .def("ConstructSystem", &BallVertexMeshMoving<2, SparseSpaceType, LinearSolverType>::ConstructSystem)
        .def("BuildAndSolveSystem", &BallVertexMeshMoving<2, SparseSpaceType, LinearSolverType>::BuildAndSolveSystem)
        .def("ClearSystem", &BallVertexMeshMoving<2, SparseSpaceType, LinearSolverType>::ClearSystem);

    py::class_<BallVertexMeshMoving3D<3, SparseSpaceType, LinearSolverType>>(m,"BallVertexMeshMoving3D")
        .def(py::init<>())
        .def("ConstructSystem", &BallVertexMeshMoving3D<3, SparseSpaceType, LinearSolverType>::ConstructSystem)
        .def("BuildAndSolveSystem", &BallVertexMeshMoving3D<3, SparseSpaceType, LinearSolverType>::BuildAndSolveSystem)
        .def("ClearSystem", &BallVertexMeshMoving3D<3, SparseSpaceType, LinearSolverType>::ClearSystem);

    py::class_<FixedMeshALEUtilities, FixedMeshALEUtilities::Pointer>(m, "FixedMeshALEUtilities")
        .def(py::init<Model &, Parameters &>())
        .def(py::init<ModelPart &, ModelPart &, const std::string>())
        .def("Initialize", &FixedMeshALEUtilities::Initialize)
        .def("ComputeMeshMovement", &FixedMeshALEUtilities::ComputeMeshMovement)
        .def("ProjectVirtualValues2D", &FixedMeshALEUtilities::ProjectVirtualValues<2>)
        .def("ProjectVirtualValues3D", &FixedMeshALEUtilities::ProjectVirtualValues<3>)
        .def("UndoMeshMovement", &FixedMeshALEUtilities::UndoMeshMovement);

    py::class_<ExplicitFixedMeshALEUtilities, ExplicitFixedMeshALEUtilities::Pointer, FixedMeshALEUtilities>(m, "ExplicitFixedMeshALEUtilities")
        .def(py::init<Model &, Parameters &>())
        .def(py::init<ModelPart &, ModelPart &, const double>())
        .def("Initialize", &ExplicitFixedMeshALEUtilities::Initialize)
        .def("ComputeMeshMovement", &ExplicitFixedMeshALEUtilities::ComputeMeshMovement)
        .def("ProjectVirtualValues2D", &ExplicitFixedMeshALEUtilities::ProjectVirtualValues<2>)
        .def("ProjectVirtualValues3D", &ExplicitFixedMeshALEUtilities::ProjectVirtualValues<3>)
        .def("UndoMeshMovement", &ExplicitFixedMeshALEUtilities::UndoMeshMovement);

    void (*CalculateMeshVelocitiesBDF1)(ModelPart&, const TimeDiscretization::BDF1&) = &MeshVelocityCalculation::CalculateMeshVelocities;
    void (*CalculateMeshVelocitiesBDF2)(ModelPart&, const TimeDiscretization::BDF2&) = &MeshVelocityCalculation::CalculateMeshVelocities;
    void (*CalculateMeshVelocitiesNewmark)(ModelPart&, const TimeDiscretization::Newmark&) = &MeshVelocityCalculation::CalculateMeshVelocities;
    void (*CalculateMeshVelocitiesBossak)(ModelPart&, const TimeDiscretization::Bossak&) = &MeshVelocityCalculation::CalculateMeshVelocities;
    void (*CalculateMeshVelocitiesGeneralizedAlpha)(ModelPart&, const TimeDiscretization::GeneralizedAlpha&) = &MeshVelocityCalculation::CalculateMeshVelocities;

    m.def("CalculateMeshVelocities", CalculateMeshVelocitiesBDF1 );
    m.def("CalculateMeshVelocities", CalculateMeshVelocitiesBDF2 );
    m.def("CalculateMeshVelocities", CalculateMeshVelocitiesNewmark );
    m.def("CalculateMeshVelocities", CalculateMeshVelocitiesBossak );
    m.def("CalculateMeshVelocities", CalculateMeshVelocitiesGeneralizedAlpha );

    m.def("MoveMesh", &MoveMeshUtilities::MoveMesh );

}

} // namespace Python.

} // Namespace Kratos
