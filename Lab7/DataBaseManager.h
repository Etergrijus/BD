#ifndef DATABASE_1_DATABASEMANAGER_H
#define DATABASE_1_DATABASEMANAGER_H

#include <QxOrm.h>
#include <QxDao/QxDao.h>
#include <memory>

class IDataRow {
public:
    virtual ~IDataRow() = default;
    virtual int getColumnCount() const = 0;
    virtual QVariant getValue(int column) const = 0;
};

struct Qualification : public IDataRow {
    int id;
    QString description;

    int getColumnCount() const override {
        return 2;
    };

    QVariant getValue(int column) const override {
        switch (column) {
            case 0: return id;
            case 1: return description;
            default: return QVariant();
        }
    }
};

struct ContactDetails : public IDataRow {
    int id;
    QString phone;
    QString fcs;

    int getColumnCount() const override {
        return 3;
    };

    QVariant getValue(int column) const override {
        switch (column) {
            case 0: return id;
            case 1: return phone;
            case 2: return fcs;
            default: return QVariant();
        }
    }
};

struct HomeOwnership : public IDataRow {
    QString address;
    QString buildingParameters;
    QString additionalInfo;

    int getColumnCount() const override {
        return 3;
    };

    QVariant getValue(int column) const override {
        switch (column) {
            case 0: return address;
            case 1: return buildingParameters;
            case 2: return additionalInfo;
            default: return QVariant();
        }
    }
};

QX_REGISTER_PRIMARY_KEY(Qualification, int)
QX_REGISTER_HPP_EXPORT_DLL(Qualification, qx::trait::no_base_class_defined, 0)

QX_REGISTER_PRIMARY_KEY(ContactDetails, int)
QX_REGISTER_HPP_EXPORT_DLL(ContactDetails, qx::trait::no_base_class_defined, 0)

QX_REGISTER_PRIMARY_KEY(HomeOwnership, QString)
QX_REGISTER_HPP_EXPORT_DLL(HomeOwnership, qx::trait::no_base_class_defined, 0)

/*struct Qualification : public IDataRow {
    int id;
    QString description;

    int getColumnCount() const override {
        return 2;
    };

    QVariant getValue(int column) const override {
        switch (column) {
            case 0: return id;
            case 1: return description;
            default: return QVariant();
        }
    }
};*/

typedef std::vector<std::shared_ptr<IDataRow>> dataVector;

class DataBaseManager {
public:
    explicit DataBaseManager(const QString& dbName);

    ~DataBaseManager();

    template<typename T>
    dataVector loadData() {
        std::vector<T> data;
        QSqlError sqlError;
        try {
            sqlError = qx::dao::fetch_all(data);
        }
        catch (std::exception &e) {
            std::cout << e.what();
        }
        return convertToDataRow(data);
    }

private:
    template<typename T>
    dataVector convertToDataRow(const std::vector<T>& data) {
        dataVector rows;
        for (auto &item : data)
            rows.push_back(std::make_shared<T>(item));
        return rows;
    }
};


#endif //DATABASE_1_DATABASEMANAGER_H
