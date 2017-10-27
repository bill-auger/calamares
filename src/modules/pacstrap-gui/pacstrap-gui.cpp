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


/* PacstrapGuiJob public instance methods */

PacstrapGuiJob::PacstrapGuiJob(QObject* parent) : PacStrapCppJob(tr(GUI_JOB_NAME)   ,
                                                                 tr(GUI_STATUS_MSG) ,
                                                                 GUI_JOB_WEIGHT     ,
                                                                 parent             ) {}


/* PacstrapGuiJob protected instance methods */

QString PacstrapGuiJob::loadPackageList()
{
globalStorage->insert("default-desktop", "mate") ; // TODO: per user option via globalStorage
cDebug() << QString("[PACSTRAP-GUI]: DesktopCppJob::exec() default_desktop=%1").arg(globalStorage->value("default-desktop").toString()) ;

  QString desktop = this->globalStorage->value("default-desktop").toString() ;
  this->packages  = QListToString(this->config.value("applications").toList() +
                                  this->config.value("multimedia"  ).toList() +
                                  this->config.value("network"     ).toList() +
                                  this->config.value("themes"      ).toList() +
                                  this->config.value("utilities"   ).toList() +
                                  this->config.value("xserver"     ).toList() +
                                  this->config.value(desktop       ).toList() ) ;
}

QString PacstrapGuiJob::chrootExec()
{
  QString pacstrap_cmd  = QString("/bin/sh -c \"pacstrap -c -C %1 %2 %3\"").arg(this->confFile , MOUNTPOINT , packages) ;
  QString wallpaper_cmd = QString("/bin/sh -c \"cp /etc/wallpaper.png %1/etc/\"").arg(MOUNTPOINT) ;

  if (QProcess::execute(pacstrap_cmd)) return "PACSTRAP_FAIL" ;

cDebug() << QString("[PACSTRAP-GUI]: ls /etc/skel") ;                QProcess::execute(QString("/bin/sh -c \"ls -al /etc/skel/\""         )               ) ;
cDebug() << QString("[PACSTRAP-GUI]: ls chroot/etc/skel/") ;         QProcess::execute(QString("/bin/sh -c \"ls -al %1/etc/skel/\""       ).arg(MOUNTPOINT)) ;
cDebug() << QString("[PACSTRAP-GUI]: ls chroot/etc/wallpaper.png") ; QProcess::execute(QString("/bin/sh -c \"ls -al %1/etc/wallpaper.png\"").arg(MOUNTPOINT)) ;
cDebug() << QString("[PACSTRAP-GUI]: ls chroot/etc/sudoers*") ;      QProcess::execute(QString("/bin/sh -c \"ls -al %1/etc/sudoers*\""    ).arg(MOUNTPOINT)) ;

  QProcess::execute(wallpaper_cmd) ;

cDebug() << QString("[PACSTRAP-GUI]: ls chroot/etc/wallpaper.png") ; QProcess::execute(QString("/bin/sh -c \"ls -al %1/etc/wallpaper.png\"").arg(MOUNTPOINT)) ;

  return QString("") ;
}


CALAMARES_PLUGIN_FACTORY_DEFINITION(PacstrapGuiJobFactory , registerPlugin<PacstrapGuiJob>() ;)
