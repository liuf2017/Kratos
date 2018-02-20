#include "custom_io/hdf5_model_part_io.h"

#include "custom_io/hdf5_properties_io.h"
#include "custom_io/hdf5_nodal_solution_step_variables_io.h"
#include "custom_io/hdf5_data_value_container_io.h"
#include "custom_io/hdf5_points_data.h"
#include "custom_io/hdf5_connectivities_data.h"
#include "custom_utilities/factor_elements_and_conditions_utility.h"
#include "utilities/builtin_timer.h"

namespace Kratos
{
namespace HDF5
{
ModelPartIO::ModelPartIO(File::Pointer pFile, std::string const& rPrefix)
: mpFile(pFile), mPrefix(rPrefix)
{
}

bool ModelPartIO::ReadNodes(NodesContainerType& rNodes)
{
    KRATOS_TRY;

    rNodes.clear();
    unsigned start_index, block_size;
    std::tie(start_index, block_size) =
        StartIndexAndBlockSize(mPrefix + "/Nodes/Local");
    Internals::PointsData points;
    points.ReadData(*mpFile, mPrefix + "/Nodes/Local", start_index, block_size);
    points.CreateNodes(rNodes);
    return true;

    KRATOS_CATCH("");
}

std::size_t ModelPartIO::ReadNodesNumber()
{
    const std::vector<unsigned> dims = mpFile->GetDataDimensions(mPrefix + "/Nodes/Local/Ids");
    return dims[0];
}

void ModelPartIO::WriteNodes(NodesContainerType const& rNodes)
{
    KRATOS_TRY;

    Internals::PointsData points;
    points.SetData(rNodes);
    WriteInfo info;
    points.WriteData(*mpFile, mPrefix + "/Nodes/Local", info);
    StoreWriteInfo(mPrefix + "/Nodes/Local", info);

    KRATOS_CATCH("");
}

void ModelPartIO::ReadProperties(PropertiesContainerType& rProperties)
{
    Internals::ReadProperties(*mpFile, mPrefix, rProperties);
}

void ModelPartIO::WriteProperties(Properties const& rProperties)
{
    Internals::WriteProperties(*mpFile, mPrefix, rProperties);
}

void ModelPartIO::WriteProperties(PropertiesContainerType const& rProperties)
{
    Internals::WriteProperties(*mpFile, mPrefix, rProperties);
}

void ModelPartIO::ReadElements(NodesContainerType& rNodes,
                               PropertiesContainerType& rProperties,
                               ElementsContainerType& rElements)
{
    KRATOS_TRY;

    rElements.clear();
    for (const auto& r_name : mpFile->GetGroupNames(mPrefix + "/Elements"))
    {
        unsigned start_index, block_size;
        std::tie(start_index, block_size) = StartIndexAndBlockSize(mPrefix + "/Elements/" + r_name);
        Internals::ConnectivitiesData connectivities;
        connectivities.ReadData(*mpFile, mPrefix + "/Elements/" + r_name, start_index, block_size);
        connectivities.CreateEntities(rNodes, rProperties, rElements);
    }

    KRATOS_CATCH("");
}

void ModelPartIO::WriteElements(ElementsContainerType const& rElements)
{
    KRATOS_TRY;

    std::vector<std::string> names;
    std::vector<ElementsContainerType> factored_elements;
    FactorElements(rElements, names, factored_elements);
    for (unsigned int i = 0; i < names.size(); ++i)
    {
        Internals::ConnectivitiesData connectivities;
        // For partitioned elements, the local container may be empty. Therefore,
        // we explicitly provide the element name here.
        connectivities.SetData(names[i], factored_elements[i]);
        WriteInfo info;
        connectivities.WriteData(*mpFile, mPrefix + "/Elements/" + connectivities.Name(), info);
        StoreWriteInfo(mPrefix + "/Elements/" + connectivities.Name(), info);
    }

    KRATOS_CATCH("");
}

void ModelPartIO::ReadConditions(NodesContainerType& rNodes,
                                 PropertiesContainerType& rProperties,
                                 ConditionsContainerType& rConditions)
{
    KRATOS_TRY;

    rConditions.clear();
    for (const auto& r_name : mpFile->GetGroupNames(mPrefix + "/Conditions"))
    {
        unsigned start_index, block_size;
        std::tie(start_index, block_size) = StartIndexAndBlockSize(mPrefix + "/Conditions/" + r_name);
        Internals::ConnectivitiesData connectivities;
        connectivities.ReadData(*mpFile, mPrefix + "/Conditions/" + r_name, start_index, block_size);
        connectivities.CreateEntities(rNodes, rProperties, rConditions);
    }

    KRATOS_CATCH("");
}

void ModelPartIO::WriteConditions(ConditionsContainerType const& rConditions)
{
    KRATOS_TRY;

    std::vector<std::string> names;
    std::vector<ConditionsContainerType> factored_conditions;
    FactorConditions(rConditions, names, factored_conditions);
    for (unsigned i = 0; i < names.size();  ++i)
    {
        Internals::ConnectivitiesData connectivities;
        // For partitioned conditions, the local container may be empty. Therefore,
        // we explicitly provide the condition name here.
        connectivities.SetData(names[i], factored_conditions[i]);
        WriteInfo info;
        connectivities.WriteData(*mpFile, mPrefix + "/Conditions/" + connectivities.Name(), info);
        StoreWriteInfo(mPrefix + "/Conditions/" + connectivities.Name(), info);
    }

    KRATOS_CATCH("");
}

void ModelPartIO::WriteModelPart(ModelPart& rModelPart)
{
    KRATOS_TRY;

    BuiltinTimer timer;
    Internals::WriteVariablesList(*mpFile, mPrefix, rModelPart);
    Internals::WriteBufferSize(*mpFile, mPrefix, rModelPart.GetBufferSize());
    WriteProperties(rModelPart.rProperties());
    Internals::WriteDataValueContainer(*mpFile, mPrefix + "/ProcessInfo", rModelPart.GetProcessInfo());
    rModelPart.Nodes().Sort(); // Avoid inadvertently reordering partway through
                               // the writing process.
    WriteNodes(rModelPart.Nodes());
    WriteElements(rModelPart.Elements());
    WriteConditions(rModelPart.Conditions());

     if (mpFile->GetEchoLevel() == 1 && mpFile->GetPID() == 0)
        std::cout << "Time to write model part \"" << rModelPart.Name() << "\": " << timer.ElapsedSeconds() << " seconds." << std::endl;

    KRATOS_CATCH("");
}

void ModelPartIO::ReadModelPart(ModelPart& rModelPart)
{
    KRATOS_TRY;

    BuiltinTimer timer;
    ReadProperties(rModelPart.rProperties());
    Internals::ReadDataValueContainer(*mpFile, mPrefix + "/ProcessInfo", rModelPart.GetProcessInfo());
    ReadNodes(rModelPart.Nodes());
    ReadElements(rModelPart.Nodes(), rModelPart.rProperties(), rModelPart.Elements());
    ReadConditions(rModelPart.Nodes(), rModelPart.rProperties(), rModelPart.Conditions());
    Internals::ReadAndAssignVariablesList(*mpFile, mPrefix, rModelPart);
    Internals::ReadAndAssignBufferSize(*mpFile, mPrefix, rModelPart);

    if (mpFile->GetEchoLevel() == 1 && mpFile->GetPID() == 0)
        std::cout << "Time to read model part \"" << rModelPart.Name() << "\": " << timer.ElapsedSeconds() << " seconds." << std::endl;

    KRATOS_CATCH("");
}

std::tuple<unsigned, unsigned> ModelPartIO::StartIndexAndBlockSize(std::string const& rPath) const
{
    KRATOS_TRY;
    int size;
    mpFile->ReadAttribute(rPath, "Size", size);
    return std::make_tuple(0, size);
    KRATOS_CATCH("");
}

void ModelPartIO::StoreWriteInfo(std::string const& rPath, WriteInfo const& rInfo)
{
    KRATOS_TRY;
    const int size = rInfo.TotalSize;
    mpFile->WriteAttribute(rPath, "Size", size);
    KRATOS_CATCH("");
}

} // namespace HDF5.
} // namespace Kratos.
