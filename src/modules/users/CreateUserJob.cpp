/* === This file is part of Calamares - <http://github.com/calamares> ===
 *
 *   Copyright 2014-2016, Teo Mrnjavac <teo@kde.org>
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

#include <CreateUserJob.h>

#include "JobQueue.h"
#include "GlobalStorage.h"
#include "utils/Logger.h"
#include "utils/CalamaresUtilsSystem.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>


CreateUserJob::CreateUserJob( const QString& userName,
                              const QString& fullName,
                              bool autologin,
                              const QStringList& defaultGroups )
    : Calamares::Job()
    , m_userName( userName )
    , m_fullName( fullName )
    , m_autologin( autologin )
    , m_defaultGroups( defaultGroups )
{
}


QString
CreateUserJob::prettyName() const
{
    return tr( "Create user %1" ).arg( m_userName );
}


QString
CreateUserJob::prettyDescription() const
{
    return tr( "Create user <strong>%1</strong>." ).arg( m_userName );
}


QString
CreateUserJob::prettyStatusMessage() const
{
    return tr( "Creating user %1." ).arg( m_userName );
}


Calamares::JobResult
CreateUserJob::exec()
{
    Calamares::GlobalStorage* gs = Calamares::JobQueue::instance()->globalStorage();
    QDir destDir( gs->value( "rootMountPoint" ).toString() );

    if ( gs->contains( "sudoersGroup" ) &&
         !gs->value( "sudoersGroup" ).toString().isEmpty() )
    {
        QFileInfo sudoersFi( destDir.absoluteFilePath( "etc/sudoers.d/10-installer" ) );

cDebug() << QString("CreateUserJob::exec() sudoersFi=%1").arg(sudoersFi.filePath());
cDebug() << QString("CreateUserJob::exec() isAbsolute=%2").arg(sudoersFi.isAbsolute());
cDebug() << QString("CreateUserJob::exec() isWritable=%3").arg(sudoersFi.isWritable());
QString etcdir = gs->value( "rootMountPoint" ).toString() + "/etc" ;
QProcess::execute( QString( "/bin/sh -c \"ls -l %1\"" ).arg( etcdir ) );
QProcess::execute( QString( "/bin/sh -c \"ls -l %1\"" ).arg( sudoersFi.filePath() ) );

        if ( !sudoersFi.absoluteDir().exists() )
            return Calamares::JobResult::error( tr( "Sudoers dir is not writable." ) );

        QFile sudoersFile( sudoersFi.absoluteFilePath() );
        if (!sudoersFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
            return Calamares::JobResult::error( tr( "Cannot create sudoers file for writing." ) );

        QString sudoersGroup = gs->value( "sudoersGroup" ).toString();

        QTextStream sudoersOut( &sudoersFile );
        sudoersOut << QString( "%%1 ALL=(ALL) ALL\n" ).arg( sudoersGroup );

        if ( QProcess::execute( "chmod", { "440", sudoersFi.absoluteFilePath() } ) )
            return Calamares::JobResult::error( tr( "Cannot chmod sudoers file." ) );
    }

    QFileInfo groupsFi( destDir.absoluteFilePath( "etc/group" ) );
    QFile groupsFile( groupsFi.absoluteFilePath() );
    if ( !groupsFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
        return Calamares::JobResult::error( tr( "Cannot open groups file for reading." ) );
    QString groupsData = QString::fromLocal8Bit( groupsFile.readAll() );
    QStringList groupsLines = groupsData.split( '\n' );
    for ( QStringList::iterator it = groupsLines.begin();
          it != groupsLines.end(); ++it )
    {
        int indexOfFirstToDrop = it->indexOf( ':' );
        it->truncate( indexOfFirstToDrop );
    }

    foreach ( const QString& group, m_defaultGroups )
        if ( !groupsLines.contains( group ) )
            CalamaresUtils::System::instance()->
                    targetEnvCall( { "groupadd", group } );

    QString defaultGroups = m_defaultGroups.join( ',' );
    if ( m_autologin )
    {
        QString autologinGroup;
        if ( gs->contains( "autologinGroup" ) &&
                !gs->value( "autologinGroup" ).toString().isEmpty() )
        {
            autologinGroup = gs->value( "autologinGroup" ).toString();
            CalamaresUtils::System::instance()->targetEnvCall( { "groupadd", autologinGroup } );
            defaultGroups.append( QString( ",%1" ).arg( autologinGroup ) );
        }
    }

    // If we're looking to reuse the contents of an existing /home
    if ( gs->value( "reuseHome" ).toBool() )
    {
        QString shellFriendlyHome = "/home/" + m_userName;
        QDir existingHome( destDir.absolutePath() + shellFriendlyHome );
        if ( existingHome.exists() )
        {
            QString backupDirName = "dotfiles_backup_" +
                                    QDateTime::currentDateTime()
                                        .toString( "yyyy-MM-dd_HH-mm-ss" );
            existingHome.mkdir( backupDirName );

            CalamaresUtils::System::instance()->
                    targetEnvCall( { "sh",
                                     "-c",
                                     "mv -f " +
                                        shellFriendlyHome + "/.* " +
                                        shellFriendlyHome + "/" +
                                        backupDirName
                                   } );
        }
    }

    int ec = CalamaresUtils::System::instance()->
             targetEnvCall( { "useradd",
                              "-m",
                              "-s",
                              "/bin/bash",
                              "-U",
                              "-c",
                              m_fullName,
                              m_userName } );
    if ( ec )
        return Calamares::JobResult::error( tr( "Cannot create user %1." )
                                                .arg( m_userName ),
                                            tr( "useradd terminated with error code %1." )
                                                .arg( ec ) );

    ec = CalamaresUtils::System::instance()->
             targetEnvCall( { "usermod",
                              "-aG",
                              defaultGroups,
                              m_userName } );
    if ( ec )
        return Calamares::JobResult::error( tr( "Cannot add user %1 to groups: %2." )
                                                .arg( m_userName )
                                                .arg( defaultGroups ),
                                            tr( "usermod terminated with error code %1." )
                                                .arg( ec ) );

    ec = CalamaresUtils::System::instance()->
                      targetEnvCall( { "chown",
                                       "-R",
                                       QString( "%1:%2" ).arg( m_userName )
                                                         .arg( m_userName ),
                                       QString( "/home/%1" ).arg( m_userName ) } );
    if ( ec )
        return Calamares::JobResult::error( tr( "Cannot set home directory ownership for user %1." )
                                                .arg( m_userName ),
                                            tr( "chown terminated with error code %1." )
                                                .arg( ec ) );


    /* parabola-specific configuration */

//    Calamares::GlobalStorage* globalStorage = Calamares::JobQueue::instance()->globalStorage();
    QString default_desktop = gs->value("default-desktop").toString();
/*
    if [ -x /usr/bin/setxkbmap ]; then
        echo "setxkbmap $(cat /.codecheck | grep XKBMAP= | cut -d '=' -f 2)" >> /home/${user#*=}/.bashrc
    fi
*/

cDebug() << QString("CreateUserJob::exec() default_desktop=%1").arg(default_desktop);

    if (default_desktop == "mate")
    {
/*
        QVariantList commands = m_configurationMap.value("gsettings-commands").toList();

        for ( const QVariant& command : commands )
          CalamaresUtils::System::instance()->targetEnvCall( { "sh", "-c", "sudo -u " + m_userName + command.toString() } );
*/
        CalamaresUtils::System::instance()->targetEnvCall( { "sh", "-c", "sudo -u " + m_userName + 
            "gsettings set org.mate.interface gtk-theme 'Radiance-Purple'" } );
        CalamaresUtils::System::instance()->targetEnvCall( { "sh", "-c", "sudo -u " + m_userName +
            "gsettings set org.mate.Marco.general theme 'Radiance-Purple'" } );
        CalamaresUtils::System::instance()->targetEnvCall( { "sh", "-c", "sudo -u " + m_userName +
            "gsettings set org.mate.interface icon-theme 'RAVE-X-Dark-Purple'" } );
        CalamaresUtils::System::instance()->targetEnvCall( { "sh", "-c", "sudo -u " + m_userName +
            "gsettings set org.mate.peripherals-mouse cursor-size '18'" } );
        CalamaresUtils::System::instance()->targetEnvCall( { "sh", "-c", "sudo -u " + m_userName +
            "gsettings set org.mate.peripherals-mouse cursor-theme 'mate'" } );
        CalamaresUtils::System::instance()->targetEnvCall( { "sh", "-c", "sudo -u " + m_userName +
            "gsettings set org.mate.background picture-filename '/etc/wallpaper.png'" } );
    }

    return Calamares::JobResult::ok();
}
