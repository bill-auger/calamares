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

#include "desktop.h"

#include <QProcess>
#include <QDateTime>
#include <QThread>

#include "CalamaresVersion.h"
#include "JobQueue.h"
#include "GlobalStorage.h"

#include "utils/Logger.h"

DesktopCppJob::DesktopCppJob( QObject* parent )
    : Calamares::CppJob( parent )
{
}


DesktopCppJob::~DesktopCppJob()
{
}


QString
DesktopCppJob::prettyName() const
{
    return tr( "Desktop C++ Job" );
}


QString
DesktopCppJob::prettyStatusMessage() const
{
    return m_status;
}


Calamares::JobResult
DesktopCppJob::exec()
{
    m_status = tr( "Installing graphical desktop environment" ); emit progress( 1 );

    Calamares::GlobalStorage *globalStorage = Calamares::JobQueue::instance()->globalStorage();
    QString target_device = globalStorage->value("target-device").toString();
    QString mountpoint = "/tmp/pacstrap";

    if (target_device.isEmpty())
        return Calamares::JobResult::error("Target device for root filesystem is unspecified.");

cDebug() << QString("[DESKTOPCPP]: DesktopCppJob::exec() default_desktop=%1").arg(globalStorage->value("default-desktop").toString());

globalStorage->insert( "default-desktop", "mate" ); // TODO: per user option via globalStorage

    QString default_desktop = globalStorage->value("default-desktop").toString();
    QVariantList package_list = m_configurationMap.value("xserver").toList() +
                                m_configurationMap.value("applications").toList() +
                                m_configurationMap.value("utilities").toList() +
                                m_configurationMap.value("multimedia").toList() +
                                m_configurationMap.value("network").toList() +
                                m_configurationMap.value(default_desktop).toList();
    QString packages = packageListToString(package_list) ;

    QString mount_cmd = QString( "/bin/sh -c \"mount %1 %2\"" ).arg( target_device, mountpoint );
    QString pacstrap_cmd = QString( "/bin/sh -c \"pacstrap -c %1 %2\"" ).arg( mountpoint, packages );
    QString wallpaper_cmd = QString( "/bin/sh -c \"cp /etc/wallpaper.png %1/etc/\"" ).arg( mountpoint );
    QString umount_cmd = QString( "/bin/sh -c \"umount %1\"" ).arg( target_device );

    // boot-strap install graphical desktop
    QProcess::execute( mount_cmd );
    QProcess::execute( pacstrap_cmd );

cDebug() << QString( "[DESKTOPCPP]: ls skel" );        QProcess::execute( QString( "/bin/sh -c \"ls -al /etc/skel/\"" ) );
cDebug() << QString( "[DESKTOPCPP]: ls chroot/skel" ); QProcess::execute( QString( "/bin/sh -c \"ls -al %1/etc/skel/\"" ).arg( mountpoint ) );
cDebug() << QString( "[DESKTOPCPP]: ls wallpaper" );   QProcess::execute( QString( "/bin/sh -c \"ls -al %1/etc/wallpaper.png\"" ).arg( mountpoint ) );
cDebug() << QString( "[DESKTOPCPP]: ls sudoers" );     QProcess::execute( QString( "/bin/sh -c \"ls -al %1/etc/sudoers*\"" ).arg( mountpoint ) );

    QProcess::execute( wallpaper_cmd );
    QProcess::execute( umount_cmd );

    emit progress( 10 );

    return Calamares::JobResult::ok();
}


void
DesktopCppJob::setConfigurationMap( const QVariantMap& configurationMap )
{
  m_configurationMap = configurationMap;
}


QString
DesktopCppJob::packageListToString( const QVariantList& package_list )
{
    QStringList result;
    for ( const QVariant& package : package_list )
        result.append( package.toString() );

    return result.join(' ');
}


CALAMARES_PLUGIN_FACTORY_DEFINITION( DesktopCppJobFactory, registerPlugin<DesktopCppJob>(); )
