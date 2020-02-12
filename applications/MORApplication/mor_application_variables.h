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

#if !defined(KRATOS_MOR_APPLICATION_VARIABLES_H_INCLUDED )
#define  KRATOS_MOR_APPLICATION_VARIABLES_H_INCLUDED

// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "includes/variables.h"
#include "includes/kratos_application.h"

namespace Kratos
{
KRATOS_DEFINE_APPLICATION_VARIABLE( MOR_APPLICATION, double, FREQUENCY )
KRATOS_DEFINE_APPLICATION_VARIABLE( MOR_APPLICATION, int, BUILD_LEVEL )
 
KRATOS_DEFINE_APPLICATION_VARIABLE( MOR_APPLICATION, double, SCALAR_OUTPUT )
KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( MOR_APPLICATION, COMPONENT_OUTPUT )

// complex dof variables
KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( MOR_APPLICATION, REAL_DISPLACEMENT )
KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( MOR_APPLICATION, IMAG_DISPLACEMENT )
KRATOS_DEFINE_APPLICATION_VARIABLE( MOR_APPLICATION, double, REAL_PRESSURE )
KRATOS_DEFINE_APPLICATION_VARIABLE( MOR_APPLICATION, double, IMAG_PRESSURE )
}

#endif	/* KRATOS_MOR_APPLICATION_VARIABLES_H_INCLUDED */
