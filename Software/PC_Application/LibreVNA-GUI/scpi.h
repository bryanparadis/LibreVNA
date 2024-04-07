﻿#ifndef SCPI_H
#define SCPI_H

#include <QString>
#include <QObject>
#include <vector>
#include <functional>

class SCPICommand {
public:
    SCPICommand(QString name, std::function<QString(QStringList)> cmd, std::function<QString(QStringList)> query, bool convertToUppercase = true) :
        _name(name),
        fn_cmd(cmd),
        fn_query(query),
        argAlwaysUppercase(convertToUppercase){}

    QString execute(QStringList params);
    QString query(QStringList params);
    QString name() {return _name;}
    bool queryable() { return fn_query != nullptr;}
    bool executable() { return fn_cmd != nullptr;}
    bool convertToUppercase() { return argAlwaysUppercase;}
private:
    const QString _name;
    std::function<QString(QStringList)> fn_cmd;
    std::function<QString(QStringList)> fn_query;
    bool argAlwaysUppercase;
};

class SCPINode {
    friend class SCPI;
public:
    SCPINode(QString name) :
        name(name), parent(nullptr), operationPending(false){}
    virtual ~SCPINode();

    bool add(SCPINode *node);
    bool remove(SCPINode *node);
    bool add(SCPICommand *cmd);

    bool addDoubleParameter(QString name, double &param, bool gettable = true, bool settable = true, std::function<void(void)> setCallback = nullptr);
    bool addUnsignedIntParameter(QString name, unsigned int &param, bool gettable = true, bool settable = true, std::function<void(void)> setCallback = nullptr);
    bool addBoolParameter(QString name, bool &param, bool gettable = true, bool settable = true, std::function<void(void)> setCallback = nullptr);

    bool changeName(QString newname);

protected:
    void setOperationPending(bool pending);

    bool isOperationPending();

private:
    QString parse(QString cmd, SCPINode* &lastNode);
    bool nameCollision(QString name);
    void createCommandList(QString prefix, QString &list);
    QString name;
    std::vector<SCPINode*> subnodes;
    std::vector<SCPICommand*> commands;
    SCPINode *parent;
    bool operationPending;
};

class SCPI : public QObject, public SCPINode
{
    Q_OBJECT
public:
    SCPI();

    static bool match(QString s1, QString s2);
    static QString alternateName(QString name);

    static bool paramToDouble(QStringList params, int index, double &dest);
    static bool paramToULongLong(QStringList params, int index, unsigned long long &dest);
    static bool paramToLong(QStringList params, int index, long &dest);
    static bool paramToBool(QStringList params, int index, bool &dest);

    enum class Result {
        Empty,
        Error,
        False,
        True
    };

    static QString getResultName(SCPI::Result r);

    // call whenever a subnode completes an operation
    void someOperationCompleted();

public slots:
    void input(QString line);
    void process();
signals:
    void output(QString line);

private:

    enum class Flag  {
        OPC = 0x01, // Operation complete
        RQC = 0x02, // device wants to become the controller (of the bus)
        QYE = 0x04, // query error
        DDE = 0x08, // device-dependent error
        EXE = 0x10, // execution error
        CME = 0x20, // command error
        URQ = 0x40, // user request
        PON = 0x80, // power on
    };

    void setFlag(Flag flag);
    void clearFlag(Flag flag);
    bool getFlag(Flag flag);

    unsigned int ESR;

    bool OCAS;
    bool OPCQueryScheduled;
    bool WAIexecuting;

    QList<QString> cmdQueue;
};

#endif // SCPI_H
