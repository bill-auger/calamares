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


class PacstrapCppJob : public Calamares::CppJob
{
    Q_OBJECT


public:

    explicit PacstrapCppJob(QString  job_name             ,
                            QString  status_msg           ,
                            qreal    job_weight = 1.0     ,
                            QObject* parent     = nullptr ) ;
    virtual ~PacstrapCppJob() ;

    void                 setConfigurationMap(const QVariantMap& config)       override ;
    qreal                getJobWeight       ()                          const override ;
    QString              prettyName         ()                          const override ;
    QString              prettyStatusMessage()                          const override ;
    Calamares::JobResult exec               ()                                override ;


protected:

    static QString     QListToString (const QVariantList& package_list) ;
    static QStringList ExecWithOutput(QString command_line) ;

    virtual QString chrootExec() = 0 ;

    void    setTargetDevice   () ;
    QString loadPackageList   () ;
    qint16  nPackagesInstalled() ;
    qint16  setNPackages      (QString packages) ;
    void    timerEvent        (QTimerEvent* event) override ;
    void    updateProgress    () ;


    static const     QString MOUNTPOINT ;
    static const     QDir    PACKAGES_CACHE_DIR ;
    static const     QDir    PACKAGES_METADATA_DIR ;
    static const     char*   BASE_JOB_NAME ;
    static const     char*   GUI_JOB_NAME ;
    static const     char*   BASE_STATUS_MSG ;
    static const     char*   GUI_STATUS_MSG ;
    static constexpr qreal   BASE_JOB_WEIGHT = 30.0 ;
    static constexpr qreal   GUI_JOB_WEIGHT  = 50.0 ;

    QString                   jobName ;
    QString                   statusMsg ;
    qreal                     jobWeight ;
    Calamares::GlobalStorage* globalStorage ;
    int                       guiTimerId ;
    QString                   confFile ;
    QString                   packages ;
    qint16                    nPackages ;
    QVariantMap               config ;
} ;


#endif // PACSTRAPCPPJOB_H
