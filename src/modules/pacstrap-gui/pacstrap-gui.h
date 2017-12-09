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

#ifndef PACSTRAP_GUI_H
#define PACSTRAP_GUI_H

#include <PacstrapCppJob.h>
#include <PluginDllMacro.h>
#include <utils/PluginFactory.h>


class PLUGINDLLEXPORT PacstrapGuiJob : public PacstrapCppJob
{
  Q_OBJECT


public:

  explicit PacstrapGuiJob(QObject* parent = nullptr) ;


protected:

  QString getPackageList() override ;
  QString chrootExec    () override ;


private:

//   static const QString WALLPAPER_FMT ;
//   static const QString WALLPAPER_ERROR_MSG ;
  static const QString GET_XKBMAP_CMD ;
  static const QString SKEL_DIR ;
//   static const QString CHROOT_SKEL_DIR ;
//   static const QString SKEL_FMT ;
//   static const QString SET_PROMPT_FMT ;
  static const QString SET_EDITOR_FMT ;
  static const QString SET_LANG_FMT ;
  static const QString SET_XKBMAP_FMT ;
  static const QString DM_DESKTOP_FMT ;
  static const QString DM_LANG_FMT ;
  static const QString WELCOME_TITLE_FMT ;
  static const QString OCTOPI_FMT ;
} ;


CALAMARES_PLUGIN_FACTORY_DECLARATION(PacstrapGuiJobFactory)

#endif // PACSTRAP_GUI_H
