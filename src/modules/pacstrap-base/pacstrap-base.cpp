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

#include "pacstrap-base.h"


/* PacstrapBaseJob public instance methods */

PacstrapBaseJob::PacstrapBaseJob(QObject* parent) : PacstrapCppJob(tr(BASE_JOB_NAME)   ,
                                                                   tr(BASE_STATUS_MSG) ,
                                                                   BASE_JOB_WEIGHT     ,
                                                                   parent              ) {}


/* PacstrapBaseJob protected getters/setters */

QString PacstrapBaseJob::getPackageList()
{
  QString init_key = this->globalStorage->value(GS::INITSYSTEM_KEY).toString() ;

  return (this->localStorage.value(BASE_PACKAGES_KEY     ).toStringList() +
          this->localStorage.value(BOOTLODER_PACKAGES_KEY).toStringList() +
          this->localStorage.value(KERNEL_PACKAGES_KEY   ).toStringList() +
          this->localStorage.value(init_key              ).toStringList() ).join(' ') ;
}


/* PacstrapBaseJob protected instance methods */

QString PacstrapBaseJob::chrootExec()
{
  QString pacstrap_cmd   = PACSTRAP_FMT  .arg(this->confFile , MOUNTPOINT , packages) ;
  QString grub_theme_cmd = GRUB_THEME_FMT.arg(MOUNTPOINT) ;

  if (!!execStatus(pacstrap_cmd , CHROOT_TASK_PROPORTION)) return PACSTRAP_ERROR_MSG ;

QString grub_theme_kludge_cmd = QString("echo GRUB_THEME=/boot/grub/themes/GNUAxiom/theme.txt >> %1/etc/default/grub").arg(MOUNTPOINT) ;
printf("[PACSTRAP-BASE]: grub_theme_cmd=%s\n" , grub_theme_cmd.toStdString().c_str()) ;
printf("[PACSTRAP-BASE]: grub_theme_cmd IN:\n");  QProcess::execute(QString("/bin/sh -c \"cat %1/etc/default/grub\"").arg(MOUNTPOINT));

  if (!!execStatus(grub_theme_cmd)) return GRUB_THEME_ERROR_MSG ;

if (!!execStatus(grub_theme_kludge_cmd)) return "grub_theme_kludge_cmd failed" ;
printf("[PACSTRAP-BASE]: grub_theme_cmd OUT:\n"); QProcess::execute(QString("/bin/sh -c \"cat %1/etc/default/grub\"").arg(MOUNTPOINT));

  return QString("") ;
}


/* PacstrapBaseJob private class constants */

const QString PacstrapBaseJob::GRUB_THEME_FMT       = "sed -i 's|[#]GRUB_THEME=.*|GRUB_THEME=/boot/grub/themes/GNUAxiom/theme.txt|' %1/etc/default/grub" ;
const QString PacstrapBaseJob::GRUB_THEME_ERROR_MSG = "The grub theme installation command has failed." ;


CALAMARES_PLUGIN_FACTORY_DEFINITION(PacstrapBaseJobFactory , registerPlugin<PacstrapBaseJob>() ;)
