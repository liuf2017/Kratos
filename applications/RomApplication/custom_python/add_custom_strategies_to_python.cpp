//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Author1 Fullname
//                   Author2 Fullname
//


// System includes


// External includes
#include <pybind11/pybind11.h>


// Project includes
#include "includes/define_python.h"
#include "custom_python/add_custom_strategies_to_python.h"


#include "spaces/ublas_space.h"

//strategies
#include "solving_strategies/strategies/solving_strategy.h"


//linear solvers
#include "linear_solvers/linear_solver.h"


namespace Kratos {
namespace Python {

void  AddCustomStrategiesToPython(pybind11::module& m)
{
    namespace py = pybind11;

    typedef UblasSpace<double, CompressedMatrix, boost::numeric::ublas::vector<double>> SparseSpaceType;
    typedef UblasSpace<double, Matrix, Vector> LocalSpaceType;

    typedef LinearSolver<SparseSpaceType, LocalSpaceType > LinearSolverType;

    typedef BuilderAndSolver< SparseSpaceType, LocalSpaceType, LinearSolverType > BuilderAndSolverType;

    //********************************************************************
    //********************************************************************
    typedef ROMBuilderAndSolver<SparseSpaceType, LocalSpaceType, LinearSolverType> ROMBuilderAndSolverType;

     py::class_<ROMBuilderAndSolverType, typename ROMBuilderAndSolverType::Pointer, BuilderAndSolverType>(m, "ROMBuilderAndSolver")
     	.def(py::init< LinearSolverType::Pointer, Parameters>() )
     	//.def("MoveNodes",&ROMBuilderAndSolverType::MoveNodes)
     	;



    typedef HROMBuilderAndSolver<SparseSpaceType, LocalSpaceType, LinearSolverType> HROMBuilderAndSolverType;

     py::class_<HROMBuilderAndSolverType, typename HROMBuilderAndSolverType::Pointer, BuilderAndSolverType>(m, "HROMBuilderAndSolver")
     	.def(py::init< LinearSolverType::Pointer, Parameters>() )
     	//.def("MoveNodes",&HROMBuilderAndSolverType::MoveNodes)
     	;
}

} // namespace Python.
} // Namespace Kratos



            // typedef ROMBuilderAndSolver< SparseSpaceType, LocalSpaceType, LinearSolverType > ROMBuilderAndSolverType;
            // py::class_< ROMBuilderAndSolverType, ROMBuilderAndSolverType::Pointer,BuilderAndSolverType>(m,"ROMBuilderAndSolver")
            // .def(py::init< LinearSolverType::Pointer, Parameters > ())
            // ;