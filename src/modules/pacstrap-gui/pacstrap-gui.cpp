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

#include <QProcess>

#include "pacstrap-gui.h"


/* PacstrapGuiJob private class constants */

const QString PacstrapGuiJob::WALLPAPER_FMT       = "cp /etc/wallpaper.png %1/etc/" ;
const QString PacstrapGuiJob::WALLPAPER_ERROR_MSG = "The wallpaper installation command has failed." ;


/* PacstrapGuiJob public instance methods */

PacstrapGuiJob::PacstrapGuiJob(QObject* parent) : PacstrapCppJob(tr(GUI_JOB_NAME)   ,
                                                                 tr(GUI_STATUS_MSG) ,
                                                                 GUI_JOB_WEIGHT     ,
                                                                 parent             ) {}


/* PacstrapGuiJob protected getters/setters */

QString PacstrapGuiJob::getPackageList()
{
globalStorage->insert(DESKTOP_PACKAGES_KEY , MATE_PACKAGES_KEY) ; // TODO: per user option via globalStorage
printf("[PACSTRAP-GUI]: DesktopCppJob::getPackageList() default_desktop=%s" , globalStorage->value(DESKTOP_PACKAGES_KEY).toString().toStdString().c_str()) ;

  QString desktop = this->globalStorage->value(DESKTOP_PACKAGES_KEY).toString() ;

  return QListToString(this->localStorage.value(APPLICATIONS_PACKAGES_KEY).toList() +
                       this->localStorage.value(MULTIMEDIA_PACKAGES_KEY  ).toList() +
                       this->localStorage.value(NETWORK_PACKAGES_KEY     ).toList() +
                       this->localStorage.value(THEMES_PACKAGES_KEY      ).toList() +
                       this->localStorage.value(UTILITIES_PACKAGES_KEY   ).toList() +
                       this->localStorage.value(XSERVER_PACKAGES_KEY     ).toList() +
                       this->localStorage.value(desktop                  ).toList() ) ;
}


/* PacstrapGuiJob protected instance methods */

QString PacstrapGuiJob::chrootExec()
{
  QString pacstrap_cmd  = PACSTRAP_FMT .arg(this->confFile , MOUNTPOINT , packages) ;
  QString wallpaper_cmd = WALLPAPER_FMT.arg(MOUNTPOINT) ;

  if (!!ExecWithStatus(pacstrap_cmd)) return PACSTRAP_ERROR_MSG ;

printf("[PACSTRAP-GUI]: ls /etc/skel") ;                QProcess::execute(QString("/bin/sh -c \"ls -al /etc/skel/\""         )               ) ;
printf("[PACSTRAP-GUI]: ls chroot/etc/skel/") ;         QProcess::execute(QString("/bin/sh -c \"ls -al %1/etc/skel/\""       ).arg(MOUNTPOINT)) ;
printf("[PACSTRAP-GUI]: ls chroot/etc/wallpaper.png") ; QProcess::execute(QString("/bin/sh -c \"ls -al %1/etc/wallpaper.png\"").arg(MOUNTPOINT)) ;
printf("[PACSTRAP-GUI]: ls chroot/etc/sudoers*") ;      QProcess::execute(QString("/bin/sh -c \"ls -al %1/etc/sudoers*\""    ).arg(MOUNTPOINT)) ;

  if (!!ExecWithStatus(wallpaper_cmd)) return WALLPAPER_ERROR_MSG ;

printf("[PACSTRAP-GUI]: ls chroot/etc/wallpaper.png") ; QProcess::execute(QString("/bin/sh -c \"ls -al %1/etc/wallpaper.png\"").arg(MOUNTPOINT)) ;

  return QString("") ;
}


CALAMARES_PLUGIN_FACTORY_DEFINITION(PacstrapGuiJobFactory , registerPlugin<PacstrapGuiJob>() ;)
