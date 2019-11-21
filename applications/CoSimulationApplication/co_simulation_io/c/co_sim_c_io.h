// KRATOS  / ___|___/ ___|(_)_ __ ___  _   _| | __ _| |_(_) ___  _ ___
//        | |   / _ \___ \| | '_ ` _ \| | | | |/ _` | __| |/ _ \| '_  |
//        | |__| (_) |__) | | | | | | | |_| | | (_| | |_| | (_) | | | |
//         \____\___/____/|_|_| |_| |_|\__,_|_|\__,_|\__|_|\___/|_| |_|
//
//  License:		 BSD License
//                   license: CoSimulationApplication/license.txt
//
//  Main authors:    Philipp Bucher
//

#ifndef KRATOS_CO_SIM_C_IO_H_INCLUDED
#define KRATOS_CO_SIM_C_IO_H_INCLUDED

#ifdef __cplusplus
extern "C" { // Define extern C if C++ compiler is used
#endif

void CoSimIO_Connect(
    const char* pConnectionName,
    const char* pSettingsFileName);

void CoSimIO_Disconnect(
    const char* pConnectionName);

void CoSimIO_ImportData(
    const char* pConnectionName,
    const char* pIdentifier,
    int* pSize,
    double** ppData);

void CoSimIO_ExportData(
    const char* pConnectionName,
    const char* pIdentifier,
    int Size,
    double* pData);

void CoSimIO_ImportMesh(
    const char* pConnectionName,
    const char* pIdentifier,
    int* pNumberOfNodes,
    int* pNumberOfElements,
    double** ppNodalCoordinates,
    int** ppElementConnectivities,
    int** ppElementTypes);

void CoSimIO_ExportMesh(
    const char* pConnectionName,
    const char* pIdentifier,
    int NumberOfNodes,
    int NumberOfElements,
    double* pNodalCoordinates,
    int* pElementConnectivities,
    int* pElementTypes);

#ifdef __cplusplus
}
#endif

#endif // KRATOS_CO_SIM_C_IO_H_INCLUDED