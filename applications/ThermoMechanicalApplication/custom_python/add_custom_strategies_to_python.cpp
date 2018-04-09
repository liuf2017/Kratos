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
//                   Jordi Rubio
//


// System includes


// External includes
#include "pybind11/pybind11.h"


// Project includes
#include "includes/define.h"
#include "custom_python/add_custom_strategies_to_python.h"

#include "spaces/ublas_space.h"

//strategies
#include "solving_strategies/strategies/solving_strategy.h"
#include "solving_strategies/schemes/residualbased_incrementalupdate_static_scheme.h"
#include "custom_strategies/residualbased_incrementalupdate_variable_property_static_scheme.h"
#include "custom_strategies/general_residualbased_incrementalupdate_variable_property_static_scheme.h"

//linear solvers
#include "linear_solvers/linear_solver.h"



namespace Kratos
{

namespace Python
{

void  AddCustomStrategiesToPython(pybind11::module& m)
{
    using namespace pybind11;

    typedef UblasSpace<double, CompressedMatrix, Vector> SparseSpaceType;
    typedef UblasSpace<double, Matrix, Vector> LocalSpaceType;

//     typedef LinearSolver<SparseSpaceType, LocalSpaceType > LinearSolverType;
    //typedef SolvingStrategy< SparseSpaceType, LocalSpaceType, LinearSolverType > BaseSolvingStrategyType;
    //typedef Scheme< SparseSpaceType, LocalSpaceType > BaseSchemeType;

    //********************************************************************
    //********************************************************************
    class_< ResidualBasedIncrementalUpdateStaticVariablePropertyScheme< SparseSpaceType, LocalSpaceType>,
	    ResidualBasedIncrementalUpdateStaticScheme <SparseSpaceType, LocalSpaceType> >
	    (m, "ResidualBasedIncrementalUpdateStaticVariablePropertyScheme")
        .def( init< >() );

    class_< GeneralResidualBasedIncrementalUpdateStaticVariablePropertyScheme< SparseSpaceType, LocalSpaceType>,
	    ResidualBasedIncrementalUpdateStaticScheme <SparseSpaceType, LocalSpaceType> >
	    (m, "GeneralResidualBasedIncrementalUpdateStaticVariablePropertyScheme")
        .def( init< >() );

// 			class_< TestStrategy< SparseSpaceType, LocalSpaceType, LinearSolverType >,
// 					bases< BaseSolvingStrategyType >,  boost::noncopyable >
// 				("TestStrategy",
// 				init<ModelPart&, LinearSolverType::Pointer, int, int, bool >() )
// 				.def("MoveNodes",&TestStrategy< SparseSpaceType, LocalSpaceType, LinearSolverType >::MoveNodes)
// 				;

}

}  // namespace Python.

} // Namespace Kratos

