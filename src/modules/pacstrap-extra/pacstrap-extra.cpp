/* === This file is part of Calamares - <http://github.com/calamares> ===
 *
 *   Copyright 2017-2018 bill-auger <bill-auger@programmer.net>
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

#include "pacstrap-extra.h"


/* PacstrapExtraJob public instance methods */

PacstrapExtraJob::PacstrapExtraJob(QObject* parent) : PacstrapCppJob(GUI_JOB_NAME   ,
                                                                     GUI_STATUS_MSG ,
                                                                     GUI_JOB_WEIGHT ,
                                                                     parent         ) {}


/* PacstrapExtraJob protected getters/setters */

QVariantList PacstrapExtraJob::getPackagesOps()
{
  QVariantList packages_data = this->globalStorage->value(GS::PACKAGES_KEY).toList() ;

  foreach (const QVariant& packages_map , packages_data)
    if (packages_map.contains(GS::PACKAGES_NONCRITICAL_KEY))
      return packages_map.value(GS::PACKAGES_NONCRITICAL_KEY).toList() ;
}

QVariantList PacstrapExtraJob::getPreScripts()
{
  QStringList preinstall_scripts ;

  foreach (const QVariant& package_ops , getPackagesOps())
    preinstall_scripts.append(package_ops.value(PACKAGE_PRE_KEY)) ;

  return preinstall_scripts ;
}

QString PacstrapExtraJob::getPackageList()
{
  QStringList extra_packages ;

  foreach (const QVariant& package_ops , getPackagesOps())
    extra_packages.append(package_ops.value(PACKAGE_KEY)) ;

DEBUG_TRACE_EXTRAPACKAGES

  return extra_packages.join(" ") ;
}

QVariantList PacstrapExtraJob::getPostScripts()
{
  QStringList postinstall_scripts ;

  foreach (const QVariant& package_ops , getPackagesOps())
    postinstall_scripts.append(package_ops.value(PACKAGE_POST_KEY)) ;

  return postinstall_scripts ;
}


/* PacstrapExtraJob protected instance methods */

QString PacstrapExtraJob::chrootExecPreInstall()
{
printf("[PACSTRAP-EXTRA]: chrootExecPreInstall nScripts=%s\n" , getPreScripts().size) ;

  foreach (const QVariant& preinstall_script , getPreScripts())
    if (!!execStatus(preinstall_script)) return PREINST_ERROR_MSG.arg(preinstall_script) ;

  return QString("") ;
}

QString PacstrapExtraJob::chrootExecPostInstall()
{
printf("[PACSTRAP-EXTRA]: chrootExecPostInstall nScripts=%s\n" , getPostScripts().size) ;

  foreach (const QVariant& postinstall_script , getPostScripts())
    if (!!execStatus(postinstall_script)) return PREINST_ERROR_MSG.arg(postinstall_script) ;

  return QString("") ;
}


CALAMARES_PLUGIN_FACTORY_DEFINITION(PacstrapExtraJobFactory , registerPlugin<PacstrapExtraJob>() ;)
