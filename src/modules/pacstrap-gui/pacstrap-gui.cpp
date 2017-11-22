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
const QString PacstrapGuiJob::GET_XKBMAP_CMD  = "grep XKBMAP= ~/.codecheck 2> /dev/null | cut -d '=' -f 2" ;
const QString PacstrapGuiJob::SKEL_DIR        = "/usr/share/calamares/skel" ;
const QString PacstrapGuiJob::CHROOT_SKEL_DIR = QString("%1/etc/skel" ).arg(MOUNTPOINT) ;
const QString PacstrapGuiJob::SKEL_FMT        = "cp -rT %1/ %2/" ;
const QString PacstrapGuiJob::SET_LANG_FMT    = "sed -i 's/^export LANG=.*/export LANG=%1/' %2/.bashrc" ;
const QString PacstrapGuiJob::SET_XKBMAP_FMT  = "sed -i 's/^setxkbmap.*/setxkbmap %1/'      %2/.bashrc" ;
const QString PacstrapGuiJob::DM_DESKTOP_FMT  = "sed -i 's/^Session=.*/Session=%1/'         %2/.dmrc" ;
const QString PacstrapGuiJob::DM_LANG_FMT     = "sed -i 's/^Language=.*/Language=%1/'       %2/.dmrc" ;


/* PacstrapGuiJob public instance methods */

PacstrapGuiJob::PacstrapGuiJob(QObject* parent) : PacstrapCppJob(tr(GUI_JOB_NAME)   ,
                                                                 tr(GUI_STATUS_MSG) ,
                                                                 GUI_JOB_WEIGHT     ,
                                                                 parent             ) {}


/* PacstrapGuiJob protected getters/setters */

QString PacstrapGuiJob::getPackageList()
{
  QString     init_key       = this->globalStorage->value(GS::INITSYSTEM_KEY).toString() ;
  QString     desktop_key    = this->globalStorage->value(GS::DESKTOP_KEY   ).toString() ;
  QString     locale         = this->globalStorage->value(GS::LOCALE_KEY    ).toMap()
                                                   .value(GS::LANG_KEY      ).toString() ;
  QStringList language_packs = LANGUAGE_PACKS.values(locale) ;

DEBUG_TRACE_DESKTOPPACKAGES

  return (this->localStorage.value(APPLICATIONS_PACKAGES_KEY).toStringList() +
          this->localStorage.value(MULTIMEDIA_PACKAGES_KEY  ).toStringList() +
          this->localStorage.value(NETWORK_PACKAGES_KEY     ).toStringList() +
          this->localStorage.value(THEMES_PACKAGES_KEY      ).toStringList() +
          this->localStorage.value(UTILITIES_PACKAGES_KEY   ).toStringList() +
          this->localStorage.value(XSERVER_PACKAGES_KEY     ).toStringList() +
          this->localStorage.value(init_key                 ).toStringList() +
          this->localStorage.value(desktop_key              ).toStringList() +
          language_packs                                                     ).join(' ') ;
}


/* PacstrapGuiJob protected instance methods */

QString PacstrapGuiJob::chrootExec()
{
  QString pacstrap_cmd    = PACSTRAP_FMT .arg(this->confFile , MOUNTPOINT , packages) ;
  QString wallpaper_cmd   = WALLPAPER_FMT.arg(MOUNTPOINT) ;
  QString default_desktop = this->globalStorage->value(GS::DESKTOP_KEY).toString() ;
  QString locale          = this->globalStorage->value(GS::LOCALE_KEY).toMap().value(GS::LANG_KEY).toString() ;
  QString xkbmap          = execOutput(GET_XKBMAP_CMD) ; if (xkbmap.isEmpty()) xkbmap = "us" ;
  QString skel_cmd        = QString(SKEL_FMT      ).arg(SKEL_DIR        , CHROOT_SKEL_DIR) ;
  QString set_lang_cmd    = QString(SET_LANG_FMT  ).arg(locale          , CHROOT_SKEL_DIR) ;
  QString set_xkbmap_cmd  = QString(SET_XKBMAP_FMT).arg(xkbmap          , CHROOT_SKEL_DIR) ;
  QString dm_desktop_cmd  = QString(DM_DESKTOP_FMT).arg(default_desktop , CHROOT_SKEL_DIR) ;
  QString dm_lang_cmd     = QString(DM_LANG_FMT   ).arg(locale          , CHROOT_SKEL_DIR) ;

  if (!!execStatus(pacstrap_cmd , CHROOT_TASK_PROPORTION)) return PACSTRAP_ERROR_MSG ;

printf("[PACSTRAP-GUI]: ls /etc/skel\n") ;                QProcess::execute(QString("/bin/sh -c \"ls -al /etc/skel/\""         )               ) ;
printf("[PACSTRAP-GUI]: ls chroot/etc/skel/\n") ;         QProcess::execute(QString("/bin/sh -c \"ls -al %1/etc/skel/\""       ).arg(MOUNTPOINT)) ;
printf("[PACSTRAP-GUI]: ls chroot/etc/wallpaper.png\n") ; QProcess::execute(QString("/bin/sh -c \"ls -al %1/etc/wallpaper.png\"").arg(MOUNTPOINT)) ;
printf("[PACSTRAP-GUI]: ls chroot/etc/sudoers*\n") ;      QProcess::execute(QString("/bin/sh -c \"ls -al %1/etc/sudoers*\""    ).arg(MOUNTPOINT)) ;

  if (!!execStatus(wallpaper_cmd)) return WALLPAPER_ERROR_MSG ;

printf("[PACSTRAP-GUI]: ls chroot/etc/wallpaper.png\n") ; QProcess::execute(QString("/bin/sh -c \"ls -al %1/etc/wallpaper.png\"").arg(MOUNTPOINT)) ;

printf("[PACSTRAP-GUI]: default_desktop=%s\n" , default_desktop) ;
printf("[PACSTRAP-GUI]: locale=%s\n" , locale) ;
printf("[PACSTRAP-GUI]: CHROOT_SKEL_DIR=%s\n" , CHROOT_SKEL_DIR) ;
printf("[PACSTRAP-GUI]: ls -al chroot/etc/skel/  IN\n%s\n" , execOutput("ls -al /tmp/pacstrap/etc/skel")) ;

  if (!!execStatus(skel_cmd                             )) return "SKEL_FMT ERROR_MSG" ;
printf("[PACSTRAP-GUI]: ls -al chroot/etc/skel/ OUT\n%s\n" , execOutput("ls -al /tmp/pacstrap/etc/skel")) ;
  if (!!execStatus(dm_desktop_cmd                       )) return "DM_DESKTOP_FMT ERROR_MSG" ;
  if (!!execStatus(dm_lang_cmd                          )) return "DM_LANG_FMT ERROR_MSG" ;
  if (!!execStatus(set_lang_cmd                         )) return "SET_LANG_FMT ERROR_MSG" ;
  if (!!execStatus(set_xkbmap_cmd                       )) return "SET_XKBMAP_FMT ERROR_MSG" ;

  return QString("") ;
}


CALAMARES_PLUGIN_FACTORY_DEFINITION(PacstrapGuiJobFactory , registerPlugin<PacstrapGuiJob>() ;)
