/* === This file is part of Calamares - <http://github.com/calamares> ===
 *
 *   Copyright 2014, Teo Mrnjavac <teo@kde.org> (original dummypython code)
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

#include "pacstrap.h"

#include <QProcess>
#include <QDateTime>
#include <QThread>

#include "CalamaresVersion.h"
#include "JobQueue.h"
#include "GlobalStorage.h"

#include "utils/Logger.h"

PacstrapCppJob::PacstrapCppJob( QObject* parent )
    : Calamares::CppJob( parent )
{
}


PacstrapCppJob::~PacstrapCppJob()
{
}


QString
PacstrapCppJob::prettyName() const
{
    return tr( "Pacstrap C++ Job" );
}


QString
PacstrapCppJob::prettyStatusMessage() const
{
    return m_status;
}


Calamares::JobResult
PacstrapCppJob::exec()
{
// Calamares::GlobalStorage *gs = Calamares::JobQueue::instance()->globalStorage();
// foreach (const QString &key, gs->keys())
//   cDebug() << QString("[PACSTRAPCPP]: globalStorage[%1]=%2\n").arg( key, gs->value( key ).toString() );

// QProcess::execute( "echo SKIPPING parabola-prepare.desc" );

    setTargetDevice();

    Calamares::GlobalStorage *globalStorage = Calamares::JobQueue::instance()->globalStorage();
    QString target_device = globalStorage->value("target-device").toString();
    QString mountpoint = "/tmp/pacstrap";

    if (target_device == "")
        return Calamares::JobResult::error("Target device for root filesystem is unspecified.");


    QString keyring_cmd = "/bin/sh -c \"pacman -Sy --noconfirm parabola-keyring && \
                                        pacman-key --populate parabola          && \
                                        pacman-key --refresh-keys                  \"";
    QString mkdir_cmd = QString( "/bin/sh -c \"mkdir %1\"" ).arg( mountpoint );
    QString mount_cmd = QString( "/bin/sh -c \"mount %1 %2\"" ).arg( target_device, mountpoint );
    QString pacstrap_cmd = QString( "/bin/sh -c \"pacstrap -c %1 base\"" ).arg( mountpoint );
    QString kernel_cmd = QString( "/bin/sh -c \"pacstrap -c %1 linux-libre sudo\"" ).arg( mountpoint );
    QString umount_cmd = QString( "/bin/sh -c \"umount %1\"" ).arg( target_device );

cDebug() << QString("[PACSTRAPCPP]: pacstrap_cmd=%1").arg(pacstrap_cmd);
// QProcess::execute( "/bin/sh -c \"ls /tmp/\"" );

    // boot-strap install root filesystem
    m_status = tr( "Bootstrapping root filesystem" ); emit progress( 1 );
//     QProcess::execute( keyring_cmd );
    QProcess::execute( mkdir_cmd );
    QProcess::execute( mount_cmd );
    QProcess::execute( pacstrap_cmd );
    m_status = tr( "Installing linux-libre kernel" ); emit progress( 5 );
//     QProcess::execute( kernel_cmd );
    QProcess::execute( umount_cmd );

    emit progress( 1 );

    return Calamares::JobResult::ok();
// return Calamares::JobResult::error("just cuz");
}


void
PacstrapCppJob::setConfigurationMap( const QVariantMap& configurationMap )
{
  m_configurationMap = configurationMap;
}


void
PacstrapCppJob::setTargetDevice()
{
    Calamares::GlobalStorage *globalStorage = Calamares::JobQueue::instance()->globalStorage();
    QString target_device = "";
    QVariantList partitions = globalStorage->value( "partitions" ).toList() ;

    // locate target device for root filesystem
    foreach (const QVariant &partition, partitions)
    {
QStringList result; for ( auto it = partition.toMap().constBegin(); it != partition.toMap().constEnd(); ++it ) result.append( it.key() + '=' + it.value().toString() );
cDebug() << QString("[DESKTOPCPP]: partition=%1").arg('[' + result.join(',') + ']');

        QVariantMap partition_map = partition.toMap();
        QString device = partition_map.value("device").toString();
        QString fs = partition_map.value("fs").toString();
        QString mount_point = partition_map.value("mountPoint").toString();
        QString uuid = partition_map.value("uuid").toString();

        if (mount_point == "/")
            target_device = device;

if (mount_point == "/") cDebug() << QString("[PACSTRAPCPP]: target_device=%1").arg(device);
    }

    globalStorage->insert( "target-device", target_device );
}


CALAMARES_PLUGIN_FACTORY_DEFINITION( PacstrapCppJobFactory, registerPlugin<PacstrapCppJob>(); )
