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


/* PacstrapBaseJob private class constants */

// const QString PacstrapBaseJob::GRUB_CRYPTO_FMT       = "sed -i 's|^#GRUB_ENABLE_CRYPTODISK=y|GRUB_ENABLE_CRYPTODISK=y|'                 %1/etc/default/grub" ;
const QString PacstrapBaseJob::GRUB_THEME_FMT        = "sed -i 's|^#GRUB_THEME=.*|GRUB_THEME=/boot/grub/themes/parabola-laf/theme.txt|' %1/etc/default/grub" ;
// const QString PacstrapBaseJob::GRUB_CRYPTO_ERROR_MSG = "The grub enable crypto command has failed." ;
const QString PacstrapBaseJob::GRUB_THEME_ERROR_MSG  = "The grub define theme command has failed." ;


/* PacstrapBaseJob public instance methods */

PacstrapBaseJob::PacstrapBaseJob(QObject* parent) : PacstrapCppJob(BASE_JOB_NAME   ,
                                                                   BASE_STATUS_MSG ,
                                                                   BASE_JOB_WEIGHT ,
                                                                   parent          ) {}


/* PacstrapBaseJob protected getters/setters */

QString PacstrapBaseJob::getPackageList()
{
  QString init_key = this->globalStorage->value(GS::INITSYSTEM_KEY).toString() ;

  return (this->localStorage.value(BOOTLODER_PACKAGES_KEY).toStringList() +
          this->localStorage.value(KERNEL_PACKAGES_KEY   ).toStringList() +
          this->localStorage.value(init_key              ).toStringList() ).join(' ') ;
}


/* PacstrapBaseJob protected instance methods */

QString PacstrapBaseJob::chrootExec()
{
//   QString grub_crypto_cmd = GRUB_CRYPTO_FMT.arg(this->mountPoint) ;
  QString grub_theme_cmd  = GRUB_THEME_FMT .arg(this->mountPoint) ;

//   if (!!execStatus(grub_crypto_cmd)) return GRUB_CRYPTO_ERROR_MSG ;
  if (!!execStatus(grub_theme_cmd )) return GRUB_THEME_ERROR_MSG ;

  return QString("") ;
}


CALAMARES_PLUGIN_FACTORY_DEFINITION(PacstrapBaseJobFactory , registerPlugin<PacstrapBaseJob>() ;)
