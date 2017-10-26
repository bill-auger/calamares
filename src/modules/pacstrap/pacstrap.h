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

#include <QDir>
#include <QObject>
#include <QTimerEvent>
#include <QVariantMap>

#include <CppJob.h>
#include "GlobalStorage.h"
#include <utils/PluginFactory.h>
#include <PluginDllMacro.h>


class PLUGINDLLEXPORT PacstrapCppJob : public Calamares::CppJob
{
    Q_OBJECT

public:

    explicit PacstrapCppJob(QObject* parent = nullptr) ;
    virtual ~PacstrapCppJob() ;

    void                 setConfigurationMap(const QVariantMap& config)       override ;
    qreal                jobWeight          ()                          const override ;
    QString              prettyName         ()                          const override ;
    QString              prettyStatusMessage()                          const override ;
    Calamares::JobResult exec               ()                                override ;



protected:

    void timerEvent    (QTimerEvent* event) override ;
    void updateProgress() ;


private:

    static QString     QListToString (const QVariantList& package_list) ;
    static QStringList ExecWithOutput(QString command_line) ;

    void   setTargetDevice   () ;
    qint16 nPackagesInstalled() ;
    qint16 setNPackages      (QString n_packages_cmd) ;

    static const qreal   BASE_PROGRESS_PERCENT = 30.0 ;
    static const qreal   GUI_PROGRESS_PERCENT  = 50.0 ;
    static const QString MOUNTPOINT ;
    static const QDir    PACKAGES_CACHE_DIR ;
    static const QDir    PACKAGES_METADATA_DIR ;

    Calamares::GlobalStorage* globalStorage ;
    int                       guiTimerId ;
//     QDir                      packagesCacheDir ;
//     QDir                      packagesMetadataDir ;
//     int                       guiTimerId = 0 ;
    QString                   statusMsg ;
    QVariantMap               config ;
    qreal                     maxProgressPercent ;
    qint16                    nPackages  = 0 ;
} ;


CALAMARES_PLUGIN_FACTORY_DECLARATION(PacstrapCppJobFactory)

#endif // PACSTRAPCPPJOB_H
