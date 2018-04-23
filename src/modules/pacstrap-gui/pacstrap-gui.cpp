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

// const QString PacstrapGuiJob::WALLPAPER_FMT       = "cp /etc/wallpaper.png %1/etc/" ;
// const QString PacstrapGuiJob::WALLPAPER_ERROR_MSG = "The wallpaper installation command has failed." ;
const QString PacstrapGuiJob::GET_XKBMAP_CMD    = "grep 'XKBMAP=' ~/.codecheck 2> /dev/null | cut -d '=' -f 2" ;
// const QString PacstrapGuiJob::SKEL_DIR          = "/usr/share/calamares/skel" ;
// const QString PacstrapGuiJob::CHROOT_SKEL_DIR   = QString("%1/etc/skel").arg(MOUNTPOINT) ;
// const QString PacstrapGuiJob::SKEL_FMT          = "cp -rT %1/ %2/" ;
// PS1="\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\n$ "
// const QString PacstrapGuiJob::SET_PROMPT_FMT    = "echo 'PS1=\"\\[\\033[01;32m\\]\\u@\\h\\[\\033[00m\\]:\\[\\033[01;34m\\]\\w\\[\\033[00m\\]\\n$ \"' >> %1/.bashrc" ;
const QString PacstrapGuiJob::SET_EDITOR_FMT    = "echo 'export EDITOR=nano'         >> %1/etc/skel/.bashrc" ;
// const QString PacstrapGuiJob::SET_LANG_FMT      = "echo 'LANG=%1'                    >> %2/.config/locale.conf" ;
const QString PacstrapGuiJob::SET_LANG_FMT      = "echo 'export LANG=%1'             >> %2/etc/skel/.bashrc" ;
const QString PacstrapGuiJob::SET_XKBMAP_FMT    = "echo 'setxkbmap %1'               >> %2/etc/skel/.bashrc" ;
const QString PacstrapGuiJob::DM_DESKTOP_FMT    = "sed -i 's|^Session=.*|Session=%1|'   %2/etc/skel/.dmrc" ;
const QString PacstrapGuiJob::DM_LANG_FMT       = "sed -i 's|^Language=.*|Language=%1|' %2/etc/skel/.dmrc" ;
const QString PacstrapGuiJob::WELCOME_TITLE_FMT = "sed -i 's|_EDITION_TITLE_|Parabola GNU/Linux-libre News|' %1/etc/skel/.config/autostart/autostart.sh" ;
const QString PacstrapGuiJob::WELCOME_TEXT_FMT  = "cp /usr/share/calamares/welcome %1/usr/lib/parabola-laf/news-0" ;
const QString PacstrapGuiJob::WELCOME_SED_FMT   = "sed -i 's|_DEFAULT_DESKTOP_|%1|' %2/usr/lib/parabola-laf/news-0" ;
const QString PacstrapGuiJob::OCTOPI_FMT        = "rm -f %1/etc/xdg/autostart/octopi-notifier.desktop" ;


/* PacstrapGuiJob public instance methods */

PacstrapGuiJob::PacstrapGuiJob(QObject* parent) : PacstrapCppJob(GUI_JOB_NAME   ,
                                                                 GUI_STATUS_MSG ,
                                                                 GUI_JOB_WEIGHT ,
                                                                 parent         ) {}


/* PacstrapGuiJob protected getters/setters */

QString PacstrapGuiJob::getPackageList()
{
  QString init_key    = this->globalStorage->value(GS::INITSYSTEM_KEY).toString() ;
  QString desktop_key = this->globalStorage->value(GS::DESKTOP_KEY   ).toString() ;
  QString locale      = this->globalStorage->value(GS::LOCALE_KEY    ).toMap()
                                            .value(GS::LANG_KEY      ).toString() ;

DEBUG_TRACE_DESKTOPPACKAGES

  return (this->localStorage.value (APPLICATIONS_PACKAGES_KEY).toStringList() +
          this->localStorage.value (MULTIMEDIA_PACKAGES_KEY  ).toStringList() +
          this->localStorage.value (NETWORK_PACKAGES_KEY     ).toStringList() +
          this->localStorage.value (LOOKANDFEEL_PACKAGES_KEY ).toStringList() +
          this->localStorage.value (UTILITIES_PACKAGES_KEY   ).toStringList() +
          this->localStorage.value (XSERVER_PACKAGES_KEY     ).toStringList() +
          this->localStorage.value (init_key                 ).toStringList() +
          this->localStorage.value (desktop_key              ).toStringList() +
          LANGUAGE_PACKS    .values(locale                   )                ).join(' ') ;
}


/* PacstrapGuiJob protected instance methods */

QString PacstrapGuiJob::chrootExec()
{
//   QString wallpaper_cmd     = WALLPAPER_FMT.arg(MOUNTPOINT) ;
  QString default_desktop   = this->globalStorage->value(GS::DESKTOP_KEY).toString() ;
  QString locale            = this->globalStorage->value(GS::LOCALE_KEY).toMap().value(GS::LANG_KEY).toString() ;
  QString xkbmap            = execOutput(GET_XKBMAP_CMD) ; if (xkbmap.isEmpty()) xkbmap = "us" ;
//   QString skel_cmd          = QString(SKEL_FMT         ).arg(SKEL_DIR        , CHROOT_SKEL_DIR) ;
//   QString set_prompt_cmd    = QString(SET_PROMPT_FMT   ).arg(                  CHROOT_SKEL_DIR) ;
  QString set_editor_cmd    = QString(SET_EDITOR_FMT   ).arg(                  this->mountPoint) ;
  QString set_lang_cmd      = QString(SET_LANG_FMT     ).arg(locale          , this->mountPoint) ;
  QString set_xkbmap_cmd    = QString(SET_XKBMAP_FMT   ).arg(xkbmap          , this->mountPoint) ;
  QString dm_desktop_cmd    = QString(DM_DESKTOP_FMT   ).arg(default_desktop , this->mountPoint) ;
  QString dm_lang_cmd       = QString(DM_LANG_FMT      ).arg(locale          , this->mountPoint) ;
  QString welcome_title_cmd = QString(WELCOME_TITLE_FMT).arg(                  this->mountPoint) ;
  QString welcome_text_cmd  = QString(WELCOME_TEXT_FMT ).arg(                  this->mountPoint) ;
  QString welcome_sed_cmd   = QString(WELCOME_SED_FMT  ).arg(default_desktop , this->mountPoint) ;
  QString octopi_cmd        = QString(OCTOPI_FMT       ).arg(                  this->mountPoint) ;

printf("[PACSTRAP-GUI]: ls host/etc/skel%s\n"                 , execOutput(        "ls -al /etc/skel"                )                       .toStdString().c_str()) ;
printf("[PACSTRAP-GUI]: ls -al chroot/etc/skel/  IN\n%s\n"    , execOutput(QString("ls -al %1/etc/skel"              ).arg(this->mountPoint)).toStdString().c_str()) ;
printf("[PACSTRAP-GUI]: ls chroot/usr/share/backgrounds/%s\n" , execOutput(QString("ls -al %1/usr/share/backgrounds/").arg(this->mountPoint)).toStdString().c_str()) ;
printf("[PACSTRAP-GUI]: ls chroot/etc/sudoers*%s\n"           , execOutput(QString("ls -al %1/etc/sudoers"           ).arg(this->mountPoint)).toStdString().c_str()) ;

//   if (!!execStatus(wallpaper_cmd)) return WALLPAPER_ERROR_MSG ;

// printf("[PACSTRAP-GUI]: ls chroot/etc/wallpaper.png\n") ; QProcess::execute(QString("/bin/sh -c \"ls -al %1/etc/wallpaper.png\"").arg(MOUNTPOINT)) ;

printf("[PACSTRAP-GUI]: default_desktop=%s\n"              , default_desktop                                                                                                                 .toStdString().c_str()) ;
printf("[PACSTRAP-GUI]: locale=%s\n"                       , locale                                                                                                                          .toStdString().c_str()) ;
printf("[PACSTRAP-GUI]: mountPoint=%s\n"                   , this->mountPoint                                                                                                                .toStdString().c_str()) ;
printf("[PACSTRAP-GUI]: ls -al chroot/etc/skel/ OUT\n%s\n" , execOutput(QString("ls -al %1/etc/skel"                                                                 ).arg(this->mountPoint)).toStdString().c_str()) ;
printf("[PACSTRAP-GUI]: ls -al chroot/etc/skel/ OUT\n%s\n" , execOutput(QString("echo 'touch ~/autostart-sh' >> %1/etc/skel/.config/autostart/autostart.sh"          ).arg(this->mountPoint)).toStdString().c_str()) ;
printf("[PACSTRAP-GUI]: ls -al chroot/etc/skel/ OUT\n%s\n" , execOutput(QString("echo 'touch ~/autostart-lxde-sh' >> %1/etc/skel/.config/autostart/autostart-lxde.sh").arg(this->mountPoint)).toStdString().c_str()) ;

//   if (!!execStatus(skel_cmd                             )) return "SKEL_FMT ERROR_MSG" ;
// printf("[PACSTRAP-GUI]: ls -al chroot/etc/skel/ OUT\n%s\n" , execOutput(QString("ls -al %1/etc/skel").arg(this->mountPoint))) ;
//   if (!!execStatus(set_prompt_cmd                       )) return "SET_PROMPT_FMT ERROR_MSG" ;
  if (!!execStatus(set_editor_cmd          )) return "SET_EDITOR_FMT ERROR_MSG" ;
  if (!!execStatus(set_lang_cmd            )) return "SET_LANG_FMT ERROR_MSG" ;
  if (!!execStatus(set_xkbmap_cmd          )) return "SET_XKBMAP_FMT ERROR_MSG" ;
  if (!!execStatus(dm_desktop_cmd          )) return "DM_DESKTOP_FMT ERROR_MSG" ;
  if (!!execStatus(dm_lang_cmd             )) return "DM_LANG_FMT ERROR_MSG" ;
  if (!!execStatus(welcome_title_cmd       )) return "WELCOME_TITLE_FMT ERROR_MSG" ;
  if (!!execStatus(welcome_text_cmd        )) return "WELCOME_TEXT_FMT ERROR_MSG" ;
  if (!!execStatus(welcome_sed_cmd         )) return "WELCOME_SED_FMT ERROR_MSG" ;
  if (default_desktop == LXDE_PACKAGES_KEY &&
      !!execStatus(octopi_cmd              )) return "OCTOPI_FMT ERROR_MSG" ;

  return QString("") ;
}


CALAMARES_PLUGIN_FACTORY_DEFINITION(PacstrapGuiJobFactory , registerPlugin<PacstrapGuiJob>() ;)
