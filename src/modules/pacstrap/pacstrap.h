/* === This file is part of Calamares - <http://github.com/calamares> ===
 *
 *   Copyright 2016, Kevin Kofler <kevin.kofler@chello.at>
 *
 *   Calamares is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Calamares is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Calamares. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PACSTRAPCPPJOB_H
#define PACSTRAPCPPJOB_H

#include <QObject>
#include <QVariantMap>

#include <CppJob.h>

#include <utils/PluginFactory.h>

#include <PluginDllMacro.h>

class PLUGINDLLEXPORT PacstrapCppJob : public Calamares::CppJob
{
    Q_OBJECT

public:
    explicit PacstrapCppJob( QObject* parent = nullptr );
    virtual ~PacstrapCppJob();

    QString prettyName() const override;
    QString prettyStatusMessage() const override;
    Calamares::JobResult exec() override;

    void setConfigurationMap( const QVariantMap& configurationMap ) override;


private:

    void setTargetDevice();
    static QString packageListToString( const QVariantList& package_list );


    QVariantMap m_configurationMap;
    QString m_status;
};

CALAMARES_PLUGIN_FACTORY_DECLARATION( PacstrapCppJobFactory )

#endif // PACSTRAPCPPJOB_H
