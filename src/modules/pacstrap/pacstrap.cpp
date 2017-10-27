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

#include "pacstrap.h"


/* PacstrapBaseJob public instance methods */

PacstrapBaseJob::PacstrapBaseJob(QObject* parent) : PacStrapCppJob(tr(BASE_JOB_NAME)   ,
                                                                   tr(BASE_STATUS_MSG) ,
                                                                   BASE_JOB_WEIGHT     ,
                                                                   parent              ) {}


/* PacstrapBaseJob protected instance methods */

QString PacstrapBaseJob::chrootExec()
{
  QString packages = QListToString(this->config.value("base"      ).toList() +
                                   this->config.value("bootloader").toList() +
                                   this->config.value("kernel"    ).toList() ) ;
  if (packages.isEmpty()) { emit progress(qreal(this->jobWeight)) ; return QString("") ; }

  QString chroot_init_cmd = QString("/bin/sh -c \"mkdir -m 0755 -p {%1,%2}\"").arg(PACKAGES_CACHE_DIR.absolutePath() , PACKAGES_METADATA_DIR.absolutePath()) ;
  QString pacman_sync_cmd = QString("/bin/sh -c \"pacman --print --config %1 --root %2 -Sy\"").arg(this->confFile , MOUNTPOINT) ;
  QString n_packages_cmd  = QString("/bin/sh -c \"pacman --print --config %1 --root %2 -S %3\"").arg(this->confFile , MOUNTPOINT , packages) ;
  QString pacstrap_cmd    = QString("/bin/sh -c \"pacstrap -C %1 %2 %3\"").arg(this->confFile , MOUNTPOINT , packages) ;
  QString grub_theme_cmd  = QString("/bin/sh -c \"sed -i 's|[#]GRUB_THEME=.*|GRUB_THEME=/boot/grub/themes/GNUAxiom/theme.txt|' %1/etc/default/grub\"").arg(MOUNTPOINT) ;

QString grub_theme_kludge_cmd = QString("/bin/sh -c \"echo GRUB_THEME=/boot/grub/themes/GNUAxiom/theme.txt >> %1/etc/default/grub\"").arg(MOUNTPOINT) ;
printf("[PACSTRAPCPP]: grub_theme_cmd=%s\n" , grub_theme_cmd.toStdString().c_str()) ;
// QProcess::execute( "/bin/sh -c \"ls /tmp/\"" );

    // boot-strap install root filesystem
//   this->guiTimerId = startTimer(1000) ; updateProgress() ;

//   #include <QTimer>
//   QTimer *timer = new QTimer(this);
//   connect(timer, SIGNAL(timeout()), this, SLOT(update()));
//   timer->start(1000);

  QProcess::execute(chroot_init_cmd) ;
  QProcess::execute(pacman_sync_cmd) ;
  if (setNPackages(n_packages_cmd) == 0) return "No packages to install." ;
// return Calamares::JobResult::error("just cause") ;
  if (QProcess::execute(pacstrap_cmd)  ) return "PACSTRAP_FAIL" ;
//     m_status = tr( "Installing linux-libre kernel" ); emit progress( 5 );

printf("[PACSTRAPCPP]: grub_theme_cmd IN:\n");  QProcess::execute(QString("/bin/sh -c \"cat %1/etc/default/grub\"").arg(MOUNTPOINT));
    QProcess::execute(grub_theme_cmd) ;
QProcess::execute(grub_theme_kludge_cmd) ;
printf("[PACSTRAPCPP]: grub_theme_cmd OUT:\n"); QProcess::execute(QString("/bin/sh -c \"cat %1/etc/default/grub\"").arg(MOUNTPOINT));
//     emit progress(5) ;
//     QProcess::execute( kernel_cmd );

  return QString("") ;
}


CALAMARES_PLUGIN_FACTORY_DEFINITION(PacstrapBaseJobFactory , registerPlugin<PacstrapBaseJob>() ;)
