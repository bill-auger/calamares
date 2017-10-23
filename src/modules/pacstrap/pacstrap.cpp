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
#include <QDateTime>
#include <QThread>

#include "CalamaresVersion.h"
#include "JobQueue.h"
#include "GlobalStorage.h"

#include "pacstrap.h"
#include "utils/Logger.h"


PacstrapCppJob::PacstrapCppJob(QObject* parent) : Calamares::CppJob(parent) {
cDebug() << "PacstrapCppJob::PacstrapCppJob()" ;
}

PacstrapCppJob::~PacstrapCppJob() {
cDebug() << "PacstrapCppJob::~PacstrapCppJob()" ;
}


/* PacstrapCppJob public instance methods */

QString PacstrapCppJob::prettyName() const { return tr("Pacstrap C++ Job") ; }

QString PacstrapCppJob::prettyStatusMessage() const { return m_status ; }

Calamares::JobResult PacstrapCppJob::exec()
{
// Calamares::GlobalStorage *gs = Calamares::JobQueue::instance()->globalStorage();
// foreach (const QString &key, gs->keys())
//   cDebug() << QString("[PACSTRAPCPP]: globalStorage[%1]=%2\n").arg( key, gs->value( key ).toString() );

// QProcess::execute( "echo SKIPPING parabola-prepare.desc" );

  setTargetDevice() ; setNPackages() ;

  Calamares::GlobalStorage *globalStorage = Calamares::JobQueue::instance()->globalStorage() ;
  bool    has_internet  = globalStorage->value("hasInternet"  ).toBool() ;
  QString target_device = globalStorage->value("target-device").toString() ;
  QString conf_file     = (has_internet) ? "/etc/pacman-online.conf" : "/etc/pacman-offline.conf" ;
  QString mountpoint    = "/tmp/pacstrap";
  QString packages      = QListToString(m_configurationMap.value("base"      ).toList() +
                                        m_configurationMap.value("bootloader").toList() +
                                        m_configurationMap.value("kernel"    ).toList() ) ;

  if (target_device.isEmpty()) return Calamares::JobResult::error("Target device for root filesystem is unspecified.");

//     QString keyring_cmd = "/bin/sh -c \"pacman -Sy --noconfirm parabola-keyring && \
//                                         pacman-key --populate parabola          && \
//                                         pacman-key --refresh-keys                  \"";
//     QString keyring_cmd = "/bin/sh -c \"pacman -Sy --noconfirm parabola-keyring\"";
  QString mkdir_cmd      = QString("/bin/sh -c \"mkdir %1 2> /dev/null\"").arg(mountpoint) ;
  QString mount_cmd      = QString("/bin/sh -c \"mount %1 %2\"").arg(target_device , mountpoint) ;
  QString pacstrap_cmd   = QString("/bin/sh -c \"pacstrap -c -C %1 %2 %3\"").arg(conf_file , mountpoint , packages);
  QString grub_theme_cmd = QString("/bin/sh -c \"sed -i 's|[#]GRUB_THEME=.*|GRUB_THEME=/boot/grub/themes/GNUAxiom/theme.txt|' %1/etc/default/grub\"").arg(mountpoint) ;
QString grub_theme_kludge_cmd = QString("/bin/sh -c \"echo GRUB_THEME=/boot/grub/themes/GNUAxiom/theme.txt >> %1/etc/default/grub\"").arg(mountpoint) ;
  QString umount_cmd     = QString("/bin/sh -c \"umount %1\"").arg(target_device) ;

cDebug() << QString("[PACSTRAPCPP]: grub_theme_cmd=%1").arg(grub_theme_cmd);
// QProcess::execute( "/bin/sh -c \"ls /tmp/\"" );

    // boot-strap install root filesystem
  this->guiTimerId = startTimer(1000) ; updateProgress() ;

//   #include <QTimer>
//   QTimer *timer = new QTimer(this);
//   connect(timer, SIGNAL(timeout()), this, SLOT(update()));
//   timer->start(1000);

//     QProcess::execute(keyring_cmd) ;
  QProcess::execute(mkdir_cmd) ;
  QProcess::execute(mount_cmd) ;
  if (QProcess::execute(pacstrap_cmd)) return Calamares::JobResult::error("PACSTRAP_FAIL") ;
//     m_status = tr( "Installing linux-libre kernel" ); emit progress( 5 );

cDebug() << QString( "[PACSTRAPCPP]: grub_theme_cmd IN" );  QProcess::execute( QString( "/bin/sh -c \"cat %1/etc/default/grub\"" ).arg( mountpoint ) );
    QProcess::execute(grub_theme_cmd) ;
QProcess::execute(grub_theme_kludge_cmd) ;
cDebug() << QString( "[PACSTRAPCPP]: grub_theme_cmd OUT" ); QProcess::execute( QString( "/bin/sh -c \"cat %1/etc/default/grub\"" ).arg( mountpoint ) );

//     emit progress(5) ;
//     QProcess::execute( kernel_cmd );
    QProcess::execute(umount_cmd) ;

//     emit progress(1) ;
    killTimer() ;

    return Calamares::JobResult::ok() ;
}

void PacstrapCppJob::setConfigurationMap(const QVariantMap& configurationMap)
{
  m_configurationMap = configurationMap ;
}


/* PacstrapCppJob protected instance methods */

void PacstrapCppJob::timerEvent(QTimerEvent* event)
{
qDebug() << "Timer ID:" << event->timerId() ;

  if (event->timerId() == this->guiTimerId) updateProgress() ;
}

void PacstrapCppJob::updateProgress()
{
qDebug() << "PacstrapCppJob::updateProgress()" ;

  unsigned int progress_val = n_packages / this->nPackages ;

  m_status = tr("Installing root filesystem") ; emit progress(progress_val) ;
}


/* PacstrapCppJob private instance methods */

void PacstrapCppJob::setTargetDevice()
{
  Calamares::GlobalStorage *globalStorage = Calamares::JobQueue::instance()->globalStorage() ;
  QString      target_device = "" ;
  QVariantList partitions    = globalStorage->value("partitions").toList() ;

  // locate target device for root filesystem
  foreach (const QVariant& partition , partitions)
  {
QStringList result; for ( auto it = partition.toMap().constBegin(); it != partition.toMap().constEnd(); ++it ) result.append( it.key() + '=' + it.value().toString() );
cDebug() << QString("[PACSTRAPCPP]: partition=%1").arg('[' + result.join(',') + ']');

    QVariantMap partition_map = partition.toMap() ;
    QString     device        = partition_map.value("device").toString();
    QString     fs            = partition_map.value("fs").toString();
    QString     mountpoint    = partition_map.value("mountPoint").toString();
    QString     uuid          = partition_map.value("uuid").toString() ;

    if (mountpoint == "/") target_device = device ;

if (mountpoint == "/") cDebug() << QString("[PACSTRAPCPP]: target_device=%1").arg(device);
    }

    globalStorage->insert("target-device" , target_device) ;
}

void PacstrapCppJob::setNPackages()
{
  QString pacstrap_cmd   = QString("/bin/sh -c \"pacstrap -c -C %1 %2 %3\"").arg(conf_file , mountpoint , packages);
  QProcess pacstrap_proc ;
  pacstrap_proc.start(pacstrap_cmd) ; pacstrap_proc.waitForFinished(-1) ;

  QString stdout = pacstrap_proc.readAllStandardOutput() ;
  QString stderr = pacstrap_proc.readAllStandardError() ;
  this->nPackages = 0 ;
}

QString PacstrapCppJob::QListToString(const QVariantList& package_list)
{
  QStringList result ;
  for (const QVariant& package : package_list) result.append(package.toString()) ;

  return result.join(' ') ;
}


CALAMARES_PLUGIN_FACTORY_DEFINITION(PacstrapCppJobFactory , registerPlugin<PacstrapCppJob>() ;)
