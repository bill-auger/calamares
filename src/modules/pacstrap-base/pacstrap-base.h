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

#ifndef PACSTRAP_BASE_H
#define PACSTRAP_BASE_H

#include <PacstrapCppJob.h>
#include <PluginDllMacro.h>
#include <utils/PluginFactory.h>


class PLUGINDLLEXPORT PacstrapBaseJob : public PacstrapCppJob
{
  Q_OBJECT


public:

  explicit PacstrapBaseJob(QObject* parent = nullptr) ;


protected:

  QString getPackageList       () override ;
  QString chrootExecPostInstall() override ;


private:

//   static const QString GRUB_CRYPTO_FMT ;
  static const QString GRUB_THEME_FMT ;
//   static const QString GRUB_CRYPTO_ERROR_MSG ;
  static const QString GRUB_THEME_ERROR_MSG ;
} ;


CALAMARES_PLUGIN_FACTORY_DECLARATION(PacstrapBaseJobFactory)

#endif // PACSTRAP_BASE_H
