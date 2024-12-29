#include "DataBaseManager.h"

#include <QxOrm_Impl.h>

DataBaseManager::DataBaseManager(const QString& dbName) {
    qx::QxSqlDatabase::getSingleton()->setDriverName("QPSQL");
    qx::QxSqlDatabase::getSingleton()->setDatabaseName("postgres");
    qx::QxSqlDatabase::getSingleton()->setPort(5432);
    qx::QxSqlDatabase::getSingleton()->setUserName("postgres");
    qx::QxSqlDatabase::getSingleton()->setPassword("postgres");
    if (!qx::QxSqlDatabase::getDatabase().isOpen())
        throw std::runtime_error("Database not found");
}

DataBaseManager::~DataBaseManager() {
    qx::QxSqlDatabase::getDatabase().close();
}

QX_REGISTER_CPP_EXPORT_DLL(Qualification)
namespace qx {
    template <> void register_class(QxClass<Qualification> &t) {
        t.setName("qualifications");
        t.id(&Qualification::id, "id");
        t.data(&Qualification::description, "description");
    }
}

QX_REGISTER_CPP_EXPORT_DLL(ContactDetails)
namespace qx {
    template <> void register_class(QxClass<ContactDetails> &t) {
        t.setName("contact_details");
        t.id(&ContactDetails::id, "id");
        t.data(&ContactDetails::phone, "phone");
        t.data(&ContactDetails::fcs, "fcs");
    }
}

QX_REGISTER_CPP_EXPORT_DLL(HomeOwnership)
namespace qx {
    template <> void register_class(QxClass<HomeOwnership> &t) {
        t.setName("home_ownerships");
        t.id(&HomeOwnership::address, "address");
        t.data(&HomeOwnership::buildingParameters, "building_parameters");
        t.data(&HomeOwnership::additionalInfo, "additional_info");
    }
}