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

#include "JobQueue.h"
#include <PacstrapCppJob.h>


/* PacstrapCppJob public class constants */

const QString PacstrapCppJob::DESKTOP_PACKAGES_KEY = "default-desktop" ;


/* PacstrapCppJob protected class constants */

const QString PacstrapCppJob::MOUNTPOINT                = "/tmp/pacstrap" ;
const char*   PacstrapCppJob::BASE_JOB_NAME             = "Pacstrap Base C++ Job" ;
const char*   PacstrapCppJob::GUI_JOB_NAME              = "Pacstrap Desktop C++ Job" ;
const char*   PacstrapCppJob::BASE_STATUS_MSG           = "Installing root filesystem" ;
const char*   PacstrapCppJob::GUI_STATUS_MSG            = "Installing graphical desktop environment" ;
const qreal   PacstrapCppJob::BASE_JOB_WEIGHT           = 23.0 ; // progress-bar job weight (1.0 normal)
const qreal   PacstrapCppJob::GUI_JOB_WEIGHT            = 69.0 ; // progress-bar job weight (1.0 normal)
const QString PacstrapCppJob::BASE_PACKAGES_KEY         = "base" ;
const QString PacstrapCppJob::BOOTLODER_PACKAGES_KEY    = "bootloader" ;
const QString PacstrapCppJob::KERNEL_PACKAGES_KEY       = "kernel" ;
const QString PacstrapCppJob::APPLICATIONS_PACKAGES_KEY = "applications" ;
const QString PacstrapCppJob::MULTIMEDIA_PACKAGES_KEY   = "multimedia" ;
const QString PacstrapCppJob::NETWORK_PACKAGES_KEY      = "network" ;
const QString PacstrapCppJob::THEMES_PACKAGES_KEY       = "themes" ;
const QString PacstrapCppJob::UTILITIES_PACKAGES_KEY    = "utilities" ;
const QString PacstrapCppJob::XSERVER_PACKAGES_KEY      = "x-server" ;
const QString PacstrapCppJob::MATE_PACKAGES_KEY         = "mate" ;
const QString PacstrapCppJob::LXDE_PACKAGES_KEY         = "lxde" ;
const QString PacstrapCppJob::PACSTRAP_FMT              = "pacstrap -c -C %1 %2 %3" ;
const QString PacstrapCppJob::PACSTRAP_ERROR_MSG        = "Failed to install packages in chroot." ;


/* PacstrapCppJob private class constants */

const QDir    PacstrapCppJob::PACKAGES_CACHE_DIR    = QDir(MOUNTPOINT + "/var/cache/pacman/pkg") ;
const QDir    PacstrapCppJob::PACKAGES_METADATA_DIR = QDir(MOUNTPOINT + "/var/lib/pacman/local") ;
const QString PacstrapCppJob::ONLINE_CONF_FILENAME  = "/etc/pacman-online.conf" ;
const QString PacstrapCppJob::OFFLINE_CONF_FILENAME = "/etc/pacman-offline.conf" ;
const QString PacstrapCppJob::IS_ONLINE_KEY         = "hasInternet" ;
const QString PacstrapCppJob::PARTITIONS_KEY        = "partitions" ;
const QString PacstrapCppJob::DEVICE_KEY            = "device" ;
const QString PacstrapCppJob::FS_KEY                = "fs" ;
const QString PacstrapCppJob::MOUNTPOINT_KEY        = "mountPoint" ;
const QString PacstrapCppJob::UUID_KEY              = "uuid" ;
const QString PacstrapCppJob::SYSTEM_EXEC_FMT       = "/bin/sh -c \"%1\"" ;
// const QString PacstrapCppJob::KEYRING_CMD           = "pacman -Sy --noconfirm parabola-keyring" ;
// const QString PacstrapCppJob::KEYRING_CMD           = "pacman -Sy --noconfirm parabola-keyring && \
//                                                        pacman-key --populate parabola          && \
//                                                        pacman-key --refresh-keys                  " ;
const QString PacstrapCppJob::MOUNT_FMT             = "mkdir %2 2> /dev/null || true && mount %1 %2" ;
const QString PacstrapCppJob::CHROOT_PREP_FMT       = "mkdir -m 0755 -p {%1,%2}" ;
const QString PacstrapCppJob::PACKAGES_SYNC_FMT     = "pacman --print --config %1 --root %2 -Sy" ;
const QString PacstrapCppJob::LIST_PACKAGES_FMT     = "pacman --print --config %1 --root %2 -S %3" ;
const QString PacstrapCppJob::UMOUNT_FMT            = "umount %1" ;
const QString PacstrapCppJob::CONFIG_ERROR_MSG      = "Invalid configuration map." ;
const QString PacstrapCppJob::TARGET_ERROR_MSG      = "Target device for root filesystem is unspecified." ;
const QString PacstrapCppJob::CONFFILE_ERROR_MSG    = "Pacman configuration not found: '%1'." ;
// const QString PacstrapCppJob::KEYRING_ERROR_MSG     = "Failed to update the pacman keyring." ;
const QString PacstrapCppJob::MOUNT_ERROR_MSG       = "Failed to mount the pacstrap chroot." ;
const QString PacstrapCppJob::CHROOT_PREP_ERROR_MSG = "Failed to prepare the pacstrap chroot." ;
const QString PacstrapCppJob::PACMAN_SYNC_ERROR_MSG = "Failed to syncronize packages in the pacstrap chroot." ;
const QString PacstrapCppJob::UMOUNT_ERROR_MSG      = "Failed to unmount the pacstrap chroot." ;


/* PacstrapCppJob public constructors/destructors */

PacstrapCppJob::PacstrapCppJob(QString job_name   , QString  status_msg ,
                               qreal   job_weight , QObject* parent     ) :
                               jobName  (job_name  ) , statusMsg        (status_msg) ,
                               jobWeight(job_weight) , Calamares::CppJob(parent    )
{
printf("PacstrapCppJob::PacstrapCppJob() '%s'\n" , jobName.toStdString().c_str()) ;

  this->globalStorage = Calamares::JobQueue::instance()->globalStorage() ;
  this->guiTimerId    = startTimer(1000) ;
//   this->guiTimerId    = -1 ;          // deferred to exec()
  this->targetDevice  = QString("") ; // deferred to exec()->runJob()
  this->confFile      = QString("") ; // deferred to exec()->runJob()
  this->packages      = QString("") ; // deferred to exec()->runJob()
  this->nPackages     = 0 ;           // deferred to exec()->runJob()
}

PacstrapCppJob::~PacstrapCppJob() { killTimer(this->guiTimerId) ; }
// PacstrapCppJob::~PacstrapCppJob() {}


/* PacstrapCppJob public getters/setters */

void PacstrapCppJob::setConfigurationMap(const QVariantMap& config) { this->localStorage = config ; }

qreal PacstrapCppJob::getJobWeight()                          const { return this->jobWeight ; }

QString PacstrapCppJob::prettyName()                          const { return this->jobName ; }

QString PacstrapCppJob::prettyStatusMessage()                 const { return this->statusMsg ; }


/* PacstrapCppJob public instance methods */

Calamares::JobResult PacstrapCppJob::exec()
{
// #include <QTimer>
// QTimer* guiTimer = new QTimer() ;
// connect(guiTimer , SIGNAL(timeout()) , this , SLOT(updateProgress())) ;
// guiTimer->start((std::chrono::milliseconds)1000) ;

//   this->guiTimerId            = startTimer(1000) ;
  Calamares::JobResult retval = runJob() ;

//   killTimer(this->guiTimerId) ;
//   guiTimer->stop() ;

  return retval ;
}


/* PacstrapCppJob protected class methods */

Calamares::JobResult PacstrapCppJob::JobErrorRetval(QString error_msg)
{
  return Calamares::JobResult::error(error_msg) ;
}

Calamares::JobResult PacstrapCppJob::JobSuccessRetval()
{
  return Calamares::JobResult::ok() ;
}

QString PacstrapCppJob::QListToString(const QVariantList& package_list)
{
  QStringList result ;
  for (const QVariant& package : package_list) result.append(package.toString()) ;

  return result.join(' ') ;
}

QString PacstrapCppJob::FindTargetDevice(const QVariantList& partitions)
{
  QString target_device = QString("") ;

  // locate target device for root filesystem
  foreach (const QVariant& partition , partitions)
  {
QStringList result; for ( auto it = partition.toMap().constBegin(); it != partition.toMap().constEnd(); ++it ) result.append( it.key() + '=' + it.value().toString() );
printf("[PACSTRAPCPP]: partition=[%s]\n" , result.join(',').toStdString().c_str()) ;

    QVariantMap partition_map = partition.toMap() ;
    QString     device        = partition_map.value(DEVICE_KEY    ).toString() ;
    QString     fs            = partition_map.value(FS_KEY        ).toString() ;
    QString     mountpoint    = partition_map.value(MOUNTPOINT_KEY).toString() ;
    QString     uuid          = partition_map.value(UUID_KEY      ).toString() ;

    if (mountpoint == "/") target_device = device ;

if (mountpoint == "/") printf("[PACSTRAPCPP]: target_device=%s\n" , device.toStdString().c_str()) ;
  }

  return target_device ;
}

unsigned int PacstrapCppJob::NPackagesInstalled()
{
  return (PACKAGES_CACHE_DIR   .entryList(QDir::Files | QDir::NoDotAndDotDot).count() +
          PACKAGES_METADATA_DIR.entryList(QDir::Dirs  | QDir::NoDotAndDotDot).count() ) / 2 ;
}

QStringList PacstrapCppJob::Exec(QString command_line)
{
  QProcess proc ;

  proc.start(QString(SYSTEM_EXEC_FMT).arg(command_line)) ; proc.waitForFinished(-1) ;

  int     status = proc.exitStatus() ;
  QString stdout = proc.readAllStandardOutput() ;
  QString stderr = proc.readAllStandardError() ;

// printf("[PACSTRAPCPP_DEBUG]: PacstrapCppJob::ExecWithOutput() status=%d\n" , status) ;
// printf("[PACSTRAPCPP_DEBUG]: PacstrapCppJob::ExecWithOutput() stdout=%s\n" , stdout) ;
// printf("[PACSTRAPCPP_DEBUG]: PacstrapCppJob::ExecWithOutput() stderr=%s\n" , stderr) ;

  return (QStringList() << QString(status) << stdout << stderr) ;
}

int PacstrapCppJob::ExecWithStatus(QString command_line)
{
printf("PacstrapCppJob::ExecWithStatus() command_line=%s\n" , command_line.toStdString().c_str()) ;

  return QProcess::execute(QString(SYSTEM_EXEC_FMT).arg(command_line)) ;
}

QString PacstrapCppJob::ExecWithOutput(QString command_line)
{
  return Exec(command_line).at(1) ;
}

QString PacstrapCppJob::ExecWithError(QString command_line)
{
  return Exec(command_line).at(2) ;
}


/* PacstrapCppJob protected instance methods */

void PacstrapCppJob::timerEvent(QTimerEvent* event)
{
  if (event->timerId() == this->guiTimerId) updateProgress() ;
}

void PacstrapCppJob::updateProgress()
{
  if (this->nPackages == 0) return ;

  qreal progress_percent = qreal(NPackagesInstalled()) / this->nPackages ;

printf("\n[PACSTRAPCPP]: n_packages=%d this->nPackages=%d progress_percent=%f\n" , NPackagesInstalled() , this->nPackages , (float)progress_percent) ;

  emit progress(progress_percent) ;
}

Calamares::JobResult PacstrapCppJob::runJob()
{
  QVariantList partitions   = this->globalStorage->value(PARTITIONS_KEY).toList() ;
  bool         has_internet = this->globalStorage->value(IS_ONLINE_KEY ).toBool() ;
  this->targetDevice        = FindTargetDevice(partitions) ;
  this->confFile            = (has_internet) ? ONLINE_CONF_FILENAME : OFFLINE_CONF_FILENAME ;
  this->packages            = getPackageList() ;

  if (this->localStorage.empty()     ) return JobErrorRetval(CONFIG_ERROR_MSG) ;
  if (this->targetDevice.isEmpty()   ) return JobErrorRetval(TARGET_ERROR_MSG) ;
  if (!QFile(this->confFile).exists()) return JobErrorRetval(CONFFILE_ERROR_MSG.arg(this->confFile)) ;

//   QString keyring_cmd       = KEYRING_CMD ;
  QString mount_cmd         = MOUNT_FMT        .arg(this->targetDevice , MOUNTPOINT) ;
  QString chroot_prep_cmd   = CHROOT_PREP_FMT  .arg(PACKAGES_CACHE_DIR   .absolutePath() ,
                                                    PACKAGES_METADATA_DIR.absolutePath() ) ;
  QString pacman_sync_cmd   = PACKAGES_SYNC_FMT.arg(this->confFile , MOUNTPOINT) ;
  QString list_packages_cmd = LIST_PACKAGES_FMT.arg(this->confFile , MOUNTPOINT , this->packages) ;
  QString umount_cmd        = UMOUNT_FMT       .arg(this->targetDevice) ;

//   if (!!ExecWithStatus(keyring_cmd    )) return JobErrorRetval(KEYRING_ERROR_MSG) ;
// "mkdir %2 2> /dev/null" "mount %1 %2"
  if (!!ExecWithStatus(mount_cmd      )) return JobErrorRetval(MOUNT_ERROR_MSG      ) ;
  if (!!ExecWithStatus(chroot_prep_cmd)) return JobErrorRetval(CHROOT_PREP_ERROR_MSG) ;
  if (!!ExecWithStatus(pacman_sync_cmd)) return JobErrorRetval(PACMAN_SYNC_ERROR_MSG) ;

  if (!this->packages.isEmpty())
  {
    QString new_packages = ExecWithOutput(list_packages_cmd) ;
    this->nPackages      = NPackagesInstalled() + new_packages.count(QChar::LineFeed) ;

    if (this->nPackages > 0)
    {
// return Calamares::JobResult::error("just cause") ;

      QString exec_error_msg = chrootExec() ;
      if (!exec_error_msg.isEmpty()) return JobErrorRetval(exec_error_msg) ;
    }
  }
  else emit progress(this->jobWeight) ;

  if (!!ExecWithStatus(umount_cmd)) return JobErrorRetval(UMOUNT_ERROR_MSG) ;

  return JobSuccessRetval() ;
}
