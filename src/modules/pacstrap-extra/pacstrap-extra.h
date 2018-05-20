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

#ifndef PACSTRAP_EXTRA_H
#define PACSTRAP_EXTRA_H

#include <PacstrapCppJob.h>
#include <PluginDllMacro.h>
#include <utils/PluginFactory.h>


class PLUGINDLLEXPORT PacstrapExtraJob : public PacstrapCppJob
{
  Q_OBJECT


public:

  explicit PacstrapExtraJob(QObject* parent = nullptr) ;


protected:

  QVariantList getPackagesOps       () override ;
  QVariantList getPreScripts        () override ;
  QString      getPackageList       () override ;
  QVariantList getPostScripts       () override ;
  QString      getPackageList       () override ;
  QString      chrootExecPreInstall () override ;
  QString      chrootExecPostInstall() override ;
} ;


CALAMARES_PLUGIN_FACTORY_DECLARATION(PacstrapExtraJobFactory)

#endif // PACSTRAP_EXTRA_H
