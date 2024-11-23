#include <Parsers/ASTUseQuery.h>
#include <Interpreters/Context.h>
#include <Interpreters/InterpreterFactory.h>
#include <Interpreters/InterpreterUseQuery.h>
#include <Access/Common/AccessFlags.h>
#include <Common/Exception.h>
#include <Common/typeid_cast.h>

#include <filesystem>
#include <fstream>

namespace DB
{

namespace ErrorCodes
{
    extern const int CANNOT_OPEN_FILE;
}

BlockIO InterpreterUseQuery::execute()
{
    const String & new_database = query_ptr->as<ASTUseQuery &>().getDatabase();
    getContext()->checkAccess(AccessType::SHOW_DATABASES, new_database);
    getContext()->getSessionContext()->setCurrentDatabase(new_database);

    // Save the current using database in default_database stored in getPath()
    // for the case when the database is changed in chDB session.
    // The default_database content is used in the LocalServer::processConfig() method.
    auto default_database_path = std::filesystem::path(getContext()->getPath()) / "default_database";
    std::ofstream tmp_path_fs(default_database_path, std::ofstream::out | std::ofstream::trunc);
    if (tmp_path_fs.is_open())
    {
        tmp_path_fs << new_database;
        tmp_path_fs.close();
    }
    //chdb todo: fix the following code on bgClickHouseLocal mode
    // else
    // {
    //     throw Exception(ErrorCodes::CANNOT_OPEN_FILE, "Cannot open file {} for writing", default_database_path.string());
    // }

    return {};
}

void registerInterpreterUseQuery(InterpreterFactory & factory)
{
    auto create_fn = [] (const InterpreterFactory::Arguments & args)
    {
        return std::make_unique<InterpreterUseQuery>(args.query, args.context);
    };
    factory.registerInterpreter("InterpreterUseQuery", create_fn);
}

}
